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
		static const short AUDIO_INPUT_SRC_FIFO = 0;

		AudioInputOpusWriter(short srctype, std::string param, AudioInputWriterListener *listener);

	private:
		std::thread readAudioThread_;
		AudioInputWriterListener *listener_;
	};

}
