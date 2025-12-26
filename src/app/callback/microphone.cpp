#include "app/callback_list.h"

// I2S fill callback for ESP-SR system
esp_err_t srAudioCallback(void *arg, void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms) {
    if (microphone) {
        return microphone->read(out, len, bytes_read, timeout_ms);
    }

    return ESP_FAIL;
}