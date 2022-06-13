use crate::bindgen::*;

mod sandbox;
use futures::Future;
pub use sandbox::{Sandbox, SandboxManager};

use wasmtime::Engine;
use wasmtime_wasi::WasiCtxBuilder;

// pub mod atomic_kv;
pub mod utils;

use crate::GLOBALS;

use anyhow::Result;
use log;

use std::collections::{BTreeMap, HashMap};
use tokio::time;

#[derive(Debug, Clone)]
pub struct Storage {
    pub space_name: String,
    pub epoch: u64,
    pub token: String,
    pub return_tables: Vec<(String, BTreeMap<String, imports::VectorResult>)>,
    pub start_time: chrono::DateTime<chrono::Local>,
    // pub statstic: BTreeMap<i64, u64>,
    pub counter: u64,
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

        let imports = Storage {
            space_name: self.space_name.clone(),
            epoch: self.epoch,
            token: self.token.clone(),
            return_tables: vec![],
            start_time: chrono::Local::now(),
            // statstic: Default::default(),
            counter: 0,
        };

        self.vmm.instantiate(wasi_ctx, imports).await
    }

    pub fn get_engine(&self) -> Engine {
        self.vmm.engine.clone()
    }

    pub fn spawn_clk<F>(&self, signal: F)
    where
        F: Future + Send + 'static,
        F::Output: Send + 'static,
    {
        let engine = self.get_engine();
        let mut signal = Box::pin(signal);
        tokio::spawn(async move {
            let duration = time::Duration::from_millis(100);
            let mut interval = time::interval(duration);
            loop {
                tokio::select! {
                    _ = interval.tick() => {
                        engine.increment_epoch();
                    },
                    _ = &mut signal => {
                        break
                    },
                }
            }
            log::info!("clk aborted");
        });
    }
}

impl Storage {
    pub async fn into_tables(self) -> Vec<DataFrame> {
        let mut dfs = vec![];
        for (name, table) in self.return_tables.into_iter() {
            let mut headers = vec![];
            let mut columns = vec![];
            for (k, v) in table.into_iter() {
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

            tokio::task::yield_now().await;
        }

        // {
        //     let ts = self.statstic.keys().map(|v| *v).collect();
        //     let ct = self.statstic.values().map(|v| *v as i64).collect();
        //     let headers = vec!["timestamp".into(), "count".into()];
        //     let columns = vec![
        //         Series {
        //             values: Some(series::Values::Int64Values(series::Int64Series {
        //                 data: ts,
        //             })),
        //         },
        //         Series {
        //             values: Some(series::Values::Int64Values(series::Int64Series {
        //                 data: ct,
        //             })),
        //         },
        //     ];

        //     dfs.push(DataFrame {
        //         headers,
        //         columns,
        //         comment: "__statstic__".into(),
        //     });
        // }
        dfs
    }
}

#[derive(Debug)]
pub struct ReturnTable {
    index: usize,
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
            .collect::<BTreeMap<_, _>>();

        let index = self.return_tables.len();
        self.return_tables.push((name.into(), data));

        Some(Self::DataFrame { index, defa })
    }

    fn data_frame_push(
        &mut self,
        this: &Self::DataFrame,
        data: imports::RowParam<'_>,
    ) -> Option<u64> {
        let table = match self.return_tables.get_mut(this.index) {
            Some(t) => &mut t.1,
            None => {
                log::error!("unknown return table: {}", this.index);
                return None;
            }
        };

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
        let table = match self.return_tables.get(this.index) {
            Some(t) => &t.1,
            None => {
                log::error!("unknown return table: {}", this.index);
                return None;
            }
        };
        Some(table.len() as u64)
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

        // let now = chrono::Local::now();
        // let v = self.statstic.entry(now.timestamp()).or_insert(0);
        // *v += 1;

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
        
        self.counter += 1;

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

        // let now = chrono::Local::now();
        // let v = self.statstic.entry(now.timestamp()).or_insert(0);
        // *v += 1;

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
        
        self.counter += 1;

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

        // let now = chrono::Local::now();
        // let v = self.statstic.entry(now.timestamp()).or_insert(0);
        // *v += 1;

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

        self.counter += 1;

        Some((dst, attr))
    }

