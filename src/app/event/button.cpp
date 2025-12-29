#include "app/events.h"

int triggerCount = 0;
bool recording = false;

void buttonEvent() {
	button.update();

	if (millis() - button.getLastTrigger() >= 1000) {
		switch(triggerCount) {
			case 1:
				break; 
			case 2: 
				recording = !recording;
				notification->send(NOTIFICATION_RECORD, recording ? 0 : 1);
				notification->send(NOTIFICATION_DISPLAY, recording ? EDISPLAY_MIC : EDISPLAY_NONE);
				ESP_LOGI("buttonEvent", "Recording: %s", recording ? "ON" : "OFF");
			break;
		}

		if (triggerCount > 0){
			ESP_LOGI("buttonEvent", "Button pressed, count: %d", triggerCount);
			sysActivity.update(millis());
			triggerCount = 0;
		}
	}

	if (!button.isPressed() || millis() - button.getLastTrigger() >= 1000) return;
	++triggerCount;
}