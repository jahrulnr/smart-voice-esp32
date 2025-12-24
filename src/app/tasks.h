#pragma once

#include <esp_log.h>
#include <WiFi.h>

#include <core/time.h>
#include "boot/init.h"
#include "display_list.h"
#include <esp_log.h>
#include <esp32-hal-log.h>
#include <WiFi.h>

extern TaskHandle_t taskMonitorerHandle;
extern TaskHandle_t mainTaskHandle;
extern TaskHandle_t networkTaskHandle;

void runTasks();

void taskMonitorer(void* param);
void mainTask(void *param);
void networkTask(void *param);
