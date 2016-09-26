#pragma once
#include <opus.h>
#include <ogg/ogg.h>
#include <iostream>
#include <fstream>

namespace libmumbot {
	void le32(unsigned char *p,int v);
	void le16(unsigned char *p, int v);
	void be32(unsigned char *p, int v);
	void be16(unsigned char *p, int v);
	class OpusOggOutputWriter {
	public:
		OpusOggOutputWriter(std::string filename);
		void writePacket(uint8_t *data, uint32_t len);
	private:
		ogg_stream_state streamState_;
		std::ofstream fileout_;
		int seq_; //packet sequence #
		ogg_int64_t granulepos_;
		ogg_packet *genOpusHead();
		ogg_packet *genOpusTags();
		int opus_samples(const unsigned char *packet, int size);
		void op_free(ogg_packet *op);
		int flush();
	};


}
