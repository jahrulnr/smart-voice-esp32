/*
 * VoiceDisplayCoordinator.cpp - Implementation of voice-display coordination
 */

#include "voice_display_coordinator.h"
#include "infrastructure/logger.h"

VoiceDisplayCoordinator::VoiceDisplayCoordinator()
    : _voiceRecognizer(nullptr)
    , _displayManager(nullptr)
    , _commandProcessor(nullptr)
    , _initialized(false) {
}

VoiceDisplayCoordinator::~VoiceDisplayCoordinator() {
    // Clean up if needed
}

bool VoiceDisplayCoordinator::init(VoiceRecognizer* voiceRecognizer, DisplayManager* displayManager, CommandProcessor* commandProcessor) {
    if (_initialized) {
        Logger::warn("VOICE_COORD", "VoiceDisplayCoordinator already initialized");
        return true;
    }

    if (!voiceRecognizer || !displayManager || !commandProcessor) {
        Logger::error("VOICE_COORD", "Invalid voice recognizer, display manager, or command processor");
        return false;
    }

    _voiceRecognizer = voiceRecognizer;
    _displayManager = displayManager;
    _commandProcessor = commandProcessor;
    _initialized = true;

    Logger::info("VOICE_COORD", "VoiceDisplayCoordinator initialized");
    return true;
}

bool VoiceDisplayCoordinator::start() {
    if (!_initialized) {
        Logger::error("VOICE_COORD", "VoiceDisplayCoordinator not initialized");
        return false;
    }

    // Set up the event callback using a lambda that captures this instance
    _voiceRecognizer->setEventCallback([this](VoiceEvent event, int commandId, int phraseId) {
        this->handleVoiceEvent(event, commandId, phraseId);
    });

    Logger::info("VOICE_COORD", "Voice-display coordination started");
    return true;
}

void VoiceDisplayCoordinator::handleVoiceEvent(VoiceEvent event, int commandId, int phraseId) {
    switch (event) {
        case VOICE_WAKEWORD_DETECTED:
            Logger::info("VOICE_COORD", "Wake word detected - switching to listening mode");
            _displayManager->setState(DisplayState::LISTENING);
            break;

        case VOICE_COMMAND_DETECTED:
            Logger::info("VOICE_COORD", "Command detected (id=%d, phrase=%d) - processing command",
                        commandId, phraseId);
            _displayManager->setState(DisplayState::PROCESSING);

            // Process the actual command
            if (_commandProcessor) {
                _commandProcessor->processCommand(commandId, phraseId);
            }
            break;

        case VOICE_TIMEOUT:
            Logger::info("VOICE_COORD", "Voice recognition timeout - returning to main status");
            _displayManager->setState(DisplayState::MAIN_STATUS);
            break;

        case VOICE_ERROR:
            Logger::error("VOICE_COORD", "Voice recognition error - switching to error display");
            _displayManager->setState(DisplayState::ERROR);
            break;

        default:
            Logger::warn("VOICE_COORD", "Unknown voice event: %d", event);
            break;
    }
}