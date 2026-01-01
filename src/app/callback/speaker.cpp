#include "app/callbacks.h"
#include <app/audio/mp3decoder.h>
#include <app/audio/converter.h>
#include <esp_heap_caps.h>

// AudioResponseCallback
void speakerAudioCallback(const uint8_t* audioData, size_t audioSize, bool isLastChunk) {
    if (!speaker || !audioData || audioSize == 0) {
        return;
    }

    // Audio data is PCM16 at 24kHz, convert to 16kHz for speaker
    const int16_t* pcmInput = reinterpret_cast<const int16_t*>(audioData);
    size_t inputSamples = audioSize / sizeof(int16_t);

    // Calculate output buffer size
    size_t outputSamples = AudioBufferConverter::calculateOutputSize(24, 16, inputSamples);
    
    // Allocate output buffer in SPIRAM
    int16_t* pcmOutput = static_cast<int16_t*>(heap_caps_malloc(outputSamples * sizeof(int16_t), MALLOC_CAP_SPIRAM));
    if (!pcmOutput) {
        ESP_LOGE("SpeakerCallback", "Failed to allocate output buffer");
        return;
    }

    // Convert sample rate from 24kHz to 16kHz
    int convertedSamples = AudioBufferConverter::convert(24, 16, pcmInput, inputSamples, pcmOutput, outputSamples);
    if (convertedSamples <= 0) {
        ESP_LOGE("SpeakerCallback", "Audio conversion failed");
        heap_caps_free(pcmOutput);
        return;
    }

    // Write converted audio to speaker
    size_t samplesWritten = 0;
    esp_err_t err = speaker->writeSamples(pcmOutput, convertedSamples * sizeof(int16_t), &samplesWritten);
    if (err != ESP_OK) {
        ESP_LOGE("SpeakerCallback", "Failed to write samples to speaker: %s", esp_err_to_name(err));
        speaker->clear();
    }

    // Free the buffer
    heap_caps_free(pcmOutput);

    // If this is the last chunk, clear speaker buffer
    if (isLastChunk) {
        speaker->clear();
    }
}