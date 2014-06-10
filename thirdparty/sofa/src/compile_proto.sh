#!/bin/sh

PROTOC=../../protobuf/bin/protoc
PROTO_INCLUDE=../../protobuf/include

$PROTOC --proto_path=. --proto_path=$PROTO_INCLUDE --cpp_out=. \
sofa/pbrpc/rpc_meta.proto sofa/pbrpc/rpc_option.proto sofa/pbrpc/builtin_service.proto

