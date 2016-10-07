import os
import grpc
import proto.MumBot_pb2

channel = grpc.insecure_channel('localhost:50080')
stub = proto.MumBot_pb2.MumBotRPCStub(channel)
req   = proto.MumBot_pb2.TextMessageRequest(regex_search_pattern = "");
response = stub.Say(proto.MumBot_pb2.TextMessage(msg = 'HELLO I AM A BOT'))
response2 = stub.SubscribeToTextMessages(req)
