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

bool TouchSensor::init() {
    if (initialized) {
        Logger::warn("TOUCH", "Touch sensor already initialized");
        return true;
    }

    Logger::info("TOUCH", "Initializing touch sensor on pin %d", TOUCH_SENSOR_PIN);
    
    // Initialize callbacks vector
    callbacks.resize(GESTURE_COUNT, nullptr);
    
    // Create task
    BaseType_t result = xTaskCreate(touchTask, "TouchTask", 2048, nullptr, 1, &taskHandle);
    if (result != pdPASS) {
        Logger::error("TOUCH", "Failed to create touch task");
        return false;
    }
    
    initialized = true;
    Logger::info("TOUCH", "Touch sensor initialized successfully");
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
    return touchRead(TOUCH_SENSOR_PIN);
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
    int value = touchRead(TOUCH_SENSOR_PIN);
    return value < TOUCH_THRESHOLD;
}

void TouchSensor::detectGesture() {
    bool currentlyTouched = isTouched();
    unsigned long currentTime = millis();

    // Check for multi-tap timeout
    if (tapCount > 0 && (currentTime - lastTapTime) > TOUCH_DOUBLE_TRIPLE_WINDOW_MS) {
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
            Logger::debug("TOUCH", "Touch started");
        }
    } else if (!currentlyTouched && isTouching) {
        // Touch ended
        unsigned long duration = currentTime - touchStartTime;
        isTouching = false;
        lastDebounceTime = currentTime;

        if (duration <= TOUCH_ONE_TAP_MAX_MS) {
            // It's a tap
            if (tapCount > 0 && (currentTime - lastTapTime) <= TOUCH_DOUBLE_TRIPLE_WINDOW_MS) {
                tapCount++;
            } else {
                tapCount = 1;
            }
            lastTapTime = currentTime;
        } else if (duration >= TOUCH_HOLD_MIN_MS && duration < TOUCH_LONG_MIN_MS) {
            Logger::info("TOUCH", "Hold tap detected");
            if (callbacks[HOLD_TAP]) callbacks[HOLD_TAP]();
            tapCount = 0; // Reset multi-tap on hold
        } else if (duration >= TOUCH_LONG_MIN_MS) {
            Logger::info("TOUCH", "Long tap detected");
            if (callbacks[LONG_TAP]) callbacks[LONG_TAP]();
            tapCount = 0; // Reset multi-tap on long
        }
    } else if (currentlyTouched && isTouching) {
        // Still touching - check for long hold timeout
        unsigned long duration = currentTime - touchStartTime;
        if (duration > 10000) { // 10 second timeout to prevent stuck state
            Logger::warn("TOUCH", "Touch timeout - resetting state");
            isTouching = false;
            lastDebounceTime = currentTime;
            tapCount = 0;
        }
    }
}