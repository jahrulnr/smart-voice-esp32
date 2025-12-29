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

		// Handle any notifications that might be relevant to SR
		if (notification->has(NOTIFICATION_COMMAND)) {
			void* event = notification->consume(NOTIFICATION_COMMAND);
			if (event) {
				const char* command = (const char*)event;
				ESP_LOGI(TAG, "Received command notification: %s", command);

				// Handle command notifications if needed
				if (strcmp(command, "pause_sr") == 0) {
					ESP_LOGI(TAG, "Pausing speech recognition");
					SR::pause();
				} else if (strcmp(command, "resume_sr") == 0) {
					ESP_LOGI(TAG, "Resuming speech recognition");
					SR::resume();
				}
			}
		}
	}

	ESP_LOGE(TAG, "Main task exited unexpectedly");
	vTaskDeleteWithCaps(NULL);
}