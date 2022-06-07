#![allow(unused_variables)]

use crate::bindgen::wart_worker_server::WartWorker;
use crate::bindgen::*;

use tokio_stream::wrappers::ReceiverStream;
use tonic::{Request, Response, Status, Streaming};

pub struct Router {}

impl Router {
    #[allow(dead_code)]
    pub fn new() -> Self {
        Self {}
    }
}

#[tonic::async_trait]
impl WartWorker for Router {
    type StreamingRunStream = ReceiverStream<Result<StreamingRunResponse, Status>>;
    // type UpdateStoreStream = ReceiverStream<Result<UpdateStoreResponse, Status>>;
    async fn open_session(
        &self,
        request: Request<OpenSessionRequest>,
    ) -> Result<Response<OpenSessionResponse>, Status> {
        super::services::open_session::open_session(request).await
    }

    async fn close_session(
        &self,
        request: Request<CloseSessionRequest>,
    ) -> Result<Response<CloseSessionResponse>, Status> {
        super::services::close_session::close_session(request).await
    }

    async fn streaming_run(
        &self,
        request: Request<Streaming<StreamingRunRequest>>,
    ) -> Result<Response<Self::StreamingRunStream>, Status> {
        super::services::streaming_run::streaming_run(request).await
    }

    async fn increment_epoch(
        &self,
        request: Request<IncrementEpochRequest>,
    ) -> Result<Response<IncrementEpochResponse>, Status> {
        super::services::increment_epoch::increment_epoch(request).await
    }

    // async fn update_store(
    //     &self,
    //     request: Request<Streaming<UpdateStoreRequest>>,
    // ) -> Result<Response<Self::UpdateStoreStream>, Status> {
    //     super::services::update_store::update_store(request).await
    // }
}
