#pragma once

#include "../interface.h"
#include "../icons.h"

class RecordDrawer : public DisplayDrawer {
public:
	enum State {
		MUTE = 0,
		IDLE,
		RECORDING
	};

	struct Icons {
		const unsigned char *data;
		int width;
		int height;
	};

	RecordDrawer(U8G2* display = nullptr): 
		_display(display), 
		_frame(0),
		_maxFrame(3) {
		if (!display) return;
		_icons = new Icons[_maxFrame] {
			{
				.data = icon32::microphone_muted,
				.width = 32,
				.height = 32
			}, {
				.data = icon48::microphone,
				.width = 48,
				.height = 48
			}, {
				.data = icon48::microphone_recording,
				.width = 48,
				.height = 48
			}
		};
	}

	~RecordDrawer() override {
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
		_display->drawXBM(_centerX, _centerY, _icons[_frame].width, _icons[_frame].width, _icons[_frame].data);
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