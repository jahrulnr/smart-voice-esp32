#include "task_scheduler.h"
#include "infrastructure/logger.h"
#include "network/wifi_manager.h"
#include "network/web_server.h"
#include "application/gpt_service.h"
#include "audio/microphone.h"
#include "audio/speaker.h"
#include "voice/pico_tts.h"
#include "network/ftp_server.h"
#include "ui/display.h"

// External references
extern Microphone mic;
extern Speaker speaker;
extern PicoTTS tts;
extern FtpServer ftpServer;
extern WifiManager wifiManager;
extern WebServerService webServerService;
extern Services::GPTService gptService;
extern DisplayManager displayManager;

// Static member initialization
TaskHandle_t TaskScheduler::mainTaskHandle = nullptr;
bool TaskScheduler::tasksStarted = false;

void TaskScheduler::init() {
    Logger::info("TASK_SCHED", "Task scheduler initialized");
}

void TaskScheduler::startTasks() {
    if (tasksStarted) {
        Logger::warn("TASK_SCHED", "Tasks already started");
        return;
    }

    Logger::info("TASK_SCHED", "Starting application tasks...");

    // Create main application task
    BaseType_t result = xTaskCreatePinnedToCoreWithCaps(
        mainTask,           // Task function
        "MainAppTask",      // Task name
        4096,               // Stack size (bytes)
        NULL,               // Parameters
        3,                  // Priority (1 = low, higher numbers = higher priority)
        &mainTaskHandle,    // Task handle
        1,
        MALLOC_CAP_SPIRAM
    );

    if (result != pdPASS) {
        Logger::error("TASK_SCHED", "Failed to create main task");
        return;
    }

    tasksStarted = true;
    Logger::info("TASK_SCHED", "All tasks started successfully");
}

void TaskScheduler::stopTasks() {
    if (!tasksStarted) {
        return;
    }

    Logger::info("TASK_SCHED", "Stopping tasks...");

    if (mainTaskHandle != nullptr) {
        vTaskDelete(mainTaskHandle);
        mainTaskHandle = nullptr;
    }

    tasksStarted = false;
    Logger::info("TASK_SCHED", "Tasks stopped");
}

void TaskScheduler::mainTask(void* parameter) {
    Logger::info("MAIN_TASK", "Main application task started");

    // Play a test tone on startup
    Logger::info("MAIN_TASK", "Playing test tone...");
    speaker.playTone(440, 500, 0.3f);  // A4 note, 500ms, 30% volume
    displayManager.onEvent(EventData(EventType::STATE_CHANGE, "Test tone", static_cast<int>(DisplayState::SPEAKING)));
    vTaskDelay(pdMS_TO_TICKS(1000));  // Wait 1 second

    // Speak a test message
    Logger::info("MAIN_TASK", "Speaking test message...");
    displayManager.onEvent(EventData(EventType::STATE_CHANGE, "Welcome message", static_cast<int>(DisplayState::SPEAKING)));
    tts.speak("Hello, ESP32 voice assistant is ready.");
    vTaskDelay(pdMS_TO_TICKS(5000));  // Wait for speech to complete

    // Demo GPT functionality if API key is configured
    if (gptService.isInitialized()) {
        Logger::info("MAIN_TASK", "Testing GPT service...");
        displayManager.onEvent(EventData(EventType::STATE_CHANGE, "Testing GPT", static_cast<int>(DisplayState::PROCESSING)));
        gptService.sendPrompt("Say hello and introduce yourself as an ESP32 voice assistant in one short sentence.",
            [](const String& response) {
                Logger::info("GPT", "Response: %s", response.c_str());
                // Note: displayManager is not accessible in this lambda
                // We'll need to handle GPT responses differently
                tts.speak(response.c_str());
            });
        vTaskDelay(pdMS_TO_TICKS(10000));  // Wait for GPT response and TTS
    }

    while (true) {
        // Update display
        displayManager.update();

        // Handle WiFi tasks
        wifiManager.handle();

        // Handle web server requests
        webServerService.handle();

        // Handle FTP requests
        ftpServer.handle();

        // Main application logic
        int level = mic.readLevel();
        if (level >= 0) {
            Logger::debug("MAIN_TASK", "Mic level: %d", level);
            bool isActive = level > 10;
            displayManager.onEvent(EventData(EventType::VOICE_ACTIVITY, "", level, &isActive));  // Show voice activity
        }

        // Periodic status updates (every ~5 seconds)
        static unsigned long lastStatusUpdate = 0;
        if (millis() - lastStatusUpdate > 5000) {
            // Memory status
            size_t freeHeap = ESP.getFreeHeap();
            size_t totalHeap = ESP.getHeapSize();
            int memPercent = ((totalHeap - freeHeap) * 100) / totalHeap;
            displayManager.onEvent(EventData(EventType::MEMORY_WARNING, "", memPercent, &freeHeap));

            // Component status updates
            int statusReady = 1;
            displayManager.onEvent(EventData(EventType::STATUS_UPDATE, "", static_cast<int>(StatusType::MICROPHONE), &statusReady));  // Assume ready
            displayManager.onEvent(EventData(EventType::STATUS_UPDATE, "", static_cast<int>(StatusType::SPEAKER), &statusReady));    // Assume ready
            int gptStatus = gptService.isInitialized() ? 1 : 0;
            displayManager.onEvent(EventData(EventType::STATUS_UPDATE, "", static_cast<int>(StatusType::GPT_SERVICE), &gptStatus));

            // WiFi status update
            int wifiStatus = wifiManager.isConnected() ? 1 : 0;
            String ipAddress = wifiManager.getIPAddress();
            displayManager.onEvent(EventData(EventType::STATUS_UPDATE, ipAddress, static_cast<int>(StatusType::WIFI), &wifiStatus));

            lastStatusUpdate = millis();
        }

        // TODO: Add voice command recognition here
        // When voice commands are recognized, they can be sent to GPT like:
        // gptService.sendPrompt(recognizedCommand, [](const String& response) {
        //     tts.speak(response);
        // });

        // Add voice processing, UI updates, etc. here later
        vTaskDelay(pdMS_TO_TICKS(100));  // Shorter delay for responsive handling
    }
}