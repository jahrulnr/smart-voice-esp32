#include "app/callbacks.h"
#include <app/audio/mp3decoder.h>
#include <app/audio/converter.h>
#include <esp_heap_caps.h>

int size16t = sizeof(int16_t);

// AudioResponseCallback
void speakerAudioCallback(const uint8_t* audioData, size_t audioSize, bool isLastChunk) {
    sysActivity->update();
    if (!speaker) return;
    else if ((!audioData || audioSize == 0) && !isLastChunk) {
        return;
    } else if ((!audioData || audioSize == 0) && isLastChunk) {
        speaker->clear();
        return;
    }

    // Audio data is PCM16 at 24kHz, convert to 16kHz for speaker
    int16_t* pcmInput = (int16_t*)audioData;
    size_t inputSamples = audioSize / size16t;

    // Calculate output buffer size
    size_t outputSamples = AudioBufferConverter::calculateOutputSize(24, 16, inputSamples);
    
    // Allocate output buffer in SPIRAM
    int16_t* pcmOutput = (int16_t*)heap_caps_malloc(outputSamples * size16t, MALLOC_CAP_SPIRAM);
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
    speaker->writeSamples(pcmOutput, convertedSamples * size16t, &samplesWritten);
        
    // Free the buffer
    heap_caps_free(pcmOutput);

    // If this is the last chunk, clear speaker buffer
    if (isLastChunk) {
        speaker->clear();
    }
}