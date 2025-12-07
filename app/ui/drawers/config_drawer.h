#ifndef CONFIG_DRAWER_H
#define CONFIG_DRAWER_H

#include "../display.h"
#include "../event_interface.h"

class ConfigDrawer : public DisplayDrawer {
public:
    void draw(U8G2* display, DisplayManager* manager) override {
        manager->drawHeader("Configuration");

        display->setCursor(0, 25);
        display->printf("WiFi Setup:");
        display->setCursor(0, 35);
        display->printf("192.168.4.1");

        display->setCursor(0, 50);
        display->printf("Use web interface");

        manager->drawFooter();
    }
};

#endif // CONFIG_DRAWER_H