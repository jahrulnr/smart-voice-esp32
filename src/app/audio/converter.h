#ifndef AUDIO_BUFFER_CONVERTER_H
#define AUDIO_BUFFER_CONVERTER_H

#include <cstdint>
#include <cstddef>

class AudioBufferConverter {
public:
    /**
     * Convert audio buffer from one sample rate to another using linear interpolation
     *
     * @param inputKhz Input sample rate in kHz (e.g., 16 for 16kHz)
     * @param outputKhz Output sample rate in kHz (e.g., 24 for 24kHz)
     * @param bufferIn Input buffer (16-bit PCM samples)
     * @param lenIn Length of input buffer in samples
     * @param bufferOut Output buffer (16-bit PCM samples)
     * @param lenOut Length of output buffer in samples (must be large enough)
     * @return Number of samples written to output buffer, or -1 on error
     */
    static int convert(uint32_t inputKhz, uint32_t outputKhz,
                      const int16_t* bufferIn, size_t lenIn,
                      int16_t* bufferOut, size_t lenOut);

    /**
     * Calculate required output buffer size for conversion
     *
     * @param inputKhz Input sample rate in kHz
     * @param outputKhz Output sample rate in kHz
     * @param inputSamples Number of input samples
     * @return Required output buffer size in samples
     */
    static size_t calculateOutputSize(uint32_t inputKhz, uint32_t outputKhz, size_t inputSamples);

private:
    // Linear interpolation between two samples
    static int16_t interpolate(int16_t sample1, int16_t sample2, float fraction);
};

#endif // AUDIO_BUFFER_CONVERTER_H