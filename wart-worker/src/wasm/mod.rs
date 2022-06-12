use crate::bindgen::*;

mod sandbox;
pub use sandbox::{Sandbox, SandboxManager};

use wasmtime::Engine;
use wasmtime_wasi::WasiCtxBuilder;

pub mod atomic_kv;
pub mod utils;

use crate::GLOBALS;

use anyhow::Result;
use log;

use std::collections::HashMap;
use std::sync::{Arc, Mutex};

#[derive(Debug, Clone)]
pub struct Storage {
    pub space_name: String,
    pub epoch: u64,
    pub token: String,
    pub return_tables: Vec<(String, Arc<Mutex<HashMap<String, imports::VectorResult>>>)>,
}

#[derive(Clone)]
pub struct StorageManager {
    pub space_name: String,
    pub epoch: u64,
    pub token: String,
    pub ttl: u64,
    pub par: usize,
    vmm: SandboxManager<Storage>,
}

impl StorageManager {
    pub async fn new(token: &str) -> Result<Self> {
        let mut con = GLOBALS.redis.get().await?;

        let key = format!("wart:session:{}", token);

        let (space_name, epoch, ttl, module, par): (String, u64, u64, Vec<u8>, usize) =
            redis::pipe()
                .atomic()
                .hget(&key, "space_name")
                .hget(&key, "epoch")
                .hget(&key, "ex_timeout")
                .hget(&key, "module")
                .hget(&key, "parallel")
                .query_async(&mut *con)
                .await?;

        let config = SandboxManager::<Storage>::default_config();
        let vmm = SandboxManager::<Storage>::from_module(&module, &config)?;

        Ok(Self {
            space_name,
            epoch,
            token: token.into(),
            ttl,
            par,
            vmm,
        })
    }

    pub async fn get_sandbox(&self, args: &[String]) -> Result<Sandbox<Storage>> {
        let wasi_ctx = WasiCtxBuilder::new().args(&args)?.build();
        // let wasi_ctx = WasiCtxBuilder::new()
        //     .inherit_stdout()
        //     .inherit_stderr()
        //     .args(&args)?
        //     .build();

        let imports = Storage {
            space_name: self.space_name.clone(),
            epoch: self.epoch,
            token: self.token.clone(),
            return_tables: vec![],
        };

        self.vmm.instantiate(wasi_ctx, imports).await
    }

    pub fn get_engine(&self) -> Engine {
        self.vmm.engine.clone()
    }
}

impl Storage {
    pub async fn into_tables(self) -> Vec<DataFrame> {
        let mut dfs = vec![];
        for (name, table) in self.return_tables.into_iter() {
            match table.lock() {
                Ok(mut table) => {
                    let mut headers = vec![];
                    let mut columns = vec![];
                    for (k, v) in table.drain() {
                        let values = match v {
                            imports::VectorResult::Nil => continue,
                            imports::VectorResult::Bol(x) => {
                                series::Values::BoolValues(series::BoolSeries { data: x })
                            }
                            imports::VectorResult::I32(x) => {
                                series::Values::Int32Values(series::Int32Series { data: x })
                            }
                            imports::VectorResult::I64(x) => {
                                series::Values::Int64Values(series::Int64Series { data: x })
                            }
                            imports::VectorResult::F32(x) => {
                                series::Values::Float32Values(series::Float32Series { data: x })
                            }
                            imports::VectorResult::F64(x) => {
                                series::Values::Float64Values(series::Float64Series { data: x })
                            }
                            imports::VectorResult::Txt(x) => {
                                series::Values::StringValues(series::StringSeries { data: x })
                            }
                        };

                        headers.push(k);
                        columns.push(Series {
                            values: Some(values),
                        });
                    }

                    dfs.push(DataFrame {
                        headers,
                        columns,
                        comment: name,
                    });
                }
                Err(err) => {
                    log::error!("return_tables: {}", err);
                    return vec![];
                }
            };
            tokio::task::yield_now().await;
        }
        dfs
    }
}

