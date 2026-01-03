#pragma once

#define WIFI_SSID "ANDROID AP"
#define WIFI_PASS "tes12345"
#define FTP_USER "pio"
#define FTP_PASS "tes12345"
#define MQTT_ENABLE 0
#define MQTT_SERVER "192.168.18.250"
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASS ""
#define MQTT_CLIENT_ID "pio-assistant"
#define MQTT_TOPIC_AUDIO "pioassistant/audio"
#define MQTT_TOPIC_STT "pioassistant/stt"

#define SCL_PIN 3
#define SDA_PIN 46
// #define BUTTON_PIN 0
#define BUTTON_PIN 20

// set to analog or i2s microphone
// #define MIC_TYPE MIC_TYPE_ANALOG
#define MIC_TYPE MIC_I2S

// i2s microphone
#ifdef SEED_XIAO_ESP32S3
#define MIC_SCK GPIO_NUM_42
#define MIC_WS  GPIO_NUM_NC
#define MIC_DIN GPIO_NUM_41
#else
#define MIC_SCK GPIO_NUM_1
#define MIC_WS  GPIO_NUM_0
#define MIC_DIN GPIO_NUM_45
#endif

// analog microphone
#define MIC_AR   GPIO_NUM_39
#define MIC_OUT	 GPIO_NUM_4 // esp32-s3 range pin (0-20)
#define MIC_GAIN GPIO_NUM_38

// i2s speaker
#define I2S_SPEAKER_DOUT_PIN GPIO_NUM_14
#define I2S_SPEAKER_BCLK_PIN GPIO_NUM_47
#define I2S_SPEAKER_LRC_PIN GPIO_NUM_21
#define I2S_SPEAKER_PORT I2S_NUM_1     
#define SPEAKER_SAMPLE_RATE 16000              
#define SPEAKER_BIT_DEPTH I2S_DATA_BIT_WIDTH_16BIT
#define SPEAKER_CHANNELS I2S_SLOT_MODE_MONO    

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define ENABLE_POWERSAVE 1
#define SAVE_AUDIO 0