#pragma once

#include <opus.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include "AudioInputWriterListener.h"

namespace libmumbot {
	class AudioInputOpusWriter {
	public:
		static constexpr short AUDIO_INPUT_SRC_FIFO = 0;
		AudioInputOpusWriter(short srctype, std::string param, AudioInputWriterListener *listener);

	private:

		void readAudioLoop();

		std::thread readAudioThread_;
		std::ifstream audioPipe_;
		AudioInputWriterListener *listener_;
		short srctype_;
		OpusEncoder *opusEncoder_;
		static constexpr int OPUSBUFFERSIZE_ = 1020;
		static constexpr int SAMPLECOUNT_ = 48000 * 8 * 1 / 8 / 100;

		uint8_t buffer_[SAMPLECOUNT_ * 2]; //16bit audio

		uint8_t opusBuffer_[OPUSBUFFERSIZE_]; //size per mumble docs
	};

}
