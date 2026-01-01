#ifndef MP3_DECODER_H
#define MP3_DECODER_H

#include <Arduino.h>
#include <MP3Decoder.h>

// Include the ESP32 Helix MP3 decoder library
extern "C" {
    #include "mp3dec.h"
}

class Mp3Decoder {
private:
	// Helix MP3 decoder instance
	HMP3Decoder helixDecoder;
	int16_t* outputBuffer;
	size_t outputBufferSize;
	
	static const size_t MAX_OUTPUT_BUFFER_SIZE = 1152 * 10; // Max PCM samples per MP3 frame
	
public:
	Mp3Decoder() : helixDecoder(nullptr), outputBuffer(nullptr), outputBufferSize(0) {}
	~Mp3Decoder() {
		if (helixDecoder) {
			MP3FreeDecoder(helixDecoder);
		}
		if (outputBuffer) {
			heap_caps_free(outputBuffer);
		}
	}

	/**
	 * Initialize streaming decoder
	 * @return true if initialization successful
	 */
	inline bool init() {
		if (streamingInitialized) {
			// Already initialized, just return true
			return true;
		}
		
		if (!ensureDecoderAndBuffer()) {
			return false;
		}
		
		if (!ensureStreamBuffer()) {
			ESP_LOGE("MP3Decoder", "Failed to allocate stream buffer");
			return false;
		}
		
		streamingInitialized = true;
		streamBufferUsed = 0;
		return true;
	}

	/**
	 * Check if streaming decoder is initialized
	 * @return true if ready for streaming
	 */
	bool isInitialized() const { return streamingInitialized; }

	/**
	 * Feed MP3 data for streaming decoding
	 * @param audioData MP3 chunk data
	 * @param audioSize Size of MP3 chunk in bytes
	 * @return true if data fed successfully
	 */
	inline bool feedData(const uint8_t* audioData, size_t audioSize) {
		if (!streamingInitialized) {
			ESP_LOGE("MP3Decoder", "Streaming decoder not initialized");
			return false;
		}
		
		if (!audioData || audioSize == 0) {
			ESP_LOGW("MP3Decoder", "No audio data to feed");
			return true; // Not an error, just no data
		}
		
		// Check if we need more space in the buffer
		if (streamBufferUsed + audioSize > streamBufferSize) {
			ESP_LOGW("MP3Decoder", "Stream buffer full, dropping old data");
			// For simplicity, we'll drop old data. In a more sophisticated implementation,
			// you might want to resize the buffer or handle this differently
			size_t keepSize = streamBufferSize / 2; // Keep half the buffer
			memmove(streamBuffer, streamBuffer + streamBufferUsed - keepSize, keepSize);
			streamBufferUsed = keepSize;
		}
		
		// Copy new data to buffer
		size_t copySize = min(audioSize, streamBufferSize - streamBufferUsed);
		memcpy(streamBuffer + streamBufferUsed, audioData, copySize);
		streamBufferUsed += copySize;
		
		ESP_LOGD("MP3Decoder", "Fed %d bytes to stream buffer (total: %d/%d)", 
				copySize, streamBufferUsed, streamBufferSize);
		
		return true;
	}

