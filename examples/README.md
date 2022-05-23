环境配置：
```bash
# 安装rustup
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh

# 安装wasi-sdk >= 15.0
wget "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-15/wasi-sdk-15.0-linux.tar.gz"
tar xvf wasi-sdk-15.0-linux.tar.gz -C ${HOME}

# 安装python环境
pip install grpcio grpcio-tools pandas
```  

启动采样服务器：
```bash
# 下载源码
git clone https://github.com/wjie98/wart-servers.git
cd wart-servers/wart-worker

# 编辑配置文件，修改端口
vim config.yaml

# 启动
cargo run --release --bin server config.yaml
```  

运行样例程序：
```bash
# 编译C库
cd wart-servers/examples
../wasm/build.sh

# 生成python客户端
./build.sh

# 编译采样脚本
./wacc a.cpp

# 启动采样程序
python client.py

# 目前未提供日志功能，请测试同学自己启动服务器查看终端输出。
```

参考资料：  
[rustup](https://rustup.rs/)  
[grpcio](https://www.grpc.io/docs/languages/python/quickstart/)  
[wasi-sdk](https://github.com/WebAssembly/wasi-sdk)  
