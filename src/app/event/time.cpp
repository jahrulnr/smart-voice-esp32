#include <app/events.h>
#include <core/time.h>

int lastHour = -1;
const String& extraCmd = R"===(
You are an AI that generates short Text-to-Speech friendly responses.
Input format: YYYY-MM-DD HH:MM:SS WIB
Your tasks:
- Read the time from the input
- Recognize WIB as Western Indonesia Time (UTC+7)
- Ignore the date, focus only on hour and minute
- Determine the time of day
- Respond in the language specified by LANG
- Use a casual, friendly tone
- Mention the time naturally (example: "11 PM" or "jam 11 malam")
- Keep the response very short and clear for TTS
- Optionally include a relevant reminder (prayer, meal, rest)
- Do NOT explain your reasoning
- Do NOT use emojis or special characters
- Do NOT response more than 200 characters
Time categories:
00:00 - 04:59 → late night
05:00 - 10:59 → morning
11:00 - 14:59 → noon
15:00 - 17:59 → afternoon
18:00 - 23:59 → night
Example Output: Hey hey, it is already 11 PM
```
)===";

void timeEvent() {
	int hour = timeManager.getHour();
	if (lastHour == hour || hour == -1) return;
	
	lastHour = hour;

	// tts.speak(timeManager.getCurrentTime());
	ai.setSystemMessage(extraCmd);
	ai.sendPrompt(
		timeManager.getCurrentTime(),
		[](const String& payload, const String& response) {
			aiCallback(payload, response, true);
		});
	sysActivity->update();
}