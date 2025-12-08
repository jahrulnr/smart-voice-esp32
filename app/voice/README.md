# Voice Processing Feature 🎙️

This folder contains the voice synthesis and speech processing components for the ESP32 voice assistant.

## 🔊 Pico TTS (`pico_tts.h/.cpp`)

**Purpose**: High-quality text-to-speech synthesis using the Pico TTS engine, providing natural voice output through the speaker system.

**Key Features**:
- **Natural Speech**: High-quality voice synthesis with natural intonation
- **Multiple Voices**: Support for different voice profiles (future)
- **Real-time Synthesis**: Low-latency text-to-speech conversion
- **Speaker Integration**: Seamless audio output through I2S speaker
- **Queue Management**: Buffered speech output with interruption support

**API Methods**:
```cpp
// Lifecycle
bool init(Speaker* speaker);    // Initialize TTS with speaker output
void deinit();                  // Clean up TTS resources

// Speech synthesis
bool speak(const String& text); // Synthesize and play text
bool speak(const char* text);   // C-string version

// Control
bool stop();                    // Stop current speech
bool isSpeaking();              // Check if currently speaking

// Configuration
bool setVoice(int voice_id);    // Select voice profile (future)
int getVoice();                 // Get current voice
bool setSpeed(float speed);     // Set speech speed (0.5-2.0)
float getSpeed();               // Get current speed
```

**Usage Example**:
```cpp
#include "voice/pico_tts.h"
#include "audio/speaker.h"

Speaker speaker;
PicoTTS tts;

// Initialize audio output first
speaker.init();
speaker.start();

// Initialize TTS
if (!tts.init(&speaker)) {
    Logger::error("TTS", "Failed to initialize Pico TTS");
    return;
}

// Speak text
tts.speak("Hello, I am your ESP32 voice assistant.");
tts.speak("The current time is " + getTimeString());

// Stop speech if needed
if (interruptRequested) {
    tts.stop();
}
```

## 🎯 Speech Recognition (`voice_recognizer.h/.cpp`)

**Purpose**: Wake word detection and command recognition using ESP-SR models with callback-based event handling.

**Key Features**:
- **Wake Word Detection**: "Hi ESP" wake word activation
- **Command Recognition**: Custom voice command processing
- **EventInterface Integration**: Consistent event handling with display system
- **Noise Suppression**: Environmental noise filtering via ESP-SR
- **Real-time Processing**: Low-latency voice recognition

**API Methods**:
```cpp
// Lifecycle
bool init(Microphone* microphone, const VoiceConfig& config);
void deinit();

// Control
bool startListening();           // Start wake word detection
bool stopListening();            // Stop recognition
bool isListening() const;        // Check listening state

// Event handling
void setEventCallback(VoiceEventCallback callback); // Set event handler
```

**Usage Example**:
```cpp
#include "voice/voice_command_handler.h"
#include "audio/microphone.h"
#include "application/gpt_service.h"
#include "ui/display.h"

Microphone microphone;
Services::GPTService gptService;
DisplayManager displayManager;
VoiceCommandHandler handler;

// Initialize components first
microphone.init();
gptService.init();
displayManager.init();

// Initialize handler
VoiceConfig config = {}; // Use defaults
if (!handler.init(&microphone, &gptService, &displayManager, config)) {

// Configure voice recognition
VoiceConfig config = {
    .commands = nullptr,  // Use defaults (hello, time, weather, music, stop, help)
    .commandCount = 0,
};

// Initialize recognizer
if (!recognizer.init(&microphone, config)) {
    Logger::error("VOICE", "Failed to initialize voice recognizer");
    return;
}

// Set up event callback
recognizer.setEventCallback([](VoiceEvent event, int commandId, int phraseId) {
    switch (event) {
        case VOICE_WAKEWORD_DETECTED:
            Logger::info("VOICE", "Wake word detected!");
            // Start command recognition UI
            break;
        case VOICE_COMMAND_DETECTED:
            Logger::info("VOICE", "Command detected: %d", commandId);
            // Process the recognized command
            break;
        case VOICE_TIMEOUT:
            Logger::info("VOICE", "Recognition timeout");
            // Return to wake word listening
            break;
        case VOICE_ERROR:
            Logger::error("VOICE", "Recognition error");
            break;
    }
});

// Start listening for wake word
recognizer.startListening();
```

## 📝 Command Parser (`command_parser.h/.cpp`) - *Planned*

**Purpose**: Natural language processing for voice commands.

**Planned Features**:
- **Intent Recognition**: Parse user intents from speech
- **Parameter Extraction**: Extract parameters from commands
- **Context Awareness**: Maintain conversation context
- **Action Mapping**: Map commands to system actions

## 🔄 Voice Pipeline

### Current Flow
```
Microphone → VoiceCommandHandler → Command Processing → Pico TTS → Speaker
                              ↓
                        UI Updates & GPT Integration
```

### Complete Flow
```
Microphone → VoiceCommandHandler → Command Parser → GPT Service → Pico TTS → Speaker
                                      ↓
                                Display Updates
```

## ⚙️ Configuration

Voice settings in `config.h`:
```cpp
// TTS Configuration
#define TTS_DEFAULT_VOICE 0
#define TTS_DEFAULT_SPEED 1.0f
#define TTS_SAMPLE_RATE 16000
```

**Note**: Speech recognition parameters (wake word, sensitivity, timeout) are handled internally by ESP-SR and not configurable through the API.

## 🎵 Audio Specifications

- **Sample Rate**: 16kHz (optimized for speech)
- **Bit Depth**: 16-bit PCM
- **Channels**: Mono
- **Format**: Linear PCM

## 🔧 Integration

### With Audio System
```cpp
// TTS requires speaker to be initialized first
speaker.init();
speaker.start();
tts.init(&speaker);
```

### With Task System
```cpp
// TTS calls should be made from FreeRTOS tasks
xTaskCreate(ttsTask, "TTS_Task", 4096, NULL, 1, NULL);
```

## 🐛 Troubleshooting

### TTS Issues
- **No Audio**: Ensure speaker is initialized before TTS
- **Garble**: Check sample rate compatibility
- **Memory Errors**: Reduce text length or increase task stack size

### Voice Recognition Issues
- **Wake Word Not Detected**: Check microphone placement and background noise
- **Low Accuracy**: Ensure proper microphone positioning and reduce background noise
- **False Triggers**: Improve microphone positioning or check for acoustic interference
- **Timeout Issues**: Speak more clearly and at normal pace
- **Model Loading**: Ensure SR models are flashed to ESP32 (see model/README.md)

## 📊 Performance

- **Synthesis Speed**: ~100-200 words per minute
- **Recognition Latency**: ~100-300ms wake word detection
- **Memory Usage**: ~50KB RAM for TTS + ~100KB for SR models
- **CPU Usage**: ~10-15% during speech synthesis, ~20-30% during recognition
- **Wake Word Accuracy**: >95% with proper microphone placement

## 🔗 Dependencies

- **esp32-picoTTS**: Text-to-speech synthesis library
- **esp32-speaker**: Audio output driver
- **esp-sr**: Speech recognition library (ESP-SR)
- **esp32-microphone**: Audio input driver
- **FreeRTOS**: Task scheduling for async operation