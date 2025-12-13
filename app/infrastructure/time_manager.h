#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>

class TimeManager {
public:
    static TimeManager& getInstance();
    bool init();
    bool syncTime();
    String getCurrentTime();

private:
    TimeManager();
    bool initialized;
};

#endif