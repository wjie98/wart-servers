use crate::bindgen::*;
use crate::GLOBALS;
use anyhow::{anyhow, Result};
use futures::StreamExt;
use redis::AsyncCommands;

use tokio::sync::mpsc;
use tokio_stream::wrappers::ReceiverStream;
use tonic::{Request, Response, Status, Streaming};

type UpdateStoreStream = ReceiverStream<Result<UpdateStoreResponse, Status>>;

pub async fn update_store(
    request: Request<Streaming<UpdateStoreRequest>>,
) -> Result<Response<UpdateStoreStream>, Status> {
    match update_store_impl(request.into_inner()).await {
        Ok(msg) => Ok(Response::new(msg)),
        Err(err) => Err(Status::aborted(err.to_string())),
    }
}

async fn update_store_impl(
    mut istream: Streaming<UpdateStoreRequest>,
) -> Result<UpdateStoreStream> {
    let (mpsc_tx, mpsc_rx) = mpsc::channel(512);
    tokio::spawn(async move {
        while let Some(message) = istream.next().await {
            let response = match message {
                Ok(request) => match write_to_redis(request).await {
                    Ok(msg) => Ok(msg),
                    Err(err) => Err(Status::aborted(err.to_string())),
                },
                Err(err) => Err(err),
            };
            if let Err(_) = mpsc_tx.send(response).await {
                break;
            }
        }
    });

    let ostream = ReceiverStream::new(mpsc_rx);
    Ok(ostream)
}

async fn write_to_redis(request: UpdateStoreRequest) -> Result<UpdateStoreResponse> {
    use update_store_request::MergeType;

    let UpdateStoreRequest {
        token,
        keys,
        vals,
        merge_type,
    } = request;

    let keys = keys.map(|s| s.data).ok_or(anyhow!("empty keys"))?;
    let vals = vals.and_then(|s| s.values);

    let store_key = format!("wart:store:{}", token);
    let mut con = GLOBALS.redis.get().await?;

    if !con.exists::<_, bool>(&store_key).await? {
        Err(anyhow!("no session: {}", token))?;
    }

    let mut ok_count = 0;
    match merge_type {
        x if x == MergeType::Del as i32 => {
            for field in keys.iter() {
                match con.hdel::<_, _, ()>(&store_key, field).await {
                    Ok(_) => ok_count += 1,
                    Err(_) => break,
                }
            }
        }
        x if x == MergeType::Add as i32 => {
            let vals = vals.ok_or(anyhow!("empty vals"))?;
            match vals {
                series::Values::Int32Values(vals) => {
                    for (field, value) in keys.iter().zip(vals.data.iter()) {
                        match con.hincr::<_, _, _, ()>(&store_key, field, *value).await {
                            Ok(_) => ok_count += 1,
                            Err(_) => break,
                        }
                    }
                }
                series::Values::Int64Values(vals) => {
                    for (field, value) in keys.iter().zip(vals.data.iter()) {
                        match con.hincr::<_, _, _, ()>(&store_key, field, *value).await {
                            Ok(_) => ok_count += 1,
                            Err(_) => break,
                        }
                    }
                }
                _ => (),
            }
        }
        x if x == MergeType::Mov as i32 => {
            let vals = vals.ok_or(anyhow!("empty vals"))?;
            match vals {
                series::Values::Int32Values(vals) => {
                    for (field, value) in keys.iter().zip(vals.data.iter()) {
                        match con.hset::<_, _, _, ()>(&store_key, field, *value).await {
                            Ok(_) => ok_count += 1,
                            Err(_) => break,
                        }
                    }
                }
                series::Values::Int64Values(vals) => {
                    for (field, value) in keys.iter().zip(vals.data.iter()) {
                        match con.hset::<_, _, _, ()>(&store_key, field, *value).await {
                            Ok(_) => ok_count += 1,
                            Err(_) => break,
                        }
                    }
                }
                series::Values::StringValues(vals) => {
                    for (field, value) in keys.iter().zip(vals.data.iter()) {
                        match con.hset::<_, _, _, ()>(&store_key, field, value).await {
                            Ok(_) => ok_count += 1,
                            Err(_) => break,
                        }
                    }
                }
                _ => (),
            }
        }
        _ => (),
    }

    Ok(UpdateStoreResponse { ok_count })
}
