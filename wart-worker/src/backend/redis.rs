use redis::{Client, RedisError};
use redis::aio::Connection;

use std::net::SocketAddr;

pub struct RedisConnectionManager {
    client: Client,
}

impl RedisConnectionManager {
    pub fn new(addr: SocketAddr) -> Self {
        let url = match addr {
            SocketAddr::V4(x) => format!("redis://{}:{}/", x.ip(), x.port()),
            SocketAddr::V6(x) => format!("redis://[{}]:{}/", x.ip(), x.port()),
        };
        Self { client: Client::open(url).unwrap() }
    }
}

#[mobc::async_trait]
impl mobc::Manager for RedisConnectionManager {
    type Connection = Connection;
    type Error = RedisError;

    async fn connect(&self) -> Result<Self::Connection, Self::Error> {
        let con = self.client.get_async_connection().await?;
        Ok(con)
    }

    async fn check(&self, mut con: Self::Connection) -> Result<Self::Connection, Self::Error> {
        let _: () = redis::cmd("PING").query_async(&mut con).await?;
        Ok(con)
    }
}


