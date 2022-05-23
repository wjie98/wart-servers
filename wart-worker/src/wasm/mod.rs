use crate::bindgen::*;

mod sandbox;
pub use sandbox::{Context, Sandbox, SandboxManager};

pub mod query;
use query::QueryRequest;

pub mod utils;

use anyhow::Result;
use tokio::sync::{mpsc, oneshot, Mutex};

#[derive(Clone)]
pub struct Storage {
    pub space_name: String,
    pub token: String,
    io_tx: mpsc::Sender<QueryRequest>,
    pub selected_nodes: DataFrame,
    pub selected_edges: DataFrame,
}

#[derive(Clone)]
pub struct StorageManager {
    pub space_name: String,
    pub token: String,
    pub io_timeout: u64,
    pub ex_timeout: u64,
}

impl StorageManager {
    pub fn new(space_name: String, token: String, io_timeout: u64, ex_timeout: u64) -> Self {
        Self {
            space_name,
            token,
            io_timeout,
            ex_timeout,
        }
    }

    pub fn instantiate(&self, io_tx: mpsc::Sender<QueryRequest>) -> Storage {
        Storage {
            space_name: self.space_name.clone(),
            token: self.token.clone(),
            io_tx,
            selected_nodes: Default::default(),
            selected_edges: Default::default(),
        }
    }
}

#[derive(Debug)]
pub struct FutureRow {
    rx: Mutex<Option<oneshot::Receiver<(Vec<String>, imports::Row)>>>,
}

#[derive(Debug)]
pub struct FutureTable {
    rx: Mutex<Option<oneshot::Receiver<(Vec<String>, imports::Table)>>>,
}

#[wit_bindgen_wasmtime::async_trait]
impl imports::Imports for Storage {
    type FutureRow = FutureRow;
    type FutureTable = FutureTable;

    fn select_nodes(&mut self, ids: imports::SeriesId<'_>) -> i64 {
        if self.selected_nodes.columns.is_empty() {
            let n = match ids {
                imports::SeriesId::I64(ids) => {
                    let data: Vec<i64> = ids.iter().map(|s| s.get()).collect();
                    self.selected_nodes.columns.push(Series {
                        values: Some(series::Values::Int64Values(series::Int64Series { data })),
                    });
                    ids.len()
                }
                imports::SeriesId::Txt(ids) => {
                    let data: Vec<String> = ids.iter().map(|s| s.to_string()).collect();
                    self.selected_nodes.columns.push(Series {
                        values: Some(series::Values::StringValues(series::StringSeries { data })),
                    });
                    ids.len()
                }
            };
            n as i64
        } else {
            let n = match self.selected_nodes.columns[0].values.as_mut().unwrap() {
                series::Values::Int64Values(values) => match ids {
                    imports::SeriesId::I64(ids) => {
                        ids.iter().for_each(|s| {
                            values.data.push(s.get());
                        });
                        ids.len()
                    }
                    imports::SeriesId::Txt(_) => 0,
                },
                series::Values::StringValues(values) => match ids {
                    imports::SeriesId::I64(_) => 0,
                    imports::SeriesId::Txt(ids) => {
                        ids.iter().for_each(|s| {
                            values.data.push(s.to_string());
                        });
                        ids.len()
                    }
                },
                _ => 0,
            };
            n as i64
        }
    }

    fn select_edges(&mut self, src: imports::SeriesId<'_>, dst: imports::SeriesId<'_>) -> i64 {
        if !match &src {
            imports::SeriesId::I64(s) => match &dst {
                imports::SeriesId::I64(d) => s.len() == d.len(),
                imports::SeriesId::Txt(_) => false,
            },
            imports::SeriesId::Txt(s) => match &dst {
                imports::SeriesId::I64(_) => false,
                imports::SeriesId::Txt(d) => s.len() == d.len(),
            },
        } {
            return 0;
        }

        if self.selected_edges.columns.is_empty() {
            let n = match src {
                imports::SeriesId::I64(ids) => {
                    let data: Vec<i64> = ids.iter().map(|s| s.get()).collect();
                    self.selected_edges.columns.push(Series {
                        values: Some(series::Values::Int64Values(series::Int64Series { data })),
                    });
                    ids.len()
                }
                imports::SeriesId::Txt(ids) => {
                    let data: Vec<String> = ids.iter().map(|s| s.to_string()).collect();
                    self.selected_edges.columns.push(Series {
                        values: Some(series::Values::StringValues(series::StringSeries { data })),
                    });
                    ids.len()
                }
            };

            let _ = match dst {
                imports::SeriesId::I64(ids) => {
                    let data: Vec<i64> = ids.iter().map(|s| s.get()).collect();
                    self.selected_edges.columns.push(Series {
                        values: Some(series::Values::Int64Values(series::Int64Series { data })),
                    });
                    ids.len() as i64
                }
                imports::SeriesId::Txt(ids) => {
                    let data: Vec<String> = ids.iter().map(|s| s.to_string()).collect();
                    self.selected_edges.columns.push(Series {
                        values: Some(series::Values::StringValues(series::StringSeries { data })),
                    });
                    ids.len() as i64
                }
            };
            n as i64
        } else {
            let n = match self.selected_edges.columns[0].values.as_mut().unwrap() {
                series::Values::Int64Values(values) => match src {
                    imports::SeriesId::I64(ids) => {
                        ids.iter().for_each(|s| {
                            values.data.push(s.get());
                        });
                        ids.len()
                    }
                    imports::SeriesId::Txt(_) => 0,
                },
                series::Values::StringValues(values) => match src {
                    imports::SeriesId::I64(_) => 0,
                    imports::SeriesId::Txt(ids) => {
                        ids.iter().for_each(|s| {
                            values.data.push(s.to_string());
                        });
                        ids.len()
                    }
                },
                _ => 0,
            };

            let _ = match self.selected_edges.columns[1].values.as_mut().unwrap() {
                series::Values::Int64Values(values) => match dst {
                    imports::SeriesId::I64(ids) => {
                        ids.iter().for_each(|s| {
                            values.data.push(s.get());
                        });
                        ids.len()
                    }
                    imports::SeriesId::Txt(_) => 0,
                },
                series::Values::StringValues(values) => match dst {
                    imports::SeriesId::I64(_) => 0,
                    imports::SeriesId::Txt(ids) => {
                        ids.iter().for_each(|s| {
                            values.data.push(s.to_string());
                        });
                        ids.len()
                    }
                },
                _ => 0,
            };
            n as i64
        }
    }

