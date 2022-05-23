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

    let store_key1 = format!("wart:session:{}", token);
    let store_key2 = format!("wart:store:{}", token);
    let mut con = GLOBALS.redis.get().await?;
    let _: () = redis::pipe()
        .atomic()
        .unlink(&store_key1)
        .ignore()
        .unlink(&store_key2)
        .ignore()
        .query_async(&mut *con)
        .await?;

    Ok(CloseSessionResponse {})
}
