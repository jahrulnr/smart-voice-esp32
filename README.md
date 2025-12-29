# ESP32 PioAssistant

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Platform](https://img.shields.io/badge/platform-ESP32--S3-green.svg)
![Framework](https://img.shields.io/badge/framework-Arduino-blue.svg)

An IoT voice assistant powered by ESP32-S3 firmware and external processing services. Features real-time speech recognition, text-to-speech, OLED display with face animations, and MQTT-based communication for seamless integration.

## Features

- **Voice Interaction**: Speech-to-text via Whisper API, text-to-speech output
- **Real-time Audio Processing**: I2S microphone input with noise suppression and VAD
- **Visual Feedback**: OLED display with animated faces and UI updates
- **MQTT Integration**: All components communicate via MQTT for scalability
- **Web Monitoring**: Flask-SocketIO web UI for real-time MQTT message monitoring
- **Dockerized Services**: External components run in containers for easy deployment
- **ESP32 Optimization**: Leverages PSRAM, dual cores, and FreeRTOS tasks

## Architecture

The system consists of three main layers:

1. **ESP32 Firmware** (`src/`): Handles hardware (audio I/O, display, WiFi) using FreeRTOS tasks
   - MainTask: Display/UI management with FaceDisplay animations
   - NetworkTask: MQTT/WiFi connectivity
   - RecorderTask: Audio capture and streaming

2. **Go Consumer** (`external/app/`): Assembles audio chunks from MQTT and processes via Whisper API

3. **Python Services** (`external/`):
   - **MQTT UI** (`mqttui/`): Web interface for MQTT monitoring with database persistence
   - **Whisper API** (`whisper/`): FastAPI service for speech-to-text transcription

**Data Flow**: ESP32 RecorderTask → MQTT "pioassistant/audio" → Go AudioAssembler → Whisper API → MQTT "pioassistant/stt"

## Hardware Requirements

- **ESP32-S3-DevKitC-1-N16R8** (16MB Flash, 8MB PSRAM)
- **OLED Display**: I2C 128x64 (e.g., SSD1306)
- **Microphone Options**:
  - I2S Digital: INMP441, ICS-43434, or SPH0645
  - Analog: MAX9814 or similar with gain control
- **Speaker**: For TTS output (optional)

## Software Requirements

- **PlatformIO**: For ESP32 firmware build and upload
- **Docker & Docker Compose**: For external services
- **Go 1.19+**: For audio consumer
- **Python 3.8+**: For web UI and Whisper API
- **MQTT Broker**: Mosquitto (included in Docker setup)

## Installation & Setup

### 1. Clone Repository
```bash
git clone https://github.com/jahrulnr/smart-voice-esp32.git
cd smart-voice-esp32
```

### 2. External Services Setup
```bash
cd external

# Start all services (MQTT UI, Whisper, Go Consumer)
make run

# Or individually:
cd mqttui; docker compose up -d
cd ../whisper; make run
cd ../app; make run
```

### 3. Configuration
- **ESP32**: Configure WiFi and MQTT in `include/secret.h` (copy from `secret.h.example`)
- **External Services**: Use `.env` files or environment variables for MQTT broker, ports, etc.
- **Default MQTT**: localhost:1883

## Usage

1. **Power on ESP32**: Device connects to WiFi and MQTT broker
2. **Speak**: Audio captured via microphone, streamed to MQTT
3. **Processing**: Go consumer assembles audio, sends to Whisper for transcription
4. **Response**: Transcribed text published back via MQTT, displayed on OLED
5. **Monitor**: Access web UI at `http://localhost:5000` for real-time MQTT messages

### Web UI Features
- Real-time MQTT message display
- Topic filtering and search
- Message history with database persistence
- Debug bar for performance metrics

## Development

### Building ESP32 Firmware
- Use PlatformIO IDE or CLI
- Custom partitions defined in `hiesp.csv`
- Build flags optimized for ESP32-S3 (PSRAM, ESP-SR, etc.)

### External Services
- **Go Consumer**: `cd external/app; make build; make test`
- **MQTT UI**: `cd external/mqttui; docker compose up --build`
- **Whisper API**: `cd external/whisper; make build`

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make changes following the established patterns
4. Test builds and functionality
5. Submit a pull request

## License

MIT License