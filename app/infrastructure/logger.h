#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

/**
 * Logger class for centralized logging in the ESP32 voice assistant
 * 
 * Features:
 * - Multiple log levels: DEBUG, INFO, WARN, ERROR
 * - Tagging for categorization (e.g., "VOICE", "AUDIO")
 * - Output to Serial (expandable to file/network later)
 * - Conditional compilation based on build flags
 */
class Logger {
public:
    enum LogLevel {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3
    };

    /**
     * Initialize the logger
     * Call this in setup() to configure Serial
     */
    static void init();

    /**
     * Set the minimum log level (messages below this level are ignored)
     * @param level Minimum level to log
     */
    static void setLogLevel(LogLevel level);

    /**
     * Log a debug message
     * @param tag Category tag (e.g., "VOICE")
     * @param format printf-style format string
     * @param ... Arguments for format
     */
    static void debug(const char* tag, const char* format, ...);

    /**
     * Log an info message
     * @param tag Category tag
     * @param format printf-style format string
     * @param ... Arguments for format
     */
    static void info(const char* tag, const char* format, ...);

    /**
     * Log a warning message
     * @param tag Category tag
     * @param format printf-style format string
     * @param ... Arguments for format
     */
    static void warn(const char* tag, const char* format, ...);

    /**
     * Log an error message
     * @param tag Category tag
     * @param format printf-style format string
     * @param ... Arguments for format
     */
    static void error(const char* tag, const char* format, ...);

private:
    static LogLevel currentLevel;
    static void log(LogLevel level, const char* levelStr, const char* tag, const char* format, va_list args);
};

#endif // LOGGER_H