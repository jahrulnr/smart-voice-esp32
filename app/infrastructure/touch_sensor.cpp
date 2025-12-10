#include "touch_sensor.h"
#include "config.h"
#include "logger.h"

TaskHandle_t TouchSensor::taskHandle = nullptr;
std::vector<std::function<void()>> TouchSensor::callbacks(GESTURE_COUNT, nullptr);
unsigned long TouchSensor::touchStartTime = 0;
bool TouchSensor::isTouching = false;
unsigned long TouchSensor::lastDebounceTime = 0;
bool TouchSensor::initialized = false;
int TouchSensor::tapCount = 0;
unsigned long TouchSensor::lastTapTime = 0;
bool TouchSensor::stableTouchState = false;
unsigned long TouchSensor::lastTouchStateChange = 0;
bool TouchSensor::lastAnalogState = false;

bool TouchSensor::init() {
    if (initialized) {
        Logger::warn("TOUCH", "Touch sensor already initialized");
        return true;
    }

    Logger::info("TOUCH", "Initializing analog touch sensor on pin %d", TOUCH_SENSOR_PIN);
    
    // Configure pin as analog input (no pull-up needed)
    pinMode(TOUCH_SENSOR_PIN, INPUT);
    
    // Initialize callbacks vector
    callbacks.resize(GESTURE_COUNT, nullptr);
    
    // Create task
    BaseType_t result = xTaskCreatePinnedToCore(touchTask, "TouchTask", 4096, nullptr, 3, &taskHandle, 0);
    if (result != pdPASS) {
        Logger::error("TOUCH", "Failed to create touch task");
        return false;
    }
    
    initialized = true;
    Logger::info("TOUCH", "Analog touch sensor initialized successfully");
    return true;
}

void TouchSensor::deinit() {
    if (!initialized) return;
    
    if (taskHandle != nullptr) {
        vTaskDelete(taskHandle);
        taskHandle = nullptr;
    }
    
    callbacks.clear();
    initialized = false;
    Logger::info("TOUCH", "Touch sensor deinitialized");
}

bool TouchSensor::setCallback(Gesture gesture, std::function<void()> callback) {
    if (gesture >= GESTURE_COUNT || gesture < 0) {
        Logger::error("TOUCH", "Invalid gesture: %d", gesture);
        return false;
    }
    callbacks[gesture] = callback;
    Logger::info("TOUCH", "Callback set for gesture %d", gesture);
    return true;
}

int TouchSensor::getTouchValue() {
    return analogRead(TOUCH_SENSOR_PIN);
}

bool TouchSensor::isCurrentlyTouching() {
    return isTouching;
}

void TouchSensor::touchTask(void* parameter) {
    while (true) {
        detectGesture();
        vTaskDelay(pdMS_TO_TICKS(TOUCH_POLL_INTERVAL_MS));
    }
}

bool TouchSensor::isTouched() {
    int value = analogRead(TOUCH_SENSOR_PIN);
    bool currentState;
    if (TOUCH_ACTIVE_LOW) {
        if (!lastAnalogState) {
            currentState = (value < TOUCH_ANALOG_THRESHOLD_PRESSED);
        } else {
            currentState = (value < TOUCH_ANALOG_THRESHOLD_RELEASED);
        }
    } else {
        if (!lastAnalogState) {
            currentState = (value > TOUCH_ANALOG_THRESHOLD_PRESSED);
        } else {
            currentState = (value > TOUCH_ANALOG_THRESHOLD_RELEASED);
        }
    }
    if (currentState != lastAnalogState) {
        Logger::debug("TOUCH", "ADC: %d, state: %s -> %s", value, lastAnalogState ? "pressed" : "released", currentState ? "pressed" : "released");
    }
    lastAnalogState = currentState;
    return currentState;
}

void TouchSensor::detectGesture() {
    bool rawTouched = isTouched();
    unsigned long currentTime = millis();

    // Debounce touch state
    if (rawTouched != stableTouchState) {
        if (currentTime - lastTouchStateChange > TOUCH_DEBOUNCE_MS) {
            stableTouchState = rawTouched;
            lastTouchStateChange = currentTime;
        }
    } else {
        lastTouchStateChange = currentTime; // Reset debounce timer when stable
    }

    // Use debounced state for gesture detection
    bool currentlyTouched = stableTouchState;

    // Check for multi-tap timeout only when not touching
    if (!currentlyTouched && tapCount > 0 && (currentTime - lastTapTime) > TOUCH_DOUBLE_TRIPLE_WINDOW_MS) {
        // Time window expired, trigger based on tap count
        if (tapCount == 1) {
            Logger::info("TOUCH", "One tap detected");
            if (callbacks[ONE_TAP]) callbacks[ONE_TAP]();
        } else if (tapCount == 2) {
            Logger::info("TOUCH", "Double tap detected");
            if (callbacks[DOUBLE_TAP]) callbacks[DOUBLE_TAP]();
        } else if (tapCount >= 3) {
            Logger::info("TOUCH", "Triple tap detected");
            if (callbacks[TRIPLE_TAP]) callbacks[TRIPLE_TAP]();
        }
        tapCount = 0;
    }

    if (currentlyTouched && !isTouching) {
        // Touch started
        if (currentTime - lastDebounceTime > TOUCH_DEBOUNCE_MS) {
            isTouching = true;
            touchStartTime = currentTime;
            Logger::debug("TOUCH", "Button pressed");
        }
    } else if (!currentlyTouched && isTouching) {
        // Touch ended
        unsigned long duration = currentTime - touchStartTime;
        isTouching = false;
        lastDebounceTime = currentTime;

        Logger::debug("TOUCH", "Touch duration: %lu ms", duration);

        if (duration <= TOUCH_ONE_TAP_MAX_MS) {
            // It's a tap
            if (tapCount > 0 && (currentTime - lastTapTime) <= TOUCH_DOUBLE_TRIPLE_WINDOW_MS) {
                tapCount++;
            } else {
                tapCount = 1;
            }
            lastTapTime = currentTime;
        } else if (duration > TOUCH_ONE_TAP_MAX_MS && duration < TOUCH_LONG_MIN_MS) {
            Logger::info("TOUCH", "Hold tap detected (%lu ms)", duration);
            if (callbacks[HOLD_TAP]) callbacks[HOLD_TAP]();
            tapCount = 0; // Reset multi-tap on hold
        } else if (duration >= TOUCH_LONG_MIN_MS) {
            Logger::info("TOUCH", "Long tap detected (%lu ms)", duration);
            if (callbacks[LONG_TAP]) callbacks[LONG_TAP]();
            tapCount = 0; // Reset multi-tap on long
        }
    } else if (currentlyTouched && isTouching) {
        // Still touching - check for long hold timeout
        unsigned long duration = currentTime - touchStartTime;
        if (duration > 10000) { // 10 second timeout to prevent stuck state
            Logger::warn("TOUCH", "Button timeout - resetting state");
            isTouching = false;
            lastDebounceTime = currentTime;
            tapCount = 0;
        }
    }
}