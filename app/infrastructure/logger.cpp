#include "logger.h"
#include <stdarg.h>

// Static member initialization
Logger::LogLevel Logger::currentLevel = Logger::INFO;  // Default to INFO level

void Logger::init() {
    Serial.begin(115200);
    Serial.setTimeout(100);
    Serial.println("Logger initialized");
}

void Logger::setLogLevel(LogLevel level) {
    currentLevel = level;
}

void Logger::debug(const char* tag, const char* format, ...) {
    if (currentLevel > DEBUG) return;
    va_list args;
    va_start(args, format);
    log(DEBUG, "DEBUG", tag, format, args);
    va_end(args);
}

void Logger::info(const char* tag, const char* format, ...) {
    if (currentLevel > INFO) return;
    va_list args;
    va_start(args, format);
    log(INFO, "INFO", tag, format, args);
    va_end(args);
}

void Logger::warn(const char* tag, const char* format, ...) {
    if (currentLevel > WARN) return;
    va_list args;
    va_start(args, format);
    log(WARN, "WARN", tag, format, args);
    va_end(args);
}

void Logger::error(const char* tag, const char* format, ...) {
    if (currentLevel > ERROR) return;
    va_list args;
    va_start(args, format);
    log(ERROR, "ERROR", tag, format, args);
    va_end(args);
}

void Logger::log(LogLevel level, const char* levelStr, const char* tag, const char* format, va_list args) {
    // Print timestamp (millis for simplicity)
    Serial.printf("[%lu] [%s] [%s] ", millis(), levelStr, tag);
    
    // Print the formatted message
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);
    Serial.println(buffer);
}