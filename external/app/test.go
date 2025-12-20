package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"
	"strings"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

const (
	MQTT_BROKER = "tcp://192.168.18.250:1883"
	MQTT_TOPIC  = "pioassistant/audio"
	CHUNK_SIZE  = 512 // Same as ESP32 buffer logic
)

func main() {
	fmt.Println("Starting MQTT Audio Upload Test...")

	// Find audio files
	audioFiles, err := findAudioFiles("./audio")
	if err != nil {
		log.Fatalf("Failed to find audio files: %v", err)
	}

	if len(audioFiles) == 0 {
		log.Fatal("No audio files found in ./audio directory")
	}

	fmt.Printf("Found %d audio files: %v\n", len(audioFiles), audioFiles)

	// Connect to MQTT
	client := connectMQTT()
	defer client.Disconnect(250)

	// Test each audio file
	for i, audioFile := range audioFiles {
		sessionID := uint32(1002 + i) // Use small session IDs like ESP32 (1000, 1001, etc.)
		fmt.Printf("\nTesting file: %s (Session ID: %d)\n", audioFile, sessionID)

		err := uploadAudioFile(client, audioFile, sessionID)
		if err != nil {
			log.Printf("Failed to upload %s: %v", audioFile, err)
			continue
		}

		fmt.Printf("Successfully uploaded %s\n", audioFile)

		// Wait between uploads
		time.Sleep(2 * time.Second)
	}

	fmt.Println("\nAll tests completed!")
}

func findAudioFiles(dir string) ([]string, error) {
	var files []string

	err := filepath.Walk(dir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if !info.IsDir() {
			ext := filepath.Ext(path)
			if ext == ".wav" || ext == ".WAV" {
				files = append(files, path)
			}
		}

		return nil
	})

	return files, err
}

func connectMQTT() mqtt.Client {
	opts := mqtt.NewClientOptions()
	opts.AddBroker(MQTT_BROKER)
	opts.SetClientID(fmt.Sprintf("audio-test-%d", time.Now().Unix()))
	opts.SetAutoReconnect(true)
	opts.SetConnectRetry(true)
	opts.SetConnectRetryInterval(5 * time.Second)

	client := mqtt.NewClient(opts)

	token := client.Connect()
	if !token.WaitTimeout(10 * time.Second) {
		log.Fatal("MQTT connection timeout")
	}
	if token.Error() != nil {
		log.Fatalf("MQTT connection failed: %v", token.Error())
	}

	fmt.Printf("Connected to MQTT broker: %s\n", MQTT_BROKER)
	return client
}

func uploadAudioFile(client mqtt.Client, filename string, sessionID uint32) error {
	// Open audio file
	file, err := os.Open(filename)
	if err != nil {
		return fmt.Errorf("failed to open file: %w", err)
	}
	defer file.Close()

	// Get file extension
	ext := strings.ToLower(filepath.Ext(filename))

	var audioData []byte
	switch ext {
	case ".wav":
		// For WAV files, extract PCM data (skip WAV header)
		audioData, err = extractPCMFromWAV(file)
		if err != nil {
			return fmt.Errorf("failed to extract PCM from WAV: %w", err)
		}
	case ".mp3":
		return fmt.Errorf("MP3 files not supported - use WAV files with PCM data (ESP32 sends raw PCM, not compressed audio)")
	default:
		return fmt.Errorf("unsupported file format: %s (only WAV files supported)", ext)
	}

	fmt.Printf("Extracted PCM data: %d bytes\n", len(audioData))

	// Send start marker
	startKey := sessionID*10000 + 0 // Start marker
	err = sendMQTTChunk(client, startKey, nil)
	if err != nil {
		return fmt.Errorf("failed to send start marker: %w", err)
	}
	fmt.Printf("Sent start marker (key: %d)\n", startKey)

	// Send audio data in chunks
	buffer := make([]byte, CHUNK_SIZE)
	chunkIndex := uint32(1)
	totalBytes := 0

	audioReader := bytes.NewReader(audioData)
	for {
		n, err := audioReader.Read(buffer)
		if err != nil && err != io.EOF {
			return fmt.Errorf("failed to read audio data: %w", err)
		}

		if n == 0 {
			break // End of data
		}

		// Send data chunk
		chunkKey := sessionID*10000 + chunkIndex
		err = sendMQTTChunk(client, chunkKey, buffer[:n])
		if err != nil {
			return fmt.Errorf("failed to send chunk %d: %w", chunkIndex, err)
		}

		fmt.Printf("Sent chunk %d (key: %d, size: %d bytes)\n", chunkIndex, chunkKey, n)
		totalBytes += n
		chunkIndex++

		// Small delay between chunks to simulate real-time streaming
		time.Sleep(10 * time.Millisecond)
	}

	// Send end marker
	endKey := sessionID*10000 + 9999 // End marker
	err = sendMQTTChunk(client, endKey, nil)
	if err != nil {
		return fmt.Errorf("failed to send end marker: %w", err)
	}
	fmt.Printf("Sent end marker (key: %d)\n", endKey)

	fmt.Printf("Upload complete: %d chunks, %d total bytes\n", chunkIndex-1, totalBytes)
	return nil
}