    // async fn storage_query_kv(
    //     &mut self,
    //     _this: &Self::Storage,
    //     keys: Vec<&str>,
    //     defva: imports::ValueParam<'_>,
    // ) -> Option<imports::VectorResult> {
    //     let mut con = GLOBALS
    //         .redis
    //         .get()
    //         .await
    //         .map_err(|err| {
    //             log::error!("redis connection pool error: {}", err);
    //         })
    //         .ok()?;

    //     let prefk = format!("wart:store:{}:", self.token);
    //     match defva {
    //         imports::ValueParam::Bol(x) => {
    //             let mut data = vec![];
    //             for field in keys.into_iter() {
    //                 let val = atomic_kv::atomic_query_kv::<bool, _>(
    //                     &prefk, self.epoch, &field, &x, &mut *con,
    //                 )
    //                 .await?;
    //                 data.push(val);
    //             }
    //             Some(imports::VectorResult::Bol(data))
    //         }
    //         imports::ValueParam::I32(x) => {
    //             let mut data = vec![];
    //             for field in keys.into_iter() {
    //                 let val = atomic_kv::atomic_query_kv::<i32, _>(
    //                     &prefk, self.epoch, &field, &x, &mut *con,
    //                 )
    //                 .await?;
    //                 data.push(val);
    //             }
    //             Some(imports::VectorResult::I32(data))
    //         }
    //         imports::ValueParam::I64(x) => {
    //             let mut data = vec![];
    //             for field in keys.into_iter() {
    //                 let val = atomic_kv::atomic_query_kv::<i64, _>(
    //                     &prefk, self.epoch, &field, &x, &mut *con,
    //                 )
    //                 .await?;
    //                 data.push(val);
    //             }
    //             Some(imports::VectorResult::I64(data))
    //         }
    //         imports::ValueParam::F32(x) => {
    //             let mut data = vec![];
    //             for field in keys.into_iter() {
    //                 let val = atomic_kv::atomic_query_kv::<f32, _>(
    //                     &prefk, self.epoch, &field, &x, &mut *con,
    //                 )
    //                 .await?;
    //                 data.push(val);
    //             }
    //             Some(imports::VectorResult::F32(data))
    //         }
    //         imports::ValueParam::F64(x) => {
    //             let mut data = vec![];
    //             for field in keys.into_iter() {
    //                 let val = atomic_kv::atomic_query_kv::<f64, _>(
    //                     &prefk, self.epoch, &field, &x, &mut *con,
    //                 )
    //                 .await?;
    //                 data.push(val);
    //             }
    //             Some(imports::VectorResult::F64(data))
    //         }
    //         imports::ValueParam::Txt(x) => {
    //             let mut data = vec![];
    //             for field in keys.into_iter() {
    //                 let val = atomic_kv::atomic_query_kv::<String, _>(
    //                     &prefk,
    //                     self.epoch,
    //                     &field,
    //                     &x.into(),
    //                     &mut *con,
    //                 )
    //                 .await?;
    //                 data.push(val);
    //             }
    //             Some(imports::VectorResult::Txt(data))
    //         }
    //         _ => None,
    //     }
    // }

    // async fn storage_update_kv(
    //     &mut self,
    //     _this: &Self::Storage,
    //     keys: Vec<&str>,
    //     vals: imports::VectorParam<'_>,
    //     ops: imports::MergeType,
    // ) -> Option<u64> {
    //     let mut con = GLOBALS
    //         .redis
    //         .get()
    //         .await
    //         .map_err(|err| {
    //             log::error!("redis connection pool error: {}", err);
    //         })
    //         .ok()?;

    //     let prefk = format!("wart:store:{}:", self.token);

