# MQTT Client Service

The MQTT Client service provides MQTT (Message Queuing Telemetry Transport) connectivity for the ESP32 voice assistant, enabling remote control and monitoring capabilities.

## Features

- **MQTT Broker Connection**: Connects to an MQTT broker for publish/subscribe messaging
- **Automatic Reconnection**: Handles connection drops and automatically reconnects
- **Command Processing**: Receives commands via MQTT and processes them through the GPT service
- **Status Publishing**: Publishes device status and responses
- **FreeRTOS Integration**: Runs in a dedicated task for non-blocking operation

## Configuration

The MQTT client is configured through `config.h`:

```cpp
// MQTT Configuration
#define MQTT_SERVER "broker.hivemq.com"  // Public MQTT broker
#define MQTT_PORT 1883                  // Standard MQTT port
#define MQTT_CLIENT_ID "esp32-voice-assistant"
#define MQTT_USERNAME ""                // Leave empty for anonymous connection
#define MQTT_PASSWORD ""                // Leave empty for anonymous connection
#define MQTT_KEEP_ALIVE 60              // Keep alive interval in seconds
#define MQTT_QOS 0                      // Quality of Service (0, 1, or 2)
#define MQTT_RETAIN false               // Retain messages
#define MQTT_TOPIC_COMMAND "esp32/command"  // Topic for receiving commands
#define MQTT_TOPIC_STATUS "esp32/status"    // Topic for publishing status
#define MQTT_TOPIC_RESPONSE "esp32/response" // Topic for publishing responses
```

## Topics

### Subscribed Topics
- `esp32/command`: Receives text commands to be processed by the GPT service

### Published Topics
- `esp32/status`: Publishes device status ("online", "offline")
- `esp32/response`: Publishes GPT responses to commands

## Usage

### Sending Commands
Publish a text message to the command topic:

```bash
mosquitto_pub -h broker.hivemq.com -t "esp32/command" -m "What is the weather like?"
```

### Receiving Responses
Subscribe to the response topic:

```bash
mosquitto_sub -h broker.hivemq.com -t "esp32/response"
```

### Monitoring Status
Subscribe to the status topic:

```bash
mosquitto_sub -h broker.hivemq.com -t "esp32/status"
```

## Architecture

The MQTT client integrates with the existing system:

- **Boot Sequence**: Initialized during the NETWORK_INIT phase after WiFi
- **Task Management**: Runs in a dedicated FreeRTOS task for continuous operation
- **Event Handling**: Processes incoming commands and publishes responses
- **Display Integration**: Updates the display state during command processing
- **TTS Integration**: Speaks responses aloud

## Dependencies

- **PubSubClient**: Arduino MQTT client library
- **WiFi**: ESP32 WiFi connectivity
- **GPT Service**: For processing text commands
- **Display Manager**: For UI state updates
- **PicoTTS**: For speech synthesis of responses

## Example Workflow

1. Device connects to WiFi and MQTT broker
2. Publishes "online" status
3. Subscribes to command topic
4. Receives command via MQTT
5. Processes command through GPT service
6. Publishes response via MQTT
7. Speaks response aloud
8. Updates display state

## MQTT Brokers

The service can connect to any standard MQTT broker:

- **Public Brokers**: HiveMQ, Eclipse Mosquitto test servers
- **Private Brokers**: Local Mosquitto installations, AWS IoT, etc.
- **Cloud Services**: AWS IoT Core, Google Cloud IoT, Azure IoT Hub

## Security

For production use, consider:

- **Authentication**: Use username/password or certificates
- **TLS/SSL**: Enable secure connections (MQTTS)
- **Topic Security**: Implement proper topic permissions
- **Client IDs**: Use unique, non-predictable client IDs

## Troubleshooting

### Connection Issues
- Verify WiFi connectivity
- Check MQTT broker address and port
- Ensure firewall allows MQTT traffic (port 1883/8883)

### Message Issues
- Check topic names match configuration
- Verify QoS settings
- Monitor broker logs for connection attempts

### Performance
- Adjust keep-alive interval based on network conditions
- Consider QoS 1 for guaranteed delivery if needed
- Monitor memory usage in constrained environments