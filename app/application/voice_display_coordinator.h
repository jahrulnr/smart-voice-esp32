/*
 * VoiceDisplayCoordinator.h - Coordinates voice recognition events with display updates
 *
 * Handles the business logic of responding to voice events by updating the display state.
 * Keeps main.cpp focused on initialization only.
 */

#pragma once

#include "voice/voice_command_handler.h"
#include "ui/display.h"

class VoiceDisplayCoordinator {
public:
    VoiceDisplayCoordinator();
    ~VoiceDisplayCoordinator();

    /**
     * Initialize the coordinator with voice command handler and display manager
     * @param voiceCommandHandler Pointer to the voice command handler instance
     * @param displayManager Pointer to the display manager instance
     * @return true if initialization successful
     */
    bool init(VoiceCommandHandler* voiceCommandHandler, DisplayManager* displayManager);

    /**
     * Start coordinating voice events with display updates
     * @return true if coordination started successfully
     */
    bool start();

private:
    VoiceCommandHandler* _voiceCommandHandler;
    DisplayManager* _displayManager;
    bool _initialized;

    /**
     * Handle voice recognition events
     * @param event The voice event type
     * @param commandId Command ID (for command events)
     * @param phraseId Phrase ID (for command events)
     */
    void handleVoiceEvent(VoiceEvent event, int commandId, int phraseId);
};