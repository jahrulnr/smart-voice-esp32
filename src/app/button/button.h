#pragma once
#include <app_config.h>
#include <esp32-hal-gpio.h>

class Button {
enum State {
	STATE_OFF = -1,
	STATE_IDLE = 0,
	STATE_PRESSED,
};

public:
	Button(): _pin(0), _state(STATE_IDLE), _lastDebounceTime(0){}
	~Button(){}

	void begin(int pin) {
		_pin = pin;
		pinMode(_pin, INPUT);
	}

	void update() {
		int reading = digitalRead(_pin);
		if (reading == 1) {
			_lastDebounceTime = millis();
			_state = STATE_PRESSED;
		} else {
			_state = STATE_IDLE;
		}
	}

	bool isPressed() const { return _state; }
	unsigned long getLastTrigger() const { return _lastDebounceTime; }
	
private:
	int _pin;
	State _state;
	unsigned long _lastDebounceTime;
};

extern Button button;