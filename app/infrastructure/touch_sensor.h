#ifndef TOUCH_SENSOR_H
#define TOUCH_SENSOR_H

#include <Arduino.h>
#include <functional>
#include <vector>

/**
 * TouchSensor class for button/digital input gesture detection on ESP32
 * 
 * Supports gestures: ONE_TAP, DOUBLE_TAP, TRIPLE_TAP, HOLD_TAP, LONG_TAP
 * Uses polling with debouncing and duration thresholds
 * 
 * Agile-friendly: Modular, configurable, testable, extensible
 */
class TouchSensor {
public:
    enum Gesture {
        ONE_TAP,
        DOUBLE_TAP,
        TRIPLE_TAP,
        HOLD_TAP,
        LONG_TAP,
        GESTURE_COUNT // For array sizing
    };

    /**
     * Initialize the touch sensor
     * Starts a FreeRTOS task for polling
     * @return true if successful, false otherwise
     */
    static bool init();

    /**
     * Deinitialize the touch sensor
     * Stops the task and cleans up
     */
    static void deinit();

    /**
     * Set callback for a specific gesture
     * @param gesture The gesture type
     * @param callback Function to call on gesture detection
     * @return true if set successfully
     */
    static bool setCallback(Gesture gesture, std::function<void()> callback);

    /**
     * Get current touch value (for testing/debugging)
     * @return Raw touch sensor value
     */
    static int getTouchValue();

    /**
     * Check if currently touching (for testing)
     * @return true if touched
     */
    static bool isCurrentlyTouching();

private:
    static void touchTask(void* parameter);
    static bool isTouched();
    static void detectGesture();

    static TaskHandle_t taskHandle;
    static std::vector<std::function<void()>> callbacks;
    static unsigned long touchStartTime;
    static bool isTouching;
    static unsigned long lastDebounceTime;
    static bool initialized;
    static int tapCount;
    static unsigned long lastTapTime;
    static bool stableTouchState;
    static unsigned long lastTouchStateChange;
    static bool lastAnalogState;
};

#endif // TOUCH_SENSOR_H