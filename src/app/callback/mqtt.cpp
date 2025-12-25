#include "app/callback_list.h"

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String topicStr = String(topic);
    String payloadStr;

    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    ESP_LOGI("MQTT", "[Received] topic %s: %s", topicStr.c_str(), payloadStr.c_str());
    if (topicStr.equalsIgnoreCase(MQTT_TOPIC_STT)) {
        JsonDocument docs;
        deserializeJson(docs, payloadStr);
        if (!docs["text"].isNull()) {
            const char* text = docs["text"].as<String>().c_str();
            if (strlen(text) > 256) {
                char newText[256];
                strncpy(newText, text, 255);
                text = newText;
            }
            tts.speak(text);
            ESP_LOGI("MQTT", "[Speak] topic %s: %s", topicStr.c_str(), text);
        }

        docs.clear();
    }
}