	/**
	 * Get available decoded PCM data from streaming decoder
	 * @param pcmBuffer Output buffer for PCM data (allocated by this function)
	 * @param pcmSize Output number of PCM samples
	 * @param sampleRate Output sample rate of decoded audio
	 * @return true if PCM data is available
	 */
	inline bool getDecodedPCM(int16_t** pcmBuffer, size_t* pcmSize, int* sampleRate) {
		if (!streamingInitialized || streamBufferUsed == 0) {
			return false; // No data available
		}
		
		ESP_LOGD("MP3Decoder", "Attempting to decode %d bytes from stream buffer", streamBufferUsed);
		
		if (!ensureDecoderAndBuffer()) {
			return false;
		}
		
		// Debug: Log first few bytes of buffer to check for valid MP3 data
		if (streamBufferUsed >= 4) {
			ESP_LOGD("MP3Decoder", "Buffer start: %02X %02X %02X %02X", 
					streamBuffer[0], streamBuffer[1], streamBuffer[2], streamBuffer[3]);
		}
		
		// Use low-level streaming decode similar to ESP32-speaker
		std::vector<int16_t> accumulatedPCM;
		size_t totalSamples = 0;
		int detectedSampleRate = 0;
		
		uint8_t* readPtr = streamBuffer;
		int bytesLeft = streamBufferUsed;
		bool firstFrame = true;
		
		// Process all available frames in the buffer
		while (bytesLeft > 0) {
			// Find the next MP3 frame sync word
			int offset = MP3FindSyncWord(readPtr, bytesLeft);
			if (offset < 0) {
				ESP_LOGD("MP3Decoder", "No more sync words found, %d bytes left", bytesLeft);
				break; // No more sync words
			}
			
			// Move to the sync word position
			readPtr += offset;
			bytesLeft -= offset;
			
			if (bytesLeft < 4) {
				ESP_LOGD("MP3Decoder", "Not enough bytes for frame header");
				break; // Not enough data for frame
			}
			
			// Get frame info
			MP3FrameInfo frameInfo;
			int result = MP3GetNextFrameInfo(helixDecoder, &frameInfo, readPtr);
			if (result != 0) {
				ESP_LOGD("MP3Decoder", "Invalid frame header, skipping byte");
				// Invalid frame, skip one byte and try again
				readPtr++;
				bytesLeft--;
				continue;
			}
			
			// Additional validation: check if we have enough data for the frame
			// Estimate frame size (rough calculation)
			int estimatedFrameSize = (frameInfo.bitrate * 144000) / (frameInfo.samprate * 8) + 4;
			if (bytesLeft < estimatedFrameSize) {
				ESP_LOGD("MP3Decoder", "Not enough data for frame (%d < %d), need more data", bytesLeft, estimatedFrameSize);
				break; // Need more data
			}
			
			// Update sample rate from first valid frame
			if (firstFrame) {
				detectedSampleRate = frameInfo.samprate;
				firstFrame = false;
				ESP_LOGD("MP3Decoder", "Detected MP3: %d Hz, %d channels, %d kbps", 
						frameInfo.samprate, frameInfo.nChans, frameInfo.bitrate);
			}
			
			// Try to decode the frame - MP3Decode will handle incomplete frames
			int decodeResult = MP3Decode(helixDecoder, &readPtr, &bytesLeft, 
										outputBuffer, 0);
			
			if (decodeResult != 0) {
				if (decodeResult == ERR_MP3_INDATA_UNDERFLOW) {
					ESP_LOGD("MP3Decoder", "Need more data for frame");
					break; // Need more data
				} else if (decodeResult == ERR_MP3_INVALID_HUFFCODES || 
						   decodeResult == ERR_MP3_INVALID_FRAMEHEADER ||
						   decodeResult == ERR_MP3_INVALID_SIDEINFO) {
					// Frame data is corrupted, skip to next sync word
					ESP_LOGW("MP3Decoder", "Corrupted frame data (error %d), skipping to next sync", decodeResult);
					
					// Skip to next potential sync word (look for 0xFF)
					bool foundSync = false;
					while (bytesLeft >= 2 && !foundSync) {
						if (readPtr[0] == 0xFF && (readPtr[1] & 0xE0) == 0xE0) {
							foundSync = true;
							break;
						}
						readPtr++;
						bytesLeft--;
						if (bytesLeft < 2) break; // Prevent underflow
					}
					
					if (!foundSync) {
						ESP_LOGW("MP3Decoder", "No more sync words found, ending decode");
						break;
					}
					
					continue;
				} else {
					ESP_LOGW("MP3Decoder", "Frame decode error %d, skipping byte", decodeResult);
					// For other errors, just skip one byte
					readPtr++;
					bytesLeft--;
					continue;
				}
			}
			
			// Successfully decoded a frame
			size_t samplesDecoded = frameInfo.outputSamps;
			ESP_LOGD("MP3Decoder", "Decoded frame: %d samples", samplesDecoded);
			
			// Convert stereo to mono if needed
			if (frameInfo.nChans == 2) {
				// Convert stereo to mono by averaging channels
				for (size_t i = 0; i < samplesDecoded; i += 2) {
					int16_t left = outputBuffer[i];
					int16_t right = outputBuffer[i + 1];
					outputBuffer[i/2] = (left + right) / 2;
				}
				samplesDecoded /= 2;
			}
			
			// Add to accumulated PCM
			accumulatedPCM.insert(accumulatedPCM.end(), 
								outputBuffer, 
								outputBuffer + samplesDecoded);
			totalSamples += samplesDecoded;
		}
		
		// Remove consumed bytes from buffer
		size_t bytesConsumed = readPtr - streamBuffer;
		if (bytesConsumed > 0) {
			size_t remainingBytes = streamBufferUsed - bytesConsumed;
			if (remainingBytes > 0) {
				memmove(streamBuffer, readPtr, remainingBytes);
			}
			streamBufferUsed = remainingBytes;
			ESP_LOGD("MP3Decoder", "Consumed %d bytes, %d bytes remaining in buffer", 
					bytesConsumed, streamBufferUsed);
		}
		
		if (totalSamples == 0) {
			ESP_LOGD("MP3Decoder", "No PCM data decoded from %d bytes", streamBufferUsed);
			return false; // No data decoded
		}
		
		// Allocate output buffer
		int16_t* outputBuffer = (int16_t*)heap_caps_malloc(totalSamples * sizeof(int16_t), 
														  MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
		if (!outputBuffer) {
			ESP_LOGE("MP3Decoder", "Failed to allocate output buffer for %d samples", totalSamples);
			return false;
		}
		
		// Copy accumulated data
		memcpy(outputBuffer, accumulatedPCM.data(), totalSamples * sizeof(int16_t));
		
		// Resample if needed
		int finalSampleRate = detectedSampleRate;
		int16_t* finalBuffer = outputBuffer;
		size_t finalSize = totalSamples;
		
		if (detectedSampleRate != 16000) {
			// Resample to 16kHz for speaker
			if (!resampleTo16kHz(outputBuffer, totalSamples, detectedSampleRate, &finalBuffer, &finalSize)) {
				ESP_LOGW("MP3Decoder", "Resampling failed, using original %d Hz", detectedSampleRate);
				finalSampleRate = detectedSampleRate;
			} else {
				finalSampleRate = 16000;
				if (finalBuffer != outputBuffer) {
					heap_caps_free(outputBuffer);
				}
			}
		}
		
		*pcmBuffer = finalBuffer;
		*pcmSize = finalSize;
		*sampleRate = finalSampleRate;
		
		return true;
	}

	/**
	 * Free PCM buffer allocated by decode functions
	 * @param pcmBuffer Buffer to free
	 */
	inline void freePCMBuffer(int16_t* pcmBuffer) {
		if (pcmBuffer) {
			heap_caps_free(pcmBuffer);
		}
	}

	/**
	 * Decode MP3 audio data to PCM (non-streaming)
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
		return true;
	}

	/**
	 * Reset streaming decoder for new stream
	 */
	inline void reset() {
		streamBufferUsed = 0;
		if (streamBuffer) {
			memset(streamBuffer, 0, streamBufferSize);
		}
	}

private:
	MP3Decoder mp3Decoder;
	bool streamingInitialized = false;
	
	// Streaming state
	uint8_t* streamBuffer = nullptr;
	size_t streamBufferSize = 0;
	size_t streamBufferUsed = 0;
	const size_t STREAM_BUFFER_SIZE = 1024 * 500;

	/**
	 * Ensure decoder and output buffer are initialized
	 * @return true if ready
	 */
	inline bool ensureDecoderAndBuffer() {
		if (!helixDecoder) {
			helixDecoder = MP3InitDecoder();
			if (!helixDecoder) {
				ESP_LOGE("MP3Decoder", "Failed to initialize Helix MP3 decoder");
				return false;
			}
		}
		
		if (!outputBuffer) {
			outputBufferSize = MAX_OUTPUT_BUFFER_SIZE;
			outputBuffer = (int16_t*)heap_caps_malloc(outputBufferSize * sizeof(int16_t), 
													MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
			if (!outputBuffer) {
				ESP_LOGE("MP3Decoder", "Failed to allocate output buffer");
				MP3FreeDecoder(helixDecoder);
				helixDecoder = nullptr;
				return false;
			}
		}
		return true;
	}

	/**
	 * Ensure stream buffer is allocated
	 * @return true if buffer is ready
	 */
	inline bool ensureStreamBuffer() {
		if (!streamBuffer) {
			streamBuffer = (uint8_t*)heap_caps_malloc(STREAM_BUFFER_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);
			if (!streamBuffer) {
				return false;
			}
			streamBufferSize = STREAM_BUFFER_SIZE;
		}
		return true;
	}

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
	}
};

extern Mp3Decoder mp3decoder;

#endif // MP3_DECODER_H
