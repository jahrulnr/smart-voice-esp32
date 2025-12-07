# Audio I/O Feature 🎵

This folder contains the complete audio input/output pipeline for the ESP32 voice assistant, handling microphone capture and speaker playback.

## 🎤 Microphone (`microphone.h/.cpp`)

**Purpose**: High-quality audio capture using I2S interface with the INMP441 MEMS microphone.

**Key Features**:
- **I2S Audio Capture**: 16kHz, 16-bit mono audio stream
- **Noise Suppression**: Integrated ESP-SR noise reduction
- **Voice Activity Detection**: Wake word and speech detection
- **Real-time Processing**: Low-latency audio pipeline
- **Level Monitoring**: Audio level indication for UI feedback

**Hardware Configuration**:
```cpp
#define I2S_MIC_SD_PIN   GPIO_NUM_32  // Data output (DOUT)
#define I2S_MIC_SCK_PIN  GPIO_NUM_2   // Bit clock (BCLK)
#define I2S_MIC_WS_PIN   GPIO_NUM_33  // Word select (WS/LRCLK)
#define I2S_MIC_PORT     I2S_NUM_0    // I2S port
```

**API Methods**:
```cpp
// Lifecycle
bool init();                    // Initialize I2S and microphone
bool start();                   // Start audio capture
void stop();                    // Stop audio capture

// Audio data
int readLevel();               // Get current audio level (0-100)
bool isActive();               // Check if microphone is capturing

// Configuration
bool setGain(float gain);      // Set microphone gain (0.0-1.0)
float getGain();               // Get current gain setting
```

**Usage Example**:
```cpp
#include "audio/microphone.h"

Microphone mic;

// Initialize
if (!mic.init()) {
    Logger::error("AUDIO", "Failed to initialize microphone");
    return;
}

// Start capture
if (!mic.start()) {
    Logger::error("AUDIO", "Failed to start microphone");
    return;
}

// Monitor audio level
int level = mic.readLevel();
if (level > 50) {
    Logger::info("AUDIO", "High audio level detected: %d", level);
}
```

## 🔊 Speaker (`speaker.h/.cpp`)

**Purpose**: Audio playback system using I2S interface with MAX98357A amplifier for text-to-speech output.

**Key Features**:
- **I2S Audio Output**: 16kHz, 16-bit mono playback
- **Volume Control**: Software volume adjustment
- **Tone Generation**: Test tones and alerts
- **TTS Integration**: Seamless integration with Pico TTS
- **Queue Management**: Buffered audio playback

**Hardware Configuration**:
```cpp
#define I2S_SPEAKER_DOUT_PIN GPIO_NUM_20  // Data input (DIN)
#define I2S_SPEAKER_BCLK_PIN GPIO_NUM_26  // Bit clock (BCLK)
#define I2S_SPEAKER_LRC_PIN  GPIO_NUM_27  // Word select (WS/LRCLK)
#define I2S_SPEAKER_PORT     I2S_NUM_1    // I2S port (separate from mic)
```

**API Methods**:
```cpp
// Lifecycle
bool init();                    // Initialize I2S and speaker
bool start();                   // Start audio output
void stop();                    // Stop audio output

// Playback
bool playTone(float frequency, uint32_t duration_ms, float volume);
bool playBuffer(const int16_t* buffer, size_t samples, float volume);

// Volume control
bool setVolume(float volume);   // Set volume (0.0-1.0)
float getVolume();              // Get current volume

// Status
bool isPlaying();               // Check if audio is playing
size_t getQueueSize();          // Get playback queue size
```

**Usage Example**:
```cpp
#include "audio/speaker.h"

Speaker speaker;

// Initialize
if (!speaker.init()) {
    Logger::error("AUDIO", "Failed to initialize speaker");
    return;
}

if (!speaker.start()) {
    Logger::error("AUDIO", "Failed to start speaker");
    return;
}

// Play test tone
speaker.playTone(440.0f, 500, 0.3f);  // A4 note, 500ms, 30% volume

// Set volume
speaker.setVolume(0.8f);
```

## 🔄 Audio Pipeline

### Data Flow
```
INMP441 → I2S_NUM_0 → Microphone → ESP-SR → Command Processing
                                      ↓
MAX98357A ← I2S_NUM_1 ← Speaker ← Pico TTS ← Text Response
```

### Sample Rates & Formats
- **Input**: 16kHz, 16-bit, Mono (I2S_NUM_0)
- **Output**: 16kHz, 16-bit, Mono (I2S_NUM_1)
- **Processing**: Real-time with minimal latency

### Integration Points
- **Speech Recognition**: Microphone feeds ESP-SR for wake word/command detection
- **Text-to-Speech**: Speaker receives synthesized audio from Pico TTS
- **UI Feedback**: Audio levels drive visual indicators
- **Network**: Audio streaming capabilities (future feature)

## ⚙️ Configuration

Audio settings are configured in `config.h`:

```cpp
// Microphone
#define MIC_SAMPLE_RATE 16000
#define MIC_BIT_DEPTH I2S_DATA_BIT_WIDTH_16BIT
#define MIC_CHANNELS I2S_SLOT_MODE_MONO

// Speaker
#define SPEAKER_SAMPLE_RATE 16000
#define SPEAKER_BIT_DEPTH I2S_DATA_BIT_WIDTH_16BIT
#define SPEAKER_CHANNELS I2S_SLOT_MODE_MONO
```

## 🔧 Hardware Setup

### INMP441 Microphone
```
ESP32    INMP441
-----    -------
3.3V  →  VDD
GND   →  GND
GPIO32 →  DOUT (SD)
GPIO2  →  BCLK (SCK)
GPIO33 →  WS   (L/R)
```

### MAX98357A Speaker Amplifier
```
ESP32     MAX98357A
-----     ---------
3.3V   →  VIN
GND    →  GND
GPIO20 →  DIN
GPIO26 →  BCLK
GPIO27 →  LRC
```

## 🐛 Troubleshooting

### Microphone Issues
- **No Audio**: Check I2S pin connections and power (3.3V)
- **Distortion**: Verify sample rate and bit depth settings
- **Low Level**: Adjust microphone gain or check placement

### Speaker Issues
- **No Sound**: Check amplifier power and I2S connections
- **Distortion**: Verify sample rate match with TTS output
- **Low Volume**: Check speaker wiring and amplifier gain

### Common Problems
- **I2S Conflicts**: Ensure mic and speaker use different I2S ports
- **Power Issues**: Microphone needs clean 3.3V supply
- **Timing**: Audio pipeline requires precise timing

## 📊 Performance

- **CPU Usage**: ~15-20% for continuous audio processing
- **Memory**: ~8KB RAM for audio buffers
- **Latency**: <50ms end-to-end audio pipeline
- **Sample Rate**: 16kHz optimized for speech recognition

## 🔗 Dependencies

- **esp32-microphone**: I2S microphone driver
- **esp32-speaker**: I2S speaker driver
- **ESP-SR**: Speech recognition integration
- **Pico TTS**: Text-to-speech synthesis