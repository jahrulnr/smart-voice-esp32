#include "time_manager.h"
#include <time.h>
#include "infrastructure/logger.h"

TimeManager& TimeManager::getInstance() {
    static TimeManager instance;
    return instance;
}

TimeManager::TimeManager() : initialized(false) {}

bool TimeManager::init() {
    if (initialized) {
        Logger::warn("TIME", "Time manager already initialized");
        return true;
    }

    Logger::info("TIME", "Initializing time manager...");

    // Set timezone to Asia/Jakarta (UTC+7, no DST)
    configTime(25200, 0, "pool.ntp.org", "time.google.com");

    initialized = true;
    Logger::info("TIME", "Time manager initialized");
    return true;
}

bool TimeManager::syncTime() {
    if (!initialized) {
        Logger::error("TIME", "Time manager not initialized");
        return false;
    }

    Logger::info("TIME", "Syncing time with NTP...");

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 10000)) { // 10 second timeout
        Logger::error("TIME", "Failed to sync time with NTP");
        return false;
    }

    Logger::info("TIME", "Time synced successfully");
    return true;
}

String TimeManager::getCurrentTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Time not synced";
    }

    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S WIB", &timeinfo);
    return String(buffer);
}