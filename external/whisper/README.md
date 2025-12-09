# Whisper API Service

A production-ready HTTP API for audio transcription using OpenAI's Whisper model, containerized with Docker for easy deployment.

## Features

- **Audio Transcription**: Transcribe audio files (.mp3, .wav, .m4a, .flac) using Whisper's tiny model.
- **Language Support**: Auto-detect language or specify a target language (e.g., "id" for Indonesian, "en" for English).
- **FastAPI Framework**: High-performance async API with automatic OpenAPI documentation.
- **Dockerized**: Easy to build and run in containers.

## Prerequisites

- Docker
- Audio files in supported formats

## Quick Start

### 1. Build the Docker Image

```bash
make build
```

This builds a Docker image named `whisper` with Python 3.11, FFmpeg, PyTorch, OpenAI Whisper, FastAPI, and Uvicorn installed.

### 2. Run the Container

```bash
make run
```

This starts the container in detached mode, exposing port 8000 for API access.

### 3. Access the API

- **Interactive Docs**: Visit `http://localhost:8000/docs` for Swagger UI.
- **Transcribe Audio**: POST to `http://localhost:8000/transcribe` with an audio file and optional `language` parameter.

Example using curl:
```bash
curl -X POST "http://localhost:8000/transcribe" \
     -H "accept: application/json" \
     -H "Content-Type: multipart/form-data" \
     -F "file=@/path/to/audio.mp3" \
     -F "language=id"
```

Response:
```json
{
  "language": "id",
  "text": "Transcribed text here..."
}
```

## Development

### File Structure

- `Dockerfile`: Defines the container environment.
- `Makefile`: Build and run commands.

Then update paths in testing scripts accordingly.