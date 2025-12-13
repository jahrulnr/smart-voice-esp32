#ifndef BOOT_SPLASH_DRAWER_H
#define BOOT_SPLASH_DRAWER_H

#include "../display.h"
#include "../event_interface.h"

class BootSplashDrawer : public DisplayDrawer {
public:
    BootSplashDrawer() : startTime(millis()), animationStep(0) {}
    
    void draw(U8G2* display, DisplayManager* manager) override {
        // Clear display with black background
        display->clearBuffer();
        
        // Set white text on black background
        display->setDrawColor(1); // White
        display->setFont(u8g2_font_helvR10_tr); // Medium font, 50% smaller than helvB18
        
        // Center the text "PioAssistent"
        const char* text = "PioAssistent";
        int textWidth = display->getStrWidth(text);
        int textHeight = display->getMaxCharHeight();
        int centerX = (display->getDisplayWidth() - textWidth) / 2;
        int centerY = (display->getDisplayHeight() + textHeight) / 2;
        
        // Simple fade-in animation (0-255 over 2 seconds)
        unsigned long elapsed = millis() - startTime;
        uint8_t brightness = _min(255, (elapsed * 255) / 2000); // 2 second fade-in
        
        // Draw text with current brightness
        display->setDrawColor(brightness > 128 ? 1 : 0); // Simple threshold for visibility
        display->drawStr(centerX, centerY, text);
        
        // Optional: Add subtle animation dots below
        if (elapsed > 1000) { // Start dots after 1 second
            int dotY = centerY + textHeight + 5;
            int dotSpacing = 8;
            int numDots = 3;
            
            for (int i = 0; i < numDots; i++) {
                int dotX = centerX + textWidth/2 - (numDots-1)*dotSpacing/2 + i*dotSpacing;
                // Animate dots appearing one by one
                if (elapsed > 1000 + i*300) {
                    display->drawPixel(dotX, dotY);
                }
            }
        }
        
        display->sendBuffer();
    }
    
private:
    unsigned long startTime;
    int animationStep;
};

#endif // BOOT_SPLASH_DRAWER_H