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

	// Send to speaker
	size_t samplesWritten = 0;

	// Calculate actual output size for the limited input
	size_t requiredOutputSamples = AudioBufferConverter::calculateOutputSize(24, 16, audioSize / sizeof(int16_t));
	
	// Allocate output buffer in SPIRAM
	int16_t* pcmOutput = (int16_t*)heap_caps_malloc(requiredOutputSamples * sizeof(int16_t), MALLOC_CAP_SPIRAM);
	if (!pcmOutput) {
		ESP_LOGE("SpeakerCallback", "Failed to allocate output buffer");
		return;
	}

	// Convert sample rate from 24kHz to 16kHz
	size_t convertedSamples = AudioBufferConverter::convert(24, 16, (int16_t*)audioData, audioSize / sizeof(int16_t), pcmOutput, requiredOutputSamples);
	if (convertedSamples <= 0) {
		ESP_LOGE("SpeakerCallback", "Audio conversion failed");
		heap_caps_free(pcmOutput);
		return;
	}
	speaker->writeSamples(pcmOutput, convertedSamples * sizeof(int16_t), &samplesWritten);

	// Calculate expected playback duration
	ESP_LOGI("AIVoiceCallback", "Sent %d samples (%d samples written) to speaker",
		convertedSamples, samplesWritten);
	heap_caps_free(pcmOutput);

	// Clear speaker buffer
	speaker->clear();
	notification->send(NOTIFICATION_DISPLAY, EDISPLAY_NONE);
}

void aiTranscriptionCallback(const String& filePath, const String& text, const String& usageJson) {
	ESP_LOGI("AITranscriptionCallback", "Transcription: %s", text.c_str());
	aiTts.textToSpeech(text.c_str(), aiVoiceCallback);
	sysActivity->update();
}