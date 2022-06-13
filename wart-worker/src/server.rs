mod backend;
mod bindgen;
mod log_tracer;
mod rpc_server;
mod wasm;

use bindgen::wart_worker_server::WartWorkerServer;
use rpc_server::Router;

use lazy_static::lazy_static;
use serde::Deserialize;

use backend::RedisConnectionManager;
use backend::StorageConnectionManager;

#[derive(Deserialize)]
pub struct Config {
    #[serde(rename = "rpc_server")]
    rpc_server: std::net::SocketAddr,

    #[serde(rename = "redis_server")]
    redis_server: std::net::SocketAddr,

    #[serde(rename = "storage_server")]
    storage_server: std::net::SocketAddr,

    #[serde(rename = "cores")]
    num_workers: usize,
}

pub struct Globals {
    config: Config,

    #[allow(dead_code)]
    redis: mobc::Pool<RedisConnectionManager>,

    #[allow(dead_code)]
    storage: mobc::Pool<StorageConnectionManager>,

    #[allow(dead_code)]
    runtime: tokio::runtime::Runtime,
}

lazy_static! {
    pub static ref GLOBALS: Globals = {
        use std::process::exit;
        use std::{env, fs};

        let config: Config = match env::args().nth(1) {
            Some(path) => {
                let s = fs::read(path).unwrap();
                serde_yaml::from_slice(&s).unwrap()
            }
            None => {
                eprintln!("Usage: wart config.yaml");
                exit(1)
            }
        };

        let redis = {
            let manager = RedisConnectionManager::new(config.redis_server);
            mobc::Pool::builder().get_timeout(None).max_open(128).build(manager)
        };

        let storage = {
            let manager = StorageConnectionManager::new(config.storage_server);
            mobc::Pool::builder().get_timeout(None).max_open(8).build(manager)
        };

        let runtime = tokio::runtime::Builder::new_multi_thread()
            .worker_threads(config.num_workers)
            .enable_all()
            .build()
            .unwrap();

        Globals {
            config,
            redis,
            storage,
            runtime,
        }
    };
}

fn main() -> Result<(), tonic::transport::Error> {
    tracing_subscriber::fmt::init();
    let runtime = tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .build()
        .unwrap();

    runtime.block_on(async {
        log::info!("rpc_server: {}", GLOBALS.config.rpc_server);
        let router = WartWorkerServer::new(Router::new());
        tonic::transport::Server::builder()
            .add_service(router)
            .serve(GLOBALS.config.rpc_server)
            .await?;
        Ok(())
    })
}

// #[tokio::main]
// async fn main() -> Result<(), tonic::transport::Error> {
//     tracing_subscriber::fmt::init();

//     log::info!("rpc_server: {}", GLOBALS.config.rpc_server);

//     let router = WartWorkerServer::new(Router::new());
//     tonic::transport::Server::builder()
//         .add_service(router)
//         .serve(GLOBALS.config.rpc_server)
//         .await?;
//     Ok(())
// }
