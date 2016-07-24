#!/bin/sh
#test
clang++ `pkg-config --libs gnutls --cflags` `pkg-config --libs protobuf --cflags` -ggdb  -std=c++1z test.cpp ./libmumbot/MumBotState.cpp ./libmumbot/MumBotConnectionMgr.cpp ./libmumbot/proto/Mumble.pb.cc -o test
