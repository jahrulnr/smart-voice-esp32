#include <Arduino.h>
#include <core/core.h>
#include "boot/init.h"
#include "app/tasks.h"
#include <LittleFS.h>
#include <WiFi.h>

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

	WiFi.begin(WIFI_SSID, WIFI_PASS);
  WiFi.setSleep(WIFI_PS_NONE);
  WiFi.setAutoReconnect(true);
  WiFi.setHostname("pio-assistant.local");
  WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SECURITY);
  WiFi.setDNS(IPAddress(1, 1, 1, 1), IPAddress(8, 8, 8, 8));
  WiFi.enableLongRange(true);
  WiFi.enableProv(true);
  WiFi.enableIPv6();
  WiFi.persistent(false);

  #if BOARD_HAS_PSRAM
  heap_caps_malloc_extmem_enable(512);
  #endif
}

void loop() {
  vTaskDelete(NULL);
  log_e("Loop task deleted");
}