use crate::bindgen::*;
use crate::wasm::StorageManager;

use anyhow::{anyhow, Result};
use futures::StreamExt;
use log;

use tokio::sync::mpsc;
use tokio::time;
use tokio_stream::wrappers::ReceiverStream;
use tonic::{Request, Response, Status, Streaming};

use crate::GLOBALS;

type StreamingRunStream = ReceiverStream<Result<StreamingRunResponse, Status>>;

pub async fn streaming_run(
    request: Request<Streaming<StreamingRunRequest>>,
) -> Result<Response<StreamingRunStream>, Status> {
    match streaming_run_impl(request.into_inner()).await {
        Ok(msg) => Ok(Response::new(msg)),
        Err(err) => Err(Status::aborted(err.to_string())),
    }
}

async fn streaming_run_impl(
    mut istream: Streaming<StreamingRunRequest>,
) -> Result<StreamingRunStream> {
    let (mpsc_tx, mpsc_rx) = mpsc::channel(8);

    if let Some(config) = istream.next().await {
        let config = config?;
        let storage_manager = streaming_run_config(config).await?;
        tokio::spawn(streaming_run_args(istream, mpsc_tx, storage_manager));
    }

    Ok(ReceiverStream::new(mpsc_rx))
}

async fn streaming_run_config(request: StreamingRunRequest) -> Result<StorageManager> {
    use streaming_run_request::{Config, Data};
    match request.data.ok_or(anyhow!("empty config"))? {
        Data::Config(config) => {
            let Config { token } = config;
            let storage_manager = StorageManager::new(&token).await?;
            Ok(storage_manager)
        }
        Data::Args(_) => Err(anyhow!("invalid config"))?,
    }
}

async fn streaming_run_args(
    mut istream: Streaming<StreamingRunRequest>,
    mpsc_tx: mpsc::Sender<Result<StreamingRunResponse, Status>>,
    storage_manager: StorageManager,
) {
    let engine = storage_manager.get_engine();
    let clk_handle = GLOBALS.epoch_interrupter.spawn(async move {
        let duration = time::Duration::from_millis(100);
        let mut interval = time::interval(duration);
        loop {
            interval.tick().await;
            engine.increment_epoch();
        }
    });

    while let Some(request) = istream.next().await {
        match request {
            Ok(request) => {
                let ret = time::timeout(
                    time::Duration::from_millis(storage_manager.ttl),
                    streaming_run_sandbox(request, storage_manager.clone()),
                )
                .await;

                match ret {
                    Ok(result) => match result {
                        Ok(tables) => {
                            let resp = StreamingRunResponse {
                                tables,
                                logs: vec![],
                            };
                            if let Err(_) = mpsc_tx.send(Ok(resp)).await {
                                break;
                            }
                        }
                        Err(err) => {
                            log::error!("streaming_run_sandbox: {}", err);
                        }
                    },
                    Err(err) => {
                        let _ = mpsc_tx.send(Err(Status::cancelled(err.to_string()))).await;
                        break;
                    }
                }
            }
            Err(err) => {
                let _ = mpsc_tx.send(Err(err)).await;
                break;
            }
        }
    }

    clk_handle.abort();
}

async fn streaming_run_sandbox(
    request: StreamingRunRequest,
    storage_manager: StorageManager,
) -> Result<Vec<DataFrame>> {
    match request.data.ok_or(anyhow!("empty args"))? {
        streaming_run_request::Data::Args(args) => {
            let args = args.args;
            let mut sandbox = storage_manager.get_sandbox(&args).await?;

            sandbox.store.epoch_deadline_async_yield_and_update(1);
            let tables = match sandbox.call_async().await {
                Ok(_) => {
                    let tables = sandbox.store.into_data().imports.into_tables().await;
                    tables
                }
                Err(err) => {
                    log::error!("streaming_run_sandbox: {}", err);
                    vec![]
                }
            };
            Ok(tables)
        }
        _ => Err(anyhow!("invalid args"))?,
    }
}
