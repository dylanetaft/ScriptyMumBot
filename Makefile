CXX = g++
LDFLAGS =
CXXFLAGS = -std=c++1z -ggdb

CXXFLAGS += $(shell pkg-config gnutls --cflags;pkg-config protobuf --cflags;pkg-config opus --cflags;pkg-config ogg --cflags)
LDFLAGS += $(shell pkg-config gnutls --libs;pkg-config protobuf --libs;pkg-config opus --libs;pkg-config ogg --libs)

default: bot

LIBMUMBOT_SRCS = ./libmumbot/MumBotState.cpp ./libmumbot/MumBotConnectionMgr.cpp ./libmumbot/opus/OpusOggOutputWriter.cpp ./libmumbot/opus/AudioInputOpusWriter.cpp ./libmumbot/libmumbot.cpp ./libmumbot/MumBotConfig.cpp ./libmumbot/proto/Mumble.pb.cc

LIBMUMBOT_OBJS = MumBotState.o MumBotConnectionMgr.o OpusOggOutputWriter.o AudioInputOpusWriter.o libmumbot.o MumBotConfig.o Mumble.pb.o

libmumbot.a: $(LIBMUMBOT_SRCS)
        $(CXX) $(CXXFLAGS) -c $(LIBMUMBOT_SRCS)
        ar rcu libmumbot.a $(LIBMUMBOT_OBJS)


scriptybot: test.cpp
        $(CXX) $(CXXFLAGS) -c test.cpp

bot: test.o libmumbot.a
        $(CXX) $(CXXFLAGS) test.o libmumbot.a $(LDFLAGS) -o bot
