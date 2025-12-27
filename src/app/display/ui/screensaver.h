#pragma once

#include "../interface.h"
#include "../icons.h"
#include <core/time.h>

class ScreenSaverDrawer : public DisplayDrawer {
public:
	enum State {
		LEFT_BOTTOM = 0,
		LEFT_TOP,
		RIGHT_BOTTOM,
		RIGHT_TOP,
	};

	ScreenSaverDrawer(U8G2* display = nullptr): 
		_display(display) {
		_lastModeUpdate = millis();
	}

	inline void draw() override {
		_display->clearBuffer();
		vTaskDelay(pdMS_TO_TICKS(400));
		if (_mode > RIGHT_TOP) _mode = LEFT_BOTTOM;

		int x, y;
		switch(_mode) {
			case LEFT_BOTTOM:
				x = 4; y = 44;
				break;
			case RIGHT_BOTTOM:
				x = 63; y = 44;
				break;
			case LEFT_TOP:
				x = 4; y = 4;
				break;
			case RIGHT_TOP:
				x = 63; y = 4;
			break;
			default:
				x = 40; y = 23;
				break;
		}

		_draw(x, y);

		if(millis() - _lastModeUpdate > 30000) {
			_mode++;
			_lastModeUpdate = millis();
		}

		_display->sendBuffer();
	}

private:
	U8G2* _display;
	int _mode = 0;
	int _frame = 0;
	unsigned long _lastModeUpdate;
	int _lastSecond = 0;
	std::vector<const unsigned char*> _loading;

	inline void _draw(int x, int y){
		_display->setFont(u8g2_font_haxrcorp4089_tr);
		int hour = timeManager.getHour();
		if (hour > 18 || (hour < 6 && hour >= 0 )) {
			_display->drawXBM(x, y, 16, 16, icon16::moon);
		} else if (hour > 6 && hour < 17) {
			_display->drawXBM(x, y, 16, 15, animateSunny());
		} else {
			_display->drawXBM(x, y, 17, 16, icon16::cloud);
		}

		// left bottom
		_display->setCursor(x +21, y +12);
		String currentTime = timeManager.getCurrentTime();
		int spaceIndex = currentTime.indexOf(' ');
		if (spaceIndex > 0 && currentTime != "Time not synced") {
			String timePart = currentTime.substring(spaceIndex + 1, spaceIndex + 9); // HH:MM:SS
			_display->printf("%s", timePart.c_str());
		} else {
			_display->printf("--:--:--");
		}
	}
};