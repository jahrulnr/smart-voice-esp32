#include "tasks.h"

TaskHandle_t taskMonitorerHandle = nullptr;
TaskHandle_t mainTaskHandle = nullptr;
TaskHandle_t networkTaskHandle = nullptr;
TaskHandle_t recorderTaskHandle = nullptr;
QueueHandle_t audioQueue = nullptr;

void runTasks(){
	audioQueue = xQueueCreateWithCaps(5, sizeof(AudioSamples), MALLOC_CAP_SPIRAM);

	xTaskCreatePinnedToCoreWithCaps(
		mainTask,
		"mainTask",
		1024 * 4,
		NULL,
		6,
		&mainTaskHandle,
		1,
		MALLOC_CAP_SPIRAM
	);

	vTaskDelay(100);
	xTaskCreatePinnedToCoreWithCaps(
		networkTask,
		"networkTask",
		1024 * 8,
		NULL,
		1,
		&networkTaskHandle,
		0,
		MALLOC_CAP_INTERNAL
	);

	vTaskDelay(100);
	xTaskCreatePinnedToCoreWithCaps(
		taskMonitorer,
		"taskMonitorer",
		1024 * 3,
		NULL,
		0,
		&taskMonitorerHandle,
		1,
		MALLOC_CAP_SPIRAM
	);

	vTaskDelay(100);
	xTaskCreatePinnedToCoreWithCaps(
		recorderTask,
		"recorderTask",
		1024 * 3,
		NULL,
		15,
		&recorderTaskHandle,
		1,
		MALLOC_CAP_SPIRAM
	);
}