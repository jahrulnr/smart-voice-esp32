# External Services

This folder contains code for services running on other devices, separate from the ESP32 application in `app/`. These may include integrations like Whisper (speech transcription) or MQTT (messaging).

## Structure
- `whisper/`: Cloud-based speech-to-text service.
- `mqtt/`: IoT messaging protocol client/server.

## Setup
- Ensure dependencies are installed (e.g., via `pip install -r requirements.txt` for Python services).
- Run services independently of the ESP32 build.

## Integration
Services can communicate with the ESP32 via `network/` (e.g., WiFi/Web/FTP).