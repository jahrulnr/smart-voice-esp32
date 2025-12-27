#include "init.h"
#include <app/display/ui/boot.h>

Notification *notification = nullptr;
Microphone* microphone = nullptr;
Speaker* speaker = nullptr;
Button button;

WifiManager wifiManager;
PubSubClient mqttClient;
TTS tts;

void setupApp(){
	log_i("[setupApp] initiate global variable");
	setupNotification();
	setupDisplay(SDA_PIN, SCL_PIN);

	BootSplashDrawer bootScreen(display);
	bootScreen.start();
	button.begin(BUTTON_PIN);
	ai.init(GPT_API_KEY);
	delay(1000);

	setupMicrophone();
	setupSpeaker();
	setupSpeechRecognition();

	delay(1);
	tts.begin();
	tts.speak("Halo! Pio Assistant is ready!");

	bootScreen.stop();
	speaker->playTone(NOTE_A4, 100);
}

void setupNotification() {
	if (!notification) {
		notification =
			new Notification();
	}
}

void setupMicrophone() {
	if (!microphone) {
		microphone = new Microphone(MIC_TYPE);
		microphone->init();
	}
}

void setupSpeaker() {
	if (!speaker) {
		speaker = new Speaker();
		speaker->init();
		speaker->start();
	}
}

void setupSpeechRecognition() {
	void* mic_instance = nullptr;

	log_i("🧠 Setting up Speech Recognition system...");

	// Start ESP-SR system with high-level API
	esp_err_t ret = SR::setup(
		srAudioCallback,                              // I2S data fill callback
		mic_instance,                                  // Microphone instance (I2SMicrophone or I2SMicrophone)
		SR_MODE_WAKEWORD,                              // Start in wake word mode
		voice_commands,                                // Commands array
		sizeof(voice_commands) / sizeof(sr_cmd_t),     // Number of commands
		srEventCallback,                             // Event callback
		NULL                                           // Event callback argument
	);

	if (ret == ESP_OK) {
		log_i("✅ Speech Recognition started successfully!");
		log_i("🎯 Say 'Hi ESP' to activate");
		log_i("📋 Loaded %d voice commands", sizeof(voice_commands) / sizeof(sr_cmd_t));
		for (int i = 0; i < (sizeof(voice_commands) / sizeof(sr_cmd_t)); i++) {
			log_i("   [%d] Group %d: '%s' -> '%s'",
				i,
				voice_commands[i].command_id,
				voice_commands[i].str,
				voice_commands[i].phoneme);
		}

		SR::start(1, 0);
	} else {
		log_i("❌ Failed to start Speech Recognition: %s", esp_err_to_name(ret));
	}
}