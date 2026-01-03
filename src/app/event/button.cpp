#include "app/events.h"

typedef void(*OnTriggerBackHandle) (void);

void buttonEvent() {
	static int triggerCount = 0;
	static bool needBackTrigger = false;
	static OnTriggerBackHandle onTriggerBack = nullptr;
	button.update();

	bool triggerTimeout = millis() - button.getLastTrigger() >= 1000;
	if (button.isPressed() && !triggerTimeout) {
		sysActivity.update();
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
					[](){
						aiSts.sendTool(GPTStsService::GPTTool{
							.description = "Send weather notification to system",
							.name = "notification_weather"
						});
					},
					nullptr,
					[](const GPTStsService::GPTToolCall& data) {
						ESP_LOGI("AIFunctionCall", "name: %s, call_id: %s", data.name, data.callId);
						// need move to command processor
						if (0 == strcmp(data.name, "notification_weather"))
							weatherService.getCurrentWeather([&data](weatherData_t wdata, bool success){
								String resp = 
									"Temperature: " + String(wdata.temperature)
									+". Humidity: " + String(wdata.humidity)
									+". Wind Speed: " + String(wdata.windSpeed)
									+". Wind Direction: " + String(wdata.windDirection)
									+". Deskripsi: " + String(wdata.description)
									+". Last Update: " + String(wdata.lastUpdated);
								
								aiSts.sendToolCallback(GPTStsService::GPTToolCallback{
									.callId = data.callId,
									.name = data.name,
									.output = resp.c_str(),
									.status = "complete"
								});
							});
					}
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
	sysActivity.update();
	triggerCount = 0;
}