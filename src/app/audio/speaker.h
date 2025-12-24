#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>
#include <app_config.h>
#include "I2SSpeaker.h"

/**
 * Speaker class for handling I2S audio output
 */
class Speaker {
public:
	Speaker() : speaker(nullptr) {}
	~Speaker() {
		if (speaker) {
			delete speaker;
			speaker = nullptr;
		}
	}

	/**
	 * Initialize the speaker
	 * @return true if successful, false otherwise
	 */
	inline bool init() {
		speaker = new I2SSpeaker(I2S_SPEAKER_DOUT_PIN, I2S_SPEAKER_BCLK_PIN, I2S_SPEAKER_LRC_PIN, I2S_SPEAKER_PORT);
		if (!speaker) {
			ESP_LOGE("SPK", "Failed to create I2SSpeaker instance");
			return false;
		}

		esp_err_t err = speaker->init(SPEAKER_SAMPLE_RATE, SPEAKER_BIT_DEPTH, SPEAKER_CHANNELS);
		if (err != ESP_OK) {
			ESP_LOGE("SPK", "Failed to initialize speaker: %s", esp_err_to_name(err));
			return false;
		}

		ESP_LOGI("SPK", "Speaker initialized successfully");
		return true;
	}
	/**
	 * Start the speaker
	 * @return true if successful, false otherwise
	 */
	inline bool start() {
		if (!speaker) {
			ESP_LOGE("SPK", "Speaker not initialized");
			return false;
		}

		esp_err_t err = speaker->start();
		if (err != ESP_OK) {
			ESP_LOGE("SPK", "Failed to start speaker: %s", esp_err_to_name(err));
			return false;
		}

		ESP_LOGI("SPK", "Speaker started");
		return true;
	}

	/**
	 * Stop the speaker
	 * @return true if successful, false otherwise
	 */
	inline bool stop(){
		if (!speaker) {
			ESP_LOGE("SPK", "Speaker not initialized");
			return false;
		}

		esp_err_t err = speaker->stop();
		if (err != ESP_OK) {
			ESP_LOGE("SPK", "Failed to stop speaker: %s", esp_err_to_name(err));
			return false;
		}

		ESP_LOGI("SPK", "Speaker stopped");
		return true;
	}

	/**
	 * Write audio samples to the speaker
	 * @param buffer Buffer containing samples (int16_t)
	 * @param sampleCount Number of samples to write
	 * @param samplesWritten Pointer to store actual samples written
	 * @return true if successful, false otherwise
	 */
	inline bool writeSamples(const int16_t* buffer, size_t sampleCount, size_t* samplesWritten){
		if (!speaker) {
			ESP_LOGE("SPK", "Speaker not initialized");
			return false;
		}

		esp_err_t err = speaker->writeAudioData(buffer, sampleCount, samplesWritten, 500);
		if (err != ESP_OK) {
			ESP_LOGE("SPK", "Failed to write samples: %s", esp_err_to_name(err));
			return false;
		}

		return true;
	}

	/**
	 * Play a tone
	 * @param frequency Frequency in Hz
	 * @param duration Duration in ms
	 * @param amplitude Amplitude (0.0 to 1.0)
	 * @return Number of samples played, or -1 on error
	 */
	inline int playTone(uint32_t frequency, uint32_t duration, float amplitude = 0.5f){
		if (!speaker) {
			ESP_LOGE("SPK", "Speaker not initialized");
			return -1;
		}

		bool result = speaker->playTone(frequency, duration, amplitude);
		speaker->clear();

		return result;
	}

	/**
	 * Clear Speaker buffer
	 *
	 * @return ESP_OK if successful, error code otherwise
	 */
	inline esp_err_t clear() {
		if (!speaker) {
			ESP_LOGE("SPK", "Speaker not initialized");
			return ESP_FAIL;
		}

		return speaker->clear();
	}

private:
	I2SSpeaker* speaker;
};

#endif // SPEAKER_H