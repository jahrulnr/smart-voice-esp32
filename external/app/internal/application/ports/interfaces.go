package ports

import "app/internal/domain"

// MessageHandler defines the interface for handling incoming messages
type MessageHandler interface {
	HandleMessage(payload []byte) error
}

// AudioAssembler defines the interface for assembling audio chunks
type AudioAssembler interface {
	AssembleChunk(sessionID uint32, chunk *domain.AudioChunk) (*domain.AudioStream, error)
}

// AudioProcessor defines the interface for processing complete audio streams
type AudioProcessor interface {
	ProcessAudio(stream *domain.AudioStream) error
}

// BrokerPublisher defines the interface for publishing MQTT messages
type BrokerPublisher interface {
	Publish(topic string, payload []byte) error
}
