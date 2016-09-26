#include "libmumbot/libmumbot.h"
#include "libmumbot/opus/OpusOggOutputWriter.h"

uint64_t readNextVint(std::string &data, uint32_t pos, uint32_t *finishpos);


class ScriptyMumBot : public libmumbot::MumBotEventListener {
	public:
		void onConnect();
		void recvACL(MumbleProto::ACL msg);
		void recvChanACL(MumbleProto::ACL_ChanACL);
		void recvACLChanGroup(MumbleProto::ACL_ChanGroup msg);
		void recvAuthenticate(MumbleProto::Authenticate msg);
		void recvBanList (MumbleProto::BanList msg);
		void recvBanlistBanEntry (MumbleProto::BanList_BanEntry msg);
		void recvChannelRemove  (MumbleProto::ChannelRemove msg);
		void recvChannelState (MumbleProto::ChannelState msg);
		void recvCodecVersion (MumbleProto::CodecVersion msg);
		void recvContextAction (MumbleProto::ContextAction msg);
		void recvContextActionModify (MumbleProto::ContextActionModify msg);
		void recvCryptSetup (MumbleProto::CryptSetup msg);
		void recvPermissionDenied (MumbleProto::PermissionDenied msg);
		void recvPermissionQuery (MumbleProto::PermissionQuery msg);
		void recvPing (MumbleProto::Ping msg);
		void recvQueryUsers (MumbleProto::QueryUsers msg);
		void recvReject (MumbleProto::Reject msg);
		void recvRequestBlob (MumbleProto::RequestBlob msg);
		void recvServerConfig (MumbleProto::ServerConfig msg);
		void recvServerSync (MumbleProto::ServerSync msg);
		void recvSuggestConfig (MumbleProto::SuggestConfig msg);
		void recvTextMessage (MumbleProto::TextMessage msg);
		void recvUDPTunnel (std::string msg);
		void recvUserList (MumbleProto::UserList msg);
		void recvUserlistUser (MumbleProto::UserList_User msg);
		void recvUserRemove (MumbleProto::UserRemove msg);
		void recvUserState (MumbleProto::UserState msg);
		void recvUserStats (MumbleProto::UserStats msg);
		void recvUserStatsStats (MumbleProto::UserStats_Stats msg);
		void recvVersion (MumbleProto::Version msg);
		void recvVoiceTarget (MumbleProto::VoiceTarget msg);
		void recvVoiceTargetTarget (MumbleProto::VoiceTarget_Target msg);
};
