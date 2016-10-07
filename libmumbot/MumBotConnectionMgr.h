#pragma once
#include <thread>
#include <string>
#include <gnutls/gnutls.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include "MumBotEventListener.h"
#include "MumBotState.h"
#include <mutex>
#include <cstring>
#include "proto/Mumble.pb.h"
#include "proto/MumBot.pb.h"
#include "proto/MumBot.grpc.pb.h"
#include <memory>

//GRPC includes
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/security/server_credentials.h>
#include "MumBotRPCWorkQueueMgr.h"



namespace libmumbot {
	class MumBotConnectionMgr : public MumBotRPC::Service {
	public:
		static const short APKT_TYPE_CELT = 0b00000000;
		static const short APKT_TYPE_PING = 0b00100000;
		static const short APKT_TYPE_SPEEX = 0b01000000;
		static const short APKT_TYPE_CELT_BETA = 0b01100000;
		static const short APKT_TYPE_OPUS = 0b10000000;

		void setListener(MumBotEventListener *listener);
		void setStateObject(MumBotState *state);
	    void startClient(std::string host, std::string port, std::string nickname);

		void sendUDPTunnelAudioData(std::string buffer);
		::grpc::Status Say(::grpc::ServerContext* context, const ::libmumbot::TextMessage* request, ::libmumbot::TextMessageResponse* response) override;
		::grpc::Status SubscribeToTextMessages(::grpc::ServerContext* context, const ::libmumbot::TextMessageRequest* request, ::grpc::ServerWriter<libmumbot::TextMessage>* writer) override;
	private:
	    static const short PKT_TYPE_VERSION = 0;
		static const short PKT_TYPE_UDPTUNNEL = 1;
	    static const short PKT_TYPE_AUTH = 2;
	    static const short PKT_TYPE_PING = 3;
	    static const short PKT_TYPE_REJECT = 4;
	    static const short PKT_TYPE_SERVERSYNC = 5;
	    static const short PKT_TYPE_CHANNELREMOVE = 6;
	    static const short PKT_TYPE_CHANNELSTATE = 7;
	    static const short PKT_TYPE_USERREMOVE = 8;
	    static const short PKT_TYPE_USERSTATE = 9;
	    static const short PKT_TYPE_TEXTMESSAGE = 11;
	    static const short PKT_TYPE_PERMISSIONDENIED = 12;
		static const short PKT_TYPE_CODECVERSION = 21;



		MumBotRPCWorkQueueMgr RPCWorkQueueMgr_;

	    uint8_t c_headerbuffer_[6]; //first 2 bytes packet type, next 4 are len
	    uint8_t c_headerpos_ = 0;
	    uint32_t c_datapos_ = 0;
	    uint8_t *c_data_;
		uint32_t myMumbleSessionId_ = -1;
	    fd_set socketSet_;
		libmumbot::MumBotEventListener *eventListener_ = NULL;
		libmumbot::MumBotState *mumState_ = NULL;

	    gnutls_session_t gnutls_session_;
	    int mumCmd_socketFD_ = -1;
	    int mum_maxSocketFD_ = 0;
	    std::mutex mutex_;
	    std::thread clientLoopThread_;
	    std::thread clientKeepAliveThread_;
	    void clientLoop();
	    void clientKeepAlive();
	    void sendData(short pktType, std::string data);
	    bool processInboundPkt();
		void startRPCSerice();

	    std::string createVersionPktData();
	    std::string createAuthPktData(std::string username);
	    std::string createPingPktData();
	    std::string createDeafMutePktData();
		std::string createVoicePktData();
	};
}
