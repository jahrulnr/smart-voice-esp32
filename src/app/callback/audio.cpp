#include "app/callbacks.h"
#include "app/tasks.h"
#include <core/datastore.h>

bool audioToWavCallback(uint32_t key, uint32_t index, const uint8_t* data, size_t dataSize){
	if (!data) return pdTRUE;
	uint8_t* payload = (uint8_t*)heap_caps_malloc(dataSize, MALLOC_CAP_SPIRAM);
	if (!payload) {
		ESP_LOGE("AudioStreamer", "Failed to allocate payload");
		return false;
	}
	memcpy(payload, data, dataSize);

	// Publish message for NetworkConsumer
	BaseType_t result;
	try {
		AudioData audioSamples = {
			.key = String(key),
			.data = payload,
			.length = dataSize,
			.stream = true
		};
		result = xQueueSend(audioChunkQueue, &audioSamples, 0);
		if (result != pdTRUE) {
			delete[] payload;
			payload = nullptr;
		}
	}
	catch (const std::exception& e) {
		ESP_LOGE("AudioStreamer", "Failed to send MQTT publish message to NetworkConsumer: %s", e.what());
		return false;
	}
	catch(...) {
		ESP_LOGW("AudioStreamer", "Failed to send MQTT loop unknown error");
	}

	return result == pdTRUE;
}

void audioTalkCallback(const String& key) {
	aiStt.transcribeAudio(key.c_str(), [](const String& filePath, const String& text, const String& usageJson){

#if SAVE_AUDIO == 0
	LittleFS.remove(filePath);
#endif

String systemCmd = R"===(
Your tasks:
- You are an AI that generates short Text-to-Speech friendly responses
- You act like a friendly chat buddy, not a formal assistant
- You may use light conversational fillers like "hmm", "oh ya", "eh", "sebentar", naturally
- You are a robot brain running inside ESP32-S3
- Use a casual, friendly tone
- Keep the response very short and clear for TTS
- Do NOT use emojis or special characters
- Do NOT respond more than 200 characters
- Do NOT ramble or tell stories
- You ONLY have the capabilities explicitly stated in this instruction
- You MUST NOT invent, assume, or describe features outside of this instruction
- If unsure, always say you do not know

Available features:
- Answer the questions

System Information:
- Timestamp: {--time--} (24-hour format)
- Module: Display 128x64, INMP441, MAX98357A, Touch Sensor
- AI system: Speech To Text, Text To Speech, LLM
- Last user interaction: {--last_interaction--}ms
)===";

		systemCmd.replace("{--time--}", timeManager.getCurrentTime());
		systemCmd.replace("{--last_interaction--}", String(sysActivity.lastUpdate(millis())));
		ai.setSystemMessage(systemCmd);
		ai.sendPrompt(text, [](const String &payload, const String &response){
			// tts.speak(response.c_str());
			aiCallback(payload, response);
		});
	});
}