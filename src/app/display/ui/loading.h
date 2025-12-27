#pragma once

#include "../interface.h"
#include "../icons.h"

class LoadingDrawer : public DisplayDrawer {
public:
	LoadingDrawer(U8G2* display = nullptr): _display(display) {
		_icons.push_back(icon24::hourglass0);
		_icons.push_back(icon24::hourglass1);
		_icons.push_back(icon24::hourglass2);
		_icons.push_back(icon24::hourglass3);
		_icons.push_back(icon24::hourglass4);
		_icons.push_back(icon24::hourglass5);
		_icons.push_back(icon24::hourglass6);
		_icons.push_back(icon24::hourglass);
		_frame = 0;
		_maxFrame = _icons.size();
		_centerX = (_display->getWidth() / 2) - 12;
		_centerY = (_display->getHeight() / 2) - 12;
	}
	~LoadingDrawer() override {
		_icons.clear();
	}

	inline void draw() override {
		_display->clearBuffer();
		if (_frame >= _maxFrame) _frame = 0;
		_display->drawXBM(_centerX, _centerY, 24, 24, _icons[_frame++]);
		_display->sendBuffer();
	}

private:
	U8G2* _display;
	int _frame;
	int _maxFrame;
	int _centerX;
	int _centerY;
	std::vector<const unsigned char*> _icons;
};