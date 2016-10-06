#include "MumBotConnectionMgr.h"
#include <string>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <thread>
#include <chrono>

#include <cerrno>
#include <typeinfo>
#include <type_traits>

#include "libmumbot.h"

using namespace std;


namespace libmumbot {
	const short MumBotConnectionMgr::APKT_TYPE_CELT;
	const short MumBotConnectionMgr::APKT_TYPE_PING;
	const short MumBotConnectionMgr::APKT_TYPE_SPEEX;
	const short MumBotConnectionMgr::APKT_TYPE_CELT_BETA;
	const short MumBotConnectionMgr::APKT_TYPE_OPUS;




	void MumBotConnectionMgr::setStateObject(MumBotState *state) {
		mumState_ = state;
	}

	void MumBotConnectionMgr::sendUDPTunnelAudioData(std::string buffer) {
		sendData(PKT_TYPE_UDPTUNNEL, buffer);
	}
	void MumBotConnectionMgr::sendData(short pktType, std::string data) { //
	  long len = data.length();
	  std::lock_guard<std::mutex> lock(mutex_);
	  uint8_t buffer[2 + 4 + len];

	  uint16_t pktTypeN = htons(pktType);
	  uint32_t pktLenN = htonl(len);
	  memcpy(buffer, (const void *) &pktTypeN, 2);
	  memcpy(buffer + 2, (const void *) &pktLenN, 4);
	  memcpy(buffer + 6, data.data(), len);
	  int sent = gnutls_record_send(gnutls_session_,buffer,len + 6);
	  D(std::cout << "Send pkt " << pktType << " len:" << len << "sent:" << sent  << "\n");
	  if (sent != len + 6) {
	    std::cout << "Can this actually happen?\n";
	    std::cout << sent << "\n";
	  }

	}


	void MumBotConnectionMgr::setListener(libmumbot::MumBotEventListener *listener) {
		eventListener_ = listener;
	}
	bool MumBotConnectionMgr::processInboundPkt() {

	    //if (gnutls_record_check_pending(gnutls_session_) == 0) return 0;
	    if (c_headerpos_< 6) { //reading header
	      	int bytesr = gnutls_record_recv(gnutls_session_,c_headerbuffer_ + c_headerpos_, 6 - c_headerpos_);
	      	if (bytesr < 0) {
				int terr = errno;
	        	std::cout << "GNUTLS recv mumble header errorno: " <<  terr << " retval:" << bytesr << "\n";
	        return 0;
	      }
	      else if (bytesr == 0) {
	        std::cout << "EOF reading header.\n";
	        return 0;
	      }
	      else {
	        c_headerpos_ += bytesr;
	      }
	    }
	    //if (gnutls_record_check_pending(gnutls_session_) == 0) return 0;

	    if (c_headerpos_ == 6) { //TODO deal with endian
	    uint16_t pkttype = c_headerbuffer_[0] << 8 | c_headerbuffer_[1];
	    uint32_t pktlen = c_headerbuffer_[2] << 24 | c_headerbuffer_[3] << 16 | c_headerbuffer_[4] << 8 | c_headerbuffer_[5];
	    std::cout << "Pkt type:" << pkttype << " Pkt len:" << pktlen << "\n";
	    if (c_datapos_ == 0) c_data_ = (uint8_t *) malloc(pktlen); //initialize new ram
	    int bytesr = gnutls_record_recv(gnutls_session_,c_data_ + c_datapos_, pktlen - c_datapos_);
	    if (bytesr < 0) {
			int terr = errno;
			std::cout << "GNUTLS errno recv mumble pkt body errno:" << terr << " retval:" << bytesr << "\n";
	    }
	    else if (bytesr == 0) {
	      std::cout << "EOF reading data.\n";
	      return 0;
	    }
	    else {
	      c_datapos_ += bytesr;
	      if (c_datapos_ == pktlen) { //finished reading packet

	        std::string packet = std::string((char *) c_data_,pktlen);
	        free(c_data_);
	        c_headerpos_ = 0; //reset so we read header next
	        c_datapos_ = 0;

	        switch (pkttype) {
	          case PKT_TYPE_VERSION: {
	            MumbleProto::Version version;
	            version.ParseFromString(packet);
				//if (mumState_ != NULL) mumState_->updateUserState(version);
				if (eventListener_ != NULL) eventListener_->recvVersion(version);
	            break;
	          }
			  case PKT_TYPE_USERSTATE: {
				MumbleProto::UserState msg;
  	            msg.ParseFromString(packet);
  				if (mumState_ != NULL) mumState_->updateUserState(msg);
  				if (eventListener_ != NULL) eventListener_->recvUserState(msg);
  	            break;
			  }
			  case PKT_TYPE_UDPTUNNEL: {
				  if (eventListener_ != NULL) eventListener_->recvUDPTunnel(packet);
				  break;
			  }
	          case PKT_TYPE_SERVERSYNC: {
	            std::cout << "Server sync packet\n";
				MumbleProto::ServerSync sync;
				sync.ParseFromString(packet);
				myMumbleSessionId_ = sync.session(); //my user id
				if (eventListener_ != NULL) eventListener_->recvServerSync(sync);
	            break;
	          }
			  case PKT_TYPE_TEXTMESSAGE: {
				  MumbleProto::TextMessage txt;
				  txt.ParseFromString(packet);
				  if (eventListener_ != NULL) eventListener_->recvTextMessage(txt);
				  break;
			  }

	        }
	      }

	    }

	  }

	          //pkttype = ntohs(pkttype);
	          //pktlen = ntohl(pktlen);
	          //std::cout << "Pkt type:" << pkttype << "\nPkt len:" << pktlen << "\n";

	          return 1;
	}
	void MumBotConnectionMgr::clientLoop() {
	  for (;;) {
	    processInboundPkt(); //keep processing packets
	  }
	}

