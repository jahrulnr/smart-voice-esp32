#include "pico_tts.h"
#include "infrastructure/logger.h"

// Static member initialization
Speaker* PicoTTS::speakerInstance = nullptr;

PicoTTS::PicoTTS() : initialized(false) {}

PicoTTS::~PicoTTS() {
    if (initialized) {
        shutdown();
    }
}

bool PicoTTS::init(Speaker* speaker) {
    if (initialized) {
        Logger::warn("TTS", "PicoTTS already initialized");
        return true;
    }

    if (!speaker) {
        Logger::error("TTS", "Speaker instance is null");
        return false;
    }

    speakerInstance = speaker;

    Logger::info("TTS", "Initializing PicoTTS...");

    // Initialize picoTTS with callbacks
    if (!picotts_init(10, outputCallback, 0)) {  // Priority 1, no core affinity
        Logger::error("TTS", "Failed to initialize PicoTTS engine");
        return false;
    }

    // Set additional callbacks
    picotts_set_error_notify(errorCallback);
    picotts_set_idle_notify(idleCallback);

    initialized = true;
    Logger::info("TTS", "PicoTTS initialized successfully");
    return true;
}

bool PicoTTS::speak(const char* text) {
    if (!initialized) {
        Logger::error("TTS", "PicoTTS not initialized");
        return false;
    }

    if (!text || strlen(text) == 0) {
        Logger::warn("TTS", "Empty text provided");
        return false;
    }

    Logger::info("TTS", "Speaking: %s", text);
    picotts_add(text, strlen(text)+1);
    return true;
}

void PicoTTS::shutdown() {
    if (initialized) {
        Logger::info("TTS", "Shutting down PicoTTS...");
        picotts_shutdown();
        initialized = false;
        speakerInstance = nullptr;
    }
}

// Static callback functions
void PicoTTS::outputCallback(int16_t* samples, unsigned count) {
    // This callback is called from picoTTS task with generated samples
    // For simplicity, write directly to speaker (blocking is ok in callback)
    size_t written;
    if (speakerInstance && !speakerInstance->writeSamples(samples, count, &written)) {
        Logger::error("TTS", "PicoTTS engine failed to write samples");
    }
}

void PicoTTS::errorCallback() {
    Logger::error("TTS", "PicoTTS engine encountered an error");
}

void PicoTTS::idleCallback() {
    Logger::info("TTS", "PicoTTS engine is idle");
}