#include "tasks.h"

TaskHandle_t taskMonitorerHandle = nullptr;
std::vector<BackgroundTask*> tasks;
QueueHandle_t audioQueue = nullptr;

void runTasks(){
	audioQueue = xQueueCreateWithCaps(20, sizeof(AudioSamples), MALLOC_CAP_SPIRAM);

	tasks.push_back(new BackgroundTask{
		.name = "mainTask",
		.handle = nullptr,
		.task = mainTask,
		.stack = 1024 * 4,
		.core = 1,
		.priority = 6,
		.caps = MALLOC_CAP_INTERNAL
	});
	tasks.push_back(new BackgroundTask{
		.name = "networkTask",
		.handle = nullptr,
		.task = networkTask,
		.stack = 1024 * 8,
		.core = 0,
		.priority = 0,
		.caps = MALLOC_CAP_INTERNAL
	});
	tasks.push_back(new BackgroundTask{
		.name = "recorderTask",
		.handle = nullptr,
		.task = recorderTask,
		.stack = 1024 * 3,
		.core = 1,
		.priority = 5,
		.caps = MALLOC_CAP_SPIRAM
	});

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
}