#[derive(Debug)]
pub struct ReturnTable {
    data: Arc<Mutex<HashMap<String, imports::VectorResult>>>,
    defa: HashMap<String, imports::ValueResult>,
}

#[derive(Debug)]
pub struct StorageCache {}

#[wit_bindgen_wasmtime::async_trait]
impl imports::Imports for Storage {
    type DataFrame = ReturnTable;
    type Storage = StorageCache;

    fn data_frame_new(
        &mut self,
        name: &str,
        default: imports::RowParam<'_>,
    ) -> Option<Self::DataFrame> {
        let defa = default
            .into_iter()
            .map(|r| {
                let imports::ItemParam { key, val } = r;
                let key = key.to_string();
                match val {
                    imports::ValueParam::Nil => (key, imports::ValueResult::Nil),
                    imports::ValueParam::Bol(x) => (key, imports::ValueResult::Bol(x)),
                    imports::ValueParam::I32(x) => (key, imports::ValueResult::I32(x)),
                    imports::ValueParam::I64(x) => (key, imports::ValueResult::I64(x)),
                    imports::ValueParam::F32(x) => (key, imports::ValueResult::F32(x)),
                    imports::ValueParam::F64(x) => (key, imports::ValueResult::F64(x)),
                    imports::ValueParam::Txt(x) => (key, imports::ValueResult::Txt(x.into())),
                }
            })
            .collect::<HashMap<_, _>>();

        let data = defa
            .iter()
            .map(|(k, v)| {
                let val = match v {
                    imports::ValueResult::Nil => imports::VectorResult::Nil,
                    imports::ValueResult::Bol(_) => imports::VectorResult::Bol(vec![]),
                    imports::ValueResult::I32(_) => imports::VectorResult::I32(vec![]),
                    imports::ValueResult::I64(_) => imports::VectorResult::I64(vec![]),
                    imports::ValueResult::F32(_) => imports::VectorResult::F32(vec![]),
                    imports::ValueResult::F64(_) => imports::VectorResult::F64(vec![]),
                    imports::ValueResult::Txt(_) => imports::VectorResult::Txt(vec![]),
                };
                let key = k.clone();
                (key, val)
            })
            .collect::<HashMap<_, _>>();

        let data = Arc::new(Mutex::new(data));
        self.return_tables.push((name.into(), data.clone()));

        Some(Self::DataFrame { data, defa })
    }

    fn data_frame_push(
        &mut self,
        this: &Self::DataFrame,
        data: imports::RowParam<'_>,
    ) -> Option<u64> {
        let mut table = this
            .data
            .lock()
            .map_err(|err| {
                log::error!("lock error: {}", err);
                err
            })
            .ok()?;

        let data = data
            .iter()
            .map(|r| {
                let imports::ItemParam { key, val } = r;
                (&key[..], val)
            })
            .collect::<HashMap<_, _>>();

        for (k, v) in table.iter_mut() {
            if let Some(vp) = data.get(&k[..]) {
                match (v, vp) {
                    (imports::VectorResult::Bol(v), imports::ValueParam::Bol(vp)) => v.push(*vp),
                    (imports::VectorResult::I32(v), imports::ValueParam::I32(vp)) => v.push(*vp),
                    (imports::VectorResult::I64(v), imports::ValueParam::I64(vp)) => v.push(*vp),
                    (imports::VectorResult::F32(v), imports::ValueParam::F32(vp)) => v.push(*vp),
                    (imports::VectorResult::F64(v), imports::ValueParam::F64(vp)) => v.push(*vp),
                    (imports::VectorResult::Txt(v), imports::ValueParam::Txt(vp)) => {
                        v.push((*vp).into())
                    }
                    _ => (),
                };
            } else {
                let vp = this.defa.get(&k[..]).unwrap();
                match (v, vp) {
                    (imports::VectorResult::Bol(v), imports::ValueResult::Bol(vp)) => v.push(*vp),
                    (imports::VectorResult::I32(v), imports::ValueResult::I32(vp)) => v.push(*vp),
                    (imports::VectorResult::I64(v), imports::ValueResult::I64(vp)) => v.push(*vp),
                    (imports::VectorResult::F32(v), imports::ValueResult::F32(vp)) => v.push(*vp),
                    (imports::VectorResult::F64(v), imports::ValueResult::F64(vp)) => v.push(*vp),
                    (imports::VectorResult::Txt(v), imports::ValueResult::Txt(vp)) => {
                        v.push(vp.clone())
                    }
                    _ => (),
                };
            }
        }

        Some(0)
    }

