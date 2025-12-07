#ifndef PICO_TTS_H
#define PICO_TTS_H

#include <Arduino.h>
#include "picotts.h"
#include "audio/speaker.h"

/**
 * PicoTTS class for text-to-speech synthesis using picoTTS
 */
class PicoTTS {
public:
    PicoTTS();
    ~PicoTTS();

    /**
     * Initialize picoTTS with speaker output
     * @param speaker Reference to speaker for audio output
     * @return true if successful, false otherwise
     */
    bool init(Speaker* speaker);

    /**
     * Speak the given text
     * @param text UTF8 text to synthesize
     * @return true if queued successfully, false otherwise
     */
    bool speak(const char* text);

    /**
     * Shutdown picoTTS
     */
    void shutdown();

private:
    static Speaker* speakerInstance;
    static void outputCallback(int16_t* samples, unsigned count);
    static void errorCallback();
    static void idleCallback();

    bool initialized;
};

#endif // PICO_TTS_H