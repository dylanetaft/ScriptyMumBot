#pragma once
#include <map>
#include "proto/Mumble.pb.h"
#include <mutex>

namespace libmumbot {
	class MumBotState {
	public:
		void updateChannelState(MumbleProto::ChannelState msg, bool del = 0);
		void updateUserState(MumbleProto::UserState msg, bool del = 0);
        uint32_t userNameToSession(std::string name);
		MumbleProto::UserState getUserState(uint32_t id);

	private:
		std::map<uint32_t, MumbleProto::UserState> userStates_;
		std::map<uint32_t, MumbleProto::ChannelState> channelStates_;
        std::mutex userStateMutex_;
        std::mutex channelStateMutex_;
	};

}
