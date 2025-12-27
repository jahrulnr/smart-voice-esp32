#pragma once

#include <Arduino.h>
#include <core/activity.h>
#include <Wire.h>
#include "app_config.h"
#include "constants.h"
#include "app/callback_list.h"
#include "app/audio/microphone.h"
#include <app/audio/speaker.h>
#include <app/audio/tts.h>
#include "Notification.h"
#include "Display.h"
#include "esp32-hal-sr.h"
#include "csr.h"
#include "app/network/WiFiManager.h"
#include "app/network/WeatherService.h"
#include <app/button/button.h>
#include <PubSubClient.h>

extern Notification* notification;
extern PubSubClient mqttClient;
extern Speaker* speaker;

void setupApp();

void setupNotification();
void setupMicrophone();
void setupSpeaker();
void setupSpeechRecognition();