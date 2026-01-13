#pragma once
#include <hw.h>

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

// analog microphone
#define MIC_AR   GPIO_NUM_39
#define MIC_OUT	 GPIO_NUM_4 // esp32-s3 range pin (0-20)
#define MIC_GAIN GPIO_NUM_38

// i2s speaker
#define SPEAKER_SAMPLE_RATE 16000              
#define SPEAKER_BIT_DEPTH I2S_DATA_BIT_WIDTH_16BIT
#define SPEAKER_CHANNELS I2S_SLOT_MODE_MONO    
#define SPEAKER_VOLUME 3.0f

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define ENABLE_POWERSAVE 0
#define SAVE_AUDIO 0