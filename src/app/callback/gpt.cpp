#include "app/callbacks.h"
#include "app/audio/mp3decoder.h"

void aiCallback(const String& payload, const String& response, bool streaming){
	if (response.isEmpty()) return;
	sysActivity->update();
	notification->send(NOTIFICATION_DISPLAY, EDISPLAY_LOADING);

	ESP_LOGI("AICallback", "Payload: %s", payload.c_str());
	ESP_LOGI("AICallback", "Response: %s", response.c_str());
	ESP_LOGI("AICallback", "Streaming mode: %s", streaming ? "enabled" : "disabled");
	
	if (wifiManager.isConnected()) {
		if (streaming) {
			aiTts.textToSpeechStream(response.c_str(), aiVoiceStreamCallback);
		} else {
			aiTts.textToSpeech(response.c_str(), aiVoiceCallback);
		}
	} else {
		tts.speak(response.c_str());
	}
}

void aiVoiceCallback(const String& text, const uint8_t* audioData, size_t audioSize) {
	ESP_LOGI("AIVoiceCallback", "Text: %s", text.c_str());
	ESP_LOGI("AIVoiceCallback", "Audio size: %d", audioSize);
	sysActivity->update();

	if (!audioData || audioSize == 0) {
		ESP_LOGE("AIVoiceCallback", "No audio data received");
		return;
	}

	// Decode MP3 to PCM
	Mp3Decoder mp3decoder;
	int16_t* pcmBuffer = nullptr;
	size_t pcmSize = 0;
	int sampleRate = 0;

	if (!mp3decoder.decodeMP3ToPCM(audioData, audioSize, &pcmBuffer, &pcmSize, &sampleRate)) {
		ESP_LOGE("AIVoiceCallback", "Failed to decode MP3");
		return;
	}

	ESP_LOGI("AIVoiceCallback", "Decoded to PCM: %d samples at %d Hz", pcmSize, sampleRate);
	notification->send(NOTIFICATION_DISPLAY, EDISPLAY_FACE);

	// Send to speaker
	size_t samplesWritten = 0;
	if (pcmBuffer) {
		speaker->writeSamples(pcmBuffer, pcmSize * sizeof(int16_t), &samplesWritten);

		// Calculate expected playback duration
		float durationSeconds = (float)pcmSize / sampleRate;
		ESP_LOGI("AIVoiceCallback", "Sent %d samples (%d samples written) to speaker - expected duration: %.2f seconds",
				 pcmSize, samplesWritten, durationSeconds);

		// Free the buffer
		mp3decoder.freePCMBuffer(pcmBuffer);
	} else {
		ESP_LOGW("AIVoiceCallback", "no buffer data");
	}

	// Clear speaker buffer
	speaker->clear();
	notification->send(NOTIFICATION_DISPLAY, EDISPLAY_NONE);
}

void aiVoiceStreamCallback(const String& text, const uint8_t* audioChunk, size_t chunkSize, bool isLastChunk) {
	static Mp3Decoder mp3decoder;
	static bool initialized = false;
	
	// Initialize decoder on first call
	if (!initialized) {
		if (!mp3decoder.init()) {
			ESP_LOGE("AIVoiceStreamCallback", "Failed to initialize streaming MP3 decoder");
			return;
		}
		initialized = true;
		notification->send(NOTIFICATION_DISPLAY, EDISPLAY_FACE);
	}
	
	if (isLastChunk && chunkSize == 0) {
		// Try to decode any remaining data
		int16_t* pcmBuffer = nullptr;
		size_t pcmSize = 0;
		int sampleRate = 0;
		int attempts = 0;
		while (attempts < 50) {  // More attempts for final decoding
			if (mp3decoder.getDecodedPCM(&pcmBuffer, &pcmSize, &sampleRate)) {
				if (pcmBuffer && pcmSize > 0) {
					size_t samplesWritten = 0;
					speaker->writeSamples(pcmBuffer, pcmSize * sizeof(int16_t), &samplesWritten);
					mp3decoder.freePCMBuffer(pcmBuffer);
				}
			} else {
				break;
			}
			attempts++;
		}
		
		// Clean up
		mp3decoder.reset();
		initialized = false;
		speaker->clear();
		notification->send(NOTIFICATION_DISPLAY, EDISPLAY_NONE);
		sysActivity->update();
		return;
	}

	if (!audioChunk || chunkSize == 0) {
		ESP_LOGE("AIVoiceStreamCallback", "No audio chunk received");
		return;
	}

	// Feed the complete frame to decoder
	if (!mp3decoder.feedData(audioChunk, chunkSize)) {
		ESP_LOGE("AIVoiceStreamCallback", "Failed to feed audio frame to decoder");
		return;
	}

	// Try to decode any available frames (may decode multiple frames)
	int decodeAttempts = 0;
	while (decodeAttempts < 10) {  // Limit attempts to prevent infinite loop
		int16_t* pcmBuffer = nullptr;
		size_t pcmSize = 0;
		int sampleRate = 0;
		
		if (mp3decoder.getDecodedPCM(&pcmBuffer, &pcmSize, &sampleRate)) {
			if (pcmBuffer && pcmSize > 0) {
				size_t samplesWritten = 0;
				speaker->writeSamples(pcmBuffer, pcmSize * sizeof(int16_t), &samplesWritten);
				mp3decoder.freePCMBuffer(pcmBuffer);
			}
		} else {
			// No more data available to decode
			break;
		}
		decodeAttempts++;
	}
}

void aiTranscriptionCallback(const String& filePath, const String& text, const String& usageJson) {
	ESP_LOGI("AITranscriptionCallback", "Transcription: %s", text.c_str());
	aiTts.textToSpeech(text.c_str(), aiVoiceCallback);
	sysActivity->update();
}