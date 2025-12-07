# ESP32 Voice Assistant 🤖

A comprehensive voice-controlled assistant for ESP32-S3, featuring speech recognition, text-to-speech, WiFi management, and remote control capabilities.

## ✨ Features

- **🎤 Real-time Speech Recognition**: Wake word detection and command recognition using ESP-SR
- **🔊 Text-to-Speech**: High-quality voice synthesis with Pico TTS
- **📡 Smart WiFi Management**: Multi-network support with auto-roaming (up to 5 saved networks)
- **🌐 Web Interface**: Bootstrap-powered configuration portal for WiFi setup
- **📁 FTP Server**: Remote file management and model updates
- **📺 OLED Display**: Visual feedback and status display
- **🔄 FreeRTOS**: Efficient multitasking for responsive performance
- **📊 Remote Monitoring**: Web-based status monitoring and control

## 🏗️ Architecture

### Feature-Based Structure
```
app/
├── audio/          # Microphone and speaker I/O
├── voice/          # Speech recognition and TTS
├── ui/            # OLED display management
├── network/       # Network infrastructure (WiFi, DNS)
├── services/      # Application services (Web UI, FTP, GPT)
├── models/        # ML model loading
└── tasks/         # FreeRTOS task management
```

### Architectural Layers

**Network Layer** (`app/network/`):
- Low-level network connectivity and protocols
- WiFi hardware management, hotspot creation, DNS server
- Transport layer concerns (TCP/IP, WiFi connectivity)

**Services Layer** (`app/services/`):
- Application-level functionality using network infrastructure
- Web interfaces, file transfer, AI integration
- Business logic and user-facing features

### Data Flow
```
Microphone → Speech Recognition → Command Processing → TTS → Speaker
                                      ↓
                                UI Updates & Network I/O
```

## 🔧 Hardware Requirements

- **ESP32-S3-DevKitC-1-N16R8** (16MB Flash, 8MB PSRAM)
- **INMP441 Microphone** (I2S interface)
- **MAX98357A Speaker Amplifier** (I2S interface)
- **SSD1306 OLED Display** (I2C interface, optional)
- **Power Supply**: 5V USB or battery

### Pin Configuration
```cpp
// Microphone (INMP441)
#define I2S_MIC_SD_PIN   GPIO_NUM_32  // DOUT
#define I2S_MIC_SCK_PIN  GPIO_NUM_2   // BCLK
#define I2S_MIC_WS_PIN   GPIO_NUM_33  // WS/LRCLK

// Speaker (MAX98357A)
#define I2S_SPEAKER_DOUT_PIN GPIO_NUM_20  // DIN
#define I2S_SPEAKER_BCLK_PIN GPIO_NUM_26  // BCLK
#define I2S_SPEAKER_LRC_PIN  GPIO_NUM_27  // WS/LRCLK

// Display (SSD1306, optional)
#define OLED_SDA GPIO_NUM_21
#define OLED_SCL GPIO_NUM_22
```

## 🚀 Quick Start

