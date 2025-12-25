# Golang MQTT Consumer

A Golang application that consumes MQTT audio streams from the ESP32 voice assistant and processes them using Whisper for speech-to-text transcription.

## Architecture

This application follows **Hexagonal Architecture** (Ports and Adapters) to maintain clean separation between business logic and external dependencies.

### Structure

```
internal/
├── application/          # Application layer (use cases)
│   ├── ports/           # Interfaces (ports)
│   ├── message_handler.go
│   ├── audio_assembler.go
│   └── audio_processor.go
├── domain/              # Domain layer (business entities)
│   └── audio_stream.go
└── infrastructure/      # Infrastructure layer (adapters)
    └── mqtt_client.go
```

### Ports and Adapters

- **Ports**: Define interfaces for external interactions
  - `MessageHandler`: Handles incoming MQTT messages
  - `AudioAssembler`: Assembles audio chunks into streams
  - `AudioProcessor`: Processes complete audio streams

- **Adapters**: Implement the ports
  - `MQTTClient`: MQTT connectivity adapter
  - `AudioProcessor`: Whisper API adapter

## Features

- Consumes MQTT audio chunks from ESP32
- Assembles audio streams by session ID
- Converts PCM audio to WAV format
- Sends audio to Whisper API for transcription
- Handles multiple concurrent audio streams

## Configuration

Environment variables:

- `MQTT_BROKER`: MQTT broker URL (default: `tcp://localhost:1883`)
- `AUDIO_TOPIC`: MQTT topic to subscribe to (default: `pioassistant/audio`)
- `STT_TOPIC`: MQTT topic to publish transcriptions to (default: `pioassistant/stt`)
- `SAVE_AUDIO`: Whether to save audio files (default: `false`)
- `AUDIO_SAVE_DIR`: Directory to save audio files (default: `./audio`)
- `ENABLE_NOISE_REDUCTION`: Whether to apply noise reduction to audio before transcription (default: `true`)

## Audio Saving

When `SAVE_AUDIO=true`, the application will save each complete audio stream as a WAV file in the specified directory. Files are named with the format `stream_{sessionID}_{timestamp}.wav`.

Example:
```
audio/stream_12345_20231216_143022.wav
```

This feature is useful for debugging audio quality, testing transcription accuracy, or archiving audio data.

### With Docker Compose

```bash
docker-compose up --build
```

### Locally

```bash
go run cmd/main.go
```

### Build

```bash
go build -o bin/consumer cmd/main.go
```

## Audio Format

The application expects 16kHz mono 16-bit PCM audio chunks from the ESP32. It automatically wraps the PCM data in a WAV header before sending to Whisper.

## Dependencies

- [Eclipse Paho MQTT Go](https://github.com/eclipse/paho.mqtt.golang): MQTT client
- [Google UUID](https://github.com/google/uuid): For unique identifiers (if needed)

## Integration

This consumer works with:

- ESP32 voice assistant (audio streamer)
- Mosquitto MQTT broker
- Whisper speech-to-text service

The transcribed text is currently logged. Future enhancements could include:
- Sending transcriptions back via MQTT
- Storing in database
- Integration with GPT for responses