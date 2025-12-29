#include "app/callbacks.h"
#include "app/tasks.h"

bool audioToWav(uint32_t key, uint32_t index, const uint8_t* data, size_t dataSize){
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