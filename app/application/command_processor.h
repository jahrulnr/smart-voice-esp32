/*
 * CommandProcessor.h - Processes voice commands and executes actions
 *
 * Handles the business logic for different voice commands detected by VoiceRecognizer.
 * Coordinates with other services to execute command actions.
 */

#pragma once

#include <Arduino.h>
#include "voice/voice_recognizer.h"
#include "gpt_service.h"
#include "ui/display.h"

class CommandProcessor {
public:
    CommandProcessor();
    ~CommandProcessor();

    /**
     * Initialize the command processor
     * @param gptService Pointer to GPT service for AI interactions
     * @param displayManager Pointer to display manager for UI updates
     * @return true if initialization successful
     */
    bool init(Services::GPTService* gptService, DisplayManager* displayManager);

    /**
     * Process a detected voice command
     * @param commandId The ID of the detected command
     * @param phraseId The phrase ID (if applicable)
     */
    void processCommand(int commandId, int phraseId);

private:
    Services::GPTService* _gptService;
    DisplayManager* _displayManager;
    bool _initialized;
};