#pragma once

#include <Arduino.h>
#include <app_config.h>
#include <secret.h>
#include <Wire.h>
#include "constants.h"
#include <Notification.h>
#include <Display.h>
#include <esp32-hal-sr.h>
#include <csr.h>
#include <WiFiManager.h>
#include <PubSubClient.h>
#include <gpt.h>
#include <tts.h>
#include <stt.h>
#include <core/activity.h>
#include <app/callbacks.h>
#include <app/events.h>
#include <app/audio/microphone.h>
#include <app/audio/speaker.h>
#include <app/audio/tts.h>
#include <app/network/WeatherService.h>
#include <app/button/button.h>

extern Notification* notification;
extern PubSubClient mqttClient;
extern Speaker* speaker;

void setupApp();

void setupNotification();
void setupMicrophone();
void setupSpeaker();
void setupSpeechRecognition();