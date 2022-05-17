#!/bin/bash

cd `dirname $0`

wit-bindgen c --import ../interface/wasm/imports.wit --out-dir ./capi

make clean && make