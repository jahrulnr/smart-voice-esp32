#ifndef MAIN_STATUS_DRAWER_H
#define MAIN_STATUS_DRAWER_H

#include "../display.h"
#include "../event_interface.h"
#include "../icons/icons.h"
#include "infrastructure/time_manager.h"

class MainStatusDrawer : public DisplayDrawer {
public:
    void draw(U8G2* display, DisplayManager* manager) override {
        display->setBitmapMode(1);
        display->setFontMode(1);
        display->setContrast(180);  // Higher contrast for better visibility

        // Draw main title with better styling
        display->setFont(u8g2_font_7x14B_tf);  // Bold font for title
        display->setCursor(20, 12);
        display->printf("ESP32 Assistant");
        
        // Draw decorative line under title
        display->drawHLine(0, 16, 128);
        display->drawHLine(0, 17, 128);

        // Weather section - top left with icon and data
        display->setFont(u8g2_font_4x6_tf);  // Smaller font for weather data
        
        // Weather box
        display->drawFrame(2, 20, 60, 24);
        
        // Weather icon (17x16)
        display->drawXBM(6, 24, 17, 16, icon16::weather_cloud_sunny);
        
        // Weather data - smaller font
        display->setCursor(26, 32);
        if (manager->getWeatherDescription().length() > 0) {
            display->printf("%d°C", manager->getTemperature());
            display->setCursor(26, 40);
            display->printf("%s", manager->getWeatherDescription().substring(0, 8).c_str());
        } else {
            display->printf("Loading...");
        }

        // Time section - top right
        display->drawFrame(66, 20, 60, 24);
        
        // Time display with bigger font
        display->setFont(u8g2_font_10x20_tf);  // Even larger font for time
        display->setCursor(68, 40);
        String currentTime = TimeManager::getInstance().getCurrentTime();
        // Extract just HH:MM:SS from the full timestamp
        int spaceIndex = currentTime.indexOf(' ');
        if (spaceIndex > 0) {
            String timePart = currentTime.substring(spaceIndex + 1, spaceIndex + 9); // HH:MM:SS
            display->printf("%s", timePart.c_str());
        } else {
            display->printf("--:--:--");
        }

        // Network section - bottom area
        display->setFont(u8g2_font_4x6_tf);  // Smaller font for IP
        display->drawFrame(2, 48, 124, 16);
        
        // Just show IP address
        display->setCursor(6, 58);
        if (manager->getWifiConnected()) {
            display->printf("%s", manager->getIpAddress().c_str());
        } else {
            display->printf("No IP");
        }

        // Status indicators at bottom
        display->drawHLine(0, 64, 128);
        
        // Small status dots
        int statusY = 66;
        // Memory indicator (simplified)
        display->drawCircle(10, statusY, 2, U8G2_DRAW_ALL);
        display->setCursor(16, statusY + 2);
        display->printf("MEM");
        
        // Voice indicator
        display->drawCircle(45, statusY, 2, U8G2_DRAW_ALL);
        display->setCursor(51, statusY + 2);
        display->printf("VOICE");
        
        // GPT indicator
        display->drawCircle(90, statusY, 2, U8G2_DRAW_ALL);
        display->setCursor(96, statusY + 2);
        display->printf("AI");
    }
};

#endif // MAIN_STATUS_DRAWER_H