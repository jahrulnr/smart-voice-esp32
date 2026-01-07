#pragma once

#include "boot/init.h"

extern QueueHandle_t audioChunkQueue;
typedef bool(*AudioCollectorCallback) (uint32_t key, uint32_t index, const uint8_t* data, size_t dataSize);
typedef void(*AudioExecutorCallback) (const String& key);

enum AUDIO_STATE {
  AUDIO_STATE_IDLE = 0,
  AUDIO_STATE_RUNNING,
  AUDIO_STATE_STOPPED,
  AUDIO_STATE_ERROR
};

struct AudioEvent {
  EVENT_MIC flag;
  AudioCollectorCallback collectorCallback;
  AudioExecutorCallback executorCallback;
  AUDIO_STATE state = AUDIO_STATE_IDLE;
};

struct AudioData {
  String key;
  uint8_t* data;
  size_t length;
  bool stream;
};

void timeEvent();
void displayEvent();
void buttonEvent();
void srEvent();
void stsTools();
void stsEvent(const GPTStsService::GPTToolCall& toolcall);
void srDisconnectCallback();

AudioEvent getMicEvent();
void setMicEvent(AudioEvent event);