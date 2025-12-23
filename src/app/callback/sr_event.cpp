#include "app/callback_list.h"

// Event callback for SR system
void sr_event_callback(void *arg, sr_event_t event, int command_id, int phrase_id) {
	switch (event) {
		case SR_EVENT_WAKEWORD:
			log_i("🎙️ Wake word 'Hi ESP' detected!");
			if (notification) {
				notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY_WAKEWORD);
			}
			// Switch to command listening mode
			SR::set_mode(SR_MODE_COMMAND);
			log_i("📞 Listening for commands...");
			break;

		case SR_EVENT_WAKEWORD_CHANNEL:
			log_i("🎙️ Wake word detected on channel: %d", command_id);
			if (notification) {
				notification->send(NOTIFICATION_DISPLAY, (void*)EVENT_DISPLAY_WAKEWORD);
			}
			SR::set_mode(SR_MODE_COMMAND);
			break;

		case SR_EVENT_COMMAND:
			log_i("✅ Command detected! ID=%d, Phrase=%d", command_id, phrase_id);

			// Map phrase_id to actual voice command (since phrase_id indexes the voice_commands array)
			if (phrase_id >= 0 && phrase_id < (sizeof(voice_commands) / sizeof(sr_cmd_t))) {
				const SR::csr_cmd_t* cmd = &voice_commands[phrase_id];
				log_i("   📝 You said: '%s'", cmd->str);
				log_i("   � Phonetic: '%s'", cmd->phoneme);
				log_i("   🆔 Command Group: %d, Phrase Index: %d", command_id, phrase_id);
			} else {
				log_i("   ❓ Unknown command mapping");
			}

			// Handle specific command groups based on command_id (from voice_commands array)
			// File: src/boot/constants.h:18
			switch (command_id) {
				case CMD_WAKEUP:
					log_i("💡 Action: %s", voice_commands[CMD_WAKEUP].str);
					if (notification) {
						// notification->send(NOTIFICATION_DISPLAY, (void*)&voice_commands[CMD_WAKEUP]);
					}
					break;
				case CMD_TIME:
					log_i("💡 Action: %s", voice_commands[CMD_TIME].str);
					if (notification) {
						// notification->send(NOTIFICATION_DISPLAY, (void*)&voice_commands[CMD_TIME]);
					}
					break;
				case CMD_WEATHER:
					log_i("💡 Action: %s", voice_commands[CMD_WEATHER].str);
					if (notification) {
						// notification->send(NOTIFICATION_DISPLAY, (void*)&voice_commands[CMD_WEATHER]);
					}
					break;
				case CMD_RECORD_AUDIO:
					log_i("💡 Action: %s", voice_commands[CMD_RECORD_AUDIO].str);
					if (notification) {
						// notification->send(NOTIFICATION_DISPLAY, (void*)&voice_commands[CMD_RECORD_AUDIO].str);
					}
					break;
				default:
					log_i("❓ Unknown command ID: %d", command_id);
					log_i("   📋 Available commands:");
					for (int i = 0; i < (sizeof(voice_commands) / sizeof(sr_cmd_t)); i++) {
						log_i("      [%d] Group %d: '%s' (%s)",
									i,
									voice_commands[i].command_id,
									voice_commands[i].str,
									voice_commands[i].phoneme);
					}
					break;
			}

			// Return to wake word mode after command
			SR::set_mode(SR_MODE_COMMAND);
			log_i("🔄 Returning to command mode");
			break;

		case SR_EVENT_TIMEOUT:
			SR::set_mode(SR_MODE_COMMAND);
			break;

		default:
			log_i("❓ Unknown SR event: %d", event);
			log_i("   📚 Known events:");
			log_i("      SR_EVENT_WAKEWORD: Wake word detected");
			log_i("      SR_EVENT_WAKEWORD_CHANNEL: Multi-channel wake word");
			log_i("      SR_EVENT_COMMAND: Voice command detected");
			log_i("      SR_EVENT_TIMEOUT: Command timeout occurred");
			SR::set_mode(SR_MODE_COMMAND);
			break;
	}
}