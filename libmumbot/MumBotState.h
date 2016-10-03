#pragma once
#include <map>
#include "proto/Mumble.pb.h"


namespace libmumbot {
	class MumBotState {
	public:
		void updateChannelState(MumbleProto::ChannelState msg, bool del = 0);
		void updateUserState(MumbleProto::UserState msg, bool del = 0);
		MumbleProto::UserState getUserState(uint32_t id);

	private:
		std::map<uint32_t, MumbleProto::UserState> userStates_;
		std::map<uint32_t, MumbleProto::ChannelState> channelStates_;
	};

}