### Prerequisites
- [PlatformIO](https://platformio.org/) (recommended) or Arduino IDE
- ESP32 board support package
- Git

### 1. Clone and Setup
```bash
git clone https://github.com/jahrulnr/esp32-microphone.git
cd esp32-microphone
```

### 2. Install Dependencies
```bash
# PlatformIO will automatically install dependencies
pio install
```

### 3. Configure WiFi
Edit `app/config.h`:
```cpp
#define HOTSPOT_SSID "ESP32-Config"
#define HOTSPOT_PASSWORD "password123"
```

### 4. Build and Upload
```bash
# Build the project
pio run

# Upload to ESP32
pio run -t upload

# Monitor serial output
pio device monitor
```

### 5. Initial Setup
1. **Connect to WiFi**: Join `ESP32-Config` network
2. **Open Browser**: Visit `http://192.168.4.1`
3. **Configure Networks**: Add your WiFi networks
4. **Test**: Speak commands to test voice recognition

## 📋 Usage

### Voice Commands
- **Wake Word**: "Hi ESP" (built into ESP-SR model)
- **Commands**: Custom voice commands (extendable)

### Web Interface
- **WiFi Management**: `http://esp32-ip/` - Add/remove networks
- **FTP Access**: `ftp://esp32-ip/` - File management
- **Status Monitoring**: Real-time system status

### Serial Commands
```bash
# Monitor logs
pio device monitor

# View available networks
# (Use web interface for network management)
```

## 🔨 Development

### Project Structure
- **`app/`**: Main application code
- **`data/`**: Static web files (CSS, JS)
- **`model/`**: ML models for speech recognition
- **`tools/`**: Build and utility scripts
- **`boards/`**: Custom board configurations

### Key Components

#### Services
- **WiFi Manager**: Multi-network support with auto-roaming
- **Logger**: Structured logging with levels
- **Task Scheduler**: FreeRTOS task management

#### Audio Pipeline
- **Microphone**: I2S audio capture (16kHz, 16-bit)
- **Speaker**: I2S audio output with volume control
- **TTS**: Text-to-speech synthesis

#### Network Services
- **FTP Server**: File upload/download
- **Web Server**: Configuration and monitoring
- **DNS Server**: Captive portal for setup

### Adding New Features

1. **Create Feature Folder**: Add to `app/` directory
2. **Implement Interfaces**: `.h` and `.cpp` files
3. **Register Tasks**: Add to `task_scheduler.cpp`
4. **Update Config**: Add settings to `config.h`

Example:
```cpp
// Add to task_scheduler.cpp
void TaskScheduler::startTasks() {
    // ... existing tasks
    xTaskCreate(yourTask, "YourTask", 2048, NULL, 1, NULL);
}
```

## ⚙️ Configuration

### Build Flags
Key PlatformIO build flags in `platformio.ini`:
```ini
build_flags =
    -DCONFIG_SR_VADN_VADNET1_MEDIUM  # Voice activity detection
    -DCONFIG_SR_NSN_NSNET2          # Noise suppression
    -DCONFIG_FLASH_COCO_DETECT_YOLO11N_320_S8_V3  # Object detection
```

### Model Management
ESP-SR models are pre-flashed. For updates:
```bash
# Flash speech recognition models
esptool.py --baud 2000000 write_flash 0x47D000 model/srmodels.bin
```

## 🐛 Troubleshooting

### Common Issues

**WiFi Connection Problems**
- Check network credentials in web interface
- Verify signal strength and network compatibility
- Reset to hotspot mode if needed

**Audio Issues**
- Verify I2S pin connections
- Check microphone power (3.3V)
- Test speaker amplifier wiring

**Build Errors**
- Ensure all dependencies are installed
- Check ESP32 board selection
- Verify partition table (`hiesp.csv`)

**Speech Recognition Not Working**
- Confirm models are flashed correctly
- Check microphone placement and environment noise
- Ensure clear pronunciation of wake word

### Debug Mode
Enable verbose logging:
```cpp
Logger::setLogLevel(Logger::DEBUG);
```

### Reset Options
- **Soft Reset**: Power cycle the ESP32
- **Factory Reset**: Hold reset button during boot
- **WiFi Reset**: Clear saved networks via web interface

## 📚 API Reference

### Core Classes

#### WifiManager
```cpp
bool init();                    // Initialize WiFi
void begin();                   // Start connection process
bool addNetwork(String ssid, String password);
bool removeNetwork(String ssid);
std::vector<String> getSavedNetworks();
```

#### Microphone
```cpp
bool init();                    // Initialize I2S
bool start();                   // Start audio capture
int readLevel();               // Get audio level (0-100)
```

#### PicoTTS
```cpp
bool init(Speaker* speaker);   // Initialize TTS
bool speak(String text);       // Synthesize speech
```

### Web Endpoints
- `GET /` - WiFi configuration page
- `POST /config` - Add WiFi network
- `POST /remove` - Remove WiFi network
- `GET /scan` - Scan available networks

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

### Development Guidelines
- Follow existing code style and structure
- Add documentation for new features
- Test on actual hardware
- Update this README for significant changes

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🙏 Acknowledgments

- **ESP-SR**: Espressif's speech recognition library
- **Pico TTS**: High-quality text-to-speech synthesis
- **PlatformIO**: Professional development environment
- **Bootstrap**: Responsive web interface framework

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/jahrulnr/esp32-microphone/issues)
- **Discussions**: [GitHub Discussions](https://github.com/jahrulnr/esp32-microphone/discussions)
- **Documentation**: See individual component READMEs

---

**Made with ❤️ for the ESP32 community**