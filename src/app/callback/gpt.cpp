#include "app/callbacks.h"

void aiCallback(const String& payload, const String& response){
	if (response.isEmpty()) return;

	ESP_LOGI("AICallback", "Payload: %s", payload.c_str());
	ESP_LOGI("AICallback", "Response: %s", response.c_str());
	tts.speak(response.c_str());
}