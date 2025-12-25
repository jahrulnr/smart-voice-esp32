package domain

import (
	"bytes"
	"fmt"
)

// AudioChunk represents a chunk of audio data with metadata
type AudioChunk struct {
	Key     uint32
	Data    []byte
	IsStart bool
	IsEnd   bool
}

// AudioStream represents a complete audio stream session
type AudioStream struct {
	SessionID uint32
	Chunks    []*AudioChunk
	buffer    *bytes.Buffer
	Volume    float64
}

// NewAudioStream creates a new audio stream
func NewAudioStream(sessionID uint32) *AudioStream {
	return &AudioStream{
		SessionID: sessionID,
		Chunks:    make([]*AudioChunk, 0),
		buffer:    &bytes.Buffer{},
		Volume:    50.0,
	}
}

// AddChunk adds a chunk to the stream
func (as *AudioStream) AddChunk(chunk *AudioChunk) error {
	as.Chunks = append(as.Chunks, chunk)

	if chunk.IsStart {
		as.buffer.Reset()
	} else if chunk.IsEnd {
		// Stream is complete
		return nil
	} else {
		// Data chunk - apply volume
		data := make([]byte, len(chunk.Data))
		for i := 0; i < len(chunk.Data); i += 2 {
			sample := int16(chunk.Data[i]) | int16(chunk.Data[i+1])<<8
			sample = int16(float64(sample) * as.Volume)
			data[i] = byte(sample & 0xFF)
			data[i+1] = byte(sample >> 8)
		}
		_, err := as.buffer.Write(data)
		if err != nil {
			return fmt.Errorf("failed to write chunk data: %w", err)
		}
	}

	return nil
}

// SetVolume sets the volume multiplier (0.0 to 2.0, where 1.0 is original)
func (as *AudioStream) SetVolume(volume float64) {
	if volume < 0.0 {
		volume = 0.0
	}
	as.Volume = volume
}

// IsComplete checks if the stream has received start and end markers
func (as *AudioStream) IsComplete() bool {
	if len(as.Chunks) == 0 {
		return false
	}
	return as.Chunks[0].IsStart && as.Chunks[len(as.Chunks)-1].IsEnd
}

// GetAudioData returns the assembled audio data
func (as *AudioStream) GetAudioData() []byte {
	return as.buffer.Bytes()
}

// GetDuration estimates duration based on data size (16kHz mono 16-bit)
func (as *AudioStream) GetDuration() float64 {
	samples := len(as.GetAudioData()) / 2 // 2 bytes per sample
	return float64(samples) / 16000.0     // seconds
}
