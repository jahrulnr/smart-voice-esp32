#include <Arduino.h>
#include "boot/init.h"
#include "app/tasks.h"
#include <LittleFS.h>

void setup() {
  setCpuFrequencyMhz(240);
  Serial.setTimeout(50);
	Serial.begin(115200);
  LittleFS.begin(true);

  #if BOARD_HAS_PSRAM
  heap_caps_malloc_extmem_enable(512);
  #endif

  setupApp();
  runTasks();
}

void loop() {
  disableLoopWDT();
  vTaskDelete(NULL);
}