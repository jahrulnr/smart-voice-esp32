#include <Arduino.h>
#include <LittleFS.h>
#include "config.h"
#include "infrastructure/logger.h"
#include "infrastructure/boot_manager.h"
#include "infrastructure/task_scheduler.h"
#include "infrastructure/time_manager.h"
#include "infrastructure/sleep_manager.h"
#include "network/wifi_manager.h"
#include "network/ftp_server.h"
#include "network/web_server.h"
#include "network/mqtt_client.h"
#include "application/gpt_service.h"
#include "application/voice_display_coordinator.h"
#include "application/weather_service.h"
#include "audio/microphone.h"
#include "audio/speaker.h"
#include "voice/pico_tts.h"
#include "voice/voice_command_handler.h"

// Global instances
Microphone mic;
Speaker speaker;
PicoTTS tts;
VoiceCommandHandler voiceCommandHandler;
FtpServer ftpServer(LittleFS);
WifiManager wifiManager;
WebServerService webServerService;
MqttClient mqttClient;
Services::GPTService gptService;
Services::WeatherService weatherService;
DisplayManager displayManager;
VoiceDisplayCoordinator voiceDisplayCoordinator;
SleepManager sleepManager;
BootManager& bootManager = BootManager::getInstance();
TimeManager& timeManager = TimeManager::getInstance();

void setup() {
    // Initialize boot manager and execute boot sequence
    if (!bootManager.init()) {
        // If boot manager fails to initialize, we can't even log
        // This is a critical failure - system cannot continue
        while (true) {
            delay(1000); // Infinite loop to indicate critical failure
        }
    }

    // Execute the complete boot sequence
    if (!bootManager.bootSystem()) {
        Logger::error("MAIN", "System boot failed - check initialization results");
        // Continue anyway for debugging, but system may not be fully functional
    }

    Logger::info("MAIN", "ESP32 Voice Assistant startup complete");
}

void loop() {
    // loop() not used - all logic in FreeRTOS tasks
    vTaskDelete(NULL);
}