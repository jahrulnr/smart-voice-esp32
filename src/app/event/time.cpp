#include <app/events.h>
#include <core/time.h>

int lastHour = -1;
const String& extraCmd = 
	"Tell this time "
	"and add extra text to prayer times (like Fajr, Dhuhr, Asr, Maghrib, or Isha), meal, or sleep time. "
	"just choose based the hour "
	"(max 100 char): ";

void timeEvent() {
	int hour = timeManager.getHour();
	if (lastHour == hour || hour == -1) return;
	
	lastHour = hour;
	bool tellExtra = false;
	switch(hour) {
		case 5:
		case 9:
		case 12:
		case 15:
		case 18:
		case 19:
		case 22:
			tellExtra = true;
			break;
	}

	// tts.speak(speakCmd.c_str());
	if (tellExtra) {
		String cmd = extraCmd + timeManager.getCurrentTime();
		ai.sendPrompt(cmd, aiCallback);
	} else {
		ai.sendPrompt(String("Tell this time (max 100 char): ") +hour, aiCallback);
	}
	sysActivity.update(millis());
}