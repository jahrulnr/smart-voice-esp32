/*
 * VoiceDisplayCoordinator.cpp - Implementation of voice-display coordination
 */

#include "voice_display_coordinator.h"
#include "infrastructure/logger.h"

VoiceDisplayCoordinator::VoiceDisplayCoordinator()
    : _voiceCommandHandler(nullptr)
    , _displayManager(nullptr)
    , _initialized(false) {
}

VoiceDisplayCoordinator::~VoiceDisplayCoordinator() {
    // Clean up if needed
}

bool VoiceDisplayCoordinator::init(VoiceCommandHandler* voiceCommandHandler, DisplayManager* displayManager) {
    if (_initialized) {
        Logger::warn("VOICE_COORD", "VoiceDisplayCoordinator already initialized");
        return true;
    }

    if (!voiceCommandHandler || !displayManager) {
        Logger::error("VOICE_COORD", "Invalid voice command handler or display manager");
        return false;
    }

    _voiceCommandHandler = voiceCommandHandler;
    _displayManager = displayManager;
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
    _voiceCommandHandler->setEventCallback([this](VoiceEvent event, int commandId, int phraseId) {
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
            Logger::info("VOICE_COORD", "Command detected (id=%d, phrase=%d) - command processing handled internally",
                        commandId, phraseId);
            _displayManager->setState(DisplayState::PROCESSING);
            // Command processing is now handled internally by VoiceCommandHandler
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