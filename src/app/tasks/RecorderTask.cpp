#include "app/tasks.h"

void recorderTask(void* param) {
	const char* TAG = "recorderTask";

	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = 1;
	uint32_t lastUpdate = millis();
	uint32_t index = 0;
	uint32_t key = millis();

	size_t maxSamples = getAfeHandle()->get_feed_chunksize(getAfeData()) * 2; // follow afe chunksize
	size_t samplesRead;
	esp_err_t err = ESP_OK;

	QueueHandle_t lock = xSemaphoreCreateMutex();
	int16_t* readBuffer = (int16_t*)heap_caps_malloc(maxSamples, MALLOC_CAP_SPIRAM);

	uint8_t* chunk = nullptr;
	TaskHandle_t recordEventHandle = nullptr;
	AUDIO_STATE lastState = AUDIO_STATE_IDLE;

	ESP_LOGI(TAG, "Recorder task started");
  while (true) {
		AudioEvent event = getMicEvent();
		if (event.state == AUDIO_STATE_IDLE && event.flag == EMIC_START) {
			event.state = AUDIO_STATE_RUNNING;
			setMicEvent(event);
			ESP_LOGI(TAG, "Received audio event: %d", event.flag);
			key = millis();
			index = 0;
			ESP_LOGW(TAG, "status: ON, key: %d", key);
			
			// need add support for mqtt
			xTaskCreatePinnedToCore(micEvent, "recordEvent", 1024 * 4, nullptr, 0, &recordEventHandle, 1);
		} 
		else if (event.state == AUDIO_STATE_RUNNING && event.flag == EMIC_STOP) {
			ESP_LOGW(TAG, "status: OFF, key: %d, last index: %d", key, index);
			vTaskDelay(pdMS_TO_TICKS(5));
			index = -1;
			
			// need add support for mqtt
			#if MQTT_ENABLE == 0
			{
				AudioData audioSamples = {
					.key = String(key),
					.data = nullptr,
					.length = 0,
					.stream = false
				};
				xQueueSend(audioChunkQueue, &audioSamples, portMAX_DELAY);
				recordEventHandle = nullptr;
			}
			#endif
			event.state = AUDIO_STATE_STOPPED;
			setMicEvent(event);
			goto end;
		}
		
		if ((event.flag != EMIC_START && event.state != AUDIO_STATE_RUNNING)
			|| !microphone 
			|| !wifiManager.isConnected()
			|| event.callback == nullptr) {
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
			if (cache.lastSampleLen == 0  // invalid cache data
				|| cache.lastSampleTime <= lastUpdate // oldest data
				|| cache.lastSampleTime == 0) goto unlock; // no data
			memcpy(readBuffer, cache.lastSample, cache.lastSampleLen); // copy new samples to chunk
			lastUpdate = cache.lastSampleTime;
			chunk = (uint8_t*)readBuffer;
		} else {
			chunk = nullptr;
		}

		// send data to callback
		if (event.callback)
			event.callback(key, index, chunk, maxSamples);
		index++;

		unlock:
		memset(readBuffer, 0, maxSamples);
		xSemaphoreGive(lock);
		end:
		vTaskDelayUntil(&lastWakeTime, updateFrequency);
		notification->send(TAG, 1);
  }
}