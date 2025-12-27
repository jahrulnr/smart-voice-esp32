#include "app/callbacks.h"

void aiCallback(const String& response){
	if (response.isEmpty()) return;

	tts.speak(response.c_str());
}