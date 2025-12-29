#ifndef MP3_DECODER_H
#define MP3_DECODER_H

#include <Arduino.h>
#include <MP3Decoder.h>

class Mp3Decoder {
public:
	Mp3Decoder(){}
	~Mp3Decoder(){}

	/**
	 * Decode MP3 audio data to PCM
	 * @param audioData Raw MP3 data
	 * @param audioSize Size of MP3 data in bytes
	 * @param pcmBuffer Output buffer for PCM data (allocated by this function)
	 * @param pcmSize Output number of PCM samples
	 * @param sampleRate Output sample rate of decoded audio
	 * @return true if decoding successful
	 */
	inline bool decodeMP3ToPCM(const uint8_t* audioData, size_t audioSize,
									  int16_t** pcmBuffer, size_t* pcmSize, int* sampleRate) {
		if (!audioData || audioSize == 0 || !pcmBuffer || !pcmSize || !sampleRate) {
			ESP_LOGE("MP3Processor", "Invalid parameters");
			return false;
		}

		ESP_LOGI("MP3Processor", "Decoding MP3: %d bytes", audioSize);

		// Initialize MP3 decoder if needed
		if (!mp3Decoder.init()) {
			ESP_LOGE("MP3Processor", "Failed to initialize MP3 decoder");
			return false;
		}

		// Decode MP3 to PCM
		int16_t* decodedBuffer = nullptr;
		size_t decodedSize = 0;
		MP3Decoder::MP3Info mp3Info;

		if (!mp3Decoder.decodeData(audioData, audioSize, &decodedBuffer, &decodedSize, &mp3Info)) {
			ESP_LOGE("MP3Processor", "Failed to decode MP3 data");
			return false;
		}

		ESP_LOGI("MP3Processor", "Decoded MP3: %d samples, %d Hz, %d channels",
				decodedSize, mp3Info.sampleRate, mp3Info.channels);

		// Convert to mono if needed
		if (mp3Info.channels != 1) {
			ESP_LOGW("MP3Processor", "Converting %d channels to mono", mp3Info.channels);
			convertStereoToMono(decodedBuffer, &decodedSize);
		}

		// Check if we need to resample
		int16_t* finalBuffer = decodedBuffer;
		size_t finalSize = decodedSize;
		int finalSampleRate = mp3Info.sampleRate;

		if (mp3Info.sampleRate != 16000) {
			// Resample to 16kHz
			if (!resampleTo16kHz(decodedBuffer, decodedSize, mp3Info.sampleRate, &finalBuffer, &finalSize)) {
				ESP_LOGW("MP3Processor", "Resampling failed, using original buffer");
				finalBuffer = decodedBuffer;
				finalSize = decodedSize;
				finalSampleRate = mp3Info.sampleRate;
			} else {
				finalSampleRate = 16000;
				// Free original buffer if resampling created a new one
				if (finalBuffer != decodedBuffer) {
					mp3Decoder.freePCMBuffer(decodedBuffer);
				}
			}
		}

		*pcmBuffer = finalBuffer;
		*pcmSize = finalSize;
		*sampleRate = finalSampleRate;

		ESP_LOGI("MP3Processor", "Final output: %d samples at %d Hz", finalSize, finalSampleRate);
		return true;
	}

	/**
	 * Free PCM buffer allocated by decodeMP3ToPCM
	 * @param buffer Buffer to free
	 */
	inline void freePCMBuffer(int16_t* buffer) {
		if (buffer) {
			mp3Decoder.freePCMBuffer(buffer);
		}
	}

private:
	MP3Decoder mp3Decoder;

	/**
	 * Resample audio from current sample rate to 16kHz
	 * @param pcmBuffer Input PCM buffer
	 * @param pcmSize Number of samples in input buffer
	 * @param currentSampleRate Current sample rate of input
	 * @param resampledBuffer Output buffer (allocated by this function)
	 * @param resampledSize Output number of samples
	 * @return true if resampling successful
	 */
	inline bool resampleTo16kHz(const int16_t* pcmBuffer, size_t pcmSize, int currentSampleRate,
									   int16_t** resampledBuffer, size_t* resampledSize) {
		// Calculate target samples
		size_t targetSamples = (pcmSize * 16000LL) / currentSampleRate;  // Use 64-bit to avoid overflow
		ESP_LOGI("MP3Processor", "Resampling from %d Hz to 16000 Hz: %d -> %d samples",
				currentSampleRate, pcmSize, targetSamples);

		// Allocate resampled buffer
		int16_t* buffer = (int16_t*)heap_caps_malloc(targetSamples * sizeof(int16_t), MALLOC_CAP_SPIRAM);
		if (!buffer) {
			ESP_LOGE("MP3Processor", "Failed to allocate resample buffer");
			return false;
		}

		// Linear interpolation resampling
		for (size_t i = 0; i < targetSamples; i++) {
			float srcPos = (float)i * (float)currentSampleRate / 16000.0f;
			size_t srcIndex = (size_t)srcPos;
			float fraction = srcPos - srcIndex;

			if (srcIndex + 1 < pcmSize) {
				// Linear interpolation between two samples
				int16_t sample1 = pcmBuffer[srcIndex];
				int16_t sample2 = pcmBuffer[srcIndex + 1];
				buffer[i] = (int16_t)(sample1 + fraction * (sample2 - sample1));
			} else if (srcIndex < pcmSize) {
				// Last sample
				buffer[i] = pcmBuffer[srcIndex];
			} else {
				// Should not happen
				buffer[i] = 0;
			}
		}

		*resampledBuffer = buffer;
		*resampledSize = targetSamples;

		ESP_LOGI("MP3Processor", "Resampling complete: %d samples at 16000 Hz", targetSamples);
		return true;
	}

	/**
	 * Convert stereo audio to mono
	 * @param buffer Input/output buffer (modified in place)
	 * @param size Number of samples (will be halved for stereo->mono)
	 */
	inline void convertStereoToMono(int16_t* buffer, size_t* size) {
		if (!buffer || *size < 2) return;

		// Convert stereo to mono by averaging channels
		for (size_t i = 0; i < *size / 2; i++) {
			buffer[i] = (buffer[i * 2] + buffer[i * 2 + 1]) / 2;
		}
		*size = *size / 2;
		ESP_LOGI("MP3Processor", "Converted stereo to mono: %d samples", *size);
	}
};

extern Mp3Decoder mp3decoder;

#endif // MP3_DECODER_H
