#include "app/tasks.h"
#include <esp_log.h>

const char* taskStatusName(eTaskState state);

void taskMonitorer(void* param){
	const char* TAG = "TaskMonitorer";

	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(100);
	size_t monitorTimer = millis();

	for(auto task: tasks) {
		xTaskCreatePinnedToCoreWithCaps(
			task.task,
			task.name,
			task.stack,
			NULL,
			task.priority,
			&task.handle,
			task.core,
			task.caps
		);
	}

	vTaskDelay(pdMS_TO_TICKS(10000));
	do {
		vTaskDelayUntil(&lastWakeTime, updateFrequency);

		portYIELD_CORE(0);
		vTaskDelay(1);
		portYIELD_CORE(1);

		if (millis() - monitorTimer > 10000) {
			// printTaskStatus();
			// notification->send("WiFi check", 1);
			monitorTimer = millis();

			// vTaskGetInfo()
			for(auto task: tasks) {
				TaskStatus_t taskStatus;
				vTaskGetInfo(task.handle, &taskStatus, pdTRUE, eInvalid);
				ESP_LOGI(TAG, "Task %s: %s", task.name, taskStatusName(taskStatus.eCurrentState));
			}

			static int record = 0;
			if (record == 0) {
				record = 1;
				SR::resume();
			}
			else {
				record = 0;
				SR::pause();
			}
			ESP_LOGW(TAG, "status: %s", record == 0 ? "ON" : "OFF");
			notification->send(NOTIFICATION_RECORD, record);
		}
	}
	while(1);
}

const char* taskStatusName(eTaskState state) {
	switch (state) {
		case eReady: return "Ready";
		case eRunning: return "Running";
		case eBlocked: return "Blocked";
		case eSuspended: return "Suspended";
		case eDeleted: return "Deleted";
		case eInvalid: return "Invalid";
	}

	return "Unknown";
}