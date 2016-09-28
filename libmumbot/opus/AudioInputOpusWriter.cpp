#include "AudioInputOpusWriter.h"

namespace libmumbot {



	AudioInputOpusWriter::AudioInputOpusWriter(short srctype, std::string param, AudioInputWriterListener *listener) {
		listener_ = listener;
		srctype_ = srctype;
		int errors;
		opusEncoder_ = opus_encoder_create(48000,1,OPUS_APPLICATION_VOIP, &errors);
		opus_encoder_ctl(opusEncoder_,OPUS_SET_BITRATE(96000));

		if (srctype_ == AUDIO_INPUT_SRC_FIFO) { //fifo buffer
			audioPipe_.open(param,std::ifstream::in);
			readAudioThread_ = std::thread(&AudioInputOpusWriter::readAudioLoop, this);

		}

	}

	void AudioInputOpusWriter::readAudioLoop() {
		int len = 0;
		for (;;) {
			if (srctype_ == AUDIO_INPUT_SRC_FIFO) {
				audioPipe_.read((char *)buffer_,sizeof(buffer_));
				len = opus_encode(opusEncoder_,(opus_int16 *)buffer_,SAMPLECOUNT_,opusBuffer_,OPUSBUFFERSIZE_);
				listener_->onAudioEncodedDataReady(opusBuffer_,len);
			}
		}
	}

}
