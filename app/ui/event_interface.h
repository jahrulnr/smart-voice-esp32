#ifndef EVENT_INTERFACE_H
#define EVENT_INTERFACE_H

#include <Arduino.h>

/**
 * Event types for system-wide communication
 */
enum class EventType {
    STATE_CHANGE,
    ERROR,
    PROGRESS,
    TEXT_UPDATE,
    STATUS_UPDATE,
    VOICE_ACTIVITY,
    NETWORK_STATUS,
    MEMORY_WARNING,
    WEATHER_UPDATE
};

/**
 * Event data structure
 */
struct EventData {
    EventType type;
    String message;
    int value;        // For progress, status codes, etc.
    void* data;       // Optional additional data

    EventData(EventType t, const String& msg = "", int val = 0, void* d = nullptr)
        : type(t), message(msg), value(val), data(d) {}
};

// Forward declaration
class DisplayManager;

/**
 * Display Drawer Interface - Defines drawing methods for different display states
 * Allows for consistent drawing implementations that can be swapped
 */
class DisplayDrawer {
public:
    virtual ~DisplayDrawer() = default;

    /**
     * Draw the display content
     * @param display U8G2 display instance
     * @param manager DisplayManager for accessing shared data
     */
    virtual void draw(U8G2* display, DisplayManager* manager) = 0;
};

#endif // EVENT_INTERFACE_H