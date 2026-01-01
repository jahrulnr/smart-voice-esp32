package main

import (
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"

	"app/internal/application"
	"app/internal/infrastructure"

	"github.com/joho/godotenv"
)

func main() {
	// Load .env file if it exists
	if err := godotenv.Load(); err != nil {
		log.Printf("No .env file found or error loading it: %v", err)
	}

	// Check for test mode
	if len(os.Args) > 1 && os.Args[1] == "test" {
		runOpenAITest()
		return
	}

	// Configuration
	mqttBroker := getEnv("MQTT_BROKER", "tcp://localhost:1883")
	audioTopic := getEnv("AUDIO_TOPIC", "pioassistant/audio")
	sttTopic := getEnv("STT_TOPIC", "pioassistant/stt")
	whisperURL := getEnv("WHISPER_URL", "http://localhost:8000/transcribe")
	saveAudio := getEnv("SAVE_AUDIO", "false") == "true"
	audioSaveDir := getEnv("AUDIO_SAVE_DIR", "./audio")

	// Initialize components
	mqttClient := infrastructure.NewMQTTClient(mqttBroker, audioTopic)
	audioAssembler := application.NewAudioAssembler()
	audioProcessor := application.NewAudioProcessor(whisperURL, saveAudio, audioSaveDir, mqttClient, sttTopic)

	// Wire up the application
	messageHandler := application.NewMessageHandler(audioAssembler, audioProcessor)
	mqttClient.SetMessageHandler(messageHandler)

	// Start cleanup goroutine to handle timed-out streams
	go func() {
		ticker := time.NewTicker(10 * time.Second) // Check every 10 seconds
		defer ticker.Stop()
		for range ticker.C {
			completedStreams := audioAssembler.Cleanup()
			for _, stream := range completedStreams {
				log.Printf("Cleaned up timed-out stream %d (%.2f seconds)",
					stream.SessionID, stream.GetDuration())
				// Process the timed-out stream
				if err := audioProcessor.ProcessAudio(stream); err != nil {
					log.Printf("Failed to process timed-out audio stream %d: %v", stream.SessionID, err)
				}
			}
		}
	}()

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

func runOpenAITest() {
	log.Println("Running OpenAI Realtime API Test...")

	// Test audio files - use existing WAV files from ./audio/
	testFiles := []string{
		"./audio/stream_98012_20251227_213618.wav",
		// "./audio/stream_245588_20251227_213845.wav",
		// "./audio/stream_25308_20251227_213517.wav",
	}

	// Check if test files exist
	for _, file := range testFiles {
		if _, err := os.Stat(file); os.IsNotExist(err) {
			log.Printf("Test file not found: %s", file)
			log.Println("Please ensure WAV files exist in ./audio/ directory")
			return
		}
	}

	// Initialize audio processor for testing
	processor := application.NewAudioProcessor("", false, "", nil, "")
	audioProcessor, ok := processor.(*application.AudioProcessor)
	if !ok {
		log.Fatal("Failed to cast to concrete AudioProcessor type")
	}

	// Run the test
	err := audioProcessor.TestOpenAIRealtime(testFiles)
	if err != nil {
		log.Fatalf("OpenAI test failed: %v", err)
	}

	log.Println("OpenAI test completed successfully")
}

func getEnv(key, defaultValue string) string {
	if value := os.Getenv(key); value != "" {
		return value
	}
	return defaultValue
}
