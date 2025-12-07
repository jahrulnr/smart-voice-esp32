#ifndef PROCESSING_DRAWER_H
#define PROCESSING_DRAWER_H

#include "../display.h"
#include "../event_interface.h"

class ProcessingDrawer : public DisplayDrawer {
public:
    void draw(U8G2* display, DisplayManager* manager) override {
        manager->drawHeader("Processing");

        // Thinking icon (using simple ASCII)
        display->setCursor(45, 30);
        display->printf("...?");
        display->setFont(u8g2_font_6x10_tf);

        // Progress bar
        manager->drawProgressBar(10, 40, 108, PROGRESS_BAR_HEIGHT, manager->getProgressPercent());

        display->setCursor(0, 55);
        display->printf("Sending to GPT...");

        manager->drawFooter();
    }
};

#endif // PROCESSING_DRAWER_H