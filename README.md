# wart-servers

启动服务器:
```bash
# 假设已经配置好cargo并升级到最新版本
# 需要登录hpc01，部分组件未合并到项目里
cd wart-worker
cargo run --release --bin server config.yaml
```

客户端操作：
- 参考interface文件夹，包括grpc接口定义和wasm接口定义
- wasm文件夹里对wasm接口做了c++版封装
- example文件夹给出了一个简单的范例