    fn data_frame_size(&mut self, this: &Self::DataFrame) -> Option<u64> {
        this.data
            .lock()
            .map_err(|err| {
                log::error!("lock error: {}", err);
                err
            })
            .ok()
            .map(|s| s.len() as u64)
    }

    async fn storage_new(&mut self) -> Option<Self::Storage> {
        Some(Self::Storage {})
    }

    async fn storage_choice_nodes(
        &mut self,
        _this: &Self::Storage,
        tag: &str,
        number: i32,
    ) -> Option<imports::VectorResult> {
        let request = ChoiceNodesRequest {
            space_name: self.space_name.clone(),
            tag: tag.into(),
            number,
        };

        let data = GLOBALS
            .storage
            .get()
            .await
            .map_err(|err| {
                log::error!("connection pool: {}", err);
                err
            })
            .ok()?
            .choice_nodes(request)
            .await
            .map_err(|err| {
                log::error!("can't connect to storage nodes: {}", err);
                err
            })
            .ok()?
            .into_inner()
            .data
            .or_else(|| {
                log::error!("empty data in response");
                None
            })?;

        utils::dump_to_imports_table(data)
            .into_iter()
            .nth(0)
            .map(|s| s.val)
            .or_else(|| {
                log::error!("empty table in response");
                None
            })
    }

    async fn storage_query_nodes(
        &mut self,
        _this: &Self::Storage,
        id: imports::ValueParam<'_>,
        tag: &str,
        keys: Vec<&str>,
    ) -> Option<imports::RowResult> {
        let request = FetchNodeRequest {
            space_name: self.space_name.clone(),
            node_id: Some(match id {
                imports::ValueParam::I64(x) => fetch_node_request::NodeId::AsInt(x),
                imports::ValueParam::Txt(x) => fetch_node_request::NodeId::AsStr(x.into()),
                _ => {
                    log::error!("unrecognized type of node_id: {:?}", id);
                    return None;
                }
            }),
            tag: tag.into(),
            keys: keys.into_iter().map(|x| x.into()).collect(),
        };

        let data = GLOBALS
            .storage
            .get()
            .await
            .map_err(|err| {
                log::error!("connection pool error: {}", err);
                err
            })
            .ok()?
            .fetch_node(request)
            .await
            .map_err(|err| {
                log::error!("can't connect to storage nodes: {}", err);
                err
            })
            .ok()?
            .into_inner()
            .data
            .or_else(|| {
                log::error!("empty data in response");
                None
            })?;

        Some(utils::dump_to_imports_row(data))
    }

    async fn storage_query_neighbors(
        &mut self,
        _this: &Self::Storage,
        id: imports::ValueParam<'_>,
        tag: &str,
        keys: Vec<&str>,
        reversely: bool,
    ) -> Option<(imports::VectorResult, imports::Table)> {
        let request = FetchNeighborsRequest {
            space_name: self.space_name.clone(),
            node_id: Some(match id {
                imports::ValueParam::I64(x) => fetch_neighbors_request::NodeId::AsInt(x),
                imports::ValueParam::Txt(x) => fetch_neighbors_request::NodeId::AsStr(x.into()),
                _ => {
                    log::error!("unrecognized type of node_id: {:?}", id);
                    return None;
                }
            }),
            tag: tag.into(),
            keys: keys.into_iter().map(|x| x.into()).collect(),
            reversely,
        };

        let data = GLOBALS
            .storage
            .get()
            .await
            .map_err(|err| {
                log::error!("connection pool error: {}", err);
                err
            })
            .ok()?
            .fetch_neighbors(request)
            .await
            .map_err(|err| {
                log::error!("can't connect to storage nodes: {}", err);
                err
            })
            .ok()?
            .into_inner()
            .data
            .or_else(|| {
                log::error!("empty data in response");
                None
            })?;

        let data = utils::dump_to_imports_table(data);

        let mut it = data.into_iter();
        let dst = it.next().map(|s| s.val).or_else(|| {
            log::error!("empty table in response");
            None
        })?;

        let attr = it.collect::<Vec<_>>();

        Some((dst, attr))
    }

