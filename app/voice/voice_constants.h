#pragma once

#include <Arduino.h>

// Voice recognition events
enum VoiceEvent {
    VOICE_WAKEWORD_DETECTED,      // Wake word detected
    VOICE_COMMAND_DETECTED,       // Command recognized
    VOICE_TIMEOUT,                // Recognition timeout
    VOICE_ERROR                   // Recognition error
};

// Voice command definitions
struct VoiceCommand {
    int id;
    const char* text;
    const char* phoneme;
};

// Voice recognition configuration
struct VoiceConfig {
    VoiceCommand* commands;            // Array of voice commands
    size_t commandCount;               // Number of commands
};

// Default voice commands - Generated using ESP-IDF multinet_g2p.py tool
static VoiceCommand defaultCommands[] = {
    {0, "hello",   "hcLb"},
    {1, "time",    "TiM"},
    {2, "weather", "Wfjk"},
    {3, "music",   "MYoZgK"},
    {4, "stop",    "STnP"},
    {5, "help",    "hfLP"},
    {6, "ask",     "aSK"}
};

// Command responses - matched by command ID
static const char* commandResponses[] = {
    "Hello! How can I help you?",                                             // 0: hello
    "The current time is 12:00 PM",                                           // 1: time (placeholder)
    "The weather today is sunny with a high of 75 degrees.",                  // 2: weather
    "Playing your favorite music.",                                           // 3: music
    "Stopping all activities.",                                               // 4: stop
    "I can help with time, weather, music, and more. Just say what you need!", // 5: help
};

static const char* commandNames[] = {
    "hello", "time", "weather", "music", "stop", "help"
};

static const int commandMax = 6;