#include "app/tasks.h"

const char* topic = "pioassistant/audio";

uint32_t generateKey(uint32_t uniqueId, int index) {
    if (index == 0) return uniqueId * 10000;  // start
    if (index == -1) return uniqueId * 10000 + 9999;  // end
    return uniqueId * 10000 + index;
}

bool publishChunk(uint32_t key, const uint8_t* data, size_t dataSize) {
    // Don't send data chunks smaller than 2 bytes (1 sample) to avoid corrupted chunks
    if (data != nullptr && dataSize < 2) {
        ESP_LOGD("AudioStreamer", "Skipping small data chunk (%d bytes) for key %u", dataSize, key);
        return true;
    }

    ESP_LOGD("AudioStreamer", "Publishing chunk with key %u, dataSize %d", key, dataSize);
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
		bool result;
		try {
			mqttClient.loop();
			result = mqttClient.publish(topic, payload, payloadSize);
		}
		catch (const std::exception& e) {
			ESP_LOGE("AudioStreamer", "Failed to send MQTT publish message to NetworkConsumer: %s", e.what());
			return false;
		}
		catch(...) {
			ESP_LOGW("AudioStreamer", "Failed to send MQTT loop unknown error");
		}

    // Clean up
    delete[] payload;
    payload = nullptr;

    if (!result) {
        return false;
    }

    ESP_LOGD("AudioStreamer", "Published chunk with key %u, size %d", key, payloadSize);
    return true;
}

void recorderTask(void* param) {
	const char* TAG = "recorderTask";

	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = 1;
	uint32_t lastUpdate = millis();
	uint32_t lastIndex = 0;
	uint32_t lastKey = millis();
	uint32_t key;
	bool hasSubscribe = false;

	size_t maxSamples = 2048;
	size_t samplesRead;
	esp_err_t err = ESP_OK;

	QueueHandle_t lock = xSemaphoreCreateMutex();
	int16_t* readBuffer;


	bool streaming = false;
  while (true) {
		int signal = notification->signal(NOTIFICATION_RECORD, 0);
		if (signal == 0) streaming = true;
		else if (signal == 1) {
			streaming = false;
			if (lastIndex > 0) {
				lastIndex = -1;
				goto publish;
			}
		}
		
		if (!streaming) goto end;

    if (!microphone || !wifiManager.isConnected()) {
      goto end;
    }

		if (xSemaphoreTake(lock, 0) != pdTRUE){
			ESP_LOGW("Recorder", "Failed to acquire lock, skipping recording");
			goto end;
		}

		if (!hasSubscribe && mqttClient.connected()) {
			mqttClient.subscribe(topic);
			hasSubscribe = true;
		} 
		else if (hasSubscribe && !mqttClient.connected()) {
			hasSubscribe = false;
			goto unlock;
		}

		if (lastIndex == 0) lastKey = millis();

    // Publish start
		publish:
    key = generateKey(lastKey, lastIndex);
		if (lastIndex == 0 || lastIndex == -1) {
	    publishChunk(key, nullptr, 0);
			lastIndex++;
			goto unlock;
		}

		readBuffer = (int16_t*)heap_caps_malloc(maxSamples, MALLOC_CAP_SPIRAM);
		samplesRead = 0;
		err = microphone->read(readBuffer, maxSamples, &samplesRead, portMAX_DELAY);
		if (err != ESP_OK || samplesRead == 0) {
			heap_caps_free(readBuffer);
			readBuffer = nullptr;
			goto unlock;
		}

		try {
			publishChunk(key, (uint8_t*)readBuffer, maxSamples);
			lastIndex++;
		}
		catch(...) {}

		heap_caps_free(readBuffer);
		readBuffer = nullptr;
		unlock:
		xSemaphoreGive(lock);
		end:
		// vTaskDelayUntil(&lastWakeTime, updateFrequency);
		vTaskDelay(updateFrequency);
  }
}