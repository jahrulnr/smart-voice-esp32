#include "touch_sensor.h"
#include <Arduino.h>
#include "config.h"

// Touch detection threshold (lower values indicate touch)
const int TOUCH_THRESHOLD = TOUCH_ANALOG_THRESHOLD_PRESSED;

// Timing constants for gesture detection (in milliseconds)
const unsigned long MULTI_TAP_WINDOW_MS = TOUCH_DOUBLE_TRIPLE_WINDOW_MS;   // Window to detect multi-taps
const unsigned long HOLD_DURATION_MS = 1000;     // Duration for hold gesture (not implemented in enum)

// Static member initialization
TaskHandle_t TouchSensor::taskHandle = nullptr;
std::vector<std::function<void()>> TouchSensor::callbacks(TouchSensor::GESTURE_COUNT);
bool TouchSensor::initialized = false;
bool TouchSensor::wasTouched = false;
int TouchSensor::tapCount = 0;
unsigned long TouchSensor::lastTapTime = 0;
bool TouchSensor::waitingForMultiTap = false;

bool TouchSensor::init() {
    if (initialized) return true;

    // Initialize touch pin (analog input)
    pinMode(TOUCH_SENSOR_PIN, INPUT);

    // Resize callbacks vector to match gesture count
    callbacks.resize(GESTURE_COUNT);

    // Create FreeRTOS task for touch polling
    xTaskCreatePinnedToCoreWithCaps(touchTask, "TouchTask", 4096, nullptr, 10, &taskHandle, 0, MALLOC_CAP_SPIRAM);

    initialized = true;
    ESP_LOGI("TouchSensor", "Touch sensor initialized");
    return true;
}

void TouchSensor::deinit() {
    if (!initialized) return;

    // Delete the task
    if (taskHandle != nullptr) {
        vTaskDelete(taskHandle);
        taskHandle = nullptr;
    }

    initialized = false;
    ESP_LOGI("TouchSensor", "Touch sensor deinitialized");
}

bool TouchSensor::setCallback(Gesture gesture, std::function<void()> callback) {
    if (gesture >= GESTURE_COUNT) return false;
    callbacks[gesture] = callback;
    return true;
}

int TouchSensor::getTouchValue() {
    return analogRead(TOUCH_SENSOR_PIN);
}

bool TouchSensor::isCurrentlyTouching() {
    return analogRead(TOUCH_SENSOR_PIN) < TOUCH_THRESHOLD;
}

void TouchSensor::touchTask(void* parameter) {
    while (true) {
        detectGesture();
        vTaskDelay(pdMS_TO_TICKS(TOUCH_POLL_INTERVAL_MS));  // Poll every configured interval
    }
}

bool TouchSensor::isTouched() {
    return analogRead(TOUCH_SENSOR_PIN) < TOUCH_THRESHOLD;
}

void TouchSensor::detectGesture() {
    bool currentlyTouched = isTouched();
    unsigned long currentTime = millis();

    if (currentlyTouched && !wasTouched) {
        // Touch started
        wasTouched = true;
    } else if (!currentlyTouched && wasTouched) {
        // Touch ended
				handleTap(currentTime);
        wasTouched = false;
    }

    // Check for multi-tap timeout
    if (waitingForMultiTap && (currentTime - lastTapTime > MULTI_TAP_WINDOW_MS)) {
        processTaps();
    }
}

void TouchSensor::handleTap(unsigned long currentTime) {
    tapCount++;
    lastTapTime = currentTime;
    waitingForMultiTap = true;
}

void TouchSensor::processTaps() {
    Gesture gesture;
    if (tapCount == 1) {
        gesture = ONE_TAP;
    } else if (tapCount == 2) {
        gesture = DOUBLE_TAP;
    } else if (tapCount == 3) {
        gesture = TRIPLE_TAP;
    } else {
        // Ignore taps beyond triple
        resetState();
        return;
    }

    // Call the callback if set
    if (callbacks[gesture]) {
        callbacks[gesture]();
    }

    resetState();
}

void TouchSensor::resetState() {
    tapCount = 0;
    waitingForMultiTap = false;
    lastTapTime = 0;
}