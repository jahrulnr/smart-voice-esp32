#include "sleep_manager.h"

SleepManager::SleepManager()
    : _displayManager(nullptr)
    , _initialized(false)
    , _isSleeping(false)
    , _lastActivityTime(0) {
}

SleepManager::~SleepManager() {
    // Wake display on destruction if sleeping
    if (_isSleeping && _displayManager && _displayManager->getDisplay()) {
        _displayManager->getDisplay()->setPowerSave(0);
        _isSleeping = false;
    }
}

bool SleepManager::init(DisplayManager* displayManager) {
    if (_initialized) {
        Logger::warn("SLEEP", "Sleep manager already initialized");
        return true;
    }

    if (!displayManager) {
        Logger::error("SLEEP", "Display manager is null");
        return false;
    }

    _displayManager = displayManager;
    _initialized = true;
    _lastActivityTime = millis();

    Logger::info("SLEEP", "Sleep manager initialized with %d minute timeout", SLEEP_TIMEOUT_MS / 60000);
    return true;
}

void SleepManager::onTouchActivity() {
    if (!_initialized) return;

    updateActivityTime();

    // Wake display if sleeping
    if (_isSleeping) {
        wakeDisplay();
    }

    Logger::debug("SLEEP", "Touch activity detected");
}

void SleepManager::onVoiceActivity() {
    if (!_initialized) return;

    updateActivityTime();

    // Wake display if sleeping
    if (_isSleeping) {
        wakeDisplay();
    }

    Logger::debug("SLEEP", "Voice activity detected");
}

void SleepManager::handle() {
    if (!_initialized || !_displayManager) return;

    // Check if we should sleep
    if (!_isSleeping && shouldSleep()) {
        sleepDisplay();
    }
}

void SleepManager::wakeDisplay() {
    if (!_initialized || !_displayManager || !_displayManager->getDisplay()) return;

    _displayManager->getDisplay()->setPowerSave(0); // Turn display on
    _isSleeping = false;
    _lastActivityTime = millis();

    Logger::info("SLEEP", "Display woken up");
}

void SleepManager::sleepDisplay() {
    if (!_initialized || !_displayManager || !_displayManager->getDisplay()) return;

    _displayManager->getDisplay()->setPowerSave(1); // Turn display off
    _isSleeping = true;

    Logger::info("SLEEP", "Display put to sleep after %d minutes of inactivity", SLEEP_TIMEOUT_MS / 60000);
}

unsigned long SleepManager::getTimeSinceLastActivity() const {
    if (_lastActivityTime == 0) return 0;
    return millis() - _lastActivityTime;
}

bool SleepManager::shouldSleep() const {
    if (_lastActivityTime == 0) return false;
    return (millis() - _lastActivityTime) >= SLEEP_TIMEOUT_MS;
}

void SleepManager::updateActivityTime() {
    _lastActivityTime = millis();
}