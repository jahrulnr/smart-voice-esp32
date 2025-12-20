package application

import (
	"app/internal/application/ports"
	"app/internal/domain"
	"sync"
)

// AudioAssembler implements the AudioAssembler port
type AudioAssembler struct {
	streams map[uint32]*domain.AudioStream
	mu      sync.RWMutex
}

// NewAudioAssembler creates a new audio assembler
func NewAudioAssembler() ports.AudioAssembler {
	return &AudioAssembler{
		streams: make(map[uint32]*domain.AudioStream),
	}
}

// AssembleChunk assembles audio chunks into streams
func (aa *AudioAssembler) AssembleChunk(sessionID uint32, chunk *domain.AudioChunk) (*domain.AudioStream, error) {
	aa.mu.Lock()
	stream, exists := aa.streams[sessionID]
	if !exists {
		stream = domain.NewAudioStream(sessionID)
		aa.streams[sessionID] = stream
	}
	aa.mu.Unlock()

	// Process chunk (stream operations are thread-safe per session)
	if err := stream.AddChunk(chunk); err != nil {
		return nil, err
	}

	if stream.IsComplete() {
		aa.mu.Lock()
		delete(aa.streams, sessionID)
		aa.mu.Unlock()
		return stream, nil
	}

	return nil, nil // Not complete yet
}