    //     match vals {
    //         imports::VectorParam::Bol(x) => {
    //             for (field, argum) in keys.into_iter().zip(x.into_iter()) {
    //                 atomic_kv::atomic_update_kv::<bool, _>(
    //                     &prefk, self.epoch, &field, &false, &argum, ops, &mut *con,
    //                 )
    //                 .await?;
    //             }
    //             Some(0)
    //         }
    //         imports::VectorParam::I32(x) => {
    //             for (field, argum) in keys.into_iter().zip(x.into_iter()) {
    //                 atomic_kv::atomic_update_kv::<i32, _>(
    //                     &prefk,
    //                     self.epoch,
    //                     &field,
    //                     &0,
    //                     &argum.get(),
    //                     ops,
    //                     &mut *con,
    //                 )
    //                 .await?;
    //             }
    //             Some(0)
    //         }
    //         imports::VectorParam::I64(x) => {
    //             for (field, argum) in keys.into_iter().zip(x.into_iter()) {
    //                 atomic_kv::atomic_update_kv::<i64, _>(
    //                     &prefk,
    //                     self.epoch,
    //                     &field,
    //                     &0,
    //                     &argum.get(),
    //                     ops,
    //                     &mut *con,
    //                 )
    //                 .await?;
    //             }
    //             Some(0)
    //         }
    //         imports::VectorParam::F32(x) => {
    //             for (field, argum) in keys.into_iter().zip(x.into_iter()) {
    //                 atomic_kv::atomic_update_kv::<f32, _>(
    //                     &prefk,
    //                     self.epoch,
    //                     &field,
    //                     &0.0,
    //                     &argum.get(),
    //                     ops,
    //                     &mut *con,
    //                 )
    //                 .await?;
    //             }
    //             Some(0)
    //         }
    //         imports::VectorParam::F64(x) => {
    //             for (field, argum) in keys.into_iter().zip(x.into_iter()) {
    //                 atomic_kv::atomic_update_kv::<f64, _>(
    //                     &prefk,
    //                     self.epoch,
    //                     &field,
    //                     &0.0,
    //                     &argum.get(),
    //                     ops,
    //                     &mut *con,
    //                 )
    //                 .await?;
    //             }
    //             Some(0)
    //         }
    //         imports::VectorParam::Txt(x) => {
    //             for (field, argum) in keys.into_iter().zip(x.into_iter()) {
    //                 let argum = argum.get(..).or_else(|| {
    //                     log::error!("invalid string");
    //                     None
    //                 })?;
    //                 atomic_kv::atomic_update_kv::<&str, _>(
    //                     &prefk, self.epoch, &field, &"0", &argum, ops, &mut *con,
    //                 )
    //                 .await?;
    //             }
    //             Some(0)
    //         }
    //         _ => None,
    //     }
    // }

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
                log::error!("redis connection pool: {}", err);
            })
            .ok()?;

        let key = format!("wart:store:{}", self.token);
        match defva {
            imports::ValueParam::Bol(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let (val,) = redis::pipe()
                        .hget(&key, &field)
                        .query_async(&mut *con)
                        .await
                        .map_err(|err| {
                            log::error!("{}", err);
                            err
                        })
                        .unwrap_or((x.into(),));
                    data.push(val);
                }
                Some(imports::VectorResult::Bol(data))
            }
            imports::ValueParam::I32(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let (val,) = redis::pipe()
                        .hget(&key, &field)
                        .query_async(&mut *con)
                        .await
                        .map_err(|err| {
                            log::error!("{}", err);
                            err
                        })
                        .unwrap_or((x.into(),));
                    data.push(val);
                }
                Some(imports::VectorResult::I32(data))
            }
            imports::ValueParam::I64(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let (val,) = redis::pipe()
                        .hget(&key, &field)
                        .query_async(&mut *con)
                        .await
                        .map_err(|err| {
                            log::error!("{}", err);
                            err
                        })
                        .unwrap_or((x.into(),));
                    data.push(val);
                }
                Some(imports::VectorResult::I64(data))
            }
            imports::ValueParam::F32(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let (val,) = redis::pipe()
                        .hget(&key, &field)
                        .query_async(&mut *con)
                        .await
                        .map_err(|err| {
                            log::error!("{}", err);
                            err
                        })
                        .unwrap_or((x.into(),));
                    data.push(val);
                }
                Some(imports::VectorResult::F32(data))
            }
            imports::ValueParam::F64(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let (val,) = redis::pipe()
                        .hget(&key, &field)
                        .query_async(&mut *con)
                        .await
                        .map_err(|err| {
                            log::error!("{}", err);
                            err
                        })
                        .unwrap_or((x.into(),));
                    data.push(val);
                }
                Some(imports::VectorResult::F64(data))
            }
            imports::ValueParam::Txt(x) => {
                let mut data = vec![];
                for field in keys.into_iter() {
                    let (val,) = redis::pipe()
                        .hget(&key, &field)
                        .query_async(&mut *con)
                        .await
                        .map_err(|err| {
                            log::error!("{}", err);
                            err
                        })
                        .unwrap_or((x.into(),));
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
        _keys: Vec<&str>,
        _vals: imports::VectorParam<'_>,
        _ops: imports::MergeType,
    ) -> Option<u64> {
        todo!()
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
