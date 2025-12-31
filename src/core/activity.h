#pragma once
#include <esp32-hal.h>

class SystemActivity {
public:
  SystemActivity(): _timer(millis()) {}
  inline void update(unsigned long time) {
    _timer = time;
  }
  inline unsigned long lastUpdate(unsigned long time) {
    return time - _timer;
  }
private:
  unsigned long _timer;
};

extern SystemActivity sysActivity;