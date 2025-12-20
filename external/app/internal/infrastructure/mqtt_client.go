package infrastructure

import (
	"fmt"
	"log"
	"time"

	"app/internal/application/ports"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

// MQTTClient implements MQTT connectivity
type MQTTClient struct {
	broker         string
	topic          string
	client         mqtt.Client
	messageHandler ports.MessageHandler
}

// NewMQTTClient creates a new MQTT client
func NewMQTTClient(broker, topic string) *MQTTClient {
	return &MQTTClient{
		broker: broker,
		topic:  topic,
	}
}

// SetMessageHandler sets the message handler
func (mc *MQTTClient) SetMessageHandler(handler ports.MessageHandler) {
	mc.messageHandler = handler
}

// Connect establishes connection to MQTT broker
func (mc *MQTTClient) Connect() error {
	opts := mqtt.NewClientOptions()
	opts.AddBroker(mc.broker)
	opts.SetClientID(fmt.Sprintf("golang-consumer-%d", time.Now().Unix()))
	opts.SetAutoReconnect(true)
	opts.SetConnectRetry(true)
	opts.SetConnectRetryInterval(5 * time.Second)

	// Set up message handler - process messages asynchronously to avoid blocking MQTT client
	opts.SetDefaultPublishHandler(func(client mqtt.Client, msg mqtt.Message) {
		if mc.messageHandler != nil {
			// Copy payload to avoid issues with message lifecycle
			payload := make([]byte, len(msg.Payload()))
			copy(payload, msg.Payload())

			// Process message asynchronously to avoid blocking MQTT client thread
			go func() {
				if err := mc.messageHandler.HandleMessage(payload); err != nil {
					log.Printf("Error handling message: %v", err)
				}
			}()
		}
	})

	mc.client = mqtt.NewClient(opts)

	token := mc.client.Connect()
	if !token.WaitTimeout(10 * time.Second) {
		return fmt.Errorf("connection timeout")
	}
	if token.Error() != nil {
		return token.Error()
	}

	// Subscribe to topic
	token = mc.client.Subscribe(mc.topic, 1, nil)
	if !token.WaitTimeout(10 * time.Second) {
		return fmt.Errorf("subscription timeout")
	}
	if token.Error() != nil {
		return token.Error()
	}

	log.Printf("Connected to MQTT broker %s, subscribed to %s", mc.broker, mc.topic)
	return nil
}

// Disconnect closes the MQTT connection
func (mc *MQTTClient) Disconnect() {
	if mc.client != nil && mc.client.IsConnected() {
		mc.client.Disconnect(250)
		log.Println("Disconnected from MQTT broker")
	}
}
