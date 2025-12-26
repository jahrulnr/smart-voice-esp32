#include "app/tasks.h"
#include <esp_log.h>

void createTask(BackgroundTask* task);

void taskMonitorer(void* param){
	const char* TAG = "TaskMonitorer";

	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(100);
	unsigned long monitorTimer = millis();
	unsigned long monitorDelay = 10000;

	std::map<String, unsigned long> healtyCheck;
	for(auto task: tasks) {
		createTask(task);
		healtyCheck[task->name] = millis() + monitorDelay;
		vTaskDelay(10);
	}

	ESP_LOGI(TAG, "Monitorer task started");
	vTaskDelay(pdMS_TO_TICKS(monitorDelay));
	do {
		vTaskDelayUntil(&lastWakeTime, updateFrequency);

		// task switcher
		portYIELD_CORE(0);
		portYIELD_CORE(1);

		if (millis() - monitorTimer > monitorDelay) {
			monitorTimer = millis();

			for(auto task: tasks) {
				if (notification->hasSignal(task->name) && notification->signal(task->name) == 1) {
					healtyCheck[task->name] = monitorTimer + monitorDelay;
					continue;
				}

				unsigned long lastTaskUpdate = (monitorTimer - healtyCheck[task->name]) / 1000;
				if (monitorTimer - healtyCheck[task->name] > 1000) {
					ESP_LOGE(TAG, "Task %s is not healty [%ds], restart it", task->name, lastTaskUpdate);
					if (task->handle != nullptr) {
						vTaskDeleteWithCaps(task->handle);
						task->handle = nullptr;
						vTaskDelay(pdMS_TO_TICKS(10));
						createTask(task);
						healtyCheck[task->name] = monitorTimer + monitorDelay;
					} else {
						ESP_LOGE(TAG, "Failed to restart %s task. TaskHandle is null", task->name);
					}
				}
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
			notification->send(NOTIFICATION_RECORD, record);
		}
	}
	while(1);
}

void createTask(BackgroundTask* task){
	xTaskCreatePinnedToCoreWithCaps(
		task->task,
		task->name,
		task->stack,
		NULL,
		task->priority,
		&task->handle,
		task->core,
		task->caps
	);
}