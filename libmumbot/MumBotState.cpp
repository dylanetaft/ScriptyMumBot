#include "MumBotState.h"
#include <stdexcept>
#include <iostream>


namespace libmumbot {

	MumbleProto::UserState MumBotState::getUserState(uint32_t id) {
		try {
			return userStates_.at(id);
		}
		catch (const std::exception& e) {
			throw std::out_of_range("MumBotState::getUserState no matching user id found " + std::to_string(id));
		}
	}
	void MumBotState::updateChannelState(MumbleProto::ChannelState msg, bool del) {
		uint32_t id = msg.channel_id();

		if (channelStates_.count(id) == 0) { //new user state
			channelStates_[id] = msg;
		}
		else if (!del) { //user state update
			MumbleProto::ChannelState cstate = channelStates_[id];
			cstate.MergeFrom(msg);
			channelStates_[id] = cstate;

		}
		else {
			channelStates_.erase(id);
		}

	}
	void MumBotState::updateUserState(MumbleProto::UserState msg, bool del) {
		uint32_t id = msg.user_id();

		if (userStates_.count(id) == 0) { //new user state
			userStates_[id] = msg;
		}
		else if (!del) { //user state update
			MumbleProto::UserState cstate = userStates_[id];
			cstate.MergeFrom(msg);
			userStates_[id] = cstate;

		}
		else {
			userStates_.erase(id);
		}

		std::cout << userStates_[id].name() << " " <<userStates_[id].session() << "\n";
	}

}
