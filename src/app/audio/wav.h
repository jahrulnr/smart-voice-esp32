#pragma once
#include <cstdint>
#include <FS.h>

typedef struct WAV_HEADER {
	/* RIFF Chunk Descriptor */
	uint8_t RIFF[4] = {'R', 'I', 'F', 'F'}; // RIFF Header Magic header
	uint32_t ChunkSize;                     // RIFF Chunk Size
	uint8_t WAVE[4] = {'W', 'A', 'V', 'E'}; // WAVE Header
	/* "fmt" sub-chunk */
	uint8_t fmt[4] = {'f', 'm', 't', ' '}; // FMT header
	uint32_t Subchunk1Size = 16;           // Size of the fmt chunk
	uint16_t AudioFormat = 1; // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM
							// Mu-Law, 258=IBM A-Law, 259=ADPCM
	uint16_t NumOfChan = 1;   // Number of channels 1=Mono 2=Sterio
	uint32_t SamplesPerSec = 16000;   // Sampling Frequency in Hz
	uint32_t bytesPerSec = 16000 * 2; // bytes per second
	uint16_t blockAlign = 2;          // 2=16-bit mono, 4=16-bit stereo
	uint16_t bitsPerSample = 16;      // Number of bits per sample
	/* "data" sub-chunk */
	uint8_t Subchunk2ID[4] = {'d', 'a', 't', 'a'}; // "data"  string
	uint32_t Subchunk2Size;                        // Sampled data length
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

public:
	inline void init(FS& filesystem) { _fs = &filesystem; }

	inline void setVolume(float gain) { _volumeGain = gain; }

	inline void setMinFreeSpace(size_t bytes) { _minFreeSpaceBytes = bytes; }

	inline bool start(const String& fname) {
		if (_recording || !_fs) return false;
		_filename = fname;
		_currentHeader = {};
		_currentHeader.ChunkSize = 36;
		_currentHeader.Subchunk2Size = 0;
		_audioFile = _fs->open(_filename, "a+", true);
		if (_audioFile) {
			_audioFile.write((uint8_t*)&_currentHeader, sizeof(_currentHeader));
			_recording = true;
			_recordedBytes = 0;
			return true;
		}
		return false;
	}

	inline void processChunk(uint8_t* data, size_t length) {
		if (!_recording || !data || !_audioFile) return;
		// Amplify audio data
		size_t numSamples = length / 2;
		int16_t* samples = (int16_t*)data;
		for (size_t i = 0; i < numSamples; ++i) {
			int32_t amplified = (int32_t)(samples[i] * _volumeGain);
			if (amplified > 32767) amplified = 32767;
			else if (amplified < -32768) amplified = -32768;
			samples[i] = (int16_t)amplified;
		}
		_audioFile.write(data, length);
		_recordedBytes += length;
		_currentHeader.Subchunk2Size += length;
		_currentHeader.ChunkSize = sizeof(wav_hdr) + _currentHeader.Subchunk2Size - 8;
	}

	inline void stop() {
		if (!_recording) return;
		// Update header and close file
		_audioFile.seek(0);
		_audioFile.write((uint8_t*)&_currentHeader, sizeof(_currentHeader));
		_audioFile.close();

		ESP_LOGI("WAV", "WAV file %s recorded, size: %d bytes", _filename.c_str(), _currentHeader.Subchunk2Size);
		_recording = false;
	}

	inline float info() {
		return (float)_currentHeader.Subchunk2Size / _currentHeader.bytesPerSec;
	}

	inline bool isRecording() { return _recording; }

	inline size_t getRecordedBytes() { return _recordedBytes; }
};