    async fn storage_query_kv(
        &mut self,
        _this: &Self::Storage,
        keys: Vec<&str>,
        defva: imports::ValueParam<'_>,
    ) -> Option<imports::VectorResult> {
        let mut con = GLOBALS
            .redis
            .get()
            .await
            .map_err(|err| {
                log::error!("redis connection pool error: {}", err);
            })
            .ok()?;

        let prefk = format!("wart:store:{}:", self.token);
        match defva {
            imports::ValueParam::Bol(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let val = atomic_kv::atomic_query_kv::<bool, _>(
                        &prefk, self.epoch, &field, &x, &mut *con,
                    )
                    .await?;
                    data.push(val);
                }
                Some(imports::VectorResult::Bol(data))
            }
            imports::ValueParam::I32(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let val = atomic_kv::atomic_query_kv::<i32, _>(
                        &prefk, self.epoch, &field, &x, &mut *con,
                    )
                    .await?;
                    data.push(val);
                }
                Some(imports::VectorResult::I32(data))
            }
            imports::ValueParam::I64(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let val = atomic_kv::atomic_query_kv::<i64, _>(
                        &prefk, self.epoch, &field, &x, &mut *con,
                    )
                    .await?;
                    data.push(val);
                }
                Some(imports::VectorResult::I64(data))
            }
            imports::ValueParam::F32(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let val = atomic_kv::atomic_query_kv::<f32, _>(
                        &prefk, self.epoch, &field, &x, &mut *con,
                    )
                    .await?;
                    data.push(val);
                }
                Some(imports::VectorResult::F32(data))
            }
            imports::ValueParam::F64(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let val = atomic_kv::atomic_query_kv::<f64, _>(
                        &prefk, self.epoch, &field, &x, &mut *con,
                    )
                    .await?;
                    data.push(val);
                }
                Some(imports::VectorResult::F64(data))
            }
            imports::ValueParam::Txt(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let val = atomic_kv::atomic_query_kv::<String, _>(
                        &prefk,
                        self.epoch,
                        &field,
                        &x.into(),
                        &mut *con,
                    )
                    .await?;
                    data.push(val);
                }
                Some(imports::VectorResult::Txt(data))
            }
            _ => None,
        }
    }

    async fn storage_update_kv(
        &mut self,
        _this: &Self::Storage,
        keys: Vec<&str>,
        vals: imports::VectorParam<'_>,
        ops: imports::MergeType,
    ) -> Option<u64> {
        let mut con = GLOBALS
            .redis
            .get()
            .await
            .map_err(|err| {
                log::error!("redis connection pool error: {}", err);
            })
            .ok()?;

        let prefk = format!("wart:store:{}:", self.token);

        match vals {
            imports::VectorParam::Bol(x) => {
                for (field, argum) in keys.into_iter().zip(x.into_iter()) {
                    atomic_kv::atomic_update_kv::<bool, _>(
                        &prefk, self.epoch, &field, &false, &argum, ops, &mut *con,
                    )
                    .await?;
                }
                Some(0)
            }
            imports::VectorParam::I32(x) => {
                for (field, argum) in keys.into_iter().zip(x.into_iter()) {
                    atomic_kv::atomic_update_kv::<i32, _>(
                        &prefk,
                        self.epoch,
                        &field,
                        &0,
                        &argum.get(),
                        ops,
                        &mut *con,
                    )
                    .await?;
                }
                Some(0)
            }
            imports::VectorParam::I64(x) => {
                for (field, argum) in keys.into_iter().zip(x.into_iter()) {
                    atomic_kv::atomic_update_kv::<i64, _>(
                        &prefk,
                        self.epoch,
                        &field,
                        &0,
                        &argum.get(),
                        ops,
                        &mut *con,
                    )
                    .await?;
                }
                Some(0)
            }
            imports::VectorParam::F32(x) => {
                for (field, argum) in keys.into_iter().zip(x.into_iter()) {
                    atomic_kv::atomic_update_kv::<f32, _>(
                        &prefk,
                        self.epoch,
                        &field,
                        &0.0,
                        &argum.get(),
                        ops,
                        &mut *con,
                    )
                    .await?;
                }
                Some(0)
            }
            imports::VectorParam::F64(x) => {
                for (field, argum) in keys.into_iter().zip(x.into_iter()) {
                    atomic_kv::atomic_update_kv::<f64, _>(
                        &prefk,
                        self.epoch,
                        &field,
                        &0.0,
                        &argum.get(),
                        ops,
                        &mut *con,
                    )
                    .await?;
                }
                Some(0)
            }
            imports::VectorParam::Txt(x) => {
                for (field, argum) in keys.into_iter().zip(x.into_iter()) {
                    let argum = argum.get(..).or_else(|| {
                        log::error!("invalid string");
                        None
                    })?;
                    atomic_kv::atomic_update_kv::<&str, _>(
                        &prefk, self.epoch, &field, &"0", &argum, ops, &mut *con,
                    )
                    .await?;
                }
                Some(0)
            }
            _ => None,
        }
    }

    async fn log(&mut self, lv: imports::LogLevel, msg: &str) {
        match lv {
            imports::LogLevel::Trace => log::log!(log::Level::Trace, "{}", msg),
            imports::LogLevel::Debug => log::log!(log::Level::Debug, "{}", msg),
            imports::LogLevel::Info => log::log!(log::Level::Info, "{}", msg),
            imports::LogLevel::Warn => log::log!(log::Level::Warn, "{}", msg),
            imports::LogLevel::Error => log::log!(log::Level::Error, "{}", msg),
        }
    }

    async fn log_enabled(&mut self, lv: imports::LogLevel) -> bool {
        match lv {
            imports::LogLevel::Trace => log::log_enabled!(log::Level::Trace),
            imports::LogLevel::Debug => log::log_enabled!(log::Level::Debug),
            imports::LogLevel::Info => log::log_enabled!(log::Level::Info),
            imports::LogLevel::Warn => log::log_enabled!(log::Level::Warn),
            imports::LogLevel::Error => log::log_enabled!(log::Level::Error),
        }
    }
}

