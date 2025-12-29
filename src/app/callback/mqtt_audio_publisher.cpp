#include "app/callbacks.h"
#include "app/tasks.h"

uint32_t generateKey(uint32_t uniqueId, int index) {
    if (index == 0) return uniqueId * 10000;  // start
    if (index == -1) return uniqueId * 10000 + 9999;  // end
    return uniqueId * 10000 + index;
}

bool audioBrokerPublisher(uint32_t key, uint32_t index, const uint8_t* data, size_t dataSize) {
	key = generateKey(key, index);

	// Prepare payload: key + data
	size_t payloadSize = sizeof(uint32_t) + dataSize;
	uint8_t* payload = (uint8_t*)heap_caps_malloc(payloadSize, MALLOC_CAP_SPIRAM);
	if (!payload) {
		ESP_LOGE("AudioStreamer", "Failed to allocate payload");
		return false;
	}

	memcpy(payload, &key, sizeof(uint32_t));
	if (data) memcpy(payload + sizeof(uint32_t), data, dataSize);

	// Publish message for NetworkConsumer
	BaseType_t result;
	try {
		AudioData audioSamples = {
			.key = MQTT_TOPIC_AUDIO,
			.data = payload,
			.length = payloadSize
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