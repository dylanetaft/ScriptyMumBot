import os
import grpc
import proto.MumBot_pb2

channel = grpc.insecure_channel('localhost:50080')
stub = proto.MumBot_pb2.MumBotRPCStub(channel)


response = stub.Say(proto.MumBot_pb2.TextMessage(msg = 'HELLO I AM A BOT'))
