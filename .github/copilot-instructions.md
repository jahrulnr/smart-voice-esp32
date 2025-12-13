# Copilot Instructions for ESP32 Voice Assistant Project

## Project Overview
PlatformIO-based ESP32-S3 voice assistant using Arduino framework. Integrates speech recognition (ESP-SR), text-to-speech (Pico TTS), audio I/O, OLED display, WiFi networking (FTP/Web), and GPT integration. Key features: real-time voice commands, remote control, and AI responses.

## Architecture
- **Clean Architecture by Responsibility**: Organized in `app/` by layers (infrastructure/, application/, network/) with feature folders (audio/, voice/, ui/).
- **Layer Boundaries**:
  - `infrastructure/` - System services (BootManager, Logger, TaskScheduler)
  - `application/` - Business logic (GPTService, VoiceDisplayCoordinator)
  - `network/` - Communication (WiFiManager, FtpServer, WebServer)
  - `audio/`, `voice/`, `ui/` - Feature implementations
- **Service Boundaries**: Unidirectional dependencies (infrastructure → application → features). Features use layers but not vice versa.
- **Data Flows**: Microphone → VoiceCommandHandler → command processing → TTS → Speaker. UI/network handle user interaction and remote access.
- **Why This Structure**: Maintainability, separation of concerns, testable components.

## Key Workflows
- **Build**: `pio run` (ESP32 flags like `-DCONFIG_SR_VADN_VADNET1_MEDIUM`)
- **Upload**: `pio run -t upload` (esptool)
- **Monitor/Debug**: `pio device monitor` (115200 baud, ESP exception filters). Use `ESP_LOGx` macros (e.g., `ESP_LOGI("TAG", "message")`)
- **Testing**: Manual validation via serial/UI; no automated tests yet

## Permissions
- **Build Operations**: Allowed to run `pio run` to build the firmware
- **Upload Operations**: NOT allowed to flash/upload firmware to the ESP32 device

## Conventions & Patterns
- **Language**: C++17, exceptions disabled (`-fno-exceptions`), `std::function` for callbacks
- **File Organization**: .h/.cpp pairs in same folder; include guards in .h
- **UI Pattern**: Header-only drawer classes in `ui/drawers/` with inline `draw()` method (strategy pattern)
- **Command Processing**: Unified handling in VoiceCommandHandler with response arrays
- **Concurrency**: FreeRTOS tasks (e.g., `xTaskCreate` in TaskScheduler); avoid blocking main loop
- **Dependencies**: Managed in `platformio.ini`; import via `#include <library.h>`
- **Configuration**: Global in `config.h` (pins, WiFi); build flags in `platformio.ini`
- **Error Handling**: `ESP_ERROR_CHECK` for ESP-IDF; `ESP_LOGE` for logging
- **Examples**:
  - Logging: `#include "infrastructure/logger.h"` → `Logger::info("TAG", "message")`
  - Tasks: Add in `infrastructure/task_scheduler.cpp` → `TaskScheduler::startTasks()`
  - Audio: `audio/microphone.h` for I2S input, `audio/speaker.h` for output
  - TTS: `voice/pico_tts.h` → `tts.speak("text")`
  - UI: New states as header-only classes in `ui/drawers/` with `draw()` method
  - Commands: Extend `commandResponses[]` in `voice_constants.h` for new voice commands

## Integration Points
- **External Libs**: ESP-SR models loaded from `model/` folder
- **Hardware**: ESP32-S3 pins in `config.h`; I2S for audio
- **Communication**: FTP in `network/ftp_server.cpp`; Web in `network/web_server.cpp`; WiFi in `network/wifi_manager.cpp`

## Implementation Lessons Learned
- **Refactoring**: Read full context first; use grep_search for all references; build after each change
- **Edit Precision**: Include 3-5 lines context in replace_string_in_file to avoid breaking syntax
- **Cleanup**: Search for ALL references to removed code before completion
- **Build Verification**: Always build after significant changes in embedded dev

Focus on feature folders for new code; reference infrastructure/application/network layers for shared logic. Keep dependencies unidirectional.