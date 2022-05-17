import grpc
from rpc_client import *

def update_store_iter(token):
    for _ in range(10):
        yield UpdateStoreRequest(
            token = token,
            keys = Series.StringSeries(
                data = ["a", "b", "c"]
            ),
            vals = Series(int32_values = Series.Int32Series(
                data = [1, 2, 3]
            )),
            merge_type = UpdateStoreRequest.MergeType.ADD,
        )

def streaming_run_iter(token):
    yield StreamingRunRequest(
        config = StreamingRunRequest.Config(
            token = token,
        )
    )
    
    for _ in range(10):
        yield StreamingRunRequest(
            args = StreamingRunRequest.Args(
                args = ["a", "b", "c"]
            )
        )

def run():
    with open("./a.wasm", "rb") as f:
        program = f.read()
    
    with grpc.insecure_channel("[::1]:6066") as channel:
        stub = WartWorkerStub(channel)
        resp = stub.OpenSession(OpenSessionRequest(
            space_name = "nebula:basketballplayer",
            program = program,
            io_timeout = 1000,
            ex_timeout = 2000,
        ))
        
        token = resp.ok.token
        print("token: ", token)
        
        for resp in stub.UpdateStore(update_store_iter(token)):
            print(resp)
            
        for resp in stub.StreamingRun(streaming_run_iter(token)):
            print(resp)
            
        stub.CloseSession(CloseSessionRequest(
            token = token,
        ))

if __name__ == "__main__":
    run()