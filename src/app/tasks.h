#pragma once

#include <esp_log.h>
#include <WiFi.h>

#include <core/time.h>
#include "boot/init.h"
#include <esp_log.h>
#include <esp32-hal-log.h>
#include <WiFi.h>

typedef void (*SomeTask)(void* param);

struct BackgroundTask {
	const char* name;
	TaskHandle_t handle;
	SomeTask task;
	uint32_t stack;
	BaseType_t core;
	UBaseType_t priority;
	UBaseType_t caps;
};

extern TaskHandle_t taskMonitorerHandle;
extern std::vector<BackgroundTask*> tasks;
extern QueueHandle_t audioQueue;

void runTasks();

void taskMonitorer(void* param);
void mainTask(void *param);
void networkTask(void *param);
void recorderTask(void* param);

struct AudioSamples {
	String key;
	uint8_t* data;
	size_t length;
	bool stream;
};