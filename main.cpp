#include "main.h"
#include <iostream>
#include <map>
#include <fstream>
#include "libmumbot/libmumbot.h"
//#include <string.h>
#include <stdio.h>

using namespace std;
//youtube-dl -o './mpd/music/youtube-dl/%(title)s.%(ext)s' --audio-quality 1 -x 'https://www.youtube.com/watch?v=4Tr0otuiQuU'

std::map <uint32_t,libmumbot::OpusOggOutputWriter *> filemap;


void ScriptyMumBot::start() {
    connMgr_.setListener(this);
    connMgr_.setStateObject(&mumState_);
    botConfig_.loadConfig("test.ini");
	string nickname = botConfig_.getSectionSetting("general","nickname");
	if (nickname == "") nickname = "defaultalias";
    connMgr_.startClient(botConfig_.getSectionSetting("general","server"),botConfig_.getSectionSetting("general","port"), nickname);
}

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
void ScriptyMumBot::recvTextMessage (MumbleProto::TextMessage msg) {
    std::string textMsg = msg.message();
    auto cmds = botConfig_.getScriptsForInput(textMsg);

    for (auto cmd:cmds) { //call script in ini file
        std::cout << cmd << "\n";
        FILE *fp = popen(cmd.c_str(),"w");
        if (fp != NULL) {
            std::fputs(textMsg.c_str(), fp);
            pclose(fp);
        }
        //execl("ls","./");
    }
}

void ScriptyMumBot::onAudioEncodedDataReady(uint8_t *data, uint16_t len) {
    std::string msg;

    uint8_t apkt_header = libmumbot::MumBotConnectionMgr::APKT_TYPE_OPUS;
    apkt_header += 0; //talking to channel, normal talking
    uint32_t apkt_posinfo[3] = {0,0,0};
    //uint16_t len_and_term = len | 0b1000000000000000;
    msg = msg + (char)apkt_header;
    msg = msg + getVint(audioSequence_);
    msg = msg + getVint(len);
    msg.append((char *)data,len);
    msg.append((char *)apkt_posinfo,sizeof(apkt_posinfo));
    connMgr_.sendUDPTunnelAudioData(msg);
    audioSequence_++;
}

void ScriptyMumBot::recvUDPTunnel (std::string msg) {
    //CODE TO RECORD OGG FILES, WORKING
    /*
    std::cout << "TCP TUNNEL\n";
    std::cout << msg.length() << "\n";
    uint8_t apkt_type = msg[0] & 0b11100000;
    uint8_t apkt_target = msg[0] & 0b00011111;
    uint32_t pos = 1;
    std::cout << "apkt_type:" << (int)apkt_type << " apkt_target:" << (int)apkt_target << "\n";
    std::cout << "apkt_pos:" << pos << "\n";
    uint64_t apkt_session_id = readNextVint(msg,pos,&pos);
    uint64_t apkt_seq_num = readNextVint(msg,pos,&pos);

    if (apkt_type == libmumbot::MumBotConnectionMgr::APKT_TYPE_OPUS) { //opus
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
    */

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


int main() {

  ScriptyMumBot bot;
  bot.start();
  //ShutdownSSL();

  return 0;
}
