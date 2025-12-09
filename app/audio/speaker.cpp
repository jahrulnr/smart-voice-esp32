#include "speaker.h"
#include "infrastructure/logger.h"

Speaker::Speaker() : speaker(nullptr) {}

Speaker::~Speaker() {
    if (speaker) {
        delete speaker;
    }
}

bool Speaker::init() {
    speaker = new I2SSpeaker(I2S_SPEAKER_DOUT_PIN, I2S_SPEAKER_BCLK_PIN, I2S_SPEAKER_LRC_PIN, I2S_SPEAKER_PORT);
    if (!speaker) {
        Logger::error("SPK", "Failed to create I2SSpeaker instance");
        return false;
    }

    esp_err_t err = speaker->init(SPEAKER_SAMPLE_RATE, SPEAKER_BIT_DEPTH, SPEAKER_CHANNELS);
    if (err != ESP_OK) {
        Logger::error("SPK", "Failed to initialize speaker: %s", esp_err_to_name(err));
        return false;
    }

    Logger::info("SPK", "Speaker initialized successfully");
    return true;
}

bool Speaker::start() {
    if (!speaker) {
        Logger::error("SPK", "Speaker not initialized");
        return false;
    }

    esp_err_t err = speaker->start();
    if (err != ESP_OK) {
        Logger::error("SPK", "Failed to start speaker: %s", esp_err_to_name(err));
        return false;
    }

    Logger::info("SPK", "Speaker started");
    return true;
}

bool Speaker::stop() {
    if (!speaker) {
        Logger::error("SPK", "Speaker not initialized");
        return false;
    }

    esp_err_t err = speaker->stop();
    if (err != ESP_OK) {
        Logger::error("SPK", "Failed to stop speaker: %s", esp_err_to_name(err));
        return false;
    }

    Logger::info("SPK", "Speaker stopped");
    return true;
}

bool Speaker::writeSamples(const int16_t* buffer, size_t sampleCount, size_t* samplesWritten) {
    if (!speaker) {
        Logger::error("SPK", "Speaker not initialized");
        return false;
    }

    esp_err_t err = speaker->writeSamples(buffer, sampleCount, samplesWritten);
    if (err != ESP_OK) {
        Logger::error("SPK", "Failed to write samples: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

int Speaker::playTone(uint32_t frequency, uint32_t duration, float amplitude) {
    if (!speaker) {
        Logger::error("SPK", "Speaker not initialized");
        return -1;
    }

    bool result = speaker->playTone(frequency, duration, amplitude);
    speaker->clear();
    
    return result;
}

esp_err_t Speaker::clear() {
    if (!speaker) {
        Logger::error("SPK", "Speaker not initialized");
        return ESP_FAIL;
    }

    return speaker->clear();
}