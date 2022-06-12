use crate::bindgen::*;
use crate::GLOBALS;
use anyhow::Result;
use tonic::{Request, Response, Status};

pub async fn close_session(
    request: Request<CloseSessionRequest>,
) -> Result<Response<CloseSessionResponse>, Status> {
    match close_session_impl(request.into_inner()).await {
        Ok(msg) => Ok(Response::new(msg)),
        Err(err) => Err(Status::aborted(err.to_string())),
    }
}

pub async fn close_session_impl(request: CloseSessionRequest) -> Result<CloseSessionResponse> {
    let CloseSessionRequest { token } = request;

    let mut con = GLOBALS.redis.get().await?;
    log::info!("closing session: {}", token);

    let key = format!("wart:session:{}", token);
    let (epoch,): (u64,) = redis::pipe()
        .hget(&key, "epoch")
        .query_async(&mut *con)
        .await?;

    for ep in 0..=(epoch + 1) {
        let key = format!("wart:store:{}:{}", token, ep);
        redis::pipe()
            .unlink(&key)
            .ignore()
            .query_async(&mut *con)
            .await?;
    }

    redis::pipe()
        .unlink(&key)
        .ignore()
        .query_async(&mut *con)
        .await?;

    log::info!("session {} closed", token);

    Ok(CloseSessionResponse {})
}
