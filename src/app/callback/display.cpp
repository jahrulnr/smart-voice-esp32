#include "app/callback_list.h"
#include <app/display/ui/main.h>

void displayCallback() {
	const char* TAG = "displayCallback";
	static size_t updateDelay = 0;
	static EVENT_DISPLAY lastDisplayEvent = EDISPLAY_NONE;
	static MainStatusDrawer mainDisplay = MainStatusDrawer(display);

	// Update weather data if available to display
	if (notification->has(NOTIFICATION_WEATHER)) {
		weatherData_t* data = (weatherData_t*) notification->consume(NOTIFICATION_WEATHER, 10);
		mainDisplay.updateData(data);
	}

	// Handle display event
	bool signalDisplay = notification->hasSignal(NOTIFICATION_DISPLAY);
	if (signalDisplay) {
		EVENT_DISPLAY event = (EVENT_DISPLAY) notification->signal(NOTIFICATION_DISPLAY, 100);
		if (event != lastDisplayEvent) {
			updateDelay = millis() + 1000;
			lastDisplayEvent = event;
		}
	}
	else if (updateDelay <= millis()) {
		updateDelay = 0;
		lastDisplayEvent = EDISPLAY_NONE;
		notification->signal(NOTIFICATION_DISPLAY, 0);
	}

	// Update Display
	switch (lastDisplayEvent) {
		case EDISPLAY_WAKEWORD:
			display->clearBuffer();
			faceDisplay->Update();
			break;
		default:
			mainDisplay.draw();
			break;
	}
}