#include "app/events.h"
#include <app/display/ui/screensaver.h>
#include <app/display/ui/main.h>
#include <app/display/ui/wifi.h>
#include <app/display/ui/face.h>
#include <app/display/ui/record.h>
#include <app/display/ui/loading.h>

void displayEvent() {
	const char* TAG = "displayEvent";
	static EVENT_DISPLAY lastDisplayEvent = EDISPLAY_NONE;
	static ScreenSaverDrawer screenSaverDisplay = ScreenSaverDrawer(display);
	static MainStatusDrawer mainDisplay = MainStatusDrawer(display);
	static WifiDrawer wifiDisplay = WifiDrawer(display);
	static FaceDrawer faceDisplay = FaceDrawer(display);
	static RecordDrawer recordDisplay = RecordDrawer(display);
	static LoadingDrawer loadingDisplay = LoadingDrawer(display);

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
			lastDisplayEvent = event;
		}
		if (event != EDISPLAY_SLEEP) {
			sysActivity.update(millis());
		}
	}

	if (lastDisplayEvent == EDISPLAY_SLEEP && sysActivity.lastUpdate(millis()) <= 60000) {
		lastDisplayEvent = EDISPLAY_NONE;
	}

	// Update Display
	switch (lastDisplayEvent) {
		case EDISPLAY_SLEEP:
			screenSaverDisplay.draw();
			break;
		case EDISPLAY_MIC:
		case EDISPLAY_WAKEWORD:
			{
				vad_state_t vadState = getAfeState();
				RecordDrawer::State state = RecordDrawer::MUTE;
				if (vadState == VAD_SILENCE) {
					state = RecordDrawer::IDLE;
				} else if (vadState == VAD_SPEECH) {
					state = RecordDrawer::RECORDING;
				}
				recordDisplay.setState(state);
				recordDisplay.draw();
			}
			break;
		case EDISPLAY_WIFI:
			{
				static WifiDrawer::State lastState;
				WifiDrawer::State state;
				switch (wifiManager.getState()) {
					case WL_CONNECTED:
						state = WifiDrawer::State::WIFI_FULL;
						break;
					case WL_CONNECT_FAILED:
					case WL_CONNECTION_LOST:
					case WL_DISCONNECTED:
						state = WifiDrawer::State::DISCONNECT;
						break;
					default:
						state = WifiDrawer::State::WIFI_LOW;
						break;
				}
				wifiDisplay.setState(state);
				wifiDisplay.draw();
				lastState = state;
			}
			break;
		case EDISPLAY_LOADING:
			loadingDisplay.draw();
			break;
		case EDISPLAY_FACE:
			faceDisplay.draw();
			break;
		default:
			mainDisplay.draw();
			break;
	}
}