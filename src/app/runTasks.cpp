#include "tasks.h"

void runTasks(){
	xTaskCreateUniversal(
		mainTask,
		"mainTask",
		1024 * 4,
		NULL,
		4,
		&mainTaskHandle,
		1
	);
}