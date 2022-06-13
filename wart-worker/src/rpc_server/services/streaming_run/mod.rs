use crate::bindgen::*;
use crate::wasm::StorageManager;

use anyhow::{anyhow, Result};
use futures::StreamExt;
use log;

use tokio::sync::mpsc;
use tokio::time;
use tokio_stream::wrappers::ReceiverStream;
use tonic::{Request, Response, Status, Streaming};

use tracing_futures::WithSubscriber;
use tracing_subscriber::fmt::Subscriber;
use tracing_subscriber::prelude::*;

use crate::log_tracer::WasmTracer;

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

        let sig_cls = mpsc_tx.clone();
        storage_manager.spawn_clk(async move {
            sig_cls.closed().await;
        });

        GLOBALS
            .runtime
            .spawn(streaming_run_args(istream, mpsc_tx, storage_manager));
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
    let par_n = if storage_manager.par > 0 {
        storage_manager.par
    } else {
        1
    };
    let (par_tx, mut par_rx) = mpsc::channel(par_n);
    let bypass_tx = mpsc_tx.clone();
    tokio::spawn(async move {
        while let Some(request) = istream.next().await {
            let request = match request {
                Ok(request) => request,
                Err(err) => {
                    let _ = bypass_tx.send(Err(err)).await;
                    break;
                }
            };

            tokio::select! {
                p = par_tx.reserve() => match p {
                    Ok(permit) => {
                        let task = tokio::spawn(streaming_run_launch(
                            request,
                            bypass_tx.clone(),
                            storage_manager.clone(),
                        ));
                        permit.send(task);
                    },
                    Err(err) => {
                        log::error!("{}", err);
                        break;
                    }
                },
                _ = bypass_tx.closed() => {
                    break;
                },
            }
        }
    });

    while let Some(task) = par_rx.recv().await {
        match task.await {
            Ok(result) => match result {
                Ok(resp) => {
                    if let Err(_) = mpsc_tx.send(Ok(resp)).await {
                        break;
                    }
                }
                Err(err) => {
                    log::error!("{}", err);
                    let resp = StreamingRunResponse {
                        last_err: err.to_string(),
                        ..Default::default()
                    };
                    if let Err(_) = mpsc_tx.send(Ok(resp)).await {
                        break;
                    }
                }
            },
            Err(err) => {
                log::error!("{}", err);
                break;
            }
        }
    }
}

async fn streaming_run_launch(
    request: StreamingRunRequest,
    bypass_tx: mpsc::Sender<Result<StreamingRunResponse, Status>>,
    storage_manager: StorageManager,
) -> Result<StreamingRunResponse> {
    use streaming_run_request::Data::{Args, Config};

    let request = request.data.ok_or(anyhow!("empty args"))?;
    match request {
        Config(_) => Err(anyhow!("invalid args"))?,
        Args(args) => {
            let args = &args.args;
            let mut sandbox = storage_manager.get_sandbox(args).await?;
            sandbox.store.epoch_deadline_async_yield_and_update(1);

            let tracer = WasmTracer::default();
            let logs = tracer.get_logs();
            let subscriber = Subscriber::builder()
                .with_max_level(Subscriber::DEFAULT_MAX_LEVEL)
                .finish()
                .with(tracer);

            {
                let ttl = time::Duration::from_millis(storage_manager.ttl);
                let task = time::timeout(ttl, sandbox.call_async()).with_subscriber(subscriber);
                tokio::pin!(task);

                tokio::select! {
                    result = &mut task => {
                        result??;
                    },
                    _ = bypass_tx.closed() => {
                        Err(anyhow!("reset by peer"))?;
                    }
                };
                task.dispatch();
            }

            let storage = sandbox.store.into_data().imports;
            let time_used = (chrono::Local::now() - storage.start_time).num_milliseconds();

            let tables = storage.into_tables().await;
            let logs = if let Ok(it) = logs.lock() {
                it.iter()
                    .filter_map(|x| serde_json::to_string(x).ok())
                    .collect::<Vec<_>>()
            } else {
                vec![]
            };

            Ok(StreamingRunResponse {
                tables,
                logs,
                time_used,
                ..Default::default()
            })
        }
    }
}
