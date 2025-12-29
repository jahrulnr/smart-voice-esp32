#include "app/events.h"

unsigned long lastPressed = 0;
int triggerCount = 0;
bool recording = false;

void buttonEvent() {
	button.update();

	if (!button.isPressed()) return;

	updateActivity(millis());
	if (millis() - lastPressed < 1000) {
		++triggerCount;
	} else {
		triggerCount = 1;
	}

	lastPressed = button.getLastTrigger();
	ESP_LOGI("buttonEvent", "Button pressed, count: %d", triggerCount);

	switch(triggerCount) {
		case 1: 
			// aiStt.transcribeAudio("/audio.mp3", aiTranscriptionCallback);
			recording = !recording;
			notification->send(NOTIFICATION_RECORD, recording ? 0 : 1);
			ESP_LOGI("buttonEvent", "Recording: %s", recording ? "ON" : "OFF");
		break;
	}
}