// #[wit_bindgen_wasmtime::async_trait]
// impl imports::Imports for Storage {
//     type FutureRow = FutureRow;
//     type FutureTable = FutureTable;

//     fn select_nodes(&mut self, ids: imports::SeriesId<'_>) -> i64 {
//         if self.selected_nodes.columns.is_empty() {
//             let n = match ids {
//                 imports::SeriesId::I64(ids) => {
//                     let data: Vec<i64> = ids.iter().map(|s| s.get()).collect();
//                     self.selected_nodes.columns.push(Series {
//                         values: Some(series::Values::Int64Values(series::Int64Series { data })),
//                     });
//                     ids.len()
//                 }
//                 imports::SeriesId::Txt(ids) => {
//                     let data: Vec<String> = ids.iter().map(|s| s.to_string()).collect();
//                     self.selected_nodes.columns.push(Series {
//                         values: Some(series::Values::StringValues(series::StringSeries { data })),
//                     });
//                     ids.len()
//                 }
//             };
//             n as i64
//         } else {
//             let n = match self.selected_nodes.columns[0].values.as_mut().unwrap() {
//                 series::Values::Int64Values(values) => match ids {
//                     imports::SeriesId::I64(ids) => {
//                         ids.iter().for_each(|s| {
//                             values.data.push(s.get());
//                         });
//                         ids.len()
//                     }
//                     imports::SeriesId::Txt(_) => 0,
//                 },
//                 series::Values::StringValues(values) => match ids {
//                     imports::SeriesId::I64(_) => 0,
//                     imports::SeriesId::Txt(ids) => {
//                         ids.iter().for_each(|s| {
//                             values.data.push(s.to_string());
//                         });
//                         ids.len()
//                     }
//                 },
//                 _ => 0,
//             };
//             n as i64
//         }
//     }

