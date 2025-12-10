#include "touch_sensor.h"
#include "config.h"
#include "logger.h"

TaskHandle_t TouchSensor::taskHandle = nullptr;
std::vector<std::function<void()>> TouchSensor::callbacks(GESTURE_COUNT, nullptr);
bool TouchSensor::initialized = false;

// State variables for gesture detection
bool TouchSensor::wasTouched = false;
unsigned long TouchSensor::touchStartTime = 0;
int TouchSensor::tapCount = 0;
unsigned long TouchSensor::lastTapTime = 0;
bool TouchSensor::waitingForMultiTap = false;

bool TouchSensor::init() {
    if (initialized) {
        Logger::warn("TOUCH", "Touch sensor already initialized");
        return true;
    }

    Logger::info("TOUCH", "Initializing analog touch sensor on pin %d", TOUCH_SENSOR_PIN);

    // Configure pin as analog input
    pinMode(TOUCH_SENSOR_PIN, INPUT);

    // Initialize callbacks vector
    callbacks.resize(GESTURE_COUNT, nullptr);

    // Reset state
    resetState();

    // Create task
    BaseType_t result = xTaskCreatePinnedToCore(touchTask, "TouchTask", 4096, nullptr, 10, &taskHandle, 0);
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

void TouchSensor::resetState() {
    wasTouched = false;
    touchStartTime = 0;
    tapCount = 0;
    lastTapTime = 0;
    waitingForMultiTap = false;
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
    return isTouched();
}

void TouchSensor::touchTask(void* parameter) {
    while (true) {
        detectGesture();
        vTaskDelay(pdMS_TO_TICKS(TOUCH_POLL_INTERVAL_MS));
    }
}

bool TouchSensor::isTouched() {
    int value = analogRead(TOUCH_SENSOR_PIN);
    return (value < TOUCH_ANALOG_THRESHOLD_PRESSED);
}

void TouchSensor::detectGesture() {
    unsigned long currentTime = millis();
    bool currentlyTouched = isTouched();

    // Check for multi-tap timeout
    if (waitingForMultiTap && (currentTime - lastTapTime) > TOUCH_DOUBLE_TRIPLE_WINDOW_MS) {
        // Timeout expired, process the accumulated taps
        processTaps();
        waitingForMultiTap = false;
    }

    if (currentlyTouched && !wasTouched) {
        // Touch started
        touchStartTime = currentTime;
        Logger::debug("TOUCH", "Button pressed");
        wasTouched = true;

    } else if (!currentlyTouched && wasTouched) {
        // Touch ended
        unsigned long duration = currentTime - touchStartTime;
        Logger::debug("TOUCH", "Touch duration: %lu ms", duration);
        wasTouched = false;

        // Process the touch based on duration
        if (duration <= TOUCH_ONE_TAP_MAX_MS) {
            // It's a tap - add to multi-tap sequence
            handleTap(currentTime);
        } else if (duration >= TOUCH_LONG_MIN_MS) {
            // Long press
            Logger::info("TOUCH", "Long tap detected (%lu ms)", duration);
            if (callbacks[LONG_TAP]) callbacks[LONG_TAP]();
            resetState();
        } else if (duration >= TOUCH_HOLD_MIN_MS) {
            // Hold press
            Logger::info("TOUCH", "Hold tap detected (%lu ms)", duration);
            if (callbacks[HOLD_TAP]) callbacks[HOLD_TAP]();
            resetState();
        }
    }
}

void TouchSensor::handleTap(unsigned long currentTime) {
    if (!waitingForMultiTap) {
        // First tap in sequence
        tapCount = 1;
        lastTapTime = currentTime;
        waitingForMultiTap = true;
    } else {
        // Additional tap in sequence
        if ((currentTime - lastTapTime) <= TOUCH_DOUBLE_TRIPLE_WINDOW_MS) {
            tapCount++;
            lastTapTime = currentTime;
        } else {
            // Too late, start new sequence
            tapCount = 1;
            lastTapTime = currentTime;
        }
    }
}

void TouchSensor::processTaps() {
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
}