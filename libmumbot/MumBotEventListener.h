#pragma once
#include "proto/Mumble.pb.h"
namespace libmumbot {
    class MumBotEventListener {
      public:
    		virtual void onConnect() = 0;
        virtual void recvACL(MumbleProto::ACL msg) = 0;
        virtual void recvChanACL(MumbleProto::ACL_ChanACL) = 0;
        virtual void recvACLChanGroup(MumbleProto::ACL_ChanGroup msg) = 0;
        virtual void recvAuthenticate(MumbleProto::Authenticate msg) = 0;
        virtual void recvBanList (MumbleProto::BanList msg) = 0;
        virtual void recvBanlistBanEntry (MumbleProto::BanList_BanEntry msg) = 0;
        virtual void recvChannelRemove  (MumbleProto::ChannelRemove msg) = 0;
        virtual void recvChannelState (MumbleProto::ChannelState msg) = 0;
        virtual void recvCodecVersion (MumbleProto::CodecVersion msg) = 0;
        virtual void recvContextAction (MumbleProto::ContextAction msg) = 0;
        virtual void recvContextActionModify (MumbleProto::ContextActionModify msg) = 0;
        virtual void recvCryptSetup (MumbleProto::CryptSetup msg) = 0;
        virtual void recvPermissionDenied (MumbleProto::PermissionDenied msg) = 0;
        virtual void recvPermissionQuery (MumbleProto::PermissionQuery msg) = 0;
        virtual void recvPing (MumbleProto::Ping msg) = 0;
        virtual void recvQueryUsers (MumbleProto::QueryUsers msg) = 0;
        virtual void recvReject (MumbleProto::Reject msg) = 0;
        virtual void recvRequestBlob (MumbleProto::RequestBlob msg) = 0;
        virtual void recvServerConfig (MumbleProto::ServerConfig msg) = 0;
        virtual void recvServerSync (MumbleProto::ServerSync msg) = 0;
        virtual void recvSuggestConfig (MumbleProto::SuggestConfig msg) = 0;
        virtual void recvTextMessage (MumbleProto::TextMessage msg) = 0;
        virtual void recvUDPTunnel (MumbleProto::UDPTunnel msg) = 0;
        virtual void recvUserList (MumbleProto::UserList msg) = 0;
        virtual void recvUserlistUser (MumbleProto::UserList_User msg) = 0;
        virtual void recvUserRemove (MumbleProto::UserRemove msg) = 0;
        virtual void recvUserState (MumbleProto::UserState msg) = 0;
        virtual void recvUserStats (MumbleProto::UserStats msg) = 0;
        virtual void recvUserStatsStats (MumbleProto::UserStats_Stats msg) = 0;
        virtual void recvVersion (MumbleProto::Version msg) = 0;
        virtual void recvVoiceTarget (MumbleProto::VoiceTarget msg) = 0;
        virtual void recvVoiceTargetTarget (MumbleProto::VoiceTarget_Target msg) = 0;
    };
}
