#include "app/events.h"

typedef void(*OnTriggerBackHandle) (void);

void buttonEvent() {
	static int triggerCount = 0;
	static bool needBackTrigger = false;
	static OnTriggerBackHandle onTriggerBack = nullptr;
	button.update();

	bool triggerTimeout = millis() - button.getLastTrigger() >= 1000;
	if (button.isPressed() && !triggerTimeout) {
		sysActivity->update();
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
				aiSts.start(
					micAudioCallback, 
					speakerAudioCallback,
					stsTools,
					nullptr,
					stsEvent
				);
				notification->send(NOTIFICATION_DISPLAY, EDISPLAY_MIC);
				needBackTrigger = true;
				onTriggerBack = []() {
					notification->send(NOTIFICATION_DISPLAY, EDISPLAY_NONE);
					aiSts.stop();
					delay(10);
					speaker->clear();
				};

				ESP_LOGI("buttonEvent", "Started microphone for streaming");
			}
			break;
	}

	ESP_LOGI("buttonEvent", "Button pressed, count: %d", triggerCount);
	sysActivity->update();
	triggerCount = 0;
}