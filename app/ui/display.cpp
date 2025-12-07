#include "display.h"
#include <esp_heap_caps.h>
#include <driver/gpio.h>  // For GPIO_NUM_xx definitions
#include "config.h"       // For display pin definitions

// Include drawer headers
#include "drawers/main_status_drawer.h"
#include "drawers/listening_drawer.h"
#include "drawers/processing_drawer.h"
#include "drawers/speaking_drawer.h"
#include "drawers/gp_response_drawer.h"
#include "drawers/error_drawer.h"
#include "drawers/config_drawer.h"

DisplayManager::DisplayManager()
    : _display(nullptr)
    , _currentState(DisplayState::MAIN_STATUS)
    , _lastUpdateTime(0)
    , _stateStartTime(0)
    , _scrollPosition(0)
    , _scrollableText(false)
    , _progressPercent(0)
    , _voiceActive(false)
    , _voiceActivityLevel(0)
    , _wifiConnected(false)
    , _memoryPercent(0)
    , _micReady(false)
    , _speakerReady(false)
    , _gptReady(false)
{
    // Initialize drawers
    _drawers[0] = new MainStatusDrawer();
    _drawers[1] = new ListeningDrawer();
    _drawers[2] = new ProcessingDrawer();
    _drawers[3] = new SpeakingDrawer();
    _drawers[4] = new GPResponseDrawer();
    _drawers[5] = new ErrorDrawer();
    _drawers[6] = new ConfigDrawer();
}

DisplayManager::~DisplayManager() {
    if (_display) {
        delete _display;
        _display = nullptr;
    }
    for (auto& drawer : _drawers) {
        delete drawer;
    }
}

bool DisplayManager::init() {
    Logger::info("DISPLAY", "Initializing SSD1306 OLED display...");

    // Initialize I2C
    Wire.begin(DISPLAY_SDA_PIN, DISPLAY_SCL_PIN);

    // Create display instance
    _display = new U8G2_SSD1306_128X64_NONAME_F_HW_I2C(U8G2_R0, DISPLAY_RESET_PIN, DISPLAY_SCL_PIN, DISPLAY_SDA_PIN);

    if (!_display) {
        Logger::error("DISPLAY", "Failed to create display instance");
        return false;
    }

    // Initialize display
    if (!_display->begin()) {
        Logger::error("DISPLAY", "Failed to initialize display");
        delete _display;
        _display = nullptr;
        return false;
    }

    // Configure display
    _display->setFont(u8g2_font_6x10_tf);  // Small readable font
    _display->setContrast(128);           // Medium brightness
    _display->clearBuffer();

    // Draw initial screen
    _drawers[0]->draw(_display, this);
    _display->sendBuffer();

    Logger::info("DISPLAY", "Display initialized successfully");
    return true;
}

void DisplayManager::update() {
    if (!_display) return;

    unsigned long currentTime = millis();

    // Handle scrolling text
    if (_scrollableText && _currentText.length() > 20) {
        if (currentTime - _lastUpdateTime > SCROLL_SPEED_MS) {
            _scrollPosition++;
            if (_scrollPosition > _currentText.length()) {
                _scrollPosition = 0;
            }
            _lastUpdateTime = currentTime;
        }
    }

    // Check for state timeout
    if (shouldReturnToMain()) {
        setState(DisplayState::MAIN_STATUS);
    }

    // Redraw display
    _display->clearBuffer();

    _drawers[static_cast<int>(_currentState)]->draw(_display, this);

    _display->sendBuffer();
}

void DisplayManager::setBrightness(uint8_t brightness) {
    if (_display) {
        _display->setContrast(brightness);
    }
}

void DisplayManager::setState(DisplayState state) {
    if (_currentState != state) {
        Logger::info("DISPLAY", "State change: %d -> %d", (int)_currentState, (int)state);
        _currentState = state;
        resetStateTimer();
        _scrollPosition = 0;
    }
}

