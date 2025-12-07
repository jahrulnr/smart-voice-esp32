/*
 * CommandProcessor.cpp - Implementation of voice command processing
 */

#include "command_processor.h"
#include "infrastructure/logger.h"
#include "voice/pico_tts.h"

// External dependencies (these would need to be properly integrated)
extern PicoTTS tts;  // Text-to-speech for responses

CommandProcessor::CommandProcessor()
    : _gptService(nullptr)
    , _displayManager(nullptr)
    , _initialized(false) {
}

CommandProcessor::~CommandProcessor() {
    // Clean up if needed
}

bool CommandProcessor::init(Services::GPTService* gptService, DisplayManager* displayManager) {
    if (_initialized) {
        Logger::warn("CMD_PROC", "CommandProcessor already initialized");
        return true;
    }

    if (!gptService || !displayManager) {
        Logger::error("CMD_PROC", "Invalid GPT service or display manager");
        return false;
    }

    _gptService = gptService;
    _displayManager = displayManager;
    _initialized = true;

    Logger::info("CMD_PROC", "CommandProcessor initialized");
    return true;
}

void CommandProcessor::processCommand(int commandId, int phraseId) {
    if (!_initialized) {
        Logger::error("CMD_PROC", "CommandProcessor not initialized");
        return;
    }

    Logger::info("CMD_PROC", "Processing command ID: %d, Phrase: %d", commandId, phraseId);

    // Set display to main status for all commands
    _displayManager->setState(DisplayState::MAIN_STATUS);

    // Command response lookup table
    const char* responses[] = {
        "Hello! How can I help you?",                    // 0: hello
        "The current time is 12:00 PM",                 // 1: time (placeholder)
        "The weather today is sunny with a high of 75 degrees.", // 2: weather
        "Playing your favorite music.",                 // 3: music
        "Stopping all activities.",                     // 4: stop
        "I can help with time, weather, music, and more. Just say what you need!" // 5: help
    };

    const char* commandNames[] = {
        "hello", "time", "weather", "music", "stop", "help"
    };

    // Process command
    if (commandId >= 0 && commandId < 6) {
        Logger::info("CMD_PROC", "Handling %s command", commandNames[commandId]);
        tts.speak(responses[commandId]);
    } else {
        Logger::warn("CMD_PROC", "Unknown command ID: %d", commandId);
        tts.speak("Sorry, I don't understand that command.");
    }
}