#ifndef GP_RESPONSE_DRAWER_H
#define GP_RESPONSE_DRAWER_H

#include "../display.h"
#include "../event_interface.h"

class GPResponseDrawer : public DisplayDrawer {
public:
    void draw(U8G2* display, DisplayManager* manager) override {
        manager->drawHeader("GPT Response");

        if (manager->getScrollableText() && manager->getCurrentText().length() > 20) {
            manager->drawScrollingText(0, 25, 128, manager->getCurrentText());
        } else {
            display->setCursor(0, 25);
            display->printf("%.20s", manager->getCurrentText().c_str());

            if (manager->getCurrentText().length() > 20) {
                display->setCursor(0, 35);
                display->printf("%.20s", manager->getCurrentText().substring(20).c_str());
            }
        }

        manager->drawFooter();
    }
};

#endif // GP_RESPONSE_DRAWER_H