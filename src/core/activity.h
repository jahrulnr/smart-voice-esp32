#pragma once
#include <esp32-hal.h>

class SystemActivity {
public:
  SystemActivity(): _timer(0) {}
  inline void update(unsigned long time = 0) {
    if (time == 0) time = millis();
    _timer = time;
  }
  inline unsigned long lastUpdate(unsigned long time = 0) {
    if (time == 0) time = millis();
    unsigned long res = time - _timer;
    return res;
  }
private:
  unsigned long _timer;
};

extern SystemActivity* sysActivity;