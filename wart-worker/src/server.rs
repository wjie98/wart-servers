mod backend;
mod bindgen;
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
}

pub struct Globals {
    config: Config,

    #[allow(dead_code)]
    redis: mobc::Pool<RedisConnectionManager>,

    #[allow(dead_code)]
    storage: mobc::Pool<StorageConnectionManager>,
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
            mobc::Pool::builder().max_open(64).build(manager)
        };

        let storage = {
            let manager = StorageConnectionManager::new(config.storage_server);
            mobc::Pool::builder().max_open(64).build(manager)
        };

        Globals {
            config,
            redis,
            storage,
        }
    };
}

#[tokio::main]
async fn main() -> Result<(), tonic::transport::Error> {
    // use tracing_subscriber::{fmt, layer::SubscriberExt, util::SubscriberInitExt};
    // tracing_subscriber::registry().with(fmt::layer()).init();
    tracing_subscriber::fmt::init();

    log::info!("rpc_server: {}", GLOBALS.config.rpc_server);

    let router = WartWorkerServer::new(Router::new());
    tonic::transport::Server::builder()
        .add_service(router)
        .serve(GLOBALS.config.rpc_server)
        .await?;
    Ok(())
}
