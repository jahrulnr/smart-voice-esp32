#pragma once
#include <esp32-hal.h>

static unsigned long lastActivity;

inline void updateActivity() {
  lastActivity = millis();
}

inline unsigned long getLastActivity() {
	return lastActivity;
}