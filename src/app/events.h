#pragma once

#include "boot/init.h"


extern QueueHandle_t audioChunkQueue;
typedef void(*AudioCallback) (uint32_t key, uint32_t index, const uint8_t* data, size_t dataSize);

struct AudioEvent {
	EVENT_MIC event;
	AudioCallback callback;
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

void recordEvent(void *param);