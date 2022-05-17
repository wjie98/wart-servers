mod bindgen;
mod rpc_server;
mod wasm;

use bindgen::wart_worker_server::WartWorkerServer;
use rpc_server::Router;

use lazy_static::lazy_static;
use serde::Deserialize;

use mobc_redis::{redis, RedisConnectionManager};

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
    redis_pool: mobc::Pool<RedisConnectionManager>,
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

        let redis_pool = {
            let client = redis::Client::open(format!(
                "redis://{}:{}/",
                config.redis_server.ip(),
                config.redis_server.port()
            ))
            .unwrap();
            let manager = RedisConnectionManager::new(client);
            mobc::Pool::builder().max_open(64).build(manager)
        };

        Globals { config, redis_pool }
    };
}

#[tokio::main]
async fn main() -> Result<(), tonic::transport::Error> {
    println!("rpc_server: {}", GLOBALS.config.rpc_server);

    // {
    //     let mut conn = GLOBALS.redis_pool.get().await.unwrap();

    //     let (s,): (i32,) = redis::pipe()
    //         .set::<&str, i32>("a", 1)
    //         .ignore()
    //         .get("a")
    //         .query_async(&mut *conn)
    //         .await
    //         .unwrap();
    //     println!("{}", s);
    // }

    let router = WartWorkerServer::new(Router::new());
    tonic::transport::Server::builder()
        .add_service(router)
        .serve(GLOBALS.config.rpc_server)
        .await?;
    Ok(())
}
