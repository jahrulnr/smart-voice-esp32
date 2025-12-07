/*
 * VoiceRecognizer.cpp - Speech Recognition Implementation
 */

#include "voice_recognizer.h"
#include "infrastructure/logger.h"
#include "config.h"

// Default voice commands - Generated using ESP-IDF multinet_g2p.py tool
static VoiceCommand defaultCommands[] = {
    {0, "hello", "hcLb"},
    {1, "time", "TiM"},
    {2, "weather", "Wfjk"},
    {3, "music", "MYoZgK"},
    {4, "stop", "STnP"},
    {5, "help", "hfLP"}
};

VoiceRecognizer::VoiceRecognizer()
    : _microphone(nullptr)
    , _initialized(false)
    , _listening(false)
    , _srCommands(nullptr)
    , _currentMode(SR_MODE_OFF)
    , _wakeWordCount(0)
    , _commandCount(0)
    , _timeoutCount(0) {
}

VoiceRecognizer::~VoiceRecognizer() {
    deinit();
}

bool VoiceRecognizer::init(Microphone* microphone, const VoiceConfig& config) {
    if (_initialized) {
        Logger::warn("VOICE", "VoiceRecognizer already initialized");
        return true;
    }

    if (!microphone) {
        Logger::error("VOICE", "Microphone is null");
        return false;
    }

    _microphone = microphone;
    _config = config;

    // Use default commands if none provided
    if (!_config.commands || _config.commandCount == 0) {
        _config.commands = defaultCommands;
        _config.commandCount = sizeof(defaultCommands) / sizeof(defaultCommands[0]);
        Logger::info("VOICE", "Using default voice commands (%d commands)", _config.commandCount);
    }

    // Allocate SR command array
    _srCommands = new SR::csr_cmd_t[_config.commandCount];
    if (!_srCommands) {
        Logger::error("VOICE", "Failed to allocate SR commands");
        return false;
    }

    // Convert VoiceCommand to SR::csr_cmd_t
    for (size_t i = 0; i < _config.commandCount; i++) {
        _srCommands[i].command_id = _config.commands[i].id;
        strlcpy(_srCommands[i].str, _config.commands[i].text, SR_CMD_STR_LEN_MAX);
        strlcpy(_srCommands[i].phoneme, _config.commands[i].phoneme, SR_CMD_PHONEME_LEN_MAX);
    }

    // Initialize ESP-SR
    esp_err_t ret = SR::sr_setup(
        audioFillCallback,      // fill_cb
        this,                   // fill_cb_arg
        SR_CHANNELS_MONO,       // rx_chan (mono)
        SR_MODE_WAKEWORD,       // initial mode
        _srCommands,            // sr_commands
        _config.commandCount,   // cmd_number
        srEventCallback,        // cb
        this                    // cb_arg
    );

    if (ret != ESP_OK) {
        Logger::error("VOICE", "Failed to setup ESP-SR: %s", esp_err_to_name(ret));
        delete[] _srCommands;
        _srCommands = nullptr;
        return false;
    }

    // Start ESP-SR tasks
    ret = SR::sr_start(1, 0); // feed on core 1, detect on core 0
    if (ret != ESP_OK) {
        Logger::error("VOICE", "Failed to start ESP-SR: %s", esp_err_to_name(ret));
        SR::sr_stop();
        delete[] _srCommands;
        _srCommands = nullptr;
        return false;
    }

    _initialized = true;
    Logger::info("VOICE", "VoiceRecognizer initialized with %d commands", _config.commandCount);
    return true;
}

void VoiceRecognizer::deinit() {
    if (!_initialized) {
        return;
    }

    stopListening();

    if (_srCommands) {
        delete[] _srCommands;
        _srCommands = nullptr;
    }

    SR::sr_stop();
    _initialized = false;
    _microphone = nullptr;

    Logger::info("VOICE", "VoiceRecognizer deinitialized");
}

