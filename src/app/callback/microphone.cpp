#include "app/callbacks.h"
#include <app/audio/converter.h>
#include <esp_heap_caps.h>

// I2S fill callback for ESP-SR system
esp_err_t srAudioCallback(void *arg, void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms) {
    if (microphone) {
        return microphone->read(out, len, bytes_read, timeout_ms);
    }

    return ESP_FAIL;
}

// AudioFillCallback 
size_t micAudioCallback(uint8_t* buffer, size_t maxSize) {
    sysActivity.update(millis());
    if (!microphone || !buffer || maxSize == 0) {
        return 0;
    }

    // Get microphone cache data (16kHz PCM16)
    static unsigned long lastSampleTime = 0;
    auto cache = microphone->getCache();
    
    ESP_LOGD("MicCallback", "Cache: len=%d, time=%lu, lastTime=%lu, hasSample=%d", 
             cache.lastSampleLen, cache.lastSampleTime, lastSampleTime, cache.lastSample != nullptr);
    
    if (cache.lastSampleLen == 0 
        || !cache.lastSample 
        || cache.lastSampleTime == 0
        || cache.lastSampleTime == lastSampleTime) {
        return 0; // No new data available
    }

    ESP_LOGD("MicCallback", "Processing %d bytes of audio data", cache.lastSampleLen);

    // Input is 16kHz PCM16
    lastSampleTime = cache.lastSampleTime;
    const int16_t* inputBuffer = cache.lastSample;
    size_t inputSamples = cache.lastSampleLen / sizeof(int16_t);
    size_t maxOutputSamples = maxSize / sizeof(int16_t);

    // For 16kHz to 24kHz conversion (ratio = 1.5), calculate how many input samples we can convert
    // to fit in the output buffer: inputSamples * 1.5 <= maxOutputSamples
    size_t maxInputSamples = static_cast<size_t>(maxOutputSamples / 1.5f);
    if (inputSamples > maxInputSamples) {
        inputSamples = maxInputSamples; // Limit input to fit output buffer
    }

    // Calculate actual output size for the limited input
    size_t requiredOutputSamples = AudioBufferConverter::calculateOutputSize(16, 24, inputSamples);

    // Allocate temporary buffer for conversion if needed
    int16_t* tempBuffer = nullptr;
    int16_t* outputBuffer = reinterpret_cast<int16_t*>(buffer);

    if (requiredOutputSamples > inputSamples) {
        // Need upsampling, allocate temp buffer
        tempBuffer = static_cast<int16_t*>(heap_caps_malloc(requiredOutputSamples * sizeof(int16_t), MALLOC_CAP_SPIRAM));
        if (!tempBuffer) {
            ESP_LOGE("MicCallback", "Failed to allocate temp buffer for conversion");
            return 0;
        }
        outputBuffer = tempBuffer;
    }

    // Convert from 16kHz to 24kHz
    int convertedSamples = AudioBufferConverter::convert(16, 24, inputBuffer, inputSamples, outputBuffer, requiredOutputSamples);
    if (convertedSamples <= 0) {
        ESP_LOGE("MicCallback", "Audio conversion failed: inputSamples=%d, requiredOutputSamples=%d, maxOutputSamples=%d", 
                 inputSamples, requiredOutputSamples, maxOutputSamples);
        if (tempBuffer) heap_caps_free(tempBuffer);
        return 0;
    }

    // If we used a temp buffer, copy to output buffer
    if (tempBuffer) {
        size_t copySamples = (convertedSamples < maxOutputSamples) ? convertedSamples : maxOutputSamples;
        memcpy(buffer, tempBuffer, copySamples * sizeof(int16_t));
        heap_caps_free(tempBuffer);
        ESP_LOGD("MicCallback", "Returning %d bytes (with temp buffer)", copySamples * sizeof(int16_t));
        return copySamples * sizeof(int16_t);
    }
    
    // Use this everywhere
    AudioBufferConverter::setVolume((int16_t*)buffer, convertedSamples, 15.f);

    ESP_LOGD("MicCallback", "Returning %d bytes", convertedSamples * sizeof(int16_t));
    return convertedSamples * sizeof(int16_t);
}