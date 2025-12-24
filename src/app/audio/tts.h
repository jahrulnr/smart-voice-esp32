#pragma once
#include <picotts.h>
#include <esp_log.h>
#include "speaker.h"

extern Speaker* speaker;

class TTS {
public:
	TTS(){}

	inline void begin() {
		if (!picotts_init(8, callback, 1)) {
        ESP_LOGE("TTS", "Failed to initialize PicoTTS engine");
				return;
    }

    // Set additional callbacks
    picotts_set_error_notify(error);
    picotts_set_idle_notify(idle);
	}

	inline void speak(const char* text) {
		if (!speaker) {
			ESP_LOGE("TTS", "Speaker is not initialized");
			return;
		}

		picotts_add(text, strlen(text)+1);
	}

private:
	inline static void callback(int16_t *samples, unsigned int count){
		if (!speaker) {
			ESP_LOGE("TTS", "Speaker is not initialized");
			return;
		}

		size_t written = 0;
		speaker->writeSamples(samples, count * sizeof(int16_t), &written);
	}
	
	inline static void idle(void){
    ESP_LOGI("TTS", "TTS engine is idle");
		speaker->clear();
	}
	
	inline static void error(void){
    ESP_LOGE("TTS", "TTS engine encountered an error");
		speaker->clear();
	}
};

extern TTS tts;