	void MumBotConnectionMgr::clientKeepAlive() {
	  for (;;) {
	    sendData(PKT_TYPE_PING,createPingPktData());
	    std::this_thread::sleep_for(std::chrono::milliseconds(15000));

	  }
	}
	std::string MumBotConnectionMgr::createDeafMutePktData() {
	  MumbleProto::UserState state;
	  state.set_deaf(true);
	  state.set_mute(true);
	  return state.SerializeAsString();
	}
	std::string MumBotConnectionMgr::createPingPktData() {
	  MumbleProto::Ping ping;
	  return ping.SerializeAsString();
	}

	std::string createVoicePktData() {
		uint8_t header = 0b10000000; //opus, normal talking
		return "";
	}
	std::string MumBotConnectionMgr::createVersionPktData() {
	  MumbleProto::Version version;
	  version.set_version(65536 + 768 + 0);
	  version.set_release("1.3.0");
	  version.set_os("FAKE OS");
	  return version.SerializeAsString();
	}


	std::string MumBotConnectionMgr::createAuthPktData(std::string username) {
	  MumbleProto::Authenticate auth;
	  auth.set_username(username.c_str());
	  auth.set_opus(1);
	  return auth.SerializeAsString();
	}


	static int cert_verify_callback(gnutls_session_t session) {
	  std::cout << "Verify cert\n";
	  return 0;
	}


	template<typename T, typename U>
	class RPCRequestHandler { //add destructor for cleanup TODO
	private:
		MumBotRPC::AsyncService *service_;
		grpc::ServerCompletionQueue *cq_;
		grpc::ServerContext ctx_;
		enum CallStatus {CREATE, PROCESS, FINISH};
		CallStatus status_;
		T receivedMsg_;
		grpc::ServerAsyncResponseWriter<U> *responder_;
		U response_;


		void process(libmumbot::TextMessageRequest &request, libmumbot::TextMessage &response) { //state machine for SubscribeToTextMessages

			if (status_ == CallStatus::CREATE) { //open endpoint
				status_ = CallStatus::PROCESS;
				service_->RequestSubscribeToTextMessages(&ctx_, &receivedMsg_, responder_, cq_, cq_, this);
			}
			else if (status_ == CallStatus::PROCESS) {
				new RPCRequestHandler<T, U>(cq_, service_); //spawn new instance to handle new requests while we continue processing
				std::cout << receivedMsg_.regex_search_pattern() << "\n";
				response.set_msg("LOL TEST");
				status_ = CallStatus::FINISH;
				responder_->Finish(response_,grpc::Status::OK,this);
			}
			else { //assert status is FINISH
				delete responder_;
				delete this;
			}

		}


		void process(libmumbot::TextMessage &request, libmumbot::TextMessageResponse &response) { //state machine for TextMessage

			if (status_ == CallStatus::CREATE) {
				status_ = CallStatus::PROCESS;
				service_->RequestSay(&ctx_, &receivedMsg_, responder_, cq_, cq_, this);
			}
			else if (status_ == CallStatus::PROCESS) {
				new RPCRequestHandler<T, U>(cq_, service_); //spawn new instance to handle new requests while we continue processing
				std::cout << receivedMsg_.msg() << "\n";
				response.set_success(true);
				status_ = CallStatus::FINISH;
				responder_->Finish(response_,grpc::Status::OK,this);
			}
			else { //assert status is FINISH
				delete responder_;
				delete this;
			}

		}

	public:

		RPCRequestHandler(grpc::ServerCompletionQueue *cq, MumBotRPC::AsyncService *service) {
			cq_ = cq;
			service_ = service;
			status_  = CallStatus::CREATE;
			responder_ = new grpc::ServerAsyncResponseWriter<U>(&ctx_);
			Proceed();
		}

		void Proceed() {
			process(receivedMsg_, response_);
		}

	};

