#ifndef ERROR_DRAWER_H
#define ERROR_DRAWER_H

#include "../display.h"
#include "../event_interface.h"

class ErrorDrawer : public DisplayDrawer {
public:
    void draw(U8G2* display, DisplayManager* manager) override {
        manager->drawHeader("Error");

        // Error icon
        display->setFont(u8g2_font_unifont_t_symbols);
        display->drawGlyph(50, 30, 0x274c);  // X mark emoji
        display->setFont(u8g2_font_6x10_tf);

        display->setCursor(0, 45);
        display->printf("%.15s", manager->getCurrentMessage().c_str());

        manager->drawFooter();
    }
};

#endif // ERROR_DRAWER_H