#include "app/callbacks.h"
#include "app/audio/converter.h"

void aiCallback(const String& payload, const String& response){
	if (response.isEmpty()) return;
	sysActivity->update();
	notification->send(NOTIFICATION_DISPLAY, EDISPLAY_LOADING);

	ESP_LOGI("AICallback", "Payload: %s", payload.c_str());
	ESP_LOGI("AICallback", "Response: %s", response.c_str());
	
	if (wifiManager.isConnected()) {
		aiTts.textToSpeech(response.c_str(), aiVoiceCallback);
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

	notification->send(NOTIFICATION_DISPLAY, EDISPLAY_FACE);

	// Check TTS format and decode if necessary
	if (aiTts.getFormat() == GPTAudioFormat::GPT_MP3) {
		// Initialize MP3 decoder if needed
		if (!mp3decoder.isInitialized()) {
			if (!mp3decoder.init()) {
				ESP_LOGE("AIVoiceCallback", "Failed to initialize MP3 decoder");
				// Fallback: try to play as raw data
				speakerAudioCallback(audioData, audioSize, false);
				return;
			}
		}

		// Decode MP3 to PCM
		int16_t* pcmBuffer = nullptr;
		size_t pcmSize = 0;
		int sampleRate = 0;

		if (mp3decoder.decodeMP3ToPCM(audioData, audioSize, &pcmBuffer, &pcmSize, &sampleRate)) {
			ESP_LOGI("AIVoiceCallback", "Decoded MP3 to PCM: %d samples at %d Hz", pcmSize, sampleRate);
			// Send PCM data to speaker
			speakerAudioCallback((uint8_t*)pcmBuffer, pcmSize * sizeof(int16_t), false);
			mp3decoder.freePCMBuffer(pcmBuffer);
		} else {
			ESP_LOGE("AIVoiceCallback", "Failed to decode MP3 data");
			// Fallback: try to play as raw data
			speakerAudioCallback(audioData, audioSize, false);
		}
	} else {
		// For WAV or PCM, send directly to speaker
		speakerAudioCallback(audioData, audioSize, false);
	}

	// Clear speaker buffer
	speaker->clear();
	notification->send(NOTIFICATION_DISPLAY, EDISPLAY_NONE);
}

void aiTranscriptionCallback(const String& filePath, const String& text, const String& usageJson) {
	ESP_LOGI("AITranscriptionCallback", "Transcription: %s", text.c_str());
	aiTts.textToSpeech(text.c_str(), aiVoiceCallback);
	sysActivity->update();
}