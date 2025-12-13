/*
 * VoiceCommandHandler.cpp - Combined Voice Recognition and Command Processing
 *
 * This file combines voice recognition and command processing into a single
 * maintainable unit. It handles wake word detection, command recognition,
 * and command execution with appropriate responses.
 */

#include "voice_command_handler.h"
#include "infrastructure/logger.h"
#include "voice/pico_tts.h"

// External dependencies
extern PicoTTS tts;  // Text-to-speech for responses

VoiceCommandHandler::VoiceCommandHandler()
    : _microphone(nullptr)
    , _gptService(nullptr)
    , _displayManager(nullptr)
    , _initialized(false)
    , _listening(false)
    , _srCommands(nullptr)
    , _currentMode(SR_MODE_OFF)
    , _wakeWordCount(0)
    , _commandCount(0)
    , _timeoutCount(0) {
}

VoiceCommandHandler::~VoiceCommandHandler() {
    deinit();
}

bool VoiceCommandHandler::init(Microphone* microphone, Services::GPTService* gptService, DisplayManager* displayManager, const VoiceConfig& config) {
    if (_initialized) {
        Logger::warn("VOICE_CMD", "VoiceCommandHandler already initialized");
        return true;
    }

    if (!microphone || !gptService || !displayManager) {
        Logger::error("VOICE_CMD", "Invalid microphone, GPT service, or display manager");
        return false;
    }

    _microphone = microphone;
    _gptService = gptService;
    _displayManager = displayManager;
    _config = config;

    // Use default commands if none provided
    if (!_config.commands || _config.commandCount == 0) {
        _config.commands = defaultCommands;
        _config.commandCount = sizeof(defaultCommands) / sizeof(defaultCommands[0]);
        Logger::info("VOICE_CMD", "Using default voice commands (%d commands)", _config.commandCount);
    }

    // Allocate SR command array
    _srCommands = new SR::csr_cmd_t[_config.commandCount];
    if (!_srCommands) {
        Logger::error("VOICE_CMD", "Failed to allocate SR commands");
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
        Logger::error("VOICE_CMD", "Failed to setup ESP-SR: %s", esp_err_to_name(ret));
        delete[] _srCommands;
        _srCommands = nullptr;
        return false;
    }

    // Start ESP-SR tasks
    ret = SR::sr_start(1, 0); // feed on core 1, detect on core 0
    if (ret != ESP_OK) {
        Logger::error("VOICE_CMD", "Failed to start ESP-SR: %s", esp_err_to_name(ret));
        SR::sr_stop();
        delete[] _srCommands;
        _srCommands = nullptr;
        return false;
    }

    _initialized = true;
    Logger::info("VOICE_CMD", "VoiceCommandHandler initialized with %d commands", _config.commandCount);
    return true;
}

void VoiceCommandHandler::deinit() {
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
    _gptService = nullptr;
    _displayManager = nullptr;

    Logger::info("VOICE_CMD", "VoiceCommandHandler deinitialized");
}

bool VoiceCommandHandler::startListening() {
    if (!_initialized) {
        Logger::error("VOICE_CMD", "VoiceCommandHandler not initialized");
        return false;
    }

    if (_listening) {
        // Set mode to wake word detection
        esp_err_t ret = SR::sr_set_mode(SR_MODE_COMMAND);
        if (ret != ESP_OK) {
            Logger::error("VOICE_CMD", "Failed to set command mode: %s", esp_err_to_name(ret));
            return false;
        }
        
        _currentMode = SR_MODE_COMMAND;
        return true;
    }

    // Set mode to wake word detection
    esp_err_t ret = SR::sr_set_mode(SR_MODE_WAKEWORD);
    if (ret != ESP_OK) {
        Logger::error("VOICE_CMD", "Failed to set wake word mode: %s", esp_err_to_name(ret));
        return false;
    }

    _currentMode = SR_MODE_WAKEWORD;
    _listening = true;

    Logger::info("VOICE_CMD", "Started listening for wake word");

    // Notify listeners via callback
    if (_eventCallback) {
        _eventCallback(VOICE_WAKEWORD_DETECTED, 0, 0);
    }

    return true;
}

bool VoiceCommandHandler::stopListening() {
    if (!_listening) {
        return true;
    }

    esp_err_t ret = SR::sr_set_mode(SR_MODE_OFF);
    if (ret != ESP_OK) {
        Logger::error("VOICE_CMD", "Failed to stop recognition: %s", esp_err_to_name(ret));
        return false;
    }

    _currentMode = SR_MODE_OFF;
    _listening = false;

    Logger::info("VOICE_CMD", "Stopped listening");

    return true;
}

