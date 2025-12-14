#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include <Arduino.h>
#include "infrastructure/logger.h"
#include "ui/display.h"

/**
 * Sleep Manager for ESP32 Voice Assistant
 * Manages display sleep/wake based on user activity
 * Monitors touch and voice activity to determine when to sleep/wake the display
 */
class SleepManager {
public:
    SleepManager();
    ~SleepManager();

    /**
     * Initialize sleep manager
     * @param displayManager Pointer to display manager
     * @return true if successful
     */
    bool init(DisplayManager* displayManager);

    /**
     * Notify of touch activity
     * Resets inactivity timer
     */
    void onTouchActivity();

    /**
     * Notify of voice activity (wake word or command)
     * Resets inactivity timer and wakes display if sleeping
     */
    void onVoiceActivity();

    /**
     * Handle periodic tasks (check for sleep timeout)
     * Call this in main loop
     */
    void handle();

    /**
     * Check if display is currently sleeping
     * @return true if display is in sleep mode
     */
    bool isSleeping() const { return _isSleeping; }

    /**
     * Force wake the display
     */
    void wakeDisplay();

    /**
     * Force sleep the display
     */
    void sleepDisplay();

    /**
     * Get time since last activity (in milliseconds)
     * @return milliseconds since last activity
     */
    unsigned long getTimeSinceLastActivity() const;

private:
    DisplayManager* _displayManager;
    bool _initialized;
    bool _isSleeping;
    unsigned long _lastActivityTime;

    // Configuration
    static const unsigned long SLEEP_TIMEOUT_MS = 5 * 60 * 1000; // 5 minutes

    /**
     * Check if it's time to sleep
     * @return true if should sleep
     */
    bool shouldSleep() const;

    /**
     * Update last activity time to current time
     */
    void updateActivityTime();
};

#endif // SLEEP_MANAGER_H