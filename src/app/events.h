#pragma once

#include "boot/init.h"

extern QueueHandle_t audioChunkQueue;
typedef bool(*AudioCallback) (uint32_t key, uint32_t index, const uint8_t* data, size_t dataSize);

enum AUDIO_STATE {
	AUDIO_STATE_IDLE = 0,
	AUDIO_STATE_RUNNING,
	AUDIO_STATE_STOPPED,
	AUDIO_STATE_ERROR
};

struct AudioEvent {
	EVENT_MIC flag;
	AudioCallback callback;
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

void micEvent(void *param);
AudioEvent getMicEvent();
void setMicEvent(AudioEvent event);