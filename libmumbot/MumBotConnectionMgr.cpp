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



    MumBotConnectionMgr::~MumBotConnectionMgr() {
        std::cout << "Cleanup\n";
    }
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
	    uint32_t pktlen = c_headerbuffer_[2] << 24 | c_headerbuffer_[3] << 16 | c_headerbuffer_[4] << 8 | c_headerbuffer_[5]; //FIXME if server sends invalid packet length, this can break
	    std::cout << "Pkt type:" << pkttype << " Pkt len:" << pktlen << "\n";
        if (pktlen == 0) {
            std::cout << "FIXME: Server sent 0 size packet?\n";
            c_headerpos_ = 0;
            return false; //why would the server send this? FIXME
        }
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
				case PKT_TYPE_USERREMOVE: {
					MumbleProto::UserRemove rem;
					rem.ParseFromString(packet);
					MumbleProto::UserState msg;
					msg.set_session(rem.session());
					if (mumState_ != NULL) mumState_->updateUserState(msg, true);
					if (eventListener_ != NULL) eventListener_->recvUserRemove(rem);
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
				  std::cout << "Recv message" << "\n";
				  MumbleProto::TextMessage txt;
				  txt.ParseFromString(packet);


                  MumBotProto::TextMessage mbtxt;
                  mbtxt.set_msg(txt.message());
                  try {
                      mbtxt.set_fromname(mumState_->getUserState(txt.actor()).name());
                  }
                  catch (std::exception &e) {
                      //just in case the user logged out and no longer have the id->name map
                  }
				  RPCWorkQueueMgr_.pushNextTextMessage(mbtxt);

				  std::cout << "Recv message 2" << "\n";
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
		//uint8_t header = 0b10000000; //opus, normal talking
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

	void MumBotConnectionMgr::startRPCService() {
		grpc::ServerBuilder builder;
		builder.AddListeningPort("0.0.0.0:50080",grpc::InsecureServerCredentials());
		builder.RegisterService(this);
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
		server->Wait();
        std::cout << "Quitting!!!!\n";

	}

	grpc::Status MumBotConnectionMgr::SubscribeToTextMessages(::grpc::ServerContext* context, const MumBotProto::TextMessageRequest* request, ::grpc::ServerWriter<MumBotProto::TextMessage>* writer) {
		std::cout << "Calling me\n";
		uint32_t myindex = RPCWorkQueueMgr_.createTextMessageQueue();

		for (;;) {
			MumBotProto::TextMessage tmsg = RPCWorkQueueMgr_.getNextTextMessage(myindex); //will wait
			std::cout << "ever make it here\n";
			std::cout << tmsg.msg() << "\n";
			if (context->IsCancelled()) break;
			writer->Write(tmsg);
		}
		std::cout << "NEVER make it here\n";
		RPCWorkQueueMgr_.removeTextMessageQueue(myindex);
		return grpc::Status::OK;
	}
	grpc::Status MumBotConnectionMgr::Say(::grpc::ServerContext* context, const MumBotProto::TextMessage* request, MumBotProto::TextMessageResponse* response) {
		std::cout << "Say was called\n";
        MumbleProto::TextMessage mtxt;

        int tncount = request->toname_size();
        for (int i = 0; i < tncount;i++) {
            try {
                uint32_t toUserSession = mumState_->userNameToSession(request->toname(i));
                std::cout << "User session:" << toUserSession << "\n";
                mtxt.add_session(toUserSession);
            }
            catch (std::exception &e) {
                //the user logged off, just return
                return grpc::Status::OK;
            }
        }


		mtxt.set_message(request->msg());
		MumbleProto::UserState state = mumState_->getUserState(myMumbleSessionId_);
		if (tncount == 0) mtxt.add_channel_id(state.channel_id()); //send to my channel
		sendData(PKT_TYPE_TEXTMESSAGE, mtxt.SerializeAsString());
		return grpc::Status::OK;
	}



	void MumBotConnectionMgr::startClient(string host, string port, string nickname) {

	  gnutls_global_init();
	  gnutls_anon_client_credentials_t anoncred;
	  gnutls_certificate_credentials_t xcred;
	  int ret;


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
	  gnutls_certificate_set_verify_function(xcred, cert_verify_callback);

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
	  startRPCService(); //will wait
	  for(;;) { //can do stuff here
	    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	  }





	}

}
