/*
 * VoiceCommandHandler.h - Combined Voice Recognition and Command Processing
 *
 * This header combines voice recognition and command processing into a single
 * maintainable unit. It handles wake word detection, command recognition,
 * and command execution with appropriate responses.
 */

#pragma once

#include <vector>
#include "csr.h"
#include <audio/microphone.h>
#include "../application/gpt_service.h"
#include "../application/weather_service.h"
#include "ui/display.h"

#include "voice_constants.h"

// Callback function type for voice events
typedef std::function<void(VoiceEvent, int, int)> VoiceEventCallback;

class VoiceCommandHandler {
public:
    // Constructor/Destructor
    VoiceCommandHandler();
    ~VoiceCommandHandler();

    // Lifecycle
    bool init(Microphone* microphone, Services::GPTService* gptService, Services::WeatherService* weatherService, DisplayManager* displayManager, const VoiceConfig& config = {});
    void deinit();

    // Control
    bool startListening();
    bool stopListening();
    bool isListening() const;

    // Event callback registration
    void setEventCallback(VoiceEventCallback callback) { 
        _eventCallbacks.push_back(callback); 
    }
    void removeEventCallback(VoiceEventCallback callback) {
        // Note: This is a simple implementation - in production you'd want better callback management
        // For now, we'll just clear all callbacks if needed
        _eventCallbacks.clear();
    }

    // Command processing (called internally by SR events)
    void processCommand(int commandId, int phraseId);

private:
    // ESP-SR callbacks
    static void srEventCallback(void* arg, sr_event_t event, int command_id, int phrase_id);
    static esp_err_t audioFillCallback(void* arg, void* buffer, size_t buffer_size, size_t* bytes_read, TickType_t timeout);

    // Member variables
    Microphone* _microphone;
    Services::GPTService* _gptService;
    Services::WeatherService* _weatherService;
    DisplayManager* _displayManager;
    VoiceConfig _config;
    std::vector<VoiceEventCallback> _eventCallbacks;

    bool _initialized;
    bool _listening;
    SR::csr_cmd_t* _srCommands;
    sr_mode_t _currentMode;

    // Statistics
    int _wakeWordCount;
    int _commandCount;
    int _timeoutCount;
};