    async fn async_choice_nodes(
        &mut self,
        tag: &str,
        number: i32,
    ) -> Result<Self::FutureTable, imports::FutureError> {
        let (tx, rx) = oneshot::channel();
        let query = QueryRequest::ChoiceNodes {
            request: ChoiceNodesRequest {
                space_name: self.space_name.clone(),
                tag: tag.into(),
                number,
            },
            sink: tx,
        };

        self.io_tx
            .send(query)
            .await
            .map_err(|_| imports::FutureError::ChannelClosed)?;

        Ok(Self::FutureTable {
            rx: Mutex::new(Some(rx)),
        })
    }

    async fn async_query_node(
        &mut self,
        id: imports::NodeId<'_>,
        tag: &str,
        keys: Vec<&str>,
    ) -> Result<Self::FutureRow, imports::FutureError> {
        let (tx, rx) = oneshot::channel();
        let query = QueryRequest::FetchNode {
            request: FetchNodeRequest {
                space_name: self.space_name.clone(),
                node_id: Some(match id {
                    imports::NodeId::I64(x) => fetch_node_request::NodeId::AsInt(x),
                    imports::NodeId::Txt(x) => fetch_node_request::NodeId::AsStr(x.into()),
                }),
                tag: tag.into(),
                keys: keys.into_iter().map(|x| x.into()).collect(),
            },
            sink: tx,
        };

        self.io_tx
            .send(query)
            .await
            .map_err(|_| imports::FutureError::ChannelClosed)?;

        Ok(Self::FutureRow {
            rx: Mutex::new(Some(rx)),
        })
    }

    async fn async_query_neighbors(
        &mut self,
        id: imports::NodeId<'_>,
        tag: &str,
        keys: Vec<&str>,
        reversely: bool,
    ) -> Result<Self::FutureTable, imports::FutureError> {
        let (tx, rx) = oneshot::channel();
        let query = QueryRequest::FetchNeighbors {
            request: FetchNeighborsRequest {
                space_name: self.space_name.clone(),
                node_id: Some(match id {
                    imports::NodeId::I64(x) => fetch_neighbors_request::NodeId::AsInt(x),
                    imports::NodeId::Txt(x) => fetch_neighbors_request::NodeId::AsStr(x.into()),
                }),
                tag: tag.into(),
                keys: keys.into_iter().map(|x| x.into()).collect(),
                reversely,
            },
            sink: tx,
        };

        self.io_tx
            .send(query)
            .await
            .map_err(|_| imports::FutureError::ChannelClosed)?;

        Ok(Self::FutureTable {
            rx: Mutex::new(Some(rx)),
        })
    }

    async fn async_query_kv(
        &mut self,
        keys: Vec<&str>,
    ) -> Result<Self::FutureRow, imports::FutureError> {
        let (tx, rx) = oneshot::channel();
        let query = QueryRequest::QueryKV {
            key: format!("wart:store:{}", self.token),
            fields: keys.into_iter().map(|x| x.into()).collect(),
            sink: tx,
        };

        self.io_tx
            .send(query)
            .await
            .map_err(|_| imports::FutureError::ChannelClosed)?;

        Ok(Self::FutureRow {
            rx: Mutex::new(Some(rx)),
        })
    }

    async fn get_future_row(
        &mut self,
        self_: &Self::FutureRow,
    ) -> Result<imports::Row, imports::FutureError> {
        let rx = match self_.rx.lock().await.take() {
            Some(rx) => rx,
            None => Err(imports::FutureError::Empty)?,
        };

        let (_headers, row) = rx.await.map_err(|_| imports::FutureError::ChannelDropped)?;
        Ok(row)
    }

    async fn get_future_table(
        &mut self,
        self_: &Self::FutureTable,
    ) -> Result<imports::Table, imports::FutureError> {
        let rx = match self_.rx.lock().await.take() {
            Some(rx) => rx,
            None => Err(imports::FutureError::Empty)?,
        };

        let (_headers, table) = rx.await.map_err(|_| imports::FutureError::ChannelDropped)?;
        Ok(table)
    }
}