//     fn select_edges(&mut self, src: imports::SeriesId<'_>, dst: imports::SeriesId<'_>) -> i64 {
//         if !match &src {
//             imports::SeriesId::I64(s) => match &dst {
//                 imports::SeriesId::I64(d) => s.len() == d.len(),
//                 imports::SeriesId::Txt(_) => false,
//             },
//             imports::SeriesId::Txt(s) => match &dst {
//                 imports::SeriesId::I64(_) => false,
//                 imports::SeriesId::Txt(d) => s.len() == d.len(),
//             },
//         } {
//             return 0;
//         }

//         if self.selected_edges.columns.is_empty() {
//             let n = match src {
//                 imports::SeriesId::I64(ids) => {
//                     let data: Vec<i64> = ids.iter().map(|s| s.get()).collect();
//                     self.selected_edges.columns.push(Series {
//                         values: Some(series::Values::Int64Values(series::Int64Series { data })),
//                     });
//                     ids.len()
//                 }
//                 imports::SeriesId::Txt(ids) => {
//                     let data: Vec<String> = ids.iter().map(|s| s.to_string()).collect();
//                     self.selected_edges.columns.push(Series {
//                         values: Some(series::Values::StringValues(series::StringSeries { data })),
//                     });
//                     ids.len()
//                 }
//             };

//             let _ = match dst {
//                 imports::SeriesId::I64(ids) => {
//                     let data: Vec<i64> = ids.iter().map(|s| s.get()).collect();
//                     self.selected_edges.columns.push(Series {
//                         values: Some(series::Values::Int64Values(series::Int64Series { data })),
//                     });
//                     ids.len() as i64
//                 }
//                 imports::SeriesId::Txt(ids) => {
//                     let data: Vec<String> = ids.iter().map(|s| s.to_string()).collect();
//                     self.selected_edges.columns.push(Series {
//                         values: Some(series::Values::StringValues(series::StringSeries { data })),
//                     });
//                     ids.len() as i64
//                 }
//             };
//             n as i64
//         } else {
//             let n = match self.selected_edges.columns[0].values.as_mut().unwrap() {
//                 series::Values::Int64Values(values) => match src {
//                     imports::SeriesId::I64(ids) => {
//                         ids.iter().for_each(|s| {
//                             values.data.push(s.get());
//                         });
//                         ids.len()
//                     }
//                     imports::SeriesId::Txt(_) => 0,
//                 },
//                 series::Values::StringValues(values) => match src {
//                     imports::SeriesId::I64(_) => 0,
//                     imports::SeriesId::Txt(ids) => {
//                         ids.iter().for_each(|s| {
//                             values.data.push(s.to_string());
//                         });
//                         ids.len()
//                     }
//                 },
//                 _ => 0,
//             };

//             let _ = match self.selected_edges.columns[1].values.as_mut().unwrap() {
//                 series::Values::Int64Values(values) => match dst {
//                     imports::SeriesId::I64(ids) => {
//                         ids.iter().for_each(|s| {
//                             values.data.push(s.get());
//                         });
//                         ids.len()
//                     }
//                     imports::SeriesId::Txt(_) => 0,
//                 },
//                 series::Values::StringValues(values) => match dst {
//                     imports::SeriesId::I64(_) => 0,
//                     imports::SeriesId::Txt(ids) => {
//                         ids.iter().for_each(|s| {
//                             values.data.push(s.to_string());
//                         });
//                         ids.len()
//                     }
//                 },
//                 _ => 0,
//             };
//             n as i64
//         }
//     }

//     async fn async_choice_nodes(
//         &mut self,
//         tag: &str,
//         number: i32,
//     ) -> Result<Self::FutureTable, imports::FutureError> {
//         let (tx, rx) = oneshot::channel();
//         let query = QueryRequest::ChoiceNodes {
//             request: ChoiceNodesRequest {
//                 space_name: self.space_name.clone(),
//                 tag: tag.into(),
//                 number,
//             },
//             sink: tx,
//         };

