#!/bin/sh
protoc --proto_path=./ --cpp_out=./ MumBot.proto Mumble.proto
protoc --proto_path=./ --grpc_out=./ --plugin=protoc-gen-grpc=$(which grpc_cpp_plugin) MumBot.proto
