#pragma once

#include "boot/init.h"

esp_err_t srAudioCallback(void *arg, void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms);
void srEventCallback(void *arg, sr_event_t event, int command_id, int phrase_id);
void mqttCallback(char* topic, byte* payload, unsigned int length);

void aiCallback(const String& payload, const String& response);
void aiVoiceCallback(const String& text, const uint8_t* audioData, size_t audioSize);
void aiTranscriptionCallback(const String& filePath, const String& text, const String& usageJson);