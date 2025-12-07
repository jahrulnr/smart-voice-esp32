#ifndef LISTENING_DRAWER_H
#define LISTENING_DRAWER_H

#include "../display.h"
#include "../event_interface.h"

class ListeningDrawer : public DisplayDrawer {
public:
    void draw(U8G2* display, DisplayManager* manager) override {
        manager->drawHeader("Listening");

        // Microphone icon (using simple ASCII)
        display->setCursor(50, 30);
        display->printf("MIC");
        display->setFont(u8g2_font_6x10_tf);

        // Voice activity bar
        manager->drawProgressBar(10, 40, 108, PROGRESS_BAR_HEIGHT, manager->getVoiceActivityLevel());

        display->setCursor(40, 55);
        display->printf("%d%%", manager->getVoiceActivityLevel());

        manager->drawFooter();
    }
};

#endif // LISTENING_DRAWER_H