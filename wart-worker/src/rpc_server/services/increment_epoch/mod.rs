use crate::bindgen::*;
use crate::GLOBALS;
use anyhow::Result;
use tonic::{Request, Response, Status};

pub async fn increment_epoch(
    request: Request<IncrementEpochRequest>,
) -> Result<Response<IncrementEpochResponse>, Status> {
    match increment_epoch_impl(request.into_inner()).await {
        Ok(msg) => Ok(Response::new(msg)),
        Err(err) => Err(Status::aborted(err.to_string())),
    }
}

pub async fn increment_epoch_impl(
    request: IncrementEpochRequest,
) -> Result<IncrementEpochResponse> {
    let IncrementEpochRequest { token } = request;

    let mut con = GLOBALS.redis.get().await?;

    let key = format!("wart:session:{}", token);
    let (epoch,): (u64,) = redis::pipe()
        .hincr(&key, "epoch", 1u64)
        .query_async(&mut *con)
        .await?;

    Ok(IncrementEpochResponse { epoch })
}
