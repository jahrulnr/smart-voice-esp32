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

        // Main content
        display->setCursor(0, 25);
        display->printf("WiFi: %s", manager->getWifiConnected() ? "Connected" : "Disconnected");

        display->setCursor(0, 35);
        display->printf("Mic: %s", manager->getMicReady() ? "Ready" : "Offline");

        display->setCursor(0, 45);
        display->printf("GPT: %s", manager->getGptReady() ? "Ready" : "Not Set");

        display->setCursor(0, 55);
        display->printf("Mem: %d%% Free", 100 - manager->getMemoryPercent());

        manager->drawFooter();
    }
};

#endif // MAIN_STATUS_DRAWER_H