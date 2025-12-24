#include "app/callback_list.h"

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);
    String payloadStr;

    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    ESP_LOGI("MQTT", "Received message on topic %s: %s", topicStr.c_str(), payloadStr.c_str());
}