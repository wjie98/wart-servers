#!/bin/bash

cd `dirname $0`

PROTO_PATH="../interface/protobuf"
OUT_PATH="./rpc_client"

mkdir -p $OUT_PATH
python -m grpc_tools.protoc --proto_path=$PROTO_PATH --python_out=$OUT_PATH --grpc_python_out=$OUT_PATH $PROTO_PATH/*
