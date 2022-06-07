import grpc
from rpc_client import *

import numpy as np
import pandas as pd

from typing import *
import os

def to_pd(table: DataFrame) -> pd.DataFrame:
    data = {}
    for h, s in zip(table.headers, table.columns):
        if s.bool_values.data:
            series = list(s.bool_values.data)
            data[h] = pd.Series(series, dtype=np.bool8)
        elif s.int32_values.data:
            series = list(s.int32_values.data)
            data[h] = pd.Series(series, dtype=np.int32)
        elif s.int64_values.data:
            series = list(s.int64_values.data)
            data[h] = pd.Series(series, dtype=np.int64)
        elif s.float32_values.data:
            series = list(s.float32_values.data)
            data[h] = pd.Series(series, dtype=np.float32)
        elif s.float64_values.data:
            series = list(s.float64_values.data)
            data[h] = pd.Series(series, dtype=np.float64)
        elif s.string_values.data:
            series = list(s.string_values.data)
            data[h] = pd.Series(series, dtype=np.str0)
        else:
            return table.comment, None
    return table.comment, pd.DataFrame(data)
        

def update_store_iter(token: str, pairs: Dict[str, int], batch_size = 2):
    pairs = [(k, v) for k, v in pairs.items()]
    offset = 0
    # 按batch写入，避免单次请求数据包过大
    while offset < len(pairs):
        batch = pairs[offset:offset+batch_size]
        yield UpdateStoreRequest(
            token = token,
            keys = Series.StringSeries(data = [k for k, _ in batch]),
            vals = Series(int32_values = Series.Int32Series(data = [v for _, v in batch])),
            merge_type = UpdateStoreRequest.MergeType.ADD,
        )
        offset += batch_size

def streaming_run_iter(token: str, args_lst: List[List[str]]):
    # 第一个请求必须是Config
    yield StreamingRunRequest(
        config = StreamingRunRequest.Config(
            token = token,
        )
    )
    
    # 剩下的请求必须是Args
    for args in args_lst:
        yield StreamingRunRequest(
            args = StreamingRunRequest.Args(
                args = args
            )
        )

def run():
    # 获取编译好的wasm字节码
    with open("./a.wasm", "rb") as f:
        program = f.read()
    
    # 连接采样服务器
    with grpc.insecure_channel("[::1]:6066") as channel:
        # 创建采样客户端
        stub = WartWorkerStub(channel)
        
        # 启动采样session，设置图空间名称，上传采样脚本
        resp = stub.OpenSession(OpenSessionRequest(
            space_name = "nebula:basketballplayer",
            program = program,
            io_timeout = 1000, # 单次IO限时(废弃)
            ex_timeout = 2000, # 单次采样限时
        ))
        
        # 获得session的token
        token = resp.ok.token
        print("token: ", token)
        
        # # 更新session全局状态
        # pairs = {"a": 1, "b": 1, "c": 1}
        # for resp in stub.UpdateStore(update_store_iter(token, pairs)):
        #     assert resp.ok_count > 0
        
        # 运行采样脚本
        args = [["arg1"], []] # 运行两次，可以设置命令行参数也可以不设置
        for resp in stub.StreamingRun(streaming_run_iter(token, args)):
            for t in resp.tables:
                name, table = to_pd(t)
                print("name:", name)
                print(table)
            print("==================================")
        
        # 关闭采样session
        stub.CloseSession(CloseSessionRequest(
            token = token,
        ))

if __name__ == "__main__":
    run()