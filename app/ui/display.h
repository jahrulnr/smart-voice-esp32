#ifndef DISPLAY_H
#define DISPLAY_H

#include <U8g2lib.h>
#include <Wire.h>
#include "event_interface.h"
#include "infrastructure/logger.h"

// Display timing constants
#define STATE_TIMEOUT_MS 10000  // Return to main after 10 seconds
#define SCROLL_SPEED_MS 150     // Text scroll speed
#define PROGRESS_BAR_HEIGHT 8

// Display states for the voice assistant
enum class DisplayState {
    MAIN_STATUS,
    LISTENING,
    PROCESSING,
    SPEAKING,
    GPT_RESPONSE,
    ERROR,
    CONFIG
};

// Status types for status updates
enum class StatusType {
    WIFI,
    MEMORY,
    MICROPHONE,
    SPEAKER,
    GPT_SERVICE
};

/**
 * Display Manager - Handles OLED display for ESP32 Voice Assistant
 * Uses strategy pattern with DisplayDrawer implementations
 */
class DisplayManager {
public:
    DisplayManager();
    ~DisplayManager();

    /**
     * Initialize the display
     * @return true if initialization successful
     */
    bool init();

    /**
     * Update display (call in main loop)
     */
    void update();

    /**
     * Set display brightness
     * @param brightness 0-255
     */
    void setBrightness(uint8_t brightness);

    /**
     * Force display state change
     * @param state New display state
     */
    void setState(DisplayState state);

    // Event handling
    void onEvent(const EventData& event);

    // Getters for drawer access
    bool getWifiConnected() const { return _wifiConnected; }
    const String& getIpAddress() const { return _ipAddress; }
    bool getMicReady() const { return _micReady; }
    bool getSpeakerReady() const { return _speakerReady; }
    bool getGptReady() const { return _gptReady; }
    int getMemoryPercent() const { return _memoryPercent; }
    int getVoiceActivityLevel() const { return _voiceActivityLevel; }
    int getProgressPercent() const { return _progressPercent; }
    const String& getCurrentText() const { return _currentText; }
    bool getScrollableText() const { return _scrollableText; }
    const String& getCurrentMessage() const { return _currentMessage; }

    // Public helper methods for drawers
    void drawProgressBar(int x, int y, int width, int height, int percent);
    void drawScrollingText(int x, int y, int width, const String& text);
    void drawStatusIcons();
    void drawHeader(const String& title);
    void drawFooter();

private:
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C* _display;
    DisplayState _currentState;
    unsigned long _lastUpdateTime;
    unsigned long _stateStartTime;
    int _scrollPosition;
    bool _scrollableText;
    String _currentText;
    String _currentMessage;
    int _progressPercent;
    bool _voiceActive;
    int _voiceActivityLevel;

    // Status tracking
    bool _wifiConnected;
    String _ipAddress;
    int _memoryPercent;
    bool _micReady;
    bool _speakerReady;
    bool _gptReady;

    // Drawers
    DisplayDrawer* _drawers[7];

    // State timeout handling
    bool shouldReturnToMain() const;
    void resetStateTimer();
};

#endif // DISPLAY_H