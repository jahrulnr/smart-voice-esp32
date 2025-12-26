#include <Arduino.h>
#include <core/wdt.h>
#include <core/time.h>
#include "core/nvs.h"
#include "boot/init.h"
#include "app/tasks.h"
#include <LittleFS.h>
#include <nvs_flash.h>

void init(){
  // esp_panic_handler_disable_timg_wdts();

  nvs_init();
}

void setup() {
  LittleFS.begin(true);
	Wire.begin(SDA_PIN, SCL_PIN);
  timeManager.init();
			
  setupApp();
  runTasks();

  #if BOARD_HAS_PSRAM
  heap_caps_malloc_extmem_enable(4096);
  #endif
}

void loop() {
  vTaskDelete(NULL);
  log_e("Loop task deleted");
}