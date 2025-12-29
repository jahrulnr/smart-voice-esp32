#include <app/events.h>

void srEvent() {
	// Handle any notifications that might be relevant to SR
	if (notification->has(NOTIFICATION_COMMAND)) {
		void* event = notification->consume(NOTIFICATION_COMMAND);
		if (event) {
			const char* command = (const char*)event;
			ESP_LOGI("SREvent", "Received command notification: %s", command);

			// Handle command notifications if needed
			if (strcmp(command, "pause_sr") == 0) {
				ESP_LOGI("SREvent", "Pausing speech recognition");
				SR::pause();
			} else if (strcmp(command, "resume_sr") == 0) {
				ESP_LOGI("SREvent", "Resuming speech recognition");
				SR::resume();
			}
		}
	}
}