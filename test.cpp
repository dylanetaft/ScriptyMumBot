#include "test.h"
#include <iostream>
#include <map>
#include <fstream>

std::map <uint32_t,libmumbot::OpusOggOutputWriter *> filemap;




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
void ScriptyMumBot::recvServerSync (MumbleProto::ServerSync msg) {
    opusWriter_ = std::make_shared<libmumbot::AudioInputOpusWriter> (libmumbot::AudioInputOpusWriter::AUDIO_INPUT_SRC_FIFO,"./mpd/mpd.fifo",this);

}
void ScriptyMumBot::recvSuggestConfig (MumbleProto::SuggestConfig msg) {}
void ScriptyMumBot::recvTextMessage (MumbleProto::TextMessage msg) {}

void ScriptyMumBot::onAudioEncodedDataReady(uint8_t *data, uint32_t len) {
    std::cout << "Audio data recv:" << len << "\n";

}

void ScriptyMumBot::recvUDPTunnel (std::string msg) {
    std::cout << "TCP TUNNEL\n";
    std::cout << msg.length() << "\n";
    uint8_t apkt_type = msg[0] & 0b11100000;
    uint8_t apkt_target = msg[0] & 0b00011111;
    uint32_t pos = 1;
    std::cout << "apkt_type:" << (int)apkt_type << " apkt_target:" << (int)apkt_target << "\n";
    std::cout << "apkt_pos:" << pos << "\n";
    uint64_t apkt_session_id = readNextVint(msg,pos,&pos);
    uint64_t apkt_seq_num = readNextVint(msg,pos,&pos);

    if (apkt_type == 0b10000000) { //opus
		uint64_t apkt_opus_len = readNextVint(msg,pos,&pos);
        apkt_opus_len = apkt_opus_len & 0b111111111111 ; //max voice packet size is 8192, 14th bit marks end of transmission
        std::cout << "OPUS len:" << apkt_opus_len << " apkt_session_id:" << apkt_session_id << " apkt_seq_num:" << apkt_seq_num << "\n";
        if (filemap.count(apkt_session_id) == 0) { //new session
            //std::ofstream *f = new std::ofstream;
            std::string filename = std::to_string(apkt_session_id) + ".opus";
            libmumbot::OpusOggOutputWriter *ow = new libmumbot::OpusOggOutputWriter(filename);
            filemap[apkt_session_id] = ow;
        }
        libmumbot::OpusOggOutputWriter *f = filemap[apkt_session_id];
        const char *data = msg.c_str();
        f->writePacket((uint8_t *)data + pos,apkt_opus_len);
    }

}
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

uint64_t readNextVint(std::string &data, uint32_t pos, uint32_t *finishpos) {
    //todo deal with endianness
    if (pos > data.length()) {
        std::cout << "Error reading varint\n";
        return -1;
    }

    uint8_t vint_type = data[pos];
    uint64_t vint = 0;
    if (vint_type >= 0b11111100) { //byte inverted negative two bit number
        D(std::cout << "byte inverted negative 2 bit varint read: " << vint << "\n");
    }
    else if (vint_type >= 0b11111000) { //negative recursive varint
        D(std::cout << "negative recursive varint read: " << vint << "\n");
    }
    else if (vint_type >= 0b11110100) { //64 bit positive number
        uint64_t d[8] = {0,0,0,0,0,0,0,0};
        d[0] = (uint8_t)data[pos + 1];
        d[1] = (uint8_t)data[pos + 2];
        d[2] = (uint8_t)data[pos + 3];
        d[3] = (uint8_t)data[pos + 4];
        d[4] = (uint8_t)data[pos + 5];
        d[5] = (uint8_t)data[pos + 6];
        d[6] = (uint8_t)data[pos + 7];
        d[7] = (uint8_t)data[pos + 8];
        vint = (d[0] << 56) + (d[1] << 48) + (d[2] << 40) + (d[3] << 32) + (d[4] << 24) + (d[5] << 16) + (d[6] << 8) + (d[7]);
        *finishpos = pos + 9;
		D(std::cout << "64 bit varint read: " << vint << "\n");
    }
    else if (vint_type >= 0b11110000) { //32 bit positive number
        uint64_t d[4] = {0,0,0,0};
        d[0] = (uint8_t)data[pos + 1];
        d[1] = (uint8_t)data[pos + 2];
        d[2] = (uint8_t)data[pos + 3];
        d[3] = (uint8_t)data[pos + 4];
        vint = (d[0] << 24) + (d[1] << 16) + (d[2] << 8) + (d[3]);
        *finishpos = pos + 5;
		D(std::cout << "32 bit varint read: " << vint << "\n");
    }
    else if (vint_type >= 0b11100000) { //28 bit positive number
        uint64_t d[4] = {0,0,0,0};
        d[0] = ((uint8_t)data[pos] & 0b00001111); //mask off taken bits
        d[1] = (uint8_t)data[pos + 1];
        d[2] = (uint8_t)data[pos + 2];
        d[3] = (uint8_t)data[pos + 3];
        vint = (d[0] << 24) + (d[1] << 16) + (d[2] << 8) + (d[3]);
        *finishpos = pos + 4;
		D(std::cout << "28 bit varint read: " << vint << "\n");
    }
    else if (vint_type >= 0b11000000) { //21 bit positive number
        uint64_t d[3] = {0,0,0};
        d[0] = ((uint8_t)data[pos] & 0b00011111); //mask off taken bits
        d[1] = (uint8_t)data[pos + 1];
        d[2] = (uint8_t)data[pos + 2];
        vint = (d[0] << 16) + (d[1] << 8) + (d[2]);
        *finishpos = pos + 3;
		D(std::cout << "21 bit varint read: " << vint << "\n");
    }
    else if (vint_type >= 0b10000000) { //14 bit positive number
        uint64_t d[2] = {0,0};
        d[0] = ((uint8_t)data[pos] & 0b00111111); //mask off taken bits
        d[1] = (uint8_t)data[pos + 1];
        vint = (d[0] << 8) + (d[1]);
        *finishpos = pos + 2;
		D(std::cout << "14 bit varint read: " << vint << "\n");
    }
    else { //7 bit positive number
        uint64_t d[1] = {0};
        d[0] = ((uint8_t)data[pos] & 0b01111111); //mask off taken bits
        vint = d[0];
        *finishpos = pos + 1;
		D(std::cout << "7 bit varint read: " << vint << "\n");
    }

    return vint;

}

int main() {

  ScriptyMumBot bot;
  libmumbot::MumBotState state;
  libmumbot::MumBotConnectionMgr mgr;
  mgr.setListener(&bot);
  //mgr.setAudioInputSource(libmumbot::MumBotAudioInput::INPUTTYPE_FIFO,".\\mpd\\mpd.fifo"); //where the hell should this live?
  mgr.setStateObject(&state);
  mgr.startClient("cookwithkevin.com","64738");
  //ShutdownSSL();

  return 0;
}
