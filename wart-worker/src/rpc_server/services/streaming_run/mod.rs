use crate::bindgen::*;
use crate::wasm::query::QueryRequest;
use crate::wasm::{SandboxManager, Storage, StorageManager};

use crate::GLOBALS;
use anyhow::{anyhow, Result};
use futures::StreamExt;

use tokio::sync::mpsc;
use tokio::time;
use tokio_stream::wrappers::ReceiverStream;
use tonic::{Request, Response, Status, Streaming};

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
    let (sandbox_manager, storage_manager) =
        streaming_run_config(istream.next().await.ok_or(anyhow!("empty config"))??).await?;

    let (mpsc_tx, mpsc_rx) = mpsc::channel(8);
    tokio::spawn(streaming_run_args(
        istream,
        mpsc_tx,
        sandbox_manager,
        storage_manager,
    ));

    Ok(ReceiverStream::new(mpsc_rx))
}

async fn streaming_run_config(
    request: StreamingRunRequest,
) -> Result<(SandboxManager<Storage>, StorageManager)> {
    use streaming_run_request::{Config, Data};
    match request.data.ok_or(anyhow!("empty config"))? {
        Data::Config(config) => {
            let Config { token } = config;
            let store_key = format!("wart:session:{}", token);
            let mut con = GLOBALS.redis.get().await?;
            let (space_name, module, timeout): (String, Vec<u8>, u64) = redis::pipe()
                .atomic()
                .hget(&store_key, "space_name")
                .hget(&store_key, "module")
                .hget(&store_key, "ex_timeout")
                .query_async(&mut *con)
                .await?;

            let config = SandboxManager::<Storage>::default_config();
            let sandbox_manager = SandboxManager::<Storage>::from_module(&module, &config)?;
            let storage_manager = StorageManager::new(space_name, token, timeout);
            Ok((sandbox_manager, storage_manager))
        }
        Data::Args(_) => Err(anyhow!("invalid config"))?,
    }
}

