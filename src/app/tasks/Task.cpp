#include "app/tasks.h"
#include <TaskManager.h>
#include <esp_log.h>

void cleanupTasks();
void printTaskStatus();

void taskMonitorer(void* param){
	TickType_t lastWakeTime = xTaskGetTickCount();
	TickType_t updateFrequency = pdMS_TO_TICKS(10);
	size_t monitorTimer = millis();

	vTaskDelay(pdMS_TO_TICKS(10000));
	do {
		vTaskDelayUntil(&lastWakeTime, updateFrequency);

		// portYIELD_CORE(0);
		// vTaskDelay(1);
		// portYIELD_CORE(1);

		if (millis() - monitorTimer > 10000) {
			cleanupTasks();
			printTaskStatus();
			// notification->send("WiFi check", 1);
			monitorTimer = millis();

			static int record = 0;
			if (record == 0) {
				record = 1;
				SR::resume();
			}
			else {
				record = 0;
				SR::pause();
			}
			ESP_LOGW("Record", "status: %s", record == 0 ? "ON" : "OFF");
			notification->send(NOTIFICATION_RECORD, record);
		}
	}
	while(1);
}

/**
 * Print status of all tasks managed by TaskManager library
 */
void printTaskStatus() {
	log_i("=== Task Status Report ===");

	// Scan for external tasks before getting all tasks
	TaskManager::scanExternalTasks();

	// Update memory usage for all tasks
	do { TaskManager::updateAllTasksMemoryUsage(); }while(0);

	auto allTasks = TaskManager::getAllTasks();

	if (allTasks.empty()) {
		log_i("No tasks registered in TaskManager library");
		return;
	}

	log_i("Total tasks: %d", allTasks.size());

	// Count external vs internal tasks
	int externalCount = 0;
	int internalCount = 0;
	for (const auto& task : allTasks) {
		if (task.isExternal) {
			externalCount++;
		} else {
			internalCount++;
		}
	}

	log_i("Task Types - Internal: %d, External: %d", internalCount, externalCount);

	// Print status summary
	int waiting = TaskManager::getTaskCountByStatus(TaskManager::TaskStatus::WAITING);
	int inProgress = TaskManager::getTaskCountByStatus(TaskManager::TaskStatus::INPROGRESS);
	int done = TaskManager::getTaskCountByStatus(TaskManager::TaskStatus::DONE);
	int failed = TaskManager::getTaskCountByStatus(TaskManager::TaskStatus::FAILED);
	int paused = TaskManager::getTaskCountByStatus(TaskManager::TaskStatus::PAUSED);
	int external = TaskManager::getTaskCountByStatus(TaskManager::TaskStatus::EXTERNAL_TASK);

	log_i("Status Summary - Waiting: %d, Running: %d, Done: %d, Failed: %d, Paused: %d, External: %d",
				waiting, inProgress, done, failed, paused, external);

	// Print tasks by core
	auto cpu0Tasks = TaskManager::getTasksByCore(0);
	auto cpu1Tasks = TaskManager::getTasksByCore(1);
	auto anyCoreTasks = TaskManager::getTasksByCore(tskNO_AFFINITY);

	log_i("CPU 0 tasks: %d, CPU 1 tasks: %d, Any core tasks: %d",
				cpu0Tasks.size(), cpu1Tasks.size(), anyCoreTasks.size());

	log_i("SRAM Free: %d kilo bytes, PSRAM Free: %d kilo bytes",
				ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024 );

	// Calculate total memory usage
	uint32_t totalStackAllocated = 0;
	uint32_t totalStackUsed = 0;
	for (const auto& task : allTasks) {
		totalStackAllocated += task.stackSize;
		totalStackUsed += task.stackUsed;
	}

	log_i("Memory Usage - Total Stack: %u kilo bytes, Used: %u kilo bytes (%.1f%%)",
				totalStackAllocated / 1024, totalStackUsed  / 1024,
				totalStackAllocated > 0 ? (float)totalStackUsed * 100.0f / totalStackAllocated : 0.0f);

	// Print table header
	log_i("| %-18s | %-4s | %-7s | %-4s | %-8s | %-10s | %-18s | %-7s | %-7s | %-9s |",
				"Task Name", "Type", "Status", "Core", "Priority", "Runtime", "Memory Used/Total", "Usage %", "Free", "Notes");
	log_i("|%-20s|%-6s|%-9s|%-6s|%-10s|%-12s|%-20s|%-9s|%-9s|%-11s|",
				"--------------------", "------", "---------", "------", "----------", "------------", "--------------------", "---------", "---------", "-----------");

	// Print detailed task information in table format
	for (const auto& task : allTasks) {
		const char* statusStr = "UNKNOWN";
		switch (task.status) {
			case TaskManager::TaskStatus::WAITING: statusStr = "WAITING"; break;
			case TaskManager::TaskStatus::INPROGRESS: statusStr = "RUNNING"; break;
			case TaskManager::TaskStatus::DONE: statusStr = "DONE"; break;
			case TaskManager::TaskStatus::FAILED: statusStr = "FAILED"; break;
			case TaskManager::TaskStatus::PAUSED: statusStr = "PAUSED"; break;
			case TaskManager::TaskStatus::EXTERNAL_TASK: statusStr = "EXTERNAL"; break;
		}

		unsigned long runtime = 0;
		if (task.startedAt > 0) {
			if (task.completedAt > 0) {
				runtime = task.completedAt - task.startedAt;
			} else {
				runtime = millis() - task.startedAt;
			}
		}

		const char* taskType = task.isExternal ? "EXT" : "INT";

		// Calculate memory usage percentage
		float memUsagePercent = 0.0f;
		if (task.stackSize > 0) {
			memUsagePercent = (float)task.stackUsed * 100.0f / task.stackSize;
		}

		// Format core display
		char coreStr[8];
		if (task.coreId == tskNO_AFFINITY) {
			snprintf(coreStr, sizeof(coreStr), "ANY");
		} else {
			snprintf(coreStr, sizeof(coreStr), "%d", task.coreId);
		}

		// Format runtime display
		char runtimeStr[12];
		snprintf(runtimeStr, sizeof(runtimeStr), "%lums", runtime);

		// Format memory display
		char memoryStr[20];
		snprintf(memoryStr, sizeof(memoryStr), "%u/%u bytes", task.stackUsed, task.stackSize);

		// Format notes
		char notesStr[32] = "";
		if (task.isExternal && (task.name == "cam_task" || task.name.indexOf("camera") >= 0)) {
			strcat(notesStr, "CAMERA");
		}
		if (memUsagePercent > 80.0f) {
			if (strlen(notesStr) > 0) strcat(notesStr, " ");
			strcat(notesStr, "HIGH MEM!");
		}

		log_i("| %-18s | %-4s | %-7s | %-4s | %8d | %-10s | %-18s | %6.1f%% | %7u | %-9s |",
				task.name.c_str(), taskType, statusStr, coreStr,
				task.priority, runtimeStr, memoryStr, memUsagePercent, task.stackFreeMin, notesStr);
	}

	log_i("=== End Task Status Report ===");
}

/**
 * Clean up completed and failed tasks to free memory
 */
void cleanupTasks() {
	int beforeCount = TaskManager::getTaskCount();
	TaskManager::cleanupCompletedTasks();
	int afterCount = TaskManager::getTaskCount();

	int cleanedUp = beforeCount - afterCount;
	if (cleanedUp > 0) {
		log_i("Cleaned up %d completed/failed tasks", cleanedUp);
	}
}