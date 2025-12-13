#include "boot_manager.h"
#include "config.h"
#include "audio/microphone.h"
#include "audio/speaker.h"
#include "voice/pico_tts.h"
#include "voice/voice_command_handler.h"
#include "network/wifi_manager.h"
#include "network/ftp_server.h"
#include "network/web_server.h"
#include "application/gpt_service.h"
#include "ui/display.h"
#include "application/voice_display_coordinator.h"
#include "infrastructure/task_scheduler.h"
#include "infrastructure/touch_sensor.h"
#include <esp_task_wdt.h>

// Global instances (forward declarations - actual instances in main.cpp)
extern Microphone mic;
extern Speaker speaker;
extern PicoTTS tts;
extern VoiceCommandHandler voiceCommandHandler;
extern FtpServer ftpServer;
extern WifiManager wifiManager;
extern WebServerService webServerService;
extern Services::GPTService gptService;
extern DisplayManager displayManager;
extern VoiceDisplayCoordinator voiceDisplayCoordinator;

BootManager& BootManager::getInstance() {
    static BootManager instance;
    return instance;
}

bool BootManager::init() {
    if (initialized) {
        Logger::warn("BOOT", "Boot manager already initialized");
        return true;
    }

    Logger::info("BOOT", "Initializing boot manager...");

    // Set CPU frequency to 240MHz for optimal performance
    setCpuFrequencyMhz(240);

    // Disable external memory allocation to prevent heap fragmentation
    heap_caps_malloc_extmem_enable(128);

    // Configure Task Watchdog Timer to prevent system hangs
    // Timeout: 120 seconds (2 minutes), no panic on timeout
    esp_err_t esp_task_wdt_reconfigure(const esp_task_wdt_config_t *config);
    esp_task_wdt_config_t config = {
        .timeout_ms = 120 * 1000,  // 120 seconds
        .trigger_panic = false,     // Don't panic, just reset
    };
    esp_task_wdt_reconfigure(&config);

    // Register PRE_INIT phase components
    registerComponent(BootPhase::PRE_INIT, []() -> InitResult {
        Logger::init();
        Logger::setLogLevel(Logger::INFO);
        Logger::info("BOOT", "Logger initialized");
        return {true, "Logger", "", false};
    }, "Logger", false);

    registerComponent(BootPhase::PRE_INIT, []() -> InitResult {
        if (!displayManager.init()) {
            return {false, "Display", "Failed to initialize display", false};
        }
        Logger::info("BOOT", "Display initialized");
        return {true, "Display", "", false};
    }, "Display", false);

    registerComponent(BootPhase::PRE_INIT, []() -> InitResult {
        // Show boot splash for 3 seconds
        unsigned long splashStart = millis();
        while (millis() - splashStart < 3000) {
            displayManager.update();
            vTaskDelay(pdMS_TO_TICKS(50)); // Small delay to prevent busy loop
        }
        // Transition to main status after splash
        displayManager.setState(DisplayState::MAIN_STATUS);
        Logger::info("BOOT", "Boot splash completed");
        return {true, "BootSplash", "", false};
    }, "BootSplash", false);

    registerComponent(BootPhase::PRE_INIT, []() -> InitResult {
        TaskScheduler::init();
        Logger::info("BOOT", "Task scheduler initialized");
        return {true, "TaskScheduler", "", true};
    }, "TaskScheduler", true);

    // Register HARDWARE_INIT phase components
    registerComponent(BootPhase::HARDWARE_INIT, []() -> InitResult {
        if (!mic.init()) {
            return {false, "Microphone", "Failed to initialize microphone", true};
        }
        if (!mic.start()) {
            return {false, "Microphone", "Failed to start microphone", true};
        }
        Logger::info("BOOT", "Microphone initialized");
        return {true, "Microphone", "", true};
    }, "Microphone", true);

    registerComponent(BootPhase::HARDWARE_INIT, []() -> InitResult {
        if (!speaker.init()) {
            return {false, "Speaker", "Failed to initialize speaker", true};
        }
        if (!speaker.start()) {
            return {false, "Speaker", "Failed to start speaker", true};
        }
        Logger::info("BOOT", "Speaker initialized");
        return {true, "Speaker", "", true};
    }, "Speaker", true);

    registerComponent(BootPhase::HARDWARE_INIT, []() -> InitResult {
        if (!tts.init(&speaker)) {
            return {false, "TTS", "Failed to initialize TTS", true};
        }
        Logger::info("BOOT", "TTS initialized");
        return {true, "TTS", "", true};
    }, "TTS", true);

    registerComponent(BootPhase::HARDWARE_INIT, []() -> InitResult {
        if (!TouchSensor::init()) {
            return {false, "TouchSensor", "Failed to initialize touch sensor", false};
        }
        return {true, "TouchSensor", "", false};
    }, "TouchSensor", false);

    // Register NETWORK_INIT phase components
    registerComponent(BootPhase::NETWORK_INIT, []() -> InitResult {
        if (!wifiManager.init()) {
            return {false, "WiFi", "Failed to initialize WiFi manager", true};
        }
        wifiManager.begin();
        Logger::info("BOOT", "WiFi initialized");
        return {true, "WiFi", "", true};
    }, "WiFi", true);

    registerComponent(BootPhase::NETWORK_INIT, []() -> InitResult {
        if (!webServerService.init(wifiManager.getWebServer(), &gptService)) {
            return {false, "WebServer", "Failed to initialize web server", false};
        }
        Logger::info("BOOT", "Web server initialized");
        return {true, "WebServer", "", false};
    }, "WebServer", false);

    // Register SERVICE_INIT phase components
    registerComponent(BootPhase::SERVICE_INIT, []() -> InitResult {
        // Voice commands generated using ESP-IDF G2P tool
        VoiceCommand commands[] = {
            {0, "hello", "hcLb"},
            {1, "time", "TiM"},
            {2, "weather", "Wfjk"},
            {3, "music", "MYoZgK"},
            {4, "stop", "STnP"},
            {5, "help", "hfLP"}
        };

        VoiceConfig voiceConfig = {
            .commands = commands,
            .commandCount = sizeof(commands) / sizeof(commands[0])
        };

        if (!voiceCommandHandler.init(&mic, &gptService, &displayManager, voiceConfig)) {
            return {false, "VoiceCommandHandler", "Failed to initialize voice command handler", true};
        }
        Logger::info("BOOT", "Voice command handler initialized with %d commands", voiceConfig.commandCount);
        return {true, "VoiceCommandHandler", "", true};
    }, "VoiceCommandHandler", true);

    registerComponent(BootPhase::SERVICE_INIT, []() -> InitResult {
        if (!voiceDisplayCoordinator.init(&voiceCommandHandler, &displayManager)) {
            return {false, "VoiceDisplayCoordinator", "Failed to initialize coordinator", true};
        }
        if (!voiceDisplayCoordinator.start()) {
            return {false, "VoiceDisplayCoordinator", "Failed to start coordinator", true};
        }
        Logger::info("BOOT", "Voice display coordinator initialized");
        return {true, "VoiceDisplayCoordinator", "", true};
    }, "VoiceDisplayCoordinator", true);

    registerComponent(BootPhase::SERVICE_INIT, []() -> InitResult {
        if (!gptService.init(GPT_API_KEY)) {
            Logger::warn("BOOT", "GPT service not initialized - API key not set");
            return {false, "GPT", "API key not configured", false};
        }
        Logger::info("BOOT", "GPT service initialized");
        return {true, "GPT", "", false};
    }, "GPT", false);

    registerComponent(BootPhase::SERVICE_INIT, []() -> InitResult {
        if (!ftpServer.begin()) {
            return {false, "FTP", "Failed to initialize FTP server", false};
        }
        Logger::info("BOOT", "FTP server initialized");
        return {true, "FTP", "", false};
    }, "FTP", false);

    // Register APPLICATION_START phase components
    registerComponent(BootPhase::APPLICATION_START, []() -> InitResult {
        if (!voiceCommandHandler.startListening()) {
            return {false, "VoiceListening", "Failed to start voice listening", true};
        }
        Logger::info("BOOT", "Voice listening started");
        return {true, "VoiceListening", "", true};
    }, "VoiceListening", true);

    registerComponent(BootPhase::APPLICATION_START, []() -> InitResult {
        TaskScheduler::startTasks();
        Logger::info("BOOT", "All tasks started");
        return {true, "TaskScheduler", "", true};
    }, "TaskScheduler", true);

    registerComponent(BootPhase::APPLICATION_START, []() -> InitResult {
        // Set touch sensor callbacks
        if (!TouchSensor::setCallback(TouchSensor::ONE_TAP, []() {
            Logger::info("TOUCH", "One tap: Starting voice listening");
            voiceCommandHandler.startListening();
        })) {
            Logger::warn("BOOT", "Failed to set one tap callback");
        }
        if (!TouchSensor::setCallback(TouchSensor::DOUBLE_TAP, []() {
            Logger::info("TOUCH", "Double tap: Toggling silent mode");
            // TODO: Implement silent mode toggle
        })) {
            Logger::warn("BOOT", "Failed to set double tap callback");
        }
        if (!TouchSensor::setCallback(TouchSensor::TRIPLE_TAP, []() {
            Logger::info("TOUCH", "Triple tap: Entering advanced config");
            displayManager.setState(DisplayState::CONFIG);
            // TODO: Implement advanced config mode
        })) {
            Logger::warn("BOOT", "Failed to set triple tap callback");
        }
        Logger::info("BOOT", "Touch sensor callbacks set");
        return {true, "TouchCallbacks", "", false};
    }, "TouchCallbacks", false);

    initialized = true;
    Logger::info("BOOT", "Boot manager initialized with %d components", components.size());
    return true;
}

