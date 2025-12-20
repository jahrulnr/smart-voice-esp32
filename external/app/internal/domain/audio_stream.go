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
}

// NewAudioStream creates a new audio stream
func NewAudioStream(sessionID uint32) *AudioStream {
	return &AudioStream{
		SessionID: sessionID,
		Chunks:    make([]*AudioChunk, 0),
		buffer:    &bytes.Buffer{},
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
		// Data chunk
		_, err := as.buffer.Write(chunk.Data)
		if err != nil {
			return fmt.Errorf("failed to write chunk data: %w", err)
		}
	}

	return nil
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
