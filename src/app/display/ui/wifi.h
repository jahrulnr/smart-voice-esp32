#pragma once

#include "../interface.h"
#include "../icons.h"

class WifiDrawer : public DisplayDrawer {
public:
	enum State {
		DISCONNECT = 0,
		WIFI,
		WIFI_LOW,
		WIFI_50,
		WIFI_75,
		WIFI_FULL
	};

	struct Icons {
		const unsigned char *data;
		int width;
		int height;
	};

	WifiDrawer(U8G2* display = nullptr): 
		_display(display), 
		_frame(0),
		_maxFrame(6) {
		if (!display) return;
		_icons = new Icons[_maxFrame] {
			{
				.data = icon32::wifi_not_connected,
				.width = 38,
				.height = 32
			},
			{
				.data = icon32::wifi,
				.width = 38,
				.height = 32
			},
			{
				.data = icon32::wifi_2,
				.width = 38,
				.height = 32
			},
			{
				.data = icon32::wifi_50,
				.width = 38,
				.height = 32
			},
			{
				.data = icon32::wifi_75,
				.width = 38,
				.height = 32
			},
			{
				.data = icon32::wifi_full,
				.width = 38,
				.height = 32
			},
		};
	}

	~WifiDrawer() override {
		if (!display) return;
		delete[] _icons;
	}

	inline void setState(State state) {
		if (!display) return;
		if (state >= _maxFrame) return;
		_frame = state;
	}

	inline void draw() override {
		if (!display) return;
		_centerX = (_display->getWidth() / 2) - (_icons[_frame].width / 2);
		_centerY = (_display->getHeight() / 2) - (_icons[_frame].width / 2);

		_display->clearBuffer();
		_display->drawXBM(_centerX, _centerY, _icons[_frame].width, _icons[_frame].height, _icons[_frame].data);
		_display->sendBuffer();
	}

private:
	U8G2* _display;
	int _frame;
	int _maxFrame;
	int _centerX;
	int _centerY;
	Icons* _icons;
};