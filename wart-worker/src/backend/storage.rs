use crate::bindgen::*;

use std::net::SocketAddr;

use wart_storage_client::WartStorageClient;
use tonic::transport::{Channel, Error};

pub struct StorageConnectionManager {
    pub url: String,
}

impl StorageConnectionManager {
    pub fn new(addr: SocketAddr) -> Self {
        let url = match addr {
            SocketAddr::V4(x) => format!("http://{}:{}/", x.ip(), x.port()),
            SocketAddr::V6(x) => format!("http://[{}]:{}/", x.ip(), x.port()),
        };
        Self { url }
    }
}

#[mobc::async_trait]
impl mobc::Manager for StorageConnectionManager {
    type Connection = WartStorageClient<Channel>;
    type Error = Error;

    async fn connect(&self) -> Result<Self::Connection, Self::Error> {
        let con = WartStorageClient::connect(self.url.clone()).await?;
        Ok(con)
    }

    async fn check(&self, con: Self::Connection) -> Result<Self::Connection, Self::Error> {
        Ok(con)
    }
}
