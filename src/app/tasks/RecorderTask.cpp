#include "app/tasks.h"

void recorderTask(void* param) {
	const char* TAG = "recorderTask";

	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = 1;
	uint32_t lastUpdate = millis();
	uint32_t index = 0;
	uint32_t key = millis();

	size_t maxSamples = getAfeHandle()->get_feed_chunksize(getAfeData()); // follow afe chunksize
	size_t samplesRead;
	esp_err_t err = ESP_OK;

	QueueHandle_t lock = xSemaphoreCreateMutex();
	int16_t* readBuffer = (int16_t*)heap_caps_malloc(maxSamples, MALLOC_CAP_SPIRAM);

	bool streaming = false;
	uint8_t* chunk = nullptr;
	TaskHandle_t recordEventHandle = nullptr;

	ESP_LOGI(TAG, "Recorder task started");
  while (true) {
		notification->send(TAG, 1);
		
		int signal = notification->signal(NOTIFICATION_RECORD, 0);
		if (signal == 0) {
			streaming = true;
			key = millis();
			index = 0;
			ESP_LOGW(TAG, "status: ON, key: %d", key);
#if MQTT_ENABLE == 0
			xTaskCreatePinnedToCore(recordEvent, "recordEvent", 1024 * 4, nullptr, 0, &recordEventHandle, 1);
#endif
		}
		else if (signal == 1) {
			ESP_LOGW(TAG, "status: OFF, key: %d, last index: %d", key, index);
			streaming = false;
			vTaskDelay(pdMS_TO_TICKS(5));
			index = -1;
#if MQTT_ENABLE == 0
			AudioData audioSamples = {
				.key = String(key),
				.data = nullptr,
				.length = 0,
				.stream = false
			};
			xQueueSend(audioChunkQueue, &audioSamples, portMAX_DELAY);
			recordEventHandle = nullptr;
#endif
			goto unlock;
		}
		
		if (!streaming || !microphone || !wifiManager.isConnected()) {
      goto end;
    }

		if (xSemaphoreTake(lock, 0) != pdTRUE){
			ESP_LOGW("Recorder", "Failed to acquire lock, skipping recording");
			goto end;
		}

    // Publish start
		publish:
		if (index != 0 && index != -1) {
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
#if MQTT_ENABLE
			audioBrokerPublisher(key, index, chunk, maxSamples);
#else
			audioToWav(key, index, chunk, maxSamples);
#endif
			index++;
		}
		catch(...) {}

		unlock:
		memset(readBuffer, 0, maxSamples);
		xSemaphoreGive(lock);
		end:
		vTaskDelayUntil(&lastWakeTime, updateFrequency);		
  }
}