use crate::bindgen::wart_storage_client::WartStorageClient;
use crate::bindgen::*;

use crate::GLOBALS;

use tokio::sync::oneshot;
use tokio::time;

use anyhow::Result;
use tonic::transport::Channel;

use super::utils::{dump_to_imports_rows, dump_to_imports_table};

pub enum QueryRequest {
    ChoiceNodes {
        request: ChoiceNodesRequest,
        sink: oneshot::Sender<(Vec<String>, imports::Table)>,
        ttl: u64,
    },
    FetchNode {
        request: FetchNodeRequest,
        sink: oneshot::Sender<(Vec<String>, imports::Row)>,
        ttl: u64,
    },
    FetchNeighbors {
        request: FetchNeighborsRequest,
        sink: oneshot::Sender<(Vec<String>, imports::Table)>,
        ttl: u64,
    },
    QueryKV {
        key: String,
        fields: Vec<String>,
        sink: oneshot::Sender<(Vec<String>, imports::Row)>,
        ttl: u64,
    },
}

impl QueryRequest {
    pub async fn into_query(self, mut storage: WartStorageClient<Channel>) -> Result<()> {
        match self {
            Self::ChoiceNodes {
                request,
                mut sink,
                ttl,
            } => {
                let duration = time::Duration::from_millis(ttl);
                let resp = tokio::select! {
                    x = time::timeout(duration, storage.choice_nodes(request)) => x??.into_inner(),
                    _ = sink.closed() => {
                        return Ok(());
                    }
                };

                if let Some(data) = resp.data {
                    let (headers, table) = dump_to_imports_table(data);
                    let _ = sink.send((headers, table));
                }
            }

            Self::FetchNode {
                request,
                mut sink,
                ttl,
            } => {
                let duration = time::Duration::from_millis(ttl);
                let resp = tokio::select! {
                    x = time::timeout(duration, storage.fetch_node(request)) => x??.into_inner(),
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

            Self::FetchNeighbors {
                request,
                mut sink,
                ttl,
            } => {
                let duration = time::Duration::from_millis(ttl);
                let resp = tokio::select! {
                    x = time::timeout(duration, storage.fetch_neighbors(request)) => x??.into_inner(),
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
                ttl,
            } => {
                use mobc_redis::redis::AsyncCommands;
                let mut conn = GLOBALS.redis_pool.get().await?;

                let mut row = vec![];
                for field in fields.iter() {
                    let duration = time::Duration::from_millis(ttl);
                    tokio::select! {
                        x = time::timeout(duration, conn.hget(key, field)) => {
                            match x? {
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