bool BootManager::bootSystem() {
    if (!initialized) {
        Logger::error("BOOT", "Boot manager not initialized");
        return false;
    }

    Logger::info("BOOT", "Starting system boot sequence...");

    // Define boot phases in order
    std::vector<BootPhase> phases = {
        BootPhase::PRE_INIT,
        BootPhase::HARDWARE_INIT,
        BootPhase::NETWORK_INIT,
        BootPhase::SERVICE_INIT,
        BootPhase::APPLICATION_START
    };

    // Execute each phase
    for (BootPhase phase : phases) {
        Logger::info("BOOT", "Executing phase: %s", phaseToString(phase).c_str());
        currentPhase = phase;

        InitResult result = executePhase(phase);
        initResults.push_back(result);

        if (!result.success && !shouldContinueAfterFailure(result)) {
            Logger::error("BOOT", "Critical failure in phase %s: %s",
                         phaseToString(phase).c_str(), result.errorMessage.c_str());
            currentPhase = BootPhase::PRE_INIT; // Reset to error state
            return false;
        }

        if (!result.success) {
            Logger::warn("BOOT", "Non-critical failure in phase %s: %s",
                        phaseToString(phase).c_str(), result.errorMessage.c_str());
        }
    }

    currentPhase = BootPhase::READY;
    Logger::info("BOOT", "System boot complete - all phases successful");
    return true;
}

