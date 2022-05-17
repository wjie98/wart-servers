use crate::bindgen::*;
use crate::wasm::{SandboxManager, Storage};
use crate::GLOBALS;
use anyhow::Result;
use mobc_redis::redis;
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
        io_timeout,
        ex_timeout,
    } = request;

    let module = tokio::task::spawn_blocking(move || -> Result<Vec<u8>> {
        let mut config = wasmtime::Config::default();
        // config.async_support(true);
        config.epoch_interruption(true);
        // config.consume_fuel(true);
        // config.wasm_reference_types(true);
        let manager = SandboxManager::<Storage>::from_bytes(&program, &config)?;
        manager.module.serialize()
    })
    .await??;

    let token = uuid::Uuid::new_v4().to_string();
    let store_key1 = format!("wart:session:{}", token);
    let store_key2 = format!("wart:store:{}", token);
    let mut con = GLOBALS.redis_pool.get().await?;
    let _: () = redis::pipe()
        .atomic()
        .hset(&store_key1, "space_name", &space_name)
        .ignore()
        .hset(&store_key1, "token", &token)
        .ignore()
        .hset(&store_key1, "module", &*module)
        .ignore()
        .hset(&store_key1, "io_timeout", io_timeout)
        .ignore()
        .hset(&store_key1, "ex_timeout", ex_timeout)
        .ignore()
        .hset(&store_key2, "__dummy__", &space_name)
        .ignore()
        .query_async(&mut *con)
        .await?;

    let (token,): (String,) = redis::pipe()
        .hget(&store_key1, "token")
        .query_async(&mut *con)
        .await?;

    Ok(OpenSessionResponse {
        result: Some(open_session_response::Result::Ok(
            open_session_response::Ok { token },
        )),
    })
}
