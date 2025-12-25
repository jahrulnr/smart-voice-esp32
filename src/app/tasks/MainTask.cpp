#include "app/tasks.h"
#include <app/display/ui/main.h>

void mainTask(void *param) {
	const char* TAG = "mainTask";

  TickType_t lastWakeTime = xTaskGetTickCount();
  TickType_t updateFrequency = pdMS_TO_TICKS(60);
	size_t updateDelay = 0;
	const char* lastEvent;

	MainStatusDrawer mainDisplay = MainStatusDrawer(display);

	// wait notification initiate
	while (!notification)
		vTaskDelay(1);

	while(1) {
		vTaskDelayUntil(&lastWakeTime, updateFrequency);

		if (notification->has(NOTIFICATION_WEATHER)) {
			weatherData_t* data = (weatherData_t*) notification->consume(NOTIFICATION_WEATHER, 10);
			mainDisplay.updateData(data);
		}

		display->clearBuffer();

		if (!notification->has(NOTIFICATION_DISPLAY) && updateDelay == 0) {
			mainDisplay.draw();
	    display->sendBuffer();
			continue;
		}

		if (updateDelay <= millis()) {
			updateDelay = 0;
			lastEvent = "";
			notification->consume(NOTIFICATION_DISPLAY, 0);
		}

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
}