#include "test.h"
#include <iostream>


void ScriptyMumBot::onConnect() {}
void ScriptyMumBot::recvACL(MumbleProto::ACL msg) {}
void ScriptyMumBot::recvChanACL(MumbleProto::ACL_ChanACL) {}
void ScriptyMumBot::recvACLChanGroup(MumbleProto::ACL_ChanGroup msg) {}
void ScriptyMumBot::recvAuthenticate(MumbleProto::Authenticate msg) {}
void ScriptyMumBot::recvBanList (MumbleProto::BanList msg) {}
void ScriptyMumBot::recvBanlistBanEntry (MumbleProto::BanList_BanEntry msg) {}
void ScriptyMumBot::recvChannelRemove  (MumbleProto::ChannelRemove msg) {}
void ScriptyMumBot::recvChannelState (MumbleProto::ChannelState msg) {}
void ScriptyMumBot::recvCodecVersion (MumbleProto::CodecVersion msg) {}
void ScriptyMumBot::recvContextAction (MumbleProto::ContextAction msg) {}
void ScriptyMumBot::recvContextActionModify (MumbleProto::ContextActionModify msg) {}
void ScriptyMumBot::recvCryptSetup (MumbleProto::CryptSetup msg) {}
void ScriptyMumBot::recvPermissionDenied (MumbleProto::PermissionDenied msg) {}
void ScriptyMumBot::recvPermissionQuery (MumbleProto::PermissionQuery msg) {}
void ScriptyMumBot::recvPing (MumbleProto::Ping msg) {}
void ScriptyMumBot::recvQueryUsers (MumbleProto::QueryUsers msg) {}
void ScriptyMumBot::recvReject (MumbleProto::Reject msg) {}
void ScriptyMumBot::recvRequestBlob (MumbleProto::RequestBlob msg) {}
void ScriptyMumBot::recvServerConfig (MumbleProto::ServerConfig msg) {}
void ScriptyMumBot::recvServerSync (MumbleProto::ServerSync msg) {}
void ScriptyMumBot::recvSuggestConfig (MumbleProto::SuggestConfig msg) {}
void ScriptyMumBot::recvTextMessage (MumbleProto::TextMessage msg) {}
void ScriptyMumBot::recvUDPTunnel (MumbleProto::UDPTunnel msg) {}
void ScriptyMumBot::recvUserList (MumbleProto::UserList msg) {}
void ScriptyMumBot::recvUserlistUser (MumbleProto::UserList_User msg) {}
void ScriptyMumBot::recvUserRemove (MumbleProto::UserRemove msg) {}
void ScriptyMumBot::recvUserState (MumbleProto::UserState msg) {}
void ScriptyMumBot::recvUserStats (MumbleProto::UserStats msg) {}
void ScriptyMumBot::recvUserStatsStats (MumbleProto::UserStats_Stats msg) {}
void ScriptyMumBot::recvVersion (MumbleProto::Version msg) {
    std::cout << "Version packet " << msg.release() << "\n";
}
void ScriptyMumBot::recvVoiceTarget (MumbleProto::VoiceTarget msg) {}
void ScriptyMumBot::recvVoiceTargetTarget (MumbleProto::VoiceTarget_Target msg) {}


int main() {
  ScriptyMumBot bot;
  libmumbot::MumBotConnectionMgr mgr;
  mgr.setListener(&bot);
  mgr.startClient("cookwithkevin.com","64738");
  //ShutdownSSL();

  return 0;
}