async fn streaming_run_args(
    mut istream: Streaming<StreamingRunRequest>,
    mpsc_tx: mpsc::Sender<Result<StreamingRunResponse, Status>>,
    sandbox_manager: SandboxManager<Storage>,
    storage_manager: StorageManager,
) {
    let engine = sandbox_manager.engine.clone();
    let clk_handle = tokio::spawn(async move {
        let duration = time::Duration::from_millis(100);
        let mut interval = time::interval(duration);
        loop {
            interval.tick().await;
            engine.increment_epoch();
        }
    });

    let (io_tx, mut io_rx) = mpsc::channel::<QueryRequest>(1024);
    let ioq_handle = tokio::spawn(async move {
        let duration = time::Duration::from_millis(60000);
        while let Some(query) = io_rx.recv().await {
            if let Err(err) = time::timeout(duration, query.into_query()).await {
                eprintln!("query.into_query(): {}", err);
                break;
            }
        }
    });

    while let Some(request) = istream.next().await {
        match request {
            Ok(request) => {
                let duration = time::Duration::from_millis(storage_manager.timeout);
                match time::timeout(
                    duration,
                    streaming_run_sandbox(
                        request,
                        io_tx.clone(),
                        sandbox_manager.clone(),
                        storage_manager.clone(),
                    ),
                )
                .await
                {
                    Ok(result) => {
                        let resp = result.map_err(|s| Status::aborted(s.to_string()));
                        if let Err(_) = mpsc_tx.send(resp).await {
                            break;
                        }
                    }
                    Err(err) => {
                        eprintln!("streaming_run_sandbox: {}", err);
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

    ioq_handle.abort();
    clk_handle.abort();
}

async fn streaming_run_sandbox(
    request: StreamingRunRequest,
    io_tx: mpsc::Sender<QueryRequest>,
    sandbox_manager: SandboxManager<Storage>,
    storage_manager: StorageManager,
) -> Result<StreamingRunResponse> {
    match request.data.ok_or(anyhow!("empty args"))? {
        streaming_run_request::Data::Args(args) => {
            let args = args.args;
            let wasi_ctx = wasmtime_wasi::WasiCtxBuilder::new()
                .inherit_stdout()
                .inherit_stderr()
                .args(&args)?
                .build();

            let storage = storage_manager.instantiate(io_tx);
            let mut sandbox = sandbox_manager.instantiate(wasi_ctx, storage).await?;

            sandbox.store.epoch_deadline_async_yield_and_update(1);
            let _ = sandbox.call_async().await?;

            let imports = sandbox.store.into_data().imports;
            let (nodes_table, edges_table) = (imports.selected_nodes, imports.selected_edges);

            let response = StreamingRunResponse {
                errors: None,
                nodes_table: Some(nodes_table),
                edges_table: Some(edges_table),
            };
            Ok(response)
        }
        _ => Err(anyhow!("invalid args"))?,
    }
}

// async fn streaming_run_sandbox()

// async fn streaming_run_impl2(
//     mut istream: Streaming<StreamingRunRequest>,
// ) -> Result<StreamingRunStream> {
//     let (sandbox_manager, storage_manager) =
//         streaming_run_config(istream.next().await.ok_or(anyhow!("empty config"))??).await?;

//     let (async_tx, sync_rx) = mpsc::channel(8);
//     let (sync_tx, async_rx) = mpsc::channel(8);

//     let bypass_tx = sync_tx.clone();
//     tokio::task::spawn(async move {
//         while let Some(request) = istream.next().await {
//             match request {
//                 Ok(request) => {
//                     if let Err(_) = async_tx.send(request).await {
//                         break;
//                     }
//                 }
//                 Err(err) => {
//                     let _ = bypass_tx.send(Err(err));
//                     break;
//                 }
//             }
//         }
//     });

//     tokio::task::spawn_blocking(move || {
//         match streaming_run_args(sandbox_manager, storage_manager, sync_rx, sync_tx) {
//             Ok(()) => (),
//             Err(err) => {
//                 eprintln!("ERROR: {}", err);
//             }
//         }
//     });

//     Ok(ReceiverStream::new(async_rx))
// }

// fn streaming_run_args(
//     sandbox_manager: SandboxManager<Storage>,
//     mut storage_manager: StorageManager,
//     mut sync_rx: mpsc::Receiver<StreamingRunRequest>,
//     sync_tx: mpsc::Sender<Result<StreamingRunResponse, Status>>,
// ) -> Result<()> {
//     let runtime = tokio::runtime::Builder::new_multi_thread()
//         .enable_all()
//         .build()?;

//     let mut io_rx = storage_manager.take_rx().unwrap();
//     runtime.spawn(async move {
//         // let url = match GLOBALS.config.storage_server {
//         //     std::net::SocketAddr::V4(x) => format!("http://{}:{}/", x.ip(), x.port()),
//         //     std::net::SocketAddr::V6(x) => format!("http://[{}]:{}/", x.ip(), x.port()),
//         // };
//         // match wart_storage_client::WartStorageClient::connect(url).await {
//         //     Ok(client) => {
//         //         while let Some(query) = io_rx.recv().await {
//         //             match query.into_query(client.clone()).await {
//         //                 Ok(_) => (),
//         //                 Err(err) => eprintln!("query error: {}", err),
//         //             }
//         //         }
//         //     }
//         //     Err(err) => {
//         //         eprintln!("unable to connect storage server: {}", err);
//         //     }
//         // }
//     });

//     while let Some(request) = sync_rx.blocking_recv() {
//         let (_exit_tx, exit_rx) = oneshot::channel::<u8>();
//         {
//             let engine = sandbox_manager.engine.clone();
//             let sync_tx = sync_tx.clone();
//             let exto = storage_manager.ex_timeout;
//             runtime.spawn(async move {
//                 tokio::select! {
//                     _ = exit_rx => (),
//                     _ = sync_tx.closed() => {
//                         engine.increment_epoch();
//                     },
//                     _ = time::sleep(time::Duration::from_millis(exto)) => {
//                         engine.increment_epoch();
//                     }
//                 }
//             });
//         }

//         let manager = sandbox_manager.clone();
//         let imports = storage_manager.new_imports();

//         match sandbox_block_on(request, manager, imports) {
//             Ok(response) => {
//                 if let Err(_) = sync_tx.blocking_send(Ok(response)) {
//                     break;
//                 }
//             }
//             Err(err) => {
//                 let _ = sync_tx.blocking_send(Err(Status::aborted(err.to_string())));
//                 break;
//             }
//         };
//     }

//     Ok(())
// }
