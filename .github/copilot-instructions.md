# Copilot Instructions for ESP32 Voice Assistant Project

## Project Overview
This is a PlatformIO-based ESP32 voice assistant project using Arduino framework. It integrates speech recognition (SR), text-to-speech (TTS), audio I/O, UI display, networking (FTP/Web), and ML model loading. Key features: real-time voice commands, notifications, and remote control.

## Architecture
- **Clean Architecture by Responsibility**: Code organized in `app/` by architectural layers (infrastructure/, application/, network/) with feature-based organization (audio/, voice/, ui/). Each layer has a single responsibility.
- **Layer Boundaries**:
  - `infrastructure/` - System-level concerns (BootManager, Logger, TaskScheduler)
  - `application/` - Business logic (GPTService, VoiceDisplayCoordinator, CommandProcessor)
  - `network/` - Communication infrastructure (WiFiManager, FtpServer, WebServer)
  - `audio/`, `voice/`, `ui/` - Feature implementations
- **Service Boundaries**: Infrastructure provides system services; Application contains business logic; Network handles communication. Features depend on layers but not vice versa.
- **Data Flows**: Audio input (microphone) → voice recognition (speech_recognizer) → command parsing → TTS synthesis → audio output (speaker). UI/network handle user interaction and remote access.
- **Why This Structure**: Clean architecture principles for maintainability, separation of concerns, enforcing unidirectional dependencies (infrastructure → application → features).

## Key Workflows
- **Build**: `pio run` (compiles with ESP32-specific flags like `-DCONFIG_SR_VADN_VADNET1_MEDIUM`).
- **Upload**: `pio run -t upload` (uses esptool).
- **Monitor/Debug**: `pio device monitor` (baud 115200, filters for ESP exceptions). Use `ESP_LOGx` macros (e.g., `ESP_LOGI("TAG", "message")`) for logging—levels set in config.h.
- **Testing**: No automated tests yet; validate manually via serial output or UI.

## Conventions & Patterns
- **Language**: C++17 with exceptions disabled (`-fno-exceptions`); use `std::function` for callbacks.
- **File Organization**: Headers (.h) and implementations (.cpp) in same folder; include guards in .h files.
- **UI Pattern**: Header-only drawer classes with inline implementations using strategy pattern. Single `draw()` method per drawer (see `ui/drawers/*.h`).
- **Command Processing**: Unified command handling with lookup tables instead of multiple handler methods (see `application/command_processor.cpp`).
- **Concurrency**: Use FreeRTOS tasks (e.g., `xTaskCreate` in infrastructure/task_scheduler.cpp); avoid blocking main loop.
- **Dependencies**: Managed in `platformio.ini` (e.g., U8g2 for display, esp32-microphone for audio). Import via `#include <library.h>`.
- **Configuration**: Global settings in `config.h` (pins, WiFi creds); build flags in `platformio.ini` for ESP32 features.
- **Error Handling**: Use ESP_ERROR_CHECK for ESP-IDF calls; log errors with ESP_LOGE.
- **Examples**:
  - Logging: `#include "infrastructure/logger.h"` then `Logger::info("VOICE", "Recognized command")`.
  - Tasks: Add new tasks in `infrastructure/task_scheduler.cpp` and start via `TaskScheduler::startTasks()`.
  - Audio: Use `audio/microphone.h` for input, `audio/speaker.h` for output with I2S.
  - TTS: Use `voice/pico_tts.h` to synthesize speech from text to speaker.
  - UI: Implement new display states as header-only classes in `ui/drawers/` with single `draw()` method.
  - Commands: Add new voice commands by extending lookup tables in `application/command_processor.cpp`.

## Integration Points
- **External Libs**: Speech models loaded from `model/` folder via `models/model_loader.cpp`.
- **Hardware**: ESP32S3 pins configured in `config.h`; audio via I2S.
- **Communication**: FTP server in `network/ftp_server.cpp` for file transfer; Web server in `network/web_server.cpp` for remote commands; WiFi infrastructure in `network/wifi_manager.cpp`.

## Implementation Lessons Learned

### Common Mistakes to Avoid
- **Over-editing with replace_string_in_file**: Always include sufficient context (3-5 lines before/after) to avoid accidentally removing class definitions or method signatures. Double-check the replacement doesn't break syntax.
- **Incomplete cleanup**: When refactoring, search for ALL references to removed code across all files (.h and .cpp) before declaring the task complete. Use grep_search to find lingering declarations.
- **Header staleness**: After moving structs/types between files, ensure all #include statements are updated and no stale forward declarations remain.
- **Build verification**: In embedded development, always build after each significant change - don't batch multiple risky edits together.

### Best Practices for Refactoring
- **Read full context first**: Use read_file with large ranges to understand file structure before making changes.
- **Test small changes**: Make one focused change, build, then proceed to the next.
- **Check dependencies**: Use grep_search to find all usages of moved/renamed elements.
- **Be precise with context**: When using replace_string_in_file, include enough unchanged surrounding code to make the target uniquely identifiable.
- **UI Refactoring**: When adding new display states, create header-only drawer classes in `ui/drawers/` with inline `draw()` implementations.
- **Command Refactoring**: Extend command processing through lookup tables rather than adding new handler methods.

### Development Workflow
- **Architecture first**: Get structural decisions right (layers, boundaries, data flow).
- **Implementation precision**: Execute changes carefully, avoiding over-editing and incomplete cleanup.
- **Iterative validation**: Build frequently during development, fix issues immediately rather than accumulating them.

Focus on feature folders for new code; reference `infrastructure/`, `application/`, and `network/` layers for shared logic. Keep pipeline unidirectional.