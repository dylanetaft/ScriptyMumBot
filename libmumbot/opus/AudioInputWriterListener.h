#pragma once


namespace libmumbot {
	class AudioInputWriterListener {
	public:
		virtual void onAudioEncodedDataReady(uint8_t *data, uint16_t len) = 0;
	};
}
