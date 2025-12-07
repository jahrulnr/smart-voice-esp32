/*
 * VoiceRecognizer.h - Speech Recognition for ESP32 Voice Assistant
 *
 * Provides wake word detection and command recognition using ESP-SR models.
 */

#pragma once

#include <Arduino.h>
#include <functional>
#include "csr.h"
#include <audio/microphone.h>

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

// Callback function type for voice events
typedef std::function<void(VoiceEvent, int, int)> VoiceEventCallback;

class VoiceRecognizer {
public:
    // Constructor/Destructor
    VoiceRecognizer();
    ~VoiceRecognizer();

    // Lifecycle
    bool init(Microphone* microphone, const VoiceConfig& config);
    void deinit();

    // Control
    bool startListening();
    bool stopListening();
    bool isListening() const;

    // Event callbacks
    void setEventCallback(VoiceEventCallback callback) { _eventCallback = callback; }

private:
    // ESP-SR callback
    static void srEventCallback(void* arg, sr_event_t event, int command_id, int phrase_id);

    // Audio fill callback for ESP-SR
    static esp_err_t audioFillCallback(void* arg, void* buffer, size_t buffer_size, size_t* bytes_read, TickType_t timeout);

    // Internal state
    Microphone* _microphone;
    VoiceConfig _config;
    bool _initialized;
    bool _listening;

    // ESP-SR data
    SR::csr_cmd_t* _srCommands;
    sr_mode_t _currentMode;

    // Event callback
    VoiceEventCallback _eventCallback;

    // Statistics
    uint32_t _wakeWordCount;
    uint32_t _commandCount;
    uint32_t _timeoutCount;
};