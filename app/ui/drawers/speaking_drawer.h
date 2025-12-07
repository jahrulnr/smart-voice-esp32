#ifndef SPEAKING_DRAWER_H
#define SPEAKING_DRAWER_H

#include "../display.h"
#include "../event_interface.h"

class SpeakingDrawer : public DisplayDrawer {
public:
    void draw(U8G2* display, DisplayManager* manager) override {
        manager->drawHeader("Speaking");

        // Speaker icon (using simple ASCII)
        display->setCursor(50, 30);
        display->printf("SPK");
        display->setFont(u8g2_font_6x10_tf);

        // Progress bar for speech
        manager->drawProgressBar(10, 40, 108, PROGRESS_BAR_HEIGHT, manager->getProgressPercent());

        display->setCursor(0, 55);
        display->printf("Playing audio...");

        manager->drawFooter();
    }
};

#endif // SPEAKING_DRAWER_H