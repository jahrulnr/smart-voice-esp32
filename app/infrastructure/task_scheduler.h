#ifndef TASK_SCHEDULER_H
#define TASK_SCHEDULER_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * TaskScheduler class for centralized FreeRTOS task management
 * 
 * Centralizes task creation, priorities, and stack sizes for better organization.
 * Prevents scattered xTaskCreate calls throughout the codebase.
 */
class TaskScheduler {
public:
    /**
     * Initialize the task scheduler
     */
    static void init();

    /**
     * Start all application tasks
     * Call this after hardware initialization in main.cpp
     */
    static void startTasks();

    /**
     * Stop all tasks (for shutdown/cleanup)
     */
    static void stopTasks();

private:
    static TaskHandle_t mainTaskHandle;
    static bool tasksStarted;

    // Task functions
    static void mainTask(void* parameter);
};

#endif // TASK_SCHEDULER_H