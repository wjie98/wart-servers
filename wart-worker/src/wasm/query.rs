use crate::bindgen::*;

use crate::GLOBALS;

use tokio::sync::oneshot;

use anyhow::Result;

use super::utils::{dump_to_imports_rows, dump_to_imports_table};

pub enum QueryRequest {
    ChoiceNodes {
        request: ChoiceNodesRequest,
        sink: oneshot::Sender<(Vec<String>, imports::Table)>,
    },
    FetchNode {
        request: FetchNodeRequest,
        sink: oneshot::Sender<(Vec<String>, imports::Row)>,
    },
    FetchNeighbors {
        request: FetchNeighborsRequest,
        sink: oneshot::Sender<(Vec<String>, imports::Table)>,
    },
    QueryKV {
        key: String,
        fields: Vec<String>,
        sink: oneshot::Sender<(Vec<String>, imports::Row)>,
    },
}

impl QueryRequest {
    pub async fn into_query(self) -> Result<()> {
        let mut backend = GLOBALS.storage.get().await?;
        match self {
            Self::ChoiceNodes { request, mut sink } => {
                let resp = tokio::select! {
                    x = backend.choice_nodes(request) => x?.into_inner(),
                    _ = sink.closed() => {
                        return Ok(());
                    }
                };

                if let Some(data) = resp.data {
                    let (headers, table) = dump_to_imports_table(data);
                    let _ = sink.send((headers, table));
                }
            }

            Self::FetchNode { request, mut sink } => {
                let resp = tokio::select! {
                    x = backend.fetch_node(request) => x?.into_inner(),
                    _ = sink.closed() => {
                        return Ok(());
                    }
                };

                if let Some(data) = resp.data {
                    let (headers, rows) = dump_to_imports_rows(data);
                    if let Some(row) = rows.into_iter().nth(0) {
                        let _ = sink.send((headers, row));
                    }
                }
            }

            Self::FetchNeighbors { request, mut sink } => {
                let resp = tokio::select! {
                    x = backend.fetch_neighbors(request) => x?.into_inner(),
                    _ = sink.closed() => {
                        return Ok(());
                    }
                };

                if let Some(data) = resp.data {
                    let (headers, table) = dump_to_imports_table(data);
                    let _ = sink.send((headers, table));
                }
            }

            Self::QueryKV {
                ref key,
                fields,
                mut sink,
            } => {
                use redis::AsyncCommands;
                let mut conn = GLOBALS.redis.get().await?;

                let mut row = vec![];
                for field in fields.iter() {
                    tokio::select! {
                        x = conn.hget(key, field) => {
                            match x {
                                Ok(s) => row.push(imports::Value::Txt(s)),
                                Err(_) => row.push(imports::Value::Nil),
                            }
                        },
                        _ = sink.closed() => {
                            return Ok(());
                        }
                    };
                }
                let headers = vec!["val".into()];
                let _ = sink.send((headers, row));
            }
        }
        Ok(())
    }
}
