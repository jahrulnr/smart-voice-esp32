#ifndef MAIN_STATUS_DRAWER_H
#define MAIN_STATUS_DRAWER_H

#include "../display.h"
#include "../event_interface.h"

class MainStatusDrawer : public DisplayDrawer {
public:
    void draw(U8G2* display, DisplayManager* manager) override {
        manager->drawHeader("ESP32 Assistant");

        // Status icons
        manager->drawStatusIcons();

        // Main content - ensure consistent text width to prevent overlap
        display->setCursor(0, 25);
        if (manager->getWifiConnected()) {
            display->printf("WiFi: Connected  ");
        } else {
            display->printf("WiFi: Disconnected");
        }

        display->setCursor(0, 35);
        if (manager->getMicReady()) {
            display->printf("Mic: Ready       ");
        } else {
            display->printf("Mic: Offline     ");
        }

        display->setCursor(0, 45);
        if (manager->getGptReady()) {
            display->printf("GPT: Ready       ");
        } else {
            display->printf("GPT: Not Set     ");
        }

        display->setCursor(0, 55);
        display->printf("Mem: %d%% Free    ", 100 - manager->getMemoryPercent());

        manager->drawFooter();
    }
};

#endif // MAIN_STATUS_DRAWER_H