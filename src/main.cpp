#include <Arduino.h>
#include <core/core.h>
#include "boot/init.h"
#include "app/tasks.h"
#include <LittleFS.h>

// void init(){
//   esp_panic_handler_disable_timg_wdts();
// }

void setup() {
  // setCpuFrequencyMhz(240);
	// Serial.begin(115200);
  LittleFS.begin(true);
	Wire.begin(SDA_PIN, SCL_PIN);

  setupApp();
  runTasks();

  #if BOARD_HAS_PSRAM
  heap_caps_malloc_extmem_enable(512);
  #endif
}

void loop() {
  vTaskDelete(NULL);
  log_e("Loop task deleted");
}