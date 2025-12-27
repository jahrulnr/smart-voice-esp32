#include "app/tasks.h"

void mainTask(void *param) {
	const char* TAG = "mainTask";

  TickType_t lastWakeTime = xTaskGetTickCount();
  TickType_t updateFrequency = pdMS_TO_TICKS(33);
	int lastHour = -1;

	unsigned long time5 = millis();
	unsigned long time10 = millis();
	bool record = false;

	ESP_LOGI(TAG, "Main task started");
	while(1) {
		vTaskDelayUntil(&lastWakeTime, updateFrequency);
		notification->send(TAG, 1);

		if(getAfeState() == VAD_SPEECH) {
			int16_t* lastSample = microphone->getCache().lastSample;
			ESP_LOGI(TAG, "Speech level: %d, Last detected: %dms", 
				microphone->level(), millis() - getLastSpeech());
			if(!record) notification->send(NOTIFICATION_RECORD, 0);
			record = true;
		} else {
			if (record) notification->send(NOTIFICATION_RECORD, 1);
			record = false;
		}

		int hour = timeManager.getHour();
		if (lastHour != hour && hour >= 0) {
			lastHour = hour;

			String speakCmd = "It is " + String(hour) + " o'clock. ";
			if (hour == 9) {
				speakCmd += "It is time to work.";
			}
			else if (hour == 22) {
				speakCmd += "It is time to sleep.";
			}
			else if (hour == 5) {
				speakCmd += "It is time for the Subuh prayer.";
			}
			else if (hour == 12) {
				speakCmd += "It is time for the Dzuhur prayer.";
			}
			else if (hour == 15) {
				speakCmd += "It is time for the Ashar prayer.";
			}
			else if (hour == 18) {
				speakCmd += "It is time for the Maghrib prayer.";
			}
			else if (hour == 19) {
				speakCmd += "It is time for the Isya prayer.";
			}

			// tts.speak(speakCmd.c_str());
			ai.sendPrompt("Tell this, but as a expresif girl (max 100 char): " +speakCmd, aiCallback);
			updateActivity();
		}

		// handle display
		displayEvent();
		// update button
		button.update();

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