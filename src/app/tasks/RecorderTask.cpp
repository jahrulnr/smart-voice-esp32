#include "app/tasks.h"

uint32_t generateKey(uint32_t uniqueId, int index) {
    if (index == 0) return uniqueId * 10000;  // start
    if (index == -1) return uniqueId * 10000 + 9999;  // end
    return uniqueId * 10000 + index;
}

bool publishChunk(uint32_t key, const uint8_t* data, size_t dataSize) {
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
			AudioSamples audioSamples = {
				.key = MQTT_TOPIC_AUDIO,
				.data = payload,
				.length = payloadSize
			};
			result = xQueueSend(audioQueue, &audioSamples, 0);
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

void recorderTask(void* param) {
	const char* TAG = "recorderTask";

	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = 1;
	uint32_t lastUpdate = millis();
	uint32_t lastIndex = 0;
	uint32_t lastKey = millis();
	uint32_t key;

	// 1s = 16kHz; 
	size_t maxSamples = 1024;
	size_t samplesRead;
	esp_err_t err = ESP_OK;

	QueueHandle_t lock = xSemaphoreCreateMutex();
	int16_t* readBuffer = (int16_t*)heap_caps_malloc(maxSamples, MALLOC_CAP_SPIRAM);

	bool streaming = false;
	uint8_t* chunk = nullptr;

	ESP_LOGI(TAG, "Recorder task started");
  while (true) {
		notification->send(TAG, 1);
		
		int signal = notification->signal(NOTIFICATION_RECORD, 0);
		if (signal == 0) {
			ESP_LOGW(TAG, "status: ON");
			ESP_LOGW(TAG, "status: ON, last index: %d", lastIndex);
			streaming = true;
			if (lastIndex == -1) {lastIndex = 0;}
		}
		else if (signal == 1) {
			ESP_LOGW(TAG, "status: OFF, last index: %d", lastIndex);
			streaming = false;
			if(lastIndex != -1){
				vTaskDelay(pdMS_TO_TICKS(5));
				lastIndex = -1;
				goto publish;
			}
			goto unlock;
		}
		
		if (!streaming) goto end;

    if (!microphone || !wifiManager.isConnected()) {
      goto end;
    }

		if (xSemaphoreTake(lock, 0) != pdTRUE){
			ESP_LOGW("Recorder", "Failed to acquire lock, skipping recording");
			goto end;
		}

		if (lastIndex == 0) lastKey = millis();

    // Publish start
		publish:
    key = generateKey(lastKey, lastIndex);
		if (lastIndex != 0 && lastIndex != -1) {
			auto cache = microphone->getCache();
			if (cache.lastSampleLen == 0 
				|| cache.lastSampleTime <= lastUpdate 
				|| cache.lastSampleTime == 0) goto unlock;
			memcpy(readBuffer, cache.lastSample, cache.lastSampleLen);
			lastUpdate = cache.lastSampleTime;
			chunk = (uint8_t*)readBuffer;
		} else {
			chunk = nullptr;
		}

		try {
			publishChunk(key, chunk, maxSamples);
			lastIndex++;
		}
		catch(...) {}

		unlock:
		memset(readBuffer, 0, maxSamples);
		xSemaphoreGive(lock);
		end:
		vTaskDelayUntil(&lastWakeTime, updateFrequency);		
  }
}