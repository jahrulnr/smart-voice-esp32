# ESP32 Voice Assistant - AI Coding Guidelines

## Project Overview
This is a PlatformIO-based ESP32-S3 voice assistant with modular architecture. The system integrates speech recognition (ESP-SR), text-to-speech (Pico TTS), AI responses (OpenAI GPT), WiFi management, web interface, and FTP server.

## Architecture
- **Infrastructure Layer** (`app/infrastructure/`): BootManager, Logger, TaskScheduler, TimeManager, TouchSensor - system-level concerns
- **Application Layer** (`app/application/`): GPTService, VoiceDisplayCoordinator - business logic
- **Network Layer** (`app/network/`): WifiManager, WebServer, FtpServer - communication
- **Audio Layer** (`app/audio/`): Microphone, Speaker - I2S audio I/O
- **Voice Layer** (`app/voice/`): VoiceCommandHandler, PicoTTS, CSR - speech processing
- **UI Layer** (`app/ui/`): DisplayManager, Drawers - OLED display management

## Key Patterns
- **FreeRTOS Tasks**: All logic runs in RTOS tasks, not Arduino loop()
- **Event-Driven UI**: DisplayManager uses EventData for state updates (e.g., `displayManager.onEvent(EventData(EventType::STATE_CHANGE, "message", static_cast<int>(DisplayState::SPEAKING)))`)
- **ESP-SR Integration**: Voice recognition uses `SR::sr_setup()` with wake word and command arrays
- **Async GPT Calls**: API requests run in separate RTOS tasks with callbacks
- **Context Caching**: GPTService maintains conversation history in `_contextCache`

## Configuration
- **Hardware Pins**: Defined in `config.h` (I2S mic/speaker on GPIO 12-14/5-7, I2C display on 21/22)
- **WiFi**: Multi-network support, auto-roaming up to 5 networks
- **Audio**: 16kHz mono, 16-bit I2S
- **GPT**: Uses OpenAI Responses API with conversation context

## Dependencies
- **Pico TTS**: Text-to-speech synthesis
- **Weather Service**: BMKG API integration for Indonesian weather data
- **External Services**: MQTT web app (`external/mqtt/`), Whisper STT (`external/whisper/`)
- **Libraries**: U8g2 (OLED), ArduinoJson, custom ESP32 libs (microphone, speaker, picoTTS)

## Common Tasks
- **Add Voice Command**: Update `voice_constants.h` with phoneme (use `tools/multinet_g2p.py`)
- **New UI State**: Add to `DisplayState` enum, create drawer in `ui/drawers/`
- **Network Feature**: Add to WebServer endpoints, update bootstrap UI in `data/assets/`
- **Weather Integration**: Use `weatherService.getCurrentWeather()` with callback for BMKG API data, auto-fetches on boot
- **Home UI Display**: Main status shows weather icon/temp, current time, IP address with real-time updates
- **Debug Audio**: Check I2S pin connections, verify 3.3V power to mic

## Code Style
- **C++17**: With exceptions enabled (`-fexceptions`)
- **Naming**: PascalCase for classes, camelCase for methods/variables
- **Logging**: Use `Logger::info/error/warn/debug()` with component tags
- **Error Handling**: Return bool for init methods, use ESP_OK checks for ESP-IDF calls
- **Memory**: Use SPIRAM for large allocations (`MALLOC_CAP_SPIRAM`)

## Hardware Testing
- **Boot Issues**: Check serial output for BootManager failures
- **Audio Problems**: Verify I2S wiring, test with `speaker.playTone()`
- **WiFi Setup**: Access hotspot `ESP32-Config` at 192.168.4.1
- **Voice Recognition**: Ensure quiet environment, clear pronunciation

## Integration Points
- **Web Interface**: Bootstrap UI in `data/assets/`, served by WebServer
- **FTP Access**: File management at `ftp://esp32-ip/`
- **Weather API**: BMKG weather data via `weatherService` (Indonesian locations)
- **External APIs**: GPT via HTTPS, MQTT for IoT messaging
- **Display Events**: All UI updates through DisplayManager event system

## Coding Guidelines
- **Write Production-Ready Code**: Always write code that follows industry best practices, with proper error handling, resource management, and performance considerations
- **ESP32 Expertise**: Demonstrate deep knowledge of ESP32-S3 architecture, FreeRTOS, memory management, and power optimization
- **Embedded Systems Best Practices**: Use appropriate data structures, avoid memory leaks, implement thread-safe operations, and optimize for constrained resources
- **Code Quality**: Write clean, maintainable, and well-documented code with meaningful variable names and comprehensive comments
- **Performance Optimization**: Consider memory usage, CPU cycles, and power consumption in all implementations
- **Security Awareness**: Implement secure coding practices, validate inputs, and avoid common vulnerabilities
- **Architecture Adherence**: Follow the established modular architecture and design patterns consistently
- **Task Assessment**: Always assess task feasibility and provide the best alternative solution if implementation is impossible or overly complex. Suggest practical workarounds, simplified approaches, or phased implementations when direct solutions aren't viable.
- **ArduinoJson Usage**: Use JsonDocument instead of deprecated StaticJsonDocument/DynamicJsonDocument (ArduinoJson v7+)