#include "app/tasks.h"

void mainTask(void *param) {
	const char* TAG = "mainTask";

  TickType_t lastWakeTime = xTaskGetTickCount();
  TickType_t updateFrequency = pdMS_TO_TICKS(33);

	ESP_LOGI(TAG, "Main task started");
	while(1) {
		vTaskDelayUntil(&lastWakeTime, updateFrequency);
		notification->send(TAG, 1);

		if(getAfeState() == VAD_SPEECH) {
			int16_t* lastSample = microphone->getCache().lastSample;
			ESP_LOGI(TAG, "Speech level: %d, Last detected: %dms", 
				microphone->level(), millis() - getLastSpeech());
		}

		// watch event
		timeEvent();
		displayEvent();
		buttonEvent();
		srEvent();
	}

	ESP_LOGE(TAG, "Main task exited unexpectedly");
	vTaskDeleteWithCaps(NULL);
}