bool VoiceCommandHandler::isListening() const {
    return _listening;
}

void VoiceCommandHandler::processCommand(int commandId, int phraseId) {
    if (!_initialized) {
        Logger::error("VOICE_CMD", "VoiceCommandHandler not initialized");
        return;
    }

    Logger::info("VOICE_CMD", "Processing command ID: %d, Phrase: %d", commandId, phraseId);

    // Set display to main status for all commands
    _displayManager->setState(DisplayState::MAIN_STATUS);

    // Process command
    if (commandId >= 0 && commandId < 6) {
        Logger::info("VOICE_CMD", "Handling %s command", commandNames[commandId]);
        tts.speak(commandResponses[commandId]);
    } else {
        Logger::warn("VOICE_CMD", "Unknown command ID: %d", commandId);
        tts.speak("Sorry, I don't understand that command.");
    }
}

// ESP-SR event callback
void VoiceCommandHandler::srEventCallback(void* arg, sr_event_t event, int command_id, int phrase_id) {
    VoiceCommandHandler* handler = static_cast<VoiceCommandHandler*>(arg);
    if (!handler) return;

    switch (event) {
        case SR_EVENT_WAKEWORD:
            handler->_wakeWordCount++;
            Logger::info("VOICE_CMD", "Wake word detected (%d total)", handler->_wakeWordCount);

            // Switch to command mode
            SR::sr_set_mode(SR_MODE_COMMAND);
            handler->_currentMode = SR_MODE_COMMAND;

            // Notify via callback
            if (handler->_eventCallback) {
                handler->_eventCallback(VOICE_WAKEWORD_DETECTED, command_id, phrase_id);
            }
            break;

        case SR_EVENT_WAKEWORD_CHANNEL:
            handler->_wakeWordCount++;
            Logger::info("VOICE_CMD", "Wake word channel detected (%d total)", handler->_wakeWordCount);

            // Switch to command mode
            SR::sr_set_mode(SR_MODE_COMMAND);
            handler->_currentMode = SR_MODE_COMMAND;

            // Notify via callback
            if (handler->_eventCallback) {
                handler->_eventCallback(VOICE_WAKEWORD_DETECTED, command_id, phrase_id);
            }
            break;

        case SR_EVENT_COMMAND:
            handler->_commandCount++;
            Logger::info("VOICE_CMD", "Command detected: id=%d, phrase=%d (%d total)",
                        command_id, phrase_id, handler->_commandCount);

            // Process the command
            handler->processCommand(command_id, phrase_id);

            // Switch to command mode
            SR::sr_set_mode(SR_MODE_COMMAND);
            handler->_currentMode = SR_MODE_COMMAND;

            // Notify via callback
            if (handler->_eventCallback) {
                handler->_eventCallback(VOICE_COMMAND_DETECTED, command_id, phrase_id);
            }
            break;

        case SR_EVENT_TIMEOUT:
            handler->_timeoutCount++;
            Logger::info("VOICE_CMD", "Recognition timeout (%d total)", handler->_timeoutCount);

            // Switch back to wake word mode
            SR::sr_set_mode(SR_MODE_WAKEWORD);
            handler->_currentMode = SR_MODE_WAKEWORD;

            // Notify via callback
            if (handler->_eventCallback) {
                handler->_eventCallback(VOICE_TIMEOUT, -1, -1);
            }
            break;

        default:
            Logger::warn("VOICE_CMD", "Unknown SR event: %d", event);
            break;
    }
}

// Audio fill callback for ESP-SR
esp_err_t VoiceCommandHandler::audioFillCallback(void* arg, void* buffer, size_t buffer_size, size_t* bytes_read, TickType_t timeout) {
    VoiceCommandHandler* handler = static_cast<VoiceCommandHandler*>(arg);
    if (!handler || !handler->_microphone) {
        *bytes_read = 0;
        return ESP_FAIL;
    }

    // Read audio samples from microphone
    size_t samples_requested = buffer_size / sizeof(int16_t);
    size_t samples_read = 0;

    bool success = handler->_microphone->readSamples(
        reinterpret_cast<int16_t*>(buffer),
        samples_requested,
        &samples_read
    );

    *bytes_read = samples_read * sizeof(int16_t);

    if (!success) {
        Logger::warn("VOICE_CMD", "Failed to read audio samples");
        return ESP_FAIL;
    }

    return ESP_OK;
}