#include "microphone.h"
#include "infrastructure/logger.h"

Microphone::Microphone() : mic(nullptr) {}

Microphone::~Microphone() {
    if (mic) {
        delete mic;
    }
}

bool Microphone::init() {
    mic = new I2SMicrophone(I2S_MIC_SD_PIN, I2S_MIC_SCK_PIN, I2S_MIC_WS_PIN, I2S_MIC_PORT);
    if (!mic) {
        Logger::error("MIC", "Failed to create I2SMicrophone instance");
        return false;
    }

    esp_err_t err = mic->init(MIC_SAMPLE_RATE, MIC_BIT_DEPTH, MIC_CHANNELS);
    if (err != ESP_OK) {
        Logger::error("MIC", "Failed to initialize microphone: %s", esp_err_to_name(err));
        return false;
    }

    Logger::info("MIC", "Microphone initialized successfully");
    return true;
}

bool Microphone::start() {
    if (!mic) {
        Logger::error("MIC", "Microphone not initialized");
        return false;
    }

    esp_err_t err = mic->start();
    if (err != ESP_OK) {
        Logger::error("MIC", "Failed to start microphone: %s", esp_err_to_name(err));
        return false;
    }

    Logger::info("MIC", "Microphone started");
    return true;
}

bool Microphone::stop() {
    if (!mic) {
        Logger::error("MIC", "Microphone not initialized");
        return false;
    }

    esp_err_t err = mic->stop();
    if (err != ESP_OK) {
        Logger::error("MIC", "Failed to stop microphone: %s", esp_err_to_name(err));
        return false;
    }

    Logger::info("MIC", "Microphone stopped");
    return true;
}

bool Microphone::readSamples(int16_t* buffer, size_t sampleCount, size_t* samplesRead) {
    if (!mic) {
        Logger::error("MIC", "Microphone not initialized");
        return false;
    }

    esp_err_t err = mic->readSamples(buffer, sampleCount, samplesRead);
    if (err != ESP_OK) {
        Logger::error("MIC", "Failed to read samples: %s", esp_err_to_name(err));
        return false;
    }

    return true;
}

int Microphone::readLevel() {
    if (!mic) {
        Logger::error("MIC", "Microphone not initialized");
        return -1;
    }

    return mic->readLevel();
}