bool VoiceRecognizer::startListening() {
    if (!_initialized) {
        Logger::error("VOICE", "VoiceRecognizer not initialized");
        return false;
    }

    if (_listening) {
        Logger::warn("VOICE", "Already listening");
        return true;
    }

    // Set mode to wake word detection
    esp_err_t ret = SR::sr_set_mode(SR_MODE_WAKEWORD);
    if (ret != ESP_OK) {
        Logger::error("VOICE", "Failed to set wake word mode: %s", esp_err_to_name(ret));
        return false;
    }

    _currentMode = SR_MODE_WAKEWORD;
    _listening = true;

    Logger::info("VOICE", "Started listening for wake word");

    // Notify listeners via callback
    if (_eventCallback) {
        _eventCallback(VOICE_WAKEWORD_DETECTED, 0, 0);
    }

    return true;
}

bool VoiceRecognizer::stopListening() {
    if (!_listening) {
        return true;
    }

    esp_err_t ret = SR::sr_set_mode(SR_MODE_OFF);
    if (ret != ESP_OK) {
        Logger::error("VOICE", "Failed to stop recognition: %s", esp_err_to_name(ret));
        return false;
    }

    _currentMode = SR_MODE_OFF;
    _listening = false;

    Logger::info("VOICE", "Stopped listening");

    return true;
}

bool VoiceRecognizer::isListening() const {
    return _listening;
}

// ESP-SR event callback
void VoiceRecognizer::srEventCallback(void* arg, sr_event_t event, int command_id, int phrase_id) {
    VoiceRecognizer* recognizer = static_cast<VoiceRecognizer*>(arg);
    if (!recognizer) return;

    switch (event) {
        case SR_EVENT_WAKEWORD:
            recognizer->_wakeWordCount++;
            Logger::info("VOICE", "Wake word detected (%d total)", recognizer->_wakeWordCount);

            // Switch to command mode
            SR::sr_set_mode(SR_MODE_COMMAND);
            recognizer->_currentMode = SR_MODE_COMMAND;

            // Notify via callback
            if (recognizer->_eventCallback) {
                recognizer->_eventCallback(VOICE_WAKEWORD_DETECTED, command_id, phrase_id);
            }
            break;

        case SR_EVENT_COMMAND:
            recognizer->_commandCount++;
            Logger::info("VOICE", "Command detected: id=%d, phrase=%d (%d total)",
                        command_id, phrase_id, recognizer->_commandCount);

            // Switch back to wake word mode
            SR::sr_set_mode(SR_MODE_WAKEWORD);
            recognizer->_currentMode = SR_MODE_WAKEWORD;

            // Notify via callback
            if (recognizer->_eventCallback) {
                recognizer->_eventCallback(VOICE_COMMAND_DETECTED, command_id, phrase_id);
            }
            break;

        case SR_EVENT_TIMEOUT:
            recognizer->_timeoutCount++;
            Logger::info("VOICE", "Recognition timeout (%d total)", recognizer->_timeoutCount);

            // Switch back to wake word mode
            SR::sr_set_mode(SR_MODE_WAKEWORD);
            recognizer->_currentMode = SR_MODE_WAKEWORD;

            // Notify via callback
            if (recognizer->_eventCallback) {
                recognizer->_eventCallback(VOICE_TIMEOUT, -1, -1);
            }
            break;

        default:
            Logger::warn("VOICE", "Unknown SR event: %d", event);
            break;
    }
}

// Audio fill callback for ESP-SR
esp_err_t VoiceRecognizer::audioFillCallback(void* arg, void* buffer, size_t buffer_size, size_t* bytes_read, TickType_t timeout) {
    VoiceRecognizer* recognizer = static_cast<VoiceRecognizer*>(arg);
    if (!recognizer || !recognizer->_microphone) {
        *bytes_read = 0;
        return ESP_FAIL;
    }

    // Read audio samples from microphone
    size_t samples_requested = buffer_size / sizeof(int16_t);
    size_t samples_read = 0;

    bool success = recognizer->_microphone->readSamples(
        reinterpret_cast<int16_t*>(buffer),
        samples_requested,
        &samples_read
    );

    *bytes_read = samples_read * sizeof(int16_t);

    if (!success) {
        Logger::warn("VOICE", "Failed to read audio samples");
        return ESP_FAIL;
    }

    return ESP_OK;
}