func sendMQTTChunk(client mqtt.Client, key uint32, data []byte) error {
	// Create payload: 4-byte key + data
	payloadSize := 4 + len(data)
	payload := make([]byte, payloadSize)

	// Write key in little-endian format (same as ESP32)
	binary.LittleEndian.PutUint32(payload[0:4], key)

	// Copy data if present
	if len(data) > 0 {
		copy(payload[4:], data)
	}

	// Publish to MQTT
	token := client.Publish(MQTT_TOPIC, 1, false, payload)
	if !token.WaitTimeout(5 * time.Second) {
		return fmt.Errorf("MQTT publish timeout for key %d", key)
	}
	if token.Error() != nil {
		return fmt.Errorf("MQTT publish failed for key %d: %w", key, token.Error())
	}

	return nil
}

// extractPCMFromWAV reads a WAV file and extracts the raw PCM data
func extractPCMFromWAV(file *os.File) ([]byte, error) {
	// Read and validate RIFF header
	riffHeader := make([]byte, 12)
	n, err := file.Read(riffHeader)
	if err != nil {
		return nil, fmt.Errorf("failed to read RIFF header: %w", err)
	}
	if n != 12 {
		return nil, fmt.Errorf("invalid RIFF header size: got %d, expected 12", n)
	}

	if string(riffHeader[0:4]) != "RIFF" || string(riffHeader[8:12]) != "WAVE" {
		return nil, fmt.Errorf("invalid WAV file format")
	}

	// Parse chunks until we find the 'data' chunk
	for {
		// Read chunk header (8 bytes: chunk ID + size)
		chunkHeader := make([]byte, 8)
		n, err := file.Read(chunkHeader)
		if err != nil {
			if err == io.EOF {
				return nil, fmt.Errorf("reached end of file without finding data chunk")
			}
			return nil, fmt.Errorf("failed to read chunk header: %w", err)
		}
		if n != 8 {
			return nil, fmt.Errorf("incomplete chunk header: got %d bytes", n)
		}

		chunkID := string(chunkHeader[0:4])
		chunkSize := binary.LittleEndian.Uint32(chunkHeader[4:8])

		if chunkID == "data" {
			// Found the data chunk, read the PCM data
			pcmData := make([]byte, chunkSize)
			n, err := file.Read(pcmData)
			if err != nil {
				return nil, fmt.Errorf("failed to read PCM data: %w", err)
			}
			if uint32(n) != chunkSize {
				return nil, fmt.Errorf("incomplete PCM data: got %d, expected %d", n, chunkSize)
			}
			return pcmData, nil
		} else {
			// Skip this chunk
			_, err := file.Seek(int64(chunkSize), 1) // Seek relative to current position
			if err != nil {
				return nil, fmt.Errorf("failed to skip chunk '%s': %w", chunkID, err)
			}
		}
	}
}
