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

The application supports configuration via environment variables. You can set these in your shell or create a `.env` file in the project root.

Example `.env` file:
```env
# OpenAI Realtime API Configuration
OPENAI_API_KEY=your-openai-api-key-here
OPENAI_REALTIME_MODEL=gpt-4o-realtime-preview

# MQTT Configuration
MQTT_BROKER=tcp://localhost:1883
AUDIO_TOPIC=pioassistant/audio
STT_TOPIC=pioassistant/stt

# Whisper API Configuration
WHISPER_URL=http://localhost:8000/transcribe

# Audio Saving Configuration
SAVE_AUDIO=false
AUDIO_SAVE_DIR=./audio
ENABLE_NOISE_REDUCTION=true

# GPT Audio Saving Configuration
SAVE_GPT_AUDIO=true
GPT_AUDIO_SAVE_DIR=./gpt_audio
CONVERT_GPT_AUDIO_TO_MP3=true
```

Environment variables:

- `MQTT_BROKER`: MQTT broker URL (default: `tcp://localhost:1883`)
- `AUDIO_TOPIC`: MQTT topic to subscribe to (default: `pioassistant/audio`)
- `STT_TOPIC`: MQTT topic to publish transcriptions to (default: `pioassistant/stt`)
- `WHISPER_URL`: Whisper API endpoint (default: `http://localhost:8000/transcribe`)
- `SAVE_AUDIO`: Whether to save audio files (default: `false`)
- `AUDIO_SAVE_DIR`: Directory to save audio files (default: `./audio`)
- `ENABLE_NOISE_REDUCTION`: Whether to apply noise reduction to audio before transcription (default: `true`)
- `SAVE_GPT_AUDIO`: Whether to save GPT audio responses (default: `false`)
- `GPT_AUDIO_SAVE_DIR`: Directory to save GPT audio files (default: `./gpt_audio`)
- `CONVERT_GPT_AUDIO_TO_MP3`: Whether to convert saved WAV files to MP3 (default: `false`)
- `OPENAI_API_KEY`: OpenAI API key for Realtime API access
- `OPENAI_REALTIME_MODEL`: OpenAI Realtime API model name (default: `gpt-4o-realtime-preview`)

## Audio Saving

When `SAVE_AUDIO=true`, the application will save each complete audio stream as a WAV file in the specified directory. Files are named with the format `stream_{sessionID}_{timestamp}.wav`.

Example:
```
audio/stream_12345_20231216_143022.wav
```

This feature is useful for debugging audio quality, testing transcription accuracy, or archiving audio data.

## GPT Audio Saving

When testing the OpenAI Realtime API, you can save GPT's audio responses as WAV or MP3 files:

Environment variables:
- `SAVE_GPT_AUDIO`: Whether to save GPT audio responses (default: `false`)
- `GPT_AUDIO_SAVE_DIR`: Directory to save GPT audio files (default: `./gpt_audio`)
- `CONVERT_GPT_AUDIO_TO_MP3`: Whether to convert saved WAV files to MP3 (default: `false`)

When `SAVE_GPT_AUDIO=true`, the application will save each audio delta from GPT responses as WAV files in the specified directory. Files are named with the format `gpt_response_{counter}_{timestamp}.wav`.

If `CONVERT_GPT_AUDIO_TO_MP3=true`, each WAV file will also be converted to MP3 format using ffmpeg.

Example:
```
gpt_audio/gpt_response_0_20240101_120000.wav
gpt_audio/gpt_response_0_20240101_120000.mp3
```

**Note**: Requires ffmpeg to be installed for MP3 conversion.

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

## Testing OpenAI Realtime API

For testing the OpenAI Realtime API integration before handling real ESP32 traffic:

1. Create or edit the `.env` file and set your OpenAI API key:
   ```env
   OPENAI_API_KEY=your-openai-api-key-here
   SAVE_GPT_AUDIO=true
   CONVERT_GPT_AUDIO_TO_MP3=true
   ```

2. The test uses existing WAV files from the `audio/` directory
3. Run the test mode:
   ```bash
   go run cmd/main.go test
   ```

This will:
- Load existing audio files from the `audio/` directory
- Send them to OpenAI's Realtime API via WebSocket
- Log received responses (text and audio deltas)
- Save GPT audio responses as WAV/MP3 files if enabled

**Note**: Uses WebSocket-based implementation (simpler than WebRTC for testing). Requires OpenAI API key with Realtime API access.

## Integration

This consumer works with:

- ESP32 voice assistant (audio streamer)
- Mosquitto MQTT broker
- Whisper speech-to-text service

The transcribed text is currently logged. Future enhancements could include:
- Sending transcriptions back via MQTT
- Storing in database
- Integration with GPT for responses