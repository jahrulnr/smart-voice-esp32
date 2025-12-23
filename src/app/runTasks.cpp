#include "tasks.h"

TaskHandle_t taskMonitorerHandle = nullptr;
TaskHandle_t mainTaskHandle = nullptr;
TaskHandle_t networkTaskHandle = nullptr;

void runTasks(){
	xTaskCreatePinnedToCoreWithCaps(
		mainTask,
		"mainTask",
		1024 * 4,
		NULL,
		6,
		&mainTaskHandle,
		tskNO_AFFINITY,
		MALLOC_CAP_SPIRAM
	);

	vTaskDelay(100);
	xTaskCreatePinnedToCoreWithCaps(
		networkTask,
		"networkTask",
		1024 * 4,
		NULL,
		1,
		&networkTaskHandle,
		tskNO_AFFINITY,
		MALLOC_CAP_SPIRAM
	);

	vTaskDelay(100);
	xTaskCreatePinnedToCoreWithCaps(
		taskMonitorer,
		"taskMonitorer",
		1024 * 3,
		NULL,
		0,
		&taskMonitorerHandle,
		tskNO_AFFINITY,
		MALLOC_CAP_SPIRAM
	);
}