#ifndef CONFIG_H
#define CONFIG_H

// ESP32 Voice Assistant Configuration

// I2S Microphone Pins (for INMP441)
#define I2S_MIC_SD_PIN GPIO_NUM_12   // I2S data pin (DOUT from INMP441)
#define I2S_MIC_SCK_PIN GPIO_NUM_14   // I2S clock pin (BCLK)
#define I2S_MIC_WS_PIN GPIO_NUM_13   // I2S word select pin (WS/LRCLK)
#define I2S_MIC_PORT I2S_NUM_0       // I2S port number

// Audio Configuration
#define MIC_SAMPLE_RATE 16000               // 16kHz sample rate
#define MIC_BIT_DEPTH I2S_DATA_BIT_WIDTH_16BIT // 16-bit samples
#define MIC_CHANNELS I2S_SLOT_MODE_MONO     // Mono channel

// I2S Speaker Pins (for audio output, e.g., MAX98357A)
#define I2S_SPEAKER_DOUT_PIN GPIO_NUM_11  // I2S data output pin (DIN)
#define I2S_SPEAKER_BCLK_PIN GPIO_NUM_10  // I2S bit clock pin (BCLK)
#define I2S_SPEAKER_LRC_PIN GPIO_NUM_9    // I2S word select pin (WS/LRCLK)
#define I2S_SPEAKER_PORT I2S_NUM_1        // I2S port number (separate from mic)

// Speaker Audio Configuration
#define SPEAKER_SAMPLE_RATE 16000               // 16kHz sample rate
#define SPEAKER_BIT_DEPTH I2S_DATA_BIT_WIDTH_16BIT // 16-bit samples
#define SPEAKER_CHANNELS I2S_SLOT_MODE_MONO     // Mono output (can be stereo)

// Display Configuration (SSD1306 OLED)
#define DISPLAY_SDA_PIN SDA               // I2C SDA pin (GPIO 21)
#define DISPLAY_SCL_PIN 3               // I2C SCL pin (GPIO 22)
#define DISPLAY_RESET_PIN -1             // Reset pin (not used)
#define DISPLAY_I2C_ADDRESS 0x3C         // SSD1306 default address
#define DISPLAY_WIDTH 128                // Display width in pixels
#define DISPLAY_HEIGHT 64                // Display height in pixels

// FTP Server Configuration
#define FTP_USERNAME "esp32"          // FTP username
#define FTP_PASSWORD "password"       // FTP password

// WiFi Hotspot Configuration
#define HOTSPOT_SSID "ESP32-Config"    // AP hotspot SSID
#define HOTSPOT_PASSWORD "password123" // AP hotspot password

// WiFi Manager Configuration
#define WIFI_RECONNECT_INTERVAL 30000 // Reconnect attempt interval (ms)
#define WIFI_WEB_SERVER_PORT 80       // Web server port for hotspot config
#define WIFI_DNS_SERVER_PORT 53       // DNS server port for captive portal

// GPT Service Configuration
#define GPT_API_KEY ""                // OpenAI API key (set via web interface)

#endif // CONFIG_H