void BootManager::registerComponent(BootPhase phase, ComponentInitializer initializer,
                                   const std::string& name, bool critical) {
    components.push_back({phase, initializer, name, critical});
}

float BootManager::getBootProgress() const {
    if (currentPhase == BootPhase::READY) return 1.0f;

    // Calculate progress based on completed phases
    const std::vector<BootPhase> allPhases = {
        BootPhase::PRE_INIT,
        BootPhase::HARDWARE_INIT,
        BootPhase::NETWORK_INIT,
        BootPhase::SERVICE_INIT,
        BootPhase::APPLICATION_START,
        BootPhase::READY
    };

    auto it = std::find(allPhases.begin(), allPhases.end(), currentPhase);
    if (it == allPhases.end()) return 0.0f;

    return static_cast<float>(std::distance(allPhases.begin(), it)) /
           static_cast<float>(allPhases.size() - 1);
}

BootManager::InitResult BootManager::executePhase(BootPhase phase) {
    // Count components in this phase
    size_t phaseComponentCount = 0;
    for (const auto& component : components) {
        if (component.phase == phase) {
            phaseComponentCount++;
        }
    }

    if (phaseComponentCount == 0) {
        return {true, "Phase_" + phaseToString(phase), "No components to initialize", false};
    }

    Logger::info("BOOT", "Phase %s: %d components to initialize",
                phaseToString(phase).c_str(), phaseComponentCount);

    // Execute components in this phase
    for (const auto& component : components) {
        if (component.phase == phase) {
            Logger::debug("BOOT", "Initializing component: %s", component.name.c_str());

            InitResult result = component.initializer();
            result.componentName = component.name;
            result.critical = component.critical;

            if (!result.success) {
                return result; // Return first failure
            }
        }
    }

    return {true, "Phase_" + phaseToString(phase), "All components initialized", false};
}

std::string BootManager::phaseToString(BootPhase phase) const {
    switch (phase) {
        case BootPhase::PRE_INIT: return "PRE_INIT";
        case BootPhase::HARDWARE_INIT: return "HARDWARE_INIT";
        case BootPhase::NETWORK_INIT: return "NETWORK_INIT";
        case BootPhase::SERVICE_INIT: return "SERVICE_INIT";
        case BootPhase::APPLICATION_START: return "APPLICATION_START";
        case BootPhase::READY: return "READY";
        default: return "UNKNOWN";
    }
}

bool BootManager::shouldContinueAfterFailure(const InitResult& result) const {
    return !result.critical;
}