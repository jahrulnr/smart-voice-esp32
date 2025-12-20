package application

import (
	"encoding/binary"
	"fmt"
	"log"

	"app/internal/application/ports"
	"app/internal/domain"
)

// MessageHandler handles incoming MQTT messages and coordinates audio processing
type MessageHandler struct {
	audioAssembler ports.AudioAssembler
	audioProcessor ports.AudioProcessor
}

// NewMessageHandler creates a new message handler
func NewMessageHandler(assembler ports.AudioAssembler, processor ports.AudioProcessor) *MessageHandler {
	return &MessageHandler{
		audioAssembler: assembler,
		audioProcessor: processor,
	}
}

// HandleMessage processes an incoming MQTT message
func (mh *MessageHandler) HandleMessage(payload []byte) error {
	if len(payload) < 4 {
		return fmt.Errorf("payload too short: %d bytes", len(payload))
	}

	// Extract key from first 4 bytes
	key := binary.LittleEndian.Uint32(payload[:4])
	data := payload[4:]

	// Parse key to determine chunk type
	sessionID := key / 10000
	chunkType := key % 10000

	var chunk *domain.AudioChunk
	switch {
	case chunkType == 0: // Start marker
		chunk = &domain.AudioChunk{Key: key, IsStart: true}
		log.Printf("Received start marker for session %d", sessionID)
	case chunkType >= 9999: // End marker
		chunk = &domain.AudioChunk{Key: key, IsEnd: true}
		log.Printf("Received end marker for session %d", sessionID)
	default: // Data chunk
		chunk = &domain.AudioChunk{Key: key, Data: data}
		log.Printf("Received data chunk %d for session %d (%d bytes)", chunkType, sessionID, len(data))
	}

	// Use assembler to handle the chunk
	stream, err := mh.audioAssembler.AssembleChunk(sessionID, chunk)
	if err != nil {
		return fmt.Errorf("failed to assemble chunk for session %d: %w", sessionID, err)
	}

	// If stream is complete, process it
	if stream != nil {
		log.Printf("Stream %d complete (%d chunks, %.2f seconds)",
			sessionID, len(stream.Chunks), stream.GetDuration())

		// Process the complete audio stream
		if err := mh.audioProcessor.ProcessAudio(stream); err != nil {
			log.Printf("Failed to process audio stream %d: %v", sessionID, err)
		}
	}

	return nil
}
