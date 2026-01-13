#pragma once

#include "boot/init.h"

esp_err_t srAudioCallback(void *arg, void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms);
void srEventCallback(void *arg, sr_event_t event, int command_id, int phrase_id);
void mqttCallback(char* topic, byte* payload, unsigned int length);
bool audioBrokerPublisher(uint32_t key, uint32_t index, const uint8_t* data, size_t dataSize);
bool audioToWavCallback(uint32_t key, uint32_t index, const uint8_t* data, size_t dataSize);
void audioTalkCallback(const String& key);

void aiCallback(const String& payload, const String& response);
void aiVoiceCallback(const String& text, const uint8_t* audioData, size_t audioSize);
void aiTranscriptionCallback(const String& filePath, const String& text, const String& usageJson);

size_t micAudioCallback(uint8_t* buffer, size_t maxSize);
void speakerAudioCallback(const uint8_t* audioData, size_t audioSize, bool isLastChunk);