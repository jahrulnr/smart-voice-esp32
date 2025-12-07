/*
 * VoiceDisplayCoordinator.h - Coordinates voice recognition events with display updates
 *
 * Handles the business logic of responding to voice events by updating the display state.
 * Keeps main.cpp focused on initialization only.
 */

#pragma once

#include "voice/voice_recognizer.h"
#include "ui/display.h"
#include "command_processor.h"

class VoiceDisplayCoordinator {
public:
    VoiceDisplayCoordinator();
    ~VoiceDisplayCoordinator();

    /**
     * Initialize the coordinator with voice recognizer, display manager, and command processor
     * @param voiceRecognizer Pointer to the voice recognizer instance
     * @param displayManager Pointer to the display manager instance
     * @param commandProcessor Pointer to the command processor instance
     * @return true if initialization successful
     */
    bool init(VoiceRecognizer* voiceRecognizer, DisplayManager* displayManager, CommandProcessor* commandProcessor);

    /**
     * Start coordinating voice events with display updates
     * @return true if coordination started successfully
     */
    bool start();

private:
    VoiceRecognizer* _voiceRecognizer;
    DisplayManager* _displayManager;
    CommandProcessor* _commandProcessor;
    bool _initialized;

    /**
     * Handle voice recognition events
     * @param event The voice event type
     * @param commandId Command ID (for command events)
     * @param phraseId Phrase ID (for command events)
     */
    void handleVoiceEvent(VoiceEvent event, int commandId, int phraseId);
};