#include "app/events.h"

typedef void(*OnTriggerBackHandle) (void);

void buttonEvent() {
	static int triggerCount = 0;
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
		case 2: 
			{
				ESP_LOGI("buttonEvent", "Recording: ON");
				auto event = getMicEvent();
				if (event.state != AUDIO_STATE_IDLE){
					ESP_LOGW("buttonEvent", "Mic event is not idle, state: %d", event.state);
					break;
				}
				event.flag = EMIC_START;
				event.callback = audioToWav;
				setMicEvent(event);
				notification->send(NOTIFICATION_DISPLAY, EDISPLAY_MIC);
				needBackTrigger = true;
				onTriggerBack = []() {
					auto event = getMicEvent();
					if (event.state != AUDIO_STATE_RUNNING){
						ESP_LOGW("buttonEvent", "Mic event is not running, state: %d", event.state);
						return;
					}
					notification->send(NOTIFICATION_DISPLAY, EDISPLAY_NONE);
					event.flag = EMIC_STOP;
					event.callback = audioToWav;
					setMicEvent(event);
					ESP_LOGI("buttonEvent", "Recording: OFF");
				};
			}
			break;
	}

	ESP_LOGI("buttonEvent", "Button pressed, count: %d", triggerCount);
	sysActivity.update(millis());
	triggerCount = 0;
}