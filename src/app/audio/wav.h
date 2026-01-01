#pragma once
#include <cstdint>
#include <FS.h>
#include <esp_heap_caps.h>
#include "converter.h"

typedef struct __attribute__((packed)) {
    // RIFF header
    char riff[4];           // "RIFF"
    uint32_t file_size;     // File size - 8 bytes
    char wave[4];           // "WAVE"
    
    // Format chunk
    char fmt[4];            // "fmt "
    uint32_t fmt_size;      // Format chunk size (16 for PCM)
    uint16_t audio_format;  // Audio format (1 = PCM)
    uint16_t num_channels;  // Number of channels (1 = mono)
    uint32_t sample_rate;   // Sample rate (e.g., 16000)
    uint32_t byte_rate;     // Byte rate (sample_rate * num_channels * bits_per_sample / 8)
    uint16_t block_align;   // Block align (num_channels * bits_per_sample / 8)
    uint16_t bits_per_sample; // Bits per sample (16)
    
    // Data chunk
    char data[4];           // "data"
    uint32_t data_size;     // Data size (file_size - 44)
} wav_hdr;

class WavRecorder {
private:
	FS* _fs = nullptr;
	File _audioFile;
	wav_hdr _currentHeader;
	bool _recording = false;
	float _volumeGain = 30.0f;
	size_t _recordedBytes = 0;
	size_t _minFreeSpaceBytes = 1048576;
	String _filename;
	uint32_t _inputKhz = 16;  // Default input sample rate
	uint32_t _outputKhz = 16; // Default output sample rate (no resampling)
	bool _needsResampling = false;

public:
	inline void init(FS& filesystem) { _fs = &filesystem; }

	inline void setVolume(float gain) { _volumeGain = gain; }

	inline void setMinFreeSpace(size_t bytes) { _minFreeSpaceBytes = bytes; }

	inline bool start(const String& fname, uint32_t khzIn = 16, int32_t khzOut = -1) {
		if (_recording || !_fs) return false;
		_filename = fname;
		_inputKhz = khzIn;
		_outputKhz = (khzOut == -1) ? khzIn : (uint32_t)khzOut; // -1 means no resampling
		_needsResampling = (khzOut != -1 && khzIn != (uint32_t)khzOut);

		// Create WAV header
		uint32_t sampleRate = _needsResampling ? (_outputKhz * 1000) : (_inputKhz * 1000);
		
		// RIFF header
		memcpy(_currentHeader.riff, "RIFF", 4);
		_currentHeader.file_size = 0;  // Will be updated on stop()
		
		memcpy(_currentHeader.wave, "WAVE", 4);
		
		// Format chunk
		memcpy(_currentHeader.fmt, "fmt ", 4);
		_currentHeader.fmt_size = 16;  // PCM format size
		_currentHeader.audio_format = 1;  // PCM
		_currentHeader.num_channels = 1;  // Mono
		_currentHeader.sample_rate = sampleRate;
		_currentHeader.bits_per_sample = 16;
		_currentHeader.byte_rate = _currentHeader.sample_rate * _currentHeader.num_channels * (_currentHeader.bits_per_sample / 8);
		_currentHeader.block_align = _currentHeader.num_channels * (_currentHeader.bits_per_sample / 8);
		
		// Data chunk
		memcpy(_currentHeader.data, "data", 4);
		_currentHeader.data_size = 0;  // Will be updated on stop()

		_audioFile = _fs->open(_filename, "w");
		if (_audioFile) {
			size_t written = _audioFile.write((uint8_t*)&_currentHeader, sizeof(wav_hdr));
			if (written == sizeof(wav_hdr)) {
				_recording = true;
				_recordedBytes = 0;
				return true;
			} else {
				ESP_LOGE("WAV", "Failed to write WAV header");
				_audioFile.close();
			}
		}
		return false;
	}

	inline void processChunk(uint8_t* data, size_t length) {
		if (!_recording || !data || !_audioFile) return;

		size_t numSamples = length / 2;
		int16_t* samples = (int16_t*)data;

		// Amplify audio data
		for (size_t i = 0; i < numSamples; ++i) {
			int32_t amplified = (int32_t)(samples[i] * _volumeGain);
			if (amplified > 32767) amplified = 32767;
			else if (amplified < -32768) amplified = -32768;
			samples[i] = (int16_t)amplified;
		}

		if (_needsResampling) {
			// Calculate output buffer size needed
			size_t outputSamples = AudioBufferConverter::calculateOutputSize(_inputKhz, _outputKhz, numSamples);

			// Allocate temporary buffer for resampled data using PSRAM
			int16_t* resampledBuffer = (int16_t*)heap_caps_malloc(outputSamples * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_DEFAULT);

			if (resampledBuffer) {
				int result = AudioBufferConverter::convert(_inputKhz, _outputKhz,
					samples, numSamples, resampledBuffer, outputSamples);

				if (result > 0) {
					// Write resampled data
					size_t outputBytes = result * 2; // 16-bit samples
					_audioFile.write((uint8_t*)resampledBuffer, outputBytes);
					_recordedBytes += outputBytes;
				}
				heap_caps_free(resampledBuffer);
			} else {
				// Fallback: write original data if allocation failed
				ESP_LOGW("WAV", "Failed to allocate resampling buffer, writing original data");
				_audioFile.write(data, length);
				_recordedBytes += length;
			}
		} else {
			// No resampling needed
			_audioFile.write(data, length);
			_recordedBytes += length;
		}
	}

	inline void stop() {
		if (!_recording) return;
		
		// Update header with final file size
		_currentHeader.file_size = 36 + _recordedBytes;  // 44 - 8 = 36
		_currentHeader.data_size = _recordedBytes;
		
		// Seek to beginning and rewrite header
		_audioFile.seek(0);
		_audioFile.write((uint8_t*)&_currentHeader, sizeof(wav_hdr));
		_audioFile.close();
		
		ESP_LOGI("WAV", "WAV file %s recorded, size: %d bytes", _filename.c_str(), 44 + _recordedBytes);
		_recording = false;
	}

	inline float info() {
		uint32_t sampleRate = _needsResampling ? (_outputKhz * 1000) : (_inputKhz * 1000);
		return (float)_recordedBytes / (sampleRate * 2); // sampleRate * bytes_per_sample
	}

	inline bool isRecording() { return _recording; }

	inline size_t getRecordedBytes() { return _recordedBytes; }
};