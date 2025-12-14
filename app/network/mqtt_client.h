#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <functional>
#include "infrastructure/logger.h"
#include "config.h"

/**
 * MQTT Client class for ESP32
 * Handles MQTT connection, publishing, and subscribing to topics
 * Runs in a separate FreeRTOS task for non-blocking operation
 */
class MqttClient {
public:
    MqttClient();
    ~MqttClient();

    /**
     * Initialize MQTT client
     * @return true if successful
     */
    bool init();

    /**
     * Connect to MQTT broker
     * @return true if connection successful
     */
    bool connect();

    /**
     * Disconnect from MQTT broker
     */
    void disconnect();

    /**
     * Check if connected to MQTT broker
     * @return true if connected
     */
    bool isConnected();

    /**
     * Publish message to topic
     * @param topic MQTT topic
     * @param payload Message payload
     * @return true if published successfully
     */
    bool publish(const String& topic, const String& payload);

    /**
     * Subscribe to topic
     * @param topic MQTT topic
     * @return true if subscribed successfully
     */
    bool subscribe(const String& topic);

    /**
     * Unsubscribe from topic
     * @param topic MQTT topic
     * @return true if unsubscribed successfully
     */
    bool unsubscribe(const String& topic);

    /**
     * Set callback for incoming messages
     * @param callback Function to call when message received
     */
    void setMessageCallback(std::function<void(String topic, String payload)> callback);

    /**
     * Get MQTT client ID
     * @return Client ID string
     */
    String getClientId() const { return _clientId; }

    /**
     * Handle periodic tasks (keep alive, reconnect)
     * Call this in main loop or from a task
     */
    void handle();

private:
    WiFiClient _wifiClient;
    PubSubClient _client;
    String _clientId;
    bool _initialized;
    TaskHandle_t _mqttTaskHandle;
    std::function<void(String topic, String payload)> _messageCallback;

    // Static instance for callback
    static MqttClient* _instance;

    // MQTT task function
    static void mqttTask(void* parameter);

    // MQTT callback function
    static void mqttCallback(char* topic, byte* payload, unsigned int length);

    // Generate unique client ID
    String generateClientId();

    // Reconnect to MQTT broker
    bool reconnect();
};

#endif // MQTT_CLIENT_H