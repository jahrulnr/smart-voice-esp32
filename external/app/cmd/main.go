package main

import (
	"log"
	"os"
	"os/signal"
	"syscall"

	"app/internal/application"
	"app/internal/infrastructure"
)

func main() {
	// Configuration
	mqttBroker := getEnv("MQTT_BROKER", "tcp://localhost:1883")
	mqttTopic := getEnv("MQTT_TOPIC", "pioassistant/audio")
	whisperURL := getEnv("WHISPER_URL", "http://localhost:8000/transcribe")
	saveAudio := getEnv("SAVE_AUDIO", "false") == "true"
	audioSaveDir := getEnv("AUDIO_SAVE_DIR", "./audio")

	// Initialize components
	mqttClient := infrastructure.NewMQTTClient(mqttBroker, mqttTopic)
	audioAssembler := application.NewAudioAssembler()
	audioProcessor := application.NewAudioProcessor(whisperURL, saveAudio, audioSaveDir)

	// Wire up the application
	messageHandler := application.NewMessageHandler(audioAssembler, audioProcessor)
	mqttClient.SetMessageHandler(messageHandler)

	// Start the consumer
	if err := mqttClient.Connect(); err != nil {
		log.Fatalf("Failed to connect to MQTT: %v", err)
	}
	defer mqttClient.Disconnect()

	log.Println("MQTT Consumer started. Press Ctrl+C to exit.")

	// Wait for interrupt signal
	sigChan := make(chan os.Signal, 1)
	signal.Notify(sigChan, syscall.SIGINT, syscall.SIGTERM)
	<-sigChan

	log.Println("Shutting down...")
}

func getEnv(key, defaultValue string) string {
	if value := os.Getenv(key); value != "" {
		return value
	}
	return defaultValue
}
