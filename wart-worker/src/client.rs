mod bindgen;
use bindgen::wart_worker_client::WartWorkerClient;
use bindgen::*;

#[tokio::main]
async fn main() {
    let program = tokio::fs::read("./a.wasm").await.unwrap();
    let mut client = WartWorkerClient::connect("http://[::1]:6066")
        .await
        .unwrap();

    let resp = client
        .open_session(OpenSessionRequest {
            program,
            space_name: "testing".into(),
            io_timeout: 1000,
            ex_timeout: 2000,
        })
        .await
        .unwrap();
    eprintln!("{:?}", resp);

    let token = match resp.into_inner().result.unwrap() {
        open_session_response::Result::Ok(info) => info.token,
        open_session_response::Result::Err(err) => Err(err).unwrap(),
    };
    eprintln!("token: {}", token);

    let resp = client
        .close_session(CloseSessionRequest { token })
        .await
        .unwrap();
    eprintln!("{:?}", resp);
}
