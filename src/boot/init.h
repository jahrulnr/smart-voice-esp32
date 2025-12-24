#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "app_config.h"
#include "constants.h"
#include "app/callback_list.h"
#include "app/audio/microphone.h"
#include "Notification.h"
#include "Display.h"
#include "Face.h"
#include "esp32-hal-sr.h"
#include "csr.h"
#include "app/network/WiFiManager.h"
#include "app/network/WeatherService.h"
#include <PubSubClient.h>

extern Notification* notification;
extern Face* faceDisplay;
extern PubSubClient mqttClient;

void setupApp();

void setupNotification();
void setupMicrophone();
void setupFaceDisplay(uint16_t size = 40);
void setupSpeechRecognition();