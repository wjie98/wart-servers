假设已经配置了python执行gRPC的必要组件。执行
```bash
./build.sh
```
编译WASM程序
```bash
../wasm/build.sh
./wacc a.cpp
```
执行脚本（可能要修改端口）
```bash
python client.py
```
目前未提供日志功能，请测试同学自己启动服务器查看终端输出。