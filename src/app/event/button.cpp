#include "app/events.h"

unsigned long lastPressed = 0;
int triggerCount = 0;

void buttonEvent() {
	button.update();

	if (!button.isPressed()) return;

	updateActivity();
	if (millis() - lastPressed < 1000) {
		++triggerCount;
	} else {
		triggerCount = 1;
	}

	lastPressed = button.getLastTrigger();
	ESP_LOGI("buttonEvent", "Button pressed, count: %d", triggerCount);

	switch(triggerCount) {
		case 1: 
			aiStt.transcribeAudio("/audio.mp3", aiTranscriptionCallback);
		break;
	}
}