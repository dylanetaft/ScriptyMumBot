#!/bin/sh
#test
clang++ -Wno-deprecated-declarations `pkg-config --libs gnutls --cflags` `pkg-config --libs protobuf --cflags` `pkg-config --libs opus --cflags` `pkg-config --libs ogg --cflags` -ggdb  -std=c++1z ./test.cpp ./libmumbot/MumBotState.cpp ./libmumbot/MumBotConnectionMgr.cpp ./libmumbot/opus/OpusOggOutputWriter.cpp ./libmumbot/opus/AudioInputOpusWriter.cpp ./libmumbot/libmumbot.cpp ./libmumbot/MumBotConfig.cpp ./libmumbot/proto/Mumble.pb.cc -o test
