#include "app/callbacks.h"
#include <core/time.h>

void srEventCallback(void *arg, sr_event_t event, int command_id, int phrase_id){
	ESP_LOGI("srEvent", "SR event detected, id=%d, command=%d, phrase_id=%d", event, command_id, phrase_id);
	switch (event) {
		case SR_EVENT_WAKEWORD:
			aiSts.start(
				micAudioCallback, 
				speakerAudioCallback,
				stsTools,
				nullptr,
				stsEvent,
				srDisconnectCallback
			);
			notification->send(NOTIFICATION_DISPLAY, EDISPLAY_LOADING);
			SR::set_mode(SR_MODE_WAKEWORD);
		break;
	}
}

// Event callback for SR system
// void srEventCallback(void *arg, sr_event_t event, int command_id, int phrase_id) {
// 	switch (event) {
// 		case SR_EVENT_WAKEWORD:
// 			log_i("🎙️ Wake word 'Hi ESP' detected!");
// 			if (notification) {
// 				notification->send(NOTIFICATION_DISPLAY, (int)EDISPLAY_WAKEWORD);
// 			}
// 			log_i("📞 Listening for commands...");
// 			tts.speak("Hi!");
// 			vTaskDelay(pdMS_TO_TICKS(500));
// 			// Switch to command listening mode
// 			SR::set_mode(SR_MODE_COMMAND);
// 			break;

// 		case SR_EVENT_WAKEWORD_CHANNEL:
// 			log_i("🎙️ Wake word detected on channel: %d", command_id);
// 			if (notification) {
// 				notification->send(NOTIFICATION_DISPLAY, (int)EDISPLAY_WAKEWORD);
// 			}
// 			vTaskDelay(pdMS_TO_TICKS(500));
// 			SR::set_mode(SR_MODE_COMMAND);
// 			break;

// 		case SR_EVENT_COMMAND:
// 			log_i("✅ Command detected! ID=%d, Phrase=%d", command_id, phrase_id);

// 			// Map phrase_id to actual voice command (since phrase_id indexes the voice_commands array)
// 			if (phrase_id >= 0 && phrase_id < (sizeof(voice_commands) / sizeof(sr_cmd_t))) {
// 				const SR::csr_cmd_t* cmd = &voice_commands[phrase_id];
// 				log_i("   📝 You said: '%s'", cmd->str);
// 				log_i("   � Phonetic: '%s'", cmd->phoneme);
// 				log_i("   🆔 Command Group: %d, Phrase Index: %d", command_id, phrase_id);
// 			} else {
// 				log_i("   ❓ Unknown command mapping");
// 			}

// 			// Handle specific command groups based on command_id (from voice_commands array)
// 			// File: src/boot/constants.h:18
// 			switch (command_id) {
// 				case CMD_WAKEUP:
// 					log_i("💡 Action: %s", voice_commands[CMD_WAKEUP].str);
// 					if (notification) {
// 						// notification->send(NOTIFICATION_DISPLAY, (void*)&voice_commands[CMD_WAKEUP]);
// 					}
// 					tts.speak("Hi, I'm cozmo!");
// 					break;
// 				case CMD_TIME:
// 					log_i("💡 Action: %s", voice_commands[CMD_TIME].str);
// 					if (notification) {
// 						// notification->send(NOTIFICATION_DISPLAY, (void*)&voice_commands[CMD_TIME]);
// 					}
// 					tts.speak((String("Now is ") + timeManager.getCurrentTime()).c_str());
// 					break;
// 				case CMD_WEATHER:
// 					log_i("💡 Action: %s", voice_commands[CMD_WEATHER].str);
// 					if (notification) {
// 						// notification->send(NOTIFICATION_DISPLAY, (void*)&voice_commands[CMD_WEATHER]);
// 					}
// 					tts.speak("The weather is coming soon!");
// 					break;
// 				case CMD_RECORD_AUDIO:
// 					log_i("💡 Action: %s", voice_commands[CMD_RECORD_AUDIO].str);
// 					if (notification) {
// 						notification->send(NOTIFICATION_DISPLAY, (int)EDISPLAY_WAKEWORD);
// 					}
// 					speaker->playTone(NOTE_C4, 500);
// 					break;
// 				default:
// 					log_i("❓ Unknown command ID: %d", command_id);
// 					log_i("   📋 Available commands:");
// 					for (int i = 0; i < (sizeof(voice_commands) / sizeof(sr_cmd_t)); i++) {
// 						log_i("      [%d] Group %d: '%s' (%s)",
// 									i,
// 									voice_commands[i].command_id,
// 									voice_commands[i].str,
// 									voice_commands[i].phoneme);
// 					}
// 					break;
// 			}

// 			// Return to wake word mode after command
// 			vTaskDelay(pdMS_TO_TICKS(500));
// 			log_i("🔄 Returning to command mode");
// 			SR::set_mode(SR_MODE_COMMAND);
// 			break;

// 		case SR_EVENT_TIMEOUT:
// 			notification->send(NOTIFICATION_DISPLAY, (int)EDISPLAY_NONE);
// 			SR::set_mode(SR_MODE_WAKEWORD);
// 			break;

// 		default:
// 			log_i("❓ Unknown SR event: %d", event);
// 			log_i("   📚 Known events:");
// 			log_i("      SR_EVENT_WAKEWORD: Wake word detected");
// 			log_i("      SR_EVENT_WAKEWORD_CHANNEL: Multi-channel wake word");
// 			log_i("      SR_EVENT_COMMAND: Voice command detected");
// 			log_i("      SR_EVENT_TIMEOUT: Command timeout occurred");
// 			vTaskDelay(pdMS_TO_TICKS(500));
// 			SR::set_mode(SR_MODE_COMMAND);
// 			break;
// 	}
// }