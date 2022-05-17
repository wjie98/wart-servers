#!/bin/bash

cd `dirname $0`

# conda activate ray

python -m grpc_tools.protoc --proto_path=./protobuf --python_out=. --grpc_python_out=. ./protobuf/*
wit-bindgen c --import ./wasm/imports.wit --out-dir ./
wit-bindgen wasmtime --export ./wasm/imports.wit --out-dir ./