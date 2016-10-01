#include "OpusOggOutputWriter.h"

namespace libmumbot {

	/* helper, write a little-endian 32 bit int to memory */
	void le32(unsigned char *p, int v)
	{
	  p[0] = v & 0xff;
	  p[1] = (v >> 8) & 0xff;
	  p[2] = (v >> 16) & 0xff;
	  p[3] = (v >> 24) & 0xff;
	}

	/* helper, write a little-endian 16 bit int to memory */
	void le16(unsigned char *p, int v)
	{
	  p[0] = v & 0xff;
	  p[1] = (v >> 8) & 0xff;
	}

	/* helper, write a big-endian 32 bit int to memory */
	void be32(unsigned char *p, int v)
	{
	  p[0] = (v >> 24) & 0xff;
	  p[1] = (v >> 16) & 0xff;
	  p[2] = (v >> 8) & 0xff;
	  p[3] = v & 0xff;
	}

	/* helper, write a big-endian 16 bit int to memory */
	void be16(unsigned char *p, int v)
	{
	  p[0] = (v >> 8) & 0xff;
	  p[1] = v & 0xff;
	}

	//Constructor
	OpusOggOutputWriter::OpusOggOutputWriter(std::string filename) {
		seq_ = 0;
		granulepos_ = 0;
		ogg_packet *op;
		fileout_.open(filename,std::ofstream::binary);
		ogg_stream_init(&streamState_,0);
		op = genOpusHead();
		ogg_stream_packetin(&streamState_,op);
		seq_++; //first packet in ogg stream
		op_free(op);
		op = genOpusTags();
		ogg_stream_packetin(&streamState_,op);
		seq_++; //second packet in ogg stream
		op_free(op);
		flush();
	}

	void OpusOggOutputWriter::writePacket(uint8_t *data, uint32_t len) {
		ogg_packet *pkt = (ogg_packet *)malloc(sizeof(ogg_packet));
		pkt->packet = data;
		pkt->bytes = len;
		pkt->b_o_s = 0;
		pkt->e_o_s = 0;
		seq_++; //should this start from 0 or 1? FIXME
		granulepos_ += opus_samples(data,len);
		pkt->packetno = seq_;
		pkt->granulepos = granulepos_;
		ogg_stream_packetin(&streamState_,pkt);
		free(pkt);
		flush();
	}


	int OpusOggOutputWriter::opus_samples(const unsigned char *packet, int size) {
		int samples = opus_packet_get_samples_per_frame(packet, 48000);
		int frames = opus_packet_get_nb_frames(packet, size);
		return samples * frames;
	}

	int OpusOggOutputWriter::flush() {
		ogg_page page;
		if (!fileout_.is_open()) return -1;
		while (ogg_stream_flush(&streamState_,&page)) { //need a separate write function TODO
			fileout_.write((const char *)page.header,page.header_len);
			fileout_.write((const char *)page.body,page.body_len);
		}
		return 0;
	}
	/* manufacture a generic OpusHead packet */
	ogg_packet *OpusOggOutputWriter::genOpusHead() {
		int s = 19;
		uint8_t *data = (uint8_t *)malloc(s);
		ogg_packet *pkt = (ogg_packet *)malloc(sizeof(ogg_packet));
		std::memcpy(data,"OpusHead",8);
		data[8] = 1; //version
		data[9] = 2; //channels
		le16(data +10,0); //pre-skip
		le32(data + 12,48000); //sample recvUserState
		le16(data + 16,0); //gain
		data[18] = 0; //channel mapping family
		pkt->packet = data;
		pkt->bytes = s;
		pkt->b_o_s = 1;
		pkt->e_o_s = 0;
		pkt->granulepos = 0;
		pkt->packetno = 0;
		return pkt;
	}

	/* manufacture a generic OpusTags packet */
	ogg_packet *OpusOggOutputWriter::genOpusTags()
	{
		char identifier[] = "OpusTags";
		char vendor[] = "opus packet dump";
		int size = strlen(identifier) + 4 + strlen(vendor) + 4;
		unsigned char *data = (unsigned char *) malloc(size);
		ogg_packet *pkt = (ogg_packet *)malloc(sizeof(ogg_packet));

		std::memcpy(data, identifier, 8);
		le32(data + 8, strlen(vendor));
		std::memcpy(data + 12, vendor, strlen(vendor));
		le32(data + 12 + strlen(vendor), 0);

		pkt->packet = data;
		pkt->bytes = size;
		pkt->b_o_s = 0;
		pkt->e_o_s = 0;
		pkt->granulepos = 0;
		pkt->packetno = 1;

		return pkt;
	}


	void OpusOggOutputWriter::op_free(ogg_packet *op) {
		if (op) {
			if (op->packet) {
				free(op->packet);
			}
			free(op);
		}
	}

}
