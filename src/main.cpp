#include <Arduino.h>
#include "boot/init.h"
#include "app/tasks.h"
#include <LittleFS.h>
#include <WiFi.h>

#include "hal/timer_hal.h"
#include "hal/wdt_types.h"
#include "hal/wdt_hal.h"
#include <esp_task_wdt.h>

void esp_panic_handler_disable_timg_wdts(void)
{
    wdt_hal_context_t wdt0_context = {.inst = WDT_MWDT0, .mwdt_dev = &TIMERG0};
    wdt_hal_write_protect_disable(&wdt0_context);
    wdt_hal_disable(&wdt0_context);
    wdt_hal_write_protect_enable(&wdt0_context);

    wdt_hal_context_t wdt1_context = {.inst = WDT_MWDT1, .mwdt_dev = &TIMERG1};
    wdt_hal_write_protect_disable(&wdt1_context);
    wdt_hal_disable(&wdt1_context);
    wdt_hal_write_protect_enable(&wdt1_context);
}

void setup() {
  setCpuFrequencyMhz(240);
	Serial.begin(115200);
  LittleFS.begin(true);

	Wire.begin(SDA_PIN, SCL_PIN);
	WiFi.begin("ANDROID AP", "tes12345");

	// Configure Task Watchdog Timer to prevent system hangs
	esp_err_t esp_task_wdt_reconfigure(const esp_task_wdt_config_t *config);
	esp_task_wdt_config_t config = {
			.timeout_ms = 5 * 1000,
			.trigger_panic = false,
	};
	esp_task_wdt_reconfigure(&config);
  esp_panic_handler_disable_timg_wdts();

  #if BOARD_HAS_PSRAM
  heap_caps_malloc_extmem_enable(512);
  #endif

  setupApp();
  runTasks();
  vTaskDelete(NULL);
}

void loop() {}