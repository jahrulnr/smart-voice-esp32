#include "app/events.h"

typedef void(*OnTriggerBackHandle) (void);

void buttonEvent() {
	static int triggerCount = 0;
	static bool recording = false;
	static bool needBackTrigger = false;
	static OnTriggerBackHandle onTriggerBack = nullptr;
	button.update();

	bool triggerTimeout = millis() - button.getLastTrigger() >= 1000;
	if (button.isPressed() && !triggerTimeout) {
		++triggerCount;
		return;
	}

	if (triggerCount == 0) return;

	if (needBackTrigger && onTriggerBack != nullptr) {
		onTriggerBack();
		needBackTrigger = false;
		onTriggerBack = nullptr;
		triggerCount = 0;
		return;
	}

	switch(triggerCount) {
		case 1:
			break;
		case 2: 
			recording = !recording;
			notification->send(NOTIFICATION_RECORD, recording ? 0 : 1);
			notification->send(NOTIFICATION_DISPLAY, recording ? EDISPLAY_MIC : EDISPLAY_NONE);
			if (recording) {
				needBackTrigger = true;
				onTriggerBack = []() {
					notification->send(NOTIFICATION_DISPLAY, EDISPLAY_NONE);
					notification->send(NOTIFICATION_RECORD, 1);
					ESP_LOGI("buttonEvent", "Recording: OFF");
				};
			}
			ESP_LOGI("buttonEvent", "Recording: %s", recording ? "ON" : "OFF");
			break;
	}

	ESP_LOGI("buttonEvent", "Button pressed, count: %d", triggerCount);
	sysActivity.update(millis());
	triggerCount = 0;
}