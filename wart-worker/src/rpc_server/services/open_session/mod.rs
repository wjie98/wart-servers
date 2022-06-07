use crate::bindgen::*;
use crate::wasm::{SandboxManager, Storage};
use crate::GLOBALS;
use anyhow::Result;
use tonic::{Request, Response, Status};

pub async fn open_session(
    request: Request<OpenSessionRequest>,
) -> Result<Response<OpenSessionResponse>, Status> {
    match open_session_impl(request.into_inner()).await {
        Ok(msg) => Ok(Response::new(msg)),
        Err(err) => Err(Status::aborted(err.to_string())),
    }
}

async fn open_session_impl(request: OpenSessionRequest) -> Result<OpenSessionResponse> {
    let OpenSessionRequest {
        space_name,
        program,
        io_timeout: _,
        ex_timeout,
    } = request;

    let module = tokio::task::spawn_blocking(move || -> Result<Vec<u8>> {
        let config = SandboxManager::<Storage>::default_config();
        let manager = SandboxManager::<Storage>::from_bytes(&program, &config)?;
        manager.module.serialize()
    })
    .await??;

    let mut con = GLOBALS.redis.get().await?;

    let token = uuid::Uuid::new_v4().to_string();
    let key = format!("wart:session:{}", token);
    let _: () = redis::pipe()
        .atomic()
        .hset(&key, "token", &token)
        .ignore()
        .hset(&key, "space_name", &space_name)
        .ignore()
        .hset(&key, "epoch", 0)
        .ignore()
        .hset(&key, "module", &*module)
        .ignore()
        .hset(&key, "ex_timeout", ex_timeout)
        .ignore()
        .query_async(&mut *con)
        .await?;

    let (token,): (String,) = redis::pipe()
        .atomic()
        .hget(&key, "token")
        .query_async(&mut *con)
        .await?;

    Ok(OpenSessionResponse {
        result: Some(open_session_response::Result::Ok(
            open_session_response::Ok { token },
        )),
    })
}
