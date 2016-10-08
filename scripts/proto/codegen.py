from grpc.tools import protoc

protoc.main(
	(
	'',
	'-I../../libmumbot/proto',
	'--python_out=.',
	'--grpc_python_out=.',
	'../../libmumbot/proto/MumBot.proto',

	)
)
