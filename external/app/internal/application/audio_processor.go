package application

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"mime/multipart"
	"net/http"
	"os"
	"path/filepath"
	"time"

	"app/internal/application/ports"
	"app/internal/domain"
)

// AudioProcessor implements the AudioProcessor port using Whisper API
type AudioProcessor struct {
	whisperURL   string
	saveAudio    bool
	audioSaveDir string
	client       *http.Client
}

// NewAudioProcessor creates a new audio processor
func NewAudioProcessor(whisperURL string, saveAudio bool, audioSaveDir string) ports.AudioProcessor {
	return &AudioProcessor{
		whisperURL:   whisperURL,
		saveAudio:    saveAudio,
		audioSaveDir: audioSaveDir,
		client:       &http.Client{},
	}
}

// ProcessAudio sends the audio stream to Whisper for transcription
func (ap *AudioProcessor) ProcessAudio(stream *domain.AudioStream) error {
	audioData := stream.GetAudioData()
	if len(audioData) == 0 {
		return fmt.Errorf("no audio data in stream %d", stream.SessionID)
	}

	log.Printf("Processing audio stream %d (%d bytes, %.2f seconds)",
		stream.SessionID, len(audioData), stream.GetDuration())

	// Create multipart form data
	var buf bytes.Buffer
	writer := multipart.NewWriter(&buf)

	// Add audio file
	fileWriter, err := writer.CreateFormFile("file", fmt.Sprintf("stream_%d.wav", stream.SessionID))
	if err != nil {
		return fmt.Errorf("failed to create form file: %w", err)
	}

	// Write WAV header and audio data
	wavData := ap.createWAVData(audioData)
	_, err = fileWriter.Write(wavData)
	if err != nil {
		return fmt.Errorf("failed to write audio data: %w", err)
	}

	// Save audio to file if enabled
	if ap.saveAudio {
		if err := ap.saveAudioToFile(stream.SessionID, wavData); err != nil {
			log.Printf("Failed to save audio file for stream %d: %v", stream.SessionID, err)
		} else {
			log.Printf("Saved audio file for stream %d", stream.SessionID)
		}
	}

	// Add other form fields if needed
	// writer.WriteField("model", "whisper-1")  // Not needed for this Whisper server
	writer.WriteField("language", "id") // Indonesian language hint

	writer.Close()

	// Create HTTP request
	req, err := http.NewRequest("POST", ap.whisperURL, &buf)
	if err != nil {
		return fmt.Errorf("failed to create request: %w", err)
	}
	req.Header.Set("Content-Type", writer.FormDataContentType())

	// Send request
	resp, err := ap.client.Do(req)
	if err != nil {
		return fmt.Errorf("failed to send request to Whisper: %w", err)
	}
	defer resp.Body.Close()

	// Read response
	body, err := io.ReadAll(resp.Body)
	if err != nil {
		return fmt.Errorf("failed to read response: %w", err)
	}

	if resp.StatusCode != http.StatusOK {
		return fmt.Errorf("Whisper API returned status %d: %s", resp.StatusCode, string(body))
	}

	// Parse JSON response
	var result struct {
		Text string `json:"text"`
	}
	if err := json.Unmarshal(body, &result); err != nil {
		return fmt.Errorf("failed to parse Whisper response: %w", err)
	}

	log.Printf("Transcription for stream %d: %s", stream.SessionID, result.Text)

	// TODO: Send transcription back via MQTT or other means
	// For now, just log it

	return nil
}

// saveAudioToFile saves the WAV data to a file
func (ap *AudioProcessor) saveAudioToFile(sessionID uint32, wavData []byte) error {
	// Create directory if it doesn't exist
	if err := os.MkdirAll(ap.audioSaveDir, 0755); err != nil {
		return fmt.Errorf("failed to create audio save directory: %w", err)
	}

	// Create filename with timestamp
	timestamp := time.Now().Format("20060102_150405")
	filename := fmt.Sprintf("stream_%d_%s.wav", sessionID, timestamp)
	filepath := filepath.Join(ap.audioSaveDir, filename)

	// Write file
	if err := os.WriteFile(filepath, wavData, 0644); err != nil {
		return fmt.Errorf("failed to write audio file: %w", err)
	}

	log.Printf("Audio saved to: %s", filepath)
	return nil
}

// createWAVData creates a simple WAV header for 16kHz mono 16-bit PCM
func (ap *AudioProcessor) createWAVData(pcmData []byte) []byte {
	sampleRate := uint32(16000)
	bitsPerSample := uint16(16)
	numChannels := uint16(1)
	byteRate := sampleRate * uint32(numChannels) * uint32(bitsPerSample/8)
	blockAlign := numChannels * bitsPerSample / 8
	dataSize := uint32(len(pcmData))

	var buf bytes.Buffer

	// RIFF header
	buf.WriteString("RIFF")
	buf.Write(uint32ToBytes(36 + dataSize)) // File size
	buf.WriteString("WAVE")

	// Format chunk
	buf.WriteString("fmt ")
	buf.Write(uint32ToBytes(16))            // Chunk size
	buf.Write(uint16ToBytes(1))             // Audio format (PCM)
	buf.Write(uint16ToBytes(numChannels))   // Num channels
	buf.Write(uint32ToBytes(sampleRate))    // Sample rate
	buf.Write(uint32ToBytes(byteRate))      // Byte rate
	buf.Write(uint16ToBytes(blockAlign))    // Block align
	buf.Write(uint16ToBytes(bitsPerSample)) // Bits per sample

	// Data chunk
	buf.WriteString("data")
	buf.Write(uint32ToBytes(dataSize))
	buf.Write(pcmData)

	return buf.Bytes()
}

func uint16ToBytes(v uint16) []byte {
	return []byte{byte(v), byte(v >> 8)}
}

func uint32ToBytes(v uint32) []byte {
	return []byte{byte(v), byte(v >> 8), byte(v >> 16), byte(v >> 24)}
}
