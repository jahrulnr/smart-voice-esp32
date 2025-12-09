#ifndef SPEAKER_H
#define SPEAKER_H

#include <Arduino.h>
#include "I2SSpeaker.h"
#include "config.h"

/**
 * Speaker class for handling I2S audio output
 */
class Speaker {
public:
    Speaker();
    ~Speaker();

    /**
     * Initialize the speaker
     * @return true if successful, false otherwise
     */
    bool init();

    /**
     * Start the speaker
     * @return true if successful, false otherwise
     */
    bool start();

    /**
     * Stop the speaker
     * @return true if successful, false otherwise
     */
    bool stop();

    /**
     * Write audio samples to the speaker
     * @param buffer Buffer containing samples (int16_t)
     * @param sampleCount Number of samples to write
     * @param samplesWritten Pointer to store actual samples written
     * @return true if successful, false otherwise
     */
    bool writeSamples(const int16_t* buffer, size_t sampleCount, size_t* samplesWritten);

    /**
     * Play a tone
     * @param frequency Frequency in Hz
     * @param duration Duration in ms
     * @param amplitude Amplitude (0.0 to 1.0)
     * @return Number of samples played, or -1 on error
     */
    int playTone(uint32_t frequency, uint32_t duration, float amplitude = 0.5f);

    /**
     * Clear Speaker buffer
     * 
     * @return ESP_OK if successful, error code otherwise
     */
    esp_err_t clear();

private:
    I2SSpeaker* speaker;
};

#endif // SPEAKER_H