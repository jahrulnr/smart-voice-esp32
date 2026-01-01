#include "converter.h"
#include <cmath>
#include <algorithm>
#include <cstring>

int AudioBufferConverter::convert(uint32_t inputKhz, uint32_t outputKhz,
                                 const int16_t* bufferIn, size_t lenIn,
                                 int16_t* bufferOut, size_t lenOut) {
    if (inputKhz == 0 || outputKhz == 0 || bufferIn == nullptr || bufferOut == nullptr) {
        return -1; // Invalid parameters
    }

    if (lenIn == 0) {
        return 0; // Nothing to convert
    }

    // Calculate the ratio
    float ratio = static_cast<float>(outputKhz) / static_cast<float>(inputKhz);

    // Calculate expected output size
    size_t expectedOutputSize = static_cast<size_t>(lenIn * ratio);

    if (expectedOutputSize > lenOut) {
        return -1; // Output buffer too small
    }

    // If same sample rate, just copy
    if (inputKhz == outputKhz) {
        memcpy(bufferOut, bufferIn, lenIn * sizeof(int16_t));
        return lenIn;
    }

    // Perform linear interpolation resampling
    for (size_t i = 0; i < expectedOutputSize; ++i) {
        // Map output index back to input index
        float inputIndex = static_cast<float>(i) * (static_cast<float>(lenIn - 1) / static_cast<float>(expectedOutputSize - 1));

        // Get the two nearest input samples
        size_t indexLow = static_cast<size_t>(inputIndex);
        size_t indexHigh = indexLow + 1;

        if (indexHigh >= lenIn) {
            indexHigh = lenIn - 1;
        }

        // Linear interpolation
        float fraction = inputIndex - static_cast<float>(indexLow);
        bufferOut[i] = interpolate(bufferIn[indexLow], bufferIn[indexHigh], fraction);
    }

    return expectedOutputSize;
}

size_t AudioBufferConverter::calculateOutputSize(uint32_t inputKhz, uint32_t outputKhz, size_t inputSamples) {
    if (inputKhz == 0 || outputKhz == 0) {
        return 0;
    }

    float ratio = static_cast<float>(outputKhz) / static_cast<float>(inputKhz);
    return static_cast<size_t>(inputSamples * ratio);
}

int16_t AudioBufferConverter::interpolate(int16_t sample1, int16_t sample2, float fraction) {
    // Clamp fraction to [0, 1]
    fraction = std::max(0.0f, std::min(1.0f, fraction));

    // Linear interpolation
    float result = static_cast<float>(sample1) * (1.0f - fraction) +
                   static_cast<float>(sample2) * fraction;

    // Clamp to int16_t range
    result = std::max(-32768.0f, std::min(32767.0f, result));

    return static_cast<int16_t>(result);
}