// EventInterface implementation
void DisplayManager::onEvent(const EventData& event) {
    switch (event.type) {
        case EventType::STATE_CHANGE: {
            DisplayState newState = static_cast<DisplayState>(event.value);
            setState(newState);
            if (event.message.length() > 0) {
                _currentMessage = event.message;
            }
            break;
        }
        case EventType::ERROR: {
            setState(DisplayState::ERROR);
            _currentMessage = event.message;
            Logger::error("DISPLAY", "Error %d: %s", event.value, event.message.c_str());
            break;
        }
        case EventType::PROGRESS: {
            _progressPercent = constrain(event.value, 0, 100);
            if (event.message.length() > 0) {
                _currentMessage = event.message;
            }
            break;
        }
        case EventType::TEXT_UPDATE: {
            _currentText = event.message;
            _scrollableText = (event.value != 0);
            _scrollPosition = 0;
            if (_currentState != DisplayState::GPT_RESPONSE) {
                setState(DisplayState::GPT_RESPONSE);
            }
            break;
        }
        case EventType::STATUS_UPDATE: {
            StatusType type = static_cast<StatusType>(event.value);
            int status = (event.data != nullptr) ? *(int*)event.data : 0;
            switch (type) {
                case StatusType::WIFI:
                    _wifiConnected = (status == 1);
                    if (event.message.length() > 0) _ipAddress = event.message;
                    break;
                case StatusType::MEMORY:
                    _memoryPercent = status;
                    break;
                case StatusType::MICROPHONE:
                    _micReady = (status == 1);
                    break;
                case StatusType::SPEAKER:
                    _speakerReady = (status == 1);
                    break;
                case StatusType::GPT_SERVICE:
                    _gptReady = (status == 1);
                    break;
            }
            break;
        }
        case EventType::VOICE_ACTIVITY: {
            _voiceActivityLevel = constrain(event.value, 0, 100);
            _voiceActive = (event.data != nullptr) ? *(bool*)event.data : false;
            break;
        }
        case EventType::NETWORK_STATUS: {
            _wifiConnected = (event.value != 0);
            if (event.message.length() > 0) {
                _ipAddress = event.message;
            }
            break;
        }
        case EventType::MEMORY_WARNING: {
            _memoryPercent = event.value;
            size_t freeBytes = (event.data != nullptr) ? *(size_t*)event.data : 0;
            Logger::warn("DISPLAY", "Memory warning: %d%% used, %zu bytes free", event.value, freeBytes);
            break;
        }
    }
}

// Helper methods
void DisplayManager::drawProgressBar(int x, int y, int width, int height, int percent) {
    // Background
    _display->drawFrame(x, y, width, height);

    // Fill
    int fillWidth = (width - 2) * percent / 100;
    if (fillWidth > 0) {
        _display->drawBox(x + 1, y + 1, fillWidth, height - 2);
    }
}

void DisplayManager::drawScrollingText(int x, int y, int width, const String& text) {
    String displayText = text + "   ";  // Add spaces for smooth scroll
    int textWidth = _display->getStrWidth(displayText.c_str());

    // Calculate scroll offset
    int offset = -_scrollPosition * 6;  // 6 pixels per character approx

    // Draw scrolling text
    _display->setCursor(x + offset, y);
    _display->printf("%s", displayText.c_str());

    // Reset scroll when text has scrolled completely
    if (offset < -textWidth) {
        _scrollPosition = 0;
    }
}

void DisplayManager::drawStatusIcons() {
    int iconY = 20;

    // WiFi icon
    _display->setCursor(0, iconY);
    _display->printf(_wifiConnected ? "📡" : "❌");

    // Microphone icon
    _display->setCursor(25, iconY);
    _display->printf(_micReady ? "🎤" : "❌");

    // Speaker icon
    _display->setCursor(50, iconY);
    _display->printf(_speakerReady ? "🔊" : "❌");

    // GPT icon
    _display->setCursor(75, iconY);
    _display->printf(_gptReady ? "🤖" : "❌");

    // Memory indicator
    _display->setCursor(100, iconY);
    if (_memoryPercent > 80) {
        _display->printf("🟡");
    } else {
        _display->printf("🟢");
    }
}

void DisplayManager::drawHeader(const String& title) {
    // Header line
    _display->drawLine(0, 12, 127, 12);

    // Title
    _display->setCursor(0, 10);
    _display->printf("%s", title.c_str());
}

void DisplayManager::drawFooter() {
    // Footer line
    _display->drawLine(0, 63, 127, 63);

    // IP address if connected
    if (_wifiConnected && _ipAddress.length() > 0) {
        _display->setCursor(0, 63);
        _display->printf("%.15s", _ipAddress.c_str());
    }
}

bool DisplayManager::shouldReturnToMain() const {
    if (_currentState == DisplayState::MAIN_STATUS) return false;

    unsigned long elapsed = millis() - _stateStartTime;
    return elapsed > STATE_TIMEOUT_MS;
}

void DisplayManager::resetStateTimer() {
    _stateStartTime = millis();
}