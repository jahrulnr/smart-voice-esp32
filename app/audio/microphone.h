#ifndef MICROPHONE_H
#define MICROPHONE_H

#include <Arduino.h>
#include "I2SMicrophone.h"
#include "config.h"

/**
 * Microphone class for handling I2S microphone input using INMP441
 */
class Microphone {
public:
    Microphone();
    ~Microphone();

    /**
     * Initialize the microphone
     * @return true if successful, false otherwise
     */
    bool init();

    /**
     * Start the microphone
     * @return true if successful, false otherwise
     */
    bool start();

    /**
     * Stop the microphone
     * @return true if successful, false otherwise
     */
    bool stop();

    /**
     * Read audio samples
     * @param buffer Buffer to store samples (int16_t)
     * @param sampleCount Number of samples to read
     * @param samplesRead Pointer to store actual samples read
     * @return true if successful, false otherwise
     */
    bool readSamples(int16_t* buffer, size_t sampleCount, size_t* samplesRead);

    /**
     * Read current audio level
     * @return Audio level (0-32767), or -1 on error
     */
    int readLevel();

private:
    I2SMicrophone* mic;
};

#endif // MICROPHONE_H