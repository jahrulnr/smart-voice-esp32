#include "init.h"
#include <app/display/ui/boot.h>

Notification *notification = nullptr;
Face* faceDisplay = nullptr;
Microphone* microphone = nullptr;
Speaker* speaker = nullptr;

WifiManager wifiManager;
PubSubClient mqttClient;
TTS tts;

void setupApp(){
	log_i("[setupApp] initiate global variable");
	setupNotification();
	setupDisplay(SDA_PIN, SCL_PIN);

	BootSplashDrawer bootScreen(display);
	bootScreen.start();
	delay(1000);

	setupMicrophone();
	setupSpeaker();
	setupFaceDisplay(40);
	setupSpeechRecognition();

	delay(1);
	tts.begin();
	tts.speak("Halo! Pio Assistant is ready!");

	bootScreen.stop();
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

void setupFaceDisplay(uint16_t size) {
	if (!faceDisplay) {
		faceDisplay = new Face(display, SCREEN_WIDTH, SCREEN_HEIGHT, size);
		faceDisplay->Expression.GoTo_Normal();
		faceDisplay->LookFront();

		// Assign a weight to each emotion
		// Normal emotions
		faceDisplay->Behavior.SetEmotion(eEmotions::Normal, 1.0);
		faceDisplay->Behavior.SetEmotion(eEmotions::Unimpressed, 1.0);
		faceDisplay->Behavior.SetEmotion(eEmotions::Focused, 1.0);
		faceDisplay->Behavior.SetEmotion(eEmotions::Skeptic, 1.0);

		// Happy emotions
		faceDisplay->Behavior.SetEmotion(eEmotions::Happy, 1.0);
		faceDisplay->Behavior.SetEmotion(eEmotions::Glee, 1.0);
		faceDisplay->Behavior.SetEmotion(eEmotions::Awe, 1.0);

		// Sad emotions
		faceDisplay->Behavior.SetEmotion(eEmotions::Sad, 0.2);
		faceDisplay->Behavior.SetEmotion(eEmotions::Worried, 0.2);
		faceDisplay->Behavior.SetEmotion(eEmotions::Sleepy, 0.2);

		// Other emotions
		faceDisplay->Behavior.SetEmotion(eEmotions::Angry, 0.2);
		faceDisplay->Behavior.SetEmotion(eEmotions::Annoyed, 0.2);
		faceDisplay->Behavior.SetEmotion(eEmotions::Surprised, 0.2);
		faceDisplay->Behavior.SetEmotion(eEmotions::Frustrated, 0.2);
		faceDisplay->Behavior.SetEmotion(eEmotions::Suspicious, 0.2);
		faceDisplay->Behavior.SetEmotion(eEmotions::Squint, 0.2);
		faceDisplay->Behavior.SetEmotion(eEmotions::Furious, 0.2);
		faceDisplay->Behavior.SetEmotion(eEmotions::Scared, 0.2);

		faceDisplay->Behavior.Timer.SetIntervalMillis(10000);
		faceDisplay->Blink.Timer.SetIntervalMillis(1000);
		faceDisplay->Look.Timer.SetIntervalMillis(5000);

		faceDisplay->RandomBlink = true;
		faceDisplay->RandomBehavior =
		faceDisplay->RandomLook =
			false;
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