//         self.io_tx
//             .send(query)
//             .await
//             .map_err(|_| imports::FutureError::ChannelClosed)?;

//         Ok(Self::FutureTable {
//             rx: Mutex::new(Some(rx)),
//         })
//     }

//     async fn async_query_node(
//         &mut self,
//         id: imports::NodeId<'_>,
//         tag: &str,
//         keys: Vec<&str>,
//     ) -> Result<Self::FutureRow, imports::FutureError> {
//         let (tx, rx) = oneshot::channel();
//         let query = QueryRequest::FetchNode {
//             request: FetchNodeRequest {
//                 space_name: self.space_name.clone(),
//                 node_id: Some(match id {
//                     imports::NodeId::I64(x) => fetch_node_request::NodeId::AsInt(x),
//                     imports::NodeId::Txt(x) => fetch_node_request::NodeId::AsStr(x.into()),
//                 }),
//                 tag: tag.into(),
//                 keys: keys.into_iter().map(|x| x.into()).collect(),
//             },
//             sink: tx,
//         };

//         self.io_tx
//             .send(query)
//             .await
//             .map_err(|_| imports::FutureError::ChannelClosed)?;

//         Ok(Self::FutureRow {
//             rx: Mutex::new(Some(rx)),
//         })
//     }

//     async fn async_query_neighbors(
//         &mut self,
//         id: imports::NodeId<'_>,
//         tag: &str,
//         keys: Vec<&str>,
//         reversely: bool,
//     ) -> Result<Self::FutureTable, imports::FutureError> {
//         let (tx, rx) = oneshot::channel();
//         let query = QueryRequest::FetchNeighbors {
//             request: FetchNeighborsRequest {
//                 space_name: self.space_name.clone(),
//                 node_id: Some(match id {
//                     imports::NodeId::I64(x) => fetch_neighbors_request::NodeId::AsInt(x),
//                     imports::NodeId::Txt(x) => fetch_neighbors_request::NodeId::AsStr(x.into()),
//                 }),
//                 tag: tag.into(),
//                 keys: keys.into_iter().map(|x| x.into()).collect(),
//                 reversely,
//             },
//             sink: tx,
//         };

//         self.io_tx
//             .send(query)
//             .await
//             .map_err(|_| imports::FutureError::ChannelClosed)?;

//         Ok(Self::FutureTable {
//             rx: Mutex::new(Some(rx)),
//         })
//     }

//     async fn async_query_kv(
//         &mut self,
//         keys: Vec<&str>,
//     ) -> Result<Self::FutureRow, imports::FutureError> {
//         let (tx, rx) = oneshot::channel();
//         let query = QueryRequest::QueryKV {
//             key: format!("wart:store:{}", self.token),
//             fields: keys.into_iter().map(|x| x.into()).collect(),
//             sink: tx,
//         };

//         self.io_tx
//             .send(query)
//             .await
//             .map_err(|_| imports::FutureError::ChannelClosed)?;

//         Ok(Self::FutureRow {
//             rx: Mutex::new(Some(rx)),
//         })
//     }

//     async fn get_future_row(
//         &mut self,
//         self_: &Self::FutureRow,
//     ) -> Result<imports::Row, imports::FutureError> {
//         let rx = match self_.rx.lock().await.take() {
//             Some(rx) => rx,
//             None => Err(imports::FutureError::Empty)?,
//         };

//         let (_headers, row) = rx.await.map_err(|_| imports::FutureError::ChannelDropped)?;
//         Ok(row)
//     }

//     async fn get_future_table(
//         &mut self,
//         self_: &Self::FutureTable,
//     ) -> Result<imports::Table, imports::FutureError> {
//         let rx = match self_.rx.lock().await.take() {
//             Some(rx) => rx,
//             None => Err(imports::FutureError::Empty)?,
//         };

//         let (_headers, table) = rx.await.map_err(|_| imports::FutureError::ChannelDropped)?;
//         Ok(table)
//     }
// }