	void MumBotConnectionMgr::startRPCSubscribeToTextMessages(MumBotRPC::AsyncService *service, grpc::ServerCompletionQueue *cq) {

		bool ok = false;
		void *tag;
		RPCRequestHandler<libmumbot::TextMessageRequest,libmumbot::TextMessage> *rpcRequestHandler = new RPCRequestHandler<libmumbot::TextMessageRequest,libmumbot::TextMessage>(cq,service);
		for (;;) {
			cq->Next(&tag, &ok);
			std::cout << "MUCH\n";
			static_cast<RPCRequestHandler<libmumbot::TextMessageRequest,libmumbot::TextMessage> *>(tag)->Proceed(); //worst ever
		}

	}
	void MumBotConnectionMgr::startRPCSerice() {
		grpc::ServerBuilder builder;
		builder.AddListeningPort("0.0.0.0:50080",grpc::InsecureServerCredentials());
		MumBotRPC::AsyncService asyncser;
		builder.RegisterService(&asyncser);

		auto tmCq = builder.AddCompletionQueue(); //text message completion queue

		auto cq = builder.AddCompletionQueue();
		bool ok = false;
		void *tag;
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

		RPCRequestHandler<libmumbot::TextMessage,libmumbot::TextMessageResponse> *rpcRequestHandler = new RPCRequestHandler<libmumbot::TextMessage,libmumbot::TextMessageResponse>(cq.get(),&asyncser);

		for (;;) { //put this in its own thread, one CQ for each RPC type
			cq->Next(&tag, &ok);
			std::cout << "WOW\n";
			//if (ok) {
				static_cast<RPCRequestHandler<libmumbot::TextMessage,libmumbot::TextMessageResponse> *>(tag)->Proceed(); //worst ever
			//}
		}

		//cq shutdown

		//server->Wait();

	}

	grpc::Status MumBotConnectionMgr::Say(::grpc::ServerContext* context, const ::libmumbot::TextMessage* request, ::libmumbot::TextMessageResponse* response) {
		std::cout << "Say was called\n";
		MumbleProto::TextMessage mtxt;
		mtxt.set_message(request->msg());
		MumbleProto::UserState state = mumState_->getUserState(myMumbleSessionId_);
		mtxt.add_channel_id(state.channel_id()); //send to my channel
		sendData(PKT_TYPE_TEXTMESSAGE, mtxt.SerializeAsString());
		return grpc::Status::OK;
	}



	void MumBotConnectionMgr::startClient(string host, string port, string nickname) {

	  gnutls_global_init();
	  gnutls_anon_client_credentials_t anoncred;
	  gnutls_certificate_credentials_t xcred;
	  int ret,ii;


	  gnutls_anon_allocate_client_credentials(&anoncred);
	  gnutls_certificate_allocate_credentials(&xcred);



	  gnutls_init(&gnutls_session_, GNUTLS_CLIENT);
	  gnutls_session_set_ptr(gnutls_session_, (void *) host.c_str());
	  gnutls_server_name_set(gnutls_session_,GNUTLS_NAME_DNS,host.c_str(),host.length());
	  gnutls_priority_set_direct(gnutls_session_,"NORMAL",NULL);
	  //gnutls_credentials_set(gnutls_session_,GNUTLS_CRD_ANON,anoncred);
	  gnutls_credentials_set(gnutls_session_, GNUTLS_CRD_CERTIFICATE, xcred);

	  struct addrinfo hints, *res;
	  memset(&hints,0,sizeof hints);
	  hints.ai_family = AF_UNSPEC;
	  hints.ai_socktype = SOCK_STREAM;
	  getaddrinfo(host.c_str(),port.c_str(),&hints, &res);
	  mumCmd_socketFD_  = socket(res->ai_family, res->ai_socktype, res->ai_protocol); //TODO disable Nagle's algorithm??
	  connect(mumCmd_socketFD_,res->ai_addr,res->ai_addrlen);
	  //FD_SET(mumCmd_socketFD_, &socketSet_);
	  //mum_maxSocketFD_ = mumCmd_socketFD_;


	  gnutls_transport_set_int(gnutls_session_, mumCmd_socketFD_);
	  //gnutls_certificate_set_verify_function(xcred, cert_verify_callback);

	  gnutls_handshake_set_timeout(gnutls_session_,GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);
	  ret = gnutls_handshake(gnutls_session_);

	  if (ret < 0) {
	    std::cout << gnutls_strerror(ret);
	  }

	  sendData(PKT_TYPE_VERSION,createVersionPktData());
	  sendData(PKT_TYPE_AUTH,createAuthPktData(nickname));

	  //sendData(PKT_TYPE_USERSTATE,createDeafMutePktData());


	  clientLoopThread_ = std::thread(&MumBotConnectionMgr::clientLoop,this);
	  clientKeepAliveThread_ = std::thread(&MumBotConnectionMgr::clientKeepAlive,this);
	  startRPCSerice(); //will wait
	  for(;;) { //can do stuff here
	    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	  }





	}

}
