#include <app/events.h>
#include <app/tasks.h>

AudioEvent audioEvent;

AudioEvent getMicEvent() {
	return audioEvent;
}

void setMicEvent(AudioEvent event) {
	audioEvent = event;
	ESP_LOGI("EVENT", "Mic Event: %d, state: %d", event.flag, event.state);
}