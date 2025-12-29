#pragma once
#include <esp32-hal.h>

static unsigned long lastActivity;

inline void updateActivity(unsigned long time) {
  lastActivity = time;
}

inline unsigned long getLastActivity(unsigned long time) {
	return time - lastActivity;
}