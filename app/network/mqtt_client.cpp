#include "mqtt_client.h"
#include <WiFi.h>

// Initialize static instance
MqttClient* MqttClient::_instance = nullptr;

MqttClient::MqttClient()
    : _client(_wifiClient), _initialized(false), _mqttTaskHandle(nullptr), _messageCallback(nullptr) {
    _clientId = generateClientId();
    _instance = this;  // Set static instance
}

MqttClient::~MqttClient() {
    disconnect();
    if (_mqttTaskHandle != nullptr) {
        vTaskDelete(_mqttTaskHandle);
        _mqttTaskHandle = nullptr;
    }
}

bool MqttClient::init() {
    if (_initialized) {
        Logger::warn("MQTT", "MQTT client already initialized");
        return true;
    }

    Logger::info("MQTT", "Initializing MQTT client...");

    // Set MQTT server and port
    _client.setServer(MQTT_SERVER, MQTT_PORT);

    // Set MQTT callback
    _client.setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->mqttCallback(topic, payload, length);
    });

    _initialized = true;
    Logger::info("MQTT", "MQTT client initialized with server: %s:%d", MQTT_SERVER, MQTT_PORT);
    return true;
}

bool MqttClient::connect() {
    if (!_initialized) {
        Logger::error("MQTT", "MQTT client not initialized");
        return false;
    }

    if (!WiFi.isConnected()) {
        Logger::warn("MQTT", "WiFi not connected, cannot connect to MQTT");
        return false;
    }

    if (_client.connected()) {
        Logger::info("MQTT", "Already connected to MQTT broker");
        return true;
    }

    Logger::info("MQTT", "Connecting to MQTT broker as %s...", _clientId.c_str());

    bool connected = false;
    if (strlen(MQTT_USERNAME) > 0) {
        connected = _client.connect(_clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
    } else {
        connected = _client.connect(_clientId.c_str());
    }

    if (connected) {
        Logger::info("MQTT", "Connected to MQTT broker");

        // Subscribe to command topic
        if (subscribe(MQTT_TOPIC_COMMAND)) {
            Logger::info("MQTT", "Subscribed to command topic: %s", MQTT_TOPIC_COMMAND);
        }

        // Publish status
        publish(MQTT_TOPIC_STATUS, "online");

        // Create MQTT handling task
        BaseType_t result = xTaskCreatePinnedToCore(
            mqttTask,           // Task function
            "MQTTTask",         // Task name
            4096,               // Stack size
            this,               // Parameters
            5,                  // Priority
            &_mqttTaskHandle,   // Task handle
            1                   // Core
        );

        if (result != pdPASS) {
            Logger::error("MQTT", "Failed to create MQTT task");
            disconnect();
            return false;
        }

        return true;
    } else {
        Logger::error("MQTT", "Failed to connect to MQTT broker (state: %d)", _client.state());
        return false;
    }
}

void MqttClient::disconnect() {
    if (_client.connected()) {
        publish(MQTT_TOPIC_STATUS, "offline");
        _client.disconnect();
        Logger::info("MQTT", "Disconnected from MQTT broker");
    }

    if (_mqttTaskHandle != nullptr) {
        vTaskDelete(_mqttTaskHandle);
        _mqttTaskHandle = nullptr;
    }
}

bool MqttClient::isConnected() {
    return _client.connected();
}

bool MqttClient::publish(const String& topic, const String& payload) {
    if (!_client.connected()) {
        Logger::warn("MQTT", "Not connected, cannot publish to %s", topic.c_str());
        return false;
    }

    bool result = _client.publish(topic.c_str(), payload.c_str(), MQTT_RETAIN);
    if (result) {
        Logger::debug("MQTT", "Published to %s: %s", topic.c_str(), payload.c_str());
    } else {
        Logger::error("MQTT", "Failed to publish to %s", topic.c_str());
    }
    return result;
}

bool MqttClient::subscribe(const String& topic) {
    if (!_client.connected()) {
        Logger::warn("MQTT", "Not connected, cannot subscribe to %s", topic.c_str());
        return false;
    }

    bool result = _client.subscribe(topic.c_str(), MQTT_QOS);
    if (result) {
        Logger::info("MQTT", "Subscribed to %s", topic.c_str());
    } else {
        Logger::error("MQTT", "Failed to subscribe to %s", topic.c_str());
    }
    return result;
}

bool MqttClient::unsubscribe(const String& topic) {
    if (!_client.connected()) {
        Logger::warn("MQTT", "Not connected, cannot unsubscribe from %s", topic.c_str());
        return false;
    }

    bool result = _client.unsubscribe(topic.c_str());
    if (result) {
        Logger::info("MQTT", "Unsubscribed from %s", topic.c_str());
    } else {
        Logger::error("MQTT", "Failed to unsubscribe from %s", topic.c_str());
    }
    return result;
}

void MqttClient::setMessageCallback(std::function<void(String topic, String payload)> callback) {
    _messageCallback = callback;
}

void MqttClient::handle() {
    if (_initialized && WiFi.isConnected() && !_client.connected()) {
        reconnect();
    }

    if (_client.connected()) {
        _client.loop();
    }
}

void MqttClient::mqttTask(void* parameter) {
    MqttClient* client = static_cast<MqttClient*>(parameter);

    Logger::info("MQTT", "MQTT task started");

    while (true) {
        if (client->_client.connected()) {
            client->_client.loop();
        } else {
            Logger::warn("MQTT", "MQTT connection lost, attempting reconnect...");
            if (!client->reconnect()) {
                Logger::error("MQTT", "Reconnect failed, retrying in 5 seconds...");
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // Small delay to prevent busy loop
    }
}

void MqttClient::mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (!_instance) return;

    String topicStr = String(topic);
    String payloadStr;

    for (unsigned int i = 0; i < length; i++) {
        payloadStr += (char)payload[i];
    }

    Logger::info("MQTT", "Received message on topic %s: %s", topicStr.c_str(), payloadStr.c_str());

    if (_instance->_messageCallback) {
        _instance->_messageCallback(topicStr, payloadStr);
    }
}

String MqttClient::generateClientId() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    return String(MQTT_CLIENT_ID) + "-" + mac.substring(6); // Use last 6 chars of MAC
}

bool MqttClient::reconnect() {
    if (!WiFi.isConnected()) {
        return false;
    }

    Logger::info("MQTT", "Attempting to reconnect to MQTT broker...");

    bool connected = false;
    if (strlen(MQTT_USERNAME) > 0) {
        connected = _client.connect(_clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
    } else {
        connected = _client.connect(_clientId.c_str());
    }

    if (connected) {
        Logger::info("MQTT", "Reconnected to MQTT broker");

        // Re-subscribe to topics
        subscribe(MQTT_TOPIC_COMMAND);

        // Publish status
        publish(MQTT_TOPIC_STATUS, "online");
    } else {
        Logger::error("MQTT", "Reconnect failed (state: %d)", _client.state());
    }

    return connected;
}