#include "app/tasks.h"
#include <esp_log.h>
#include <soc/rtc.h>

void createTask(BackgroundTask* task);
void resumeTasks();
void pauseTasks();

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

		auto sysLastUpdate = sysActivity.lastUpdate();
#if ENABLE_POWERSAVE
		if (sysLastUpdate > 60000 && getCpuFrequencyMhz() != 80) {
			pauseTasks();
			// 240, 160, 120, 80
			setCpuFrequencyMhz(80);
			notification->send(NOTIFICATION_DISPLAY, (int) EDISPLAY_SLEEP);
			ESP_LOGI(TAG, "Display sleep triggered, downclock cpu to %dMhz, last activity: %ds", getCpuFrequencyMhz(), sysLastUpdate / 1000);
		} else if (sysLastUpdate <= 60000 && getCpuFrequencyMhz() != 240) {
			setCpuFrequencyMhz(240);
			resumeTasks();
		}
#endif

		if (millis() - monitorTimer > monitorDelay) {
			monitorTimer = millis();

			// skip when sleep mode
			if (sysLastUpdate > 60000) {
				continue;
			}

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


void resumeTasks(){
	for(auto task: tasks) {
		if (task->suspendable && task->handle != nullptr) {
			vTaskResume(task->handle);
		}
	}
}

void pauseTasks(){
	for(auto task: tasks) {
		if (task->suspendable && task->handle != nullptr) {
			vTaskSuspend(task->handle);
		}
	}
}