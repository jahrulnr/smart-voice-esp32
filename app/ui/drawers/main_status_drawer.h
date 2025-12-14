#ifndef MAIN_STATUS_DRAWER_H
#define MAIN_STATUS_DRAWER_H

#include "../display.h"
#include "../event_interface.h"
#include "../icons/icons.h"
#include "infrastructure/time_manager.h"

class MainStatusDrawer : public DisplayDrawer {
public:
    void draw(U8G2* display, DisplayManager* manager) override {
        display->setFontMode(1);
        display->setContrast(180);  // Higher contrast for better visibility

        // Draw main title with better styling - centered
        display->setFont(u8g2_font_7x14B_tf);  // Bold font for title
        String title = "PioAssistant";
        int titleWidth = title.length() * 7;  // 7px per character
        int titleX = (128 - titleWidth) / 2;  // Center horizontally
        display->setCursor(titleX, 10);
        display->printf("%s", title.c_str());
        
        // Draw decorative line under title
        display->drawHLine(0, 14, 128);
        display->drawHLine(0, 15, 128);

        // Weather section - top left with icon and data
        display->setFont(u8g2_font_4x6_tf);  // Smaller font for weather data
        
        // Weather box
        display->drawFrame(2, 18, 60, 20);
        
        display->setBitmapMode(1);

        // Weather icon (17x16) - dynamic based on weather
        const unsigned char* weatherIcon = icon16::weather_sun; // default sunny
        String weatherDesc = manager->getWeatherDescription();
        weatherDesc.toLowerCase();
        
        if (weatherDesc.indexOf("hujan") >= 0 || weatherDesc.indexOf("rain") >= 0 || weatherDesc.indexOf("shower") >= 0) {
            weatherIcon = icon16::weather_cloud_rain;
        } else if (weatherDesc.indexOf("thunder") >= 0 || weatherDesc.indexOf("storm") >= 0 || weatherDesc.indexOf("lightning") >= 0) {
            weatherIcon = icon16::weather_cloud_lightning;
        } else if (weatherDesc.indexOf("berawan") >= 0 || weatherDesc.indexOf("cloud") >= 0) {
            weatherIcon = icon16::weather_cloud_sunny;
        }
        // Default to weather_sun for "cerah" (clear/sunny) and other conditions
        
        display->drawXBM(6, 20, 17, 16, weatherIcon);
        
        // Weather data - smaller font
        display->setCursor(26, 28);
        if (manager->getWeatherDescription().length() > 0) {
            display->printf("%dC", manager->getTemperature());
            display->setCursor(26, 36);
            display->printf("%s", manager->getWeatherDescription().substring(0, 8).c_str());
        } else {
            display->printf("Loading...");
        }

        // Time section - top right
        display->drawFrame(66, 18, 60, 20);
        
        // Time display with appropriately sized font
        display->setFont(u8g2_font_7x14B_tf);  // 7px wide chars, fits 8 chars in 60px box
        display->setCursor(68, 32);
        String currentTime = TimeManager::getInstance().getCurrentTime();
        // Extract just HH:MM:SS from the full timestamp
        int spaceIndex = currentTime.indexOf(' ');
        if (spaceIndex > 0) {
            String timePart = currentTime.substring(spaceIndex + 1, spaceIndex + 9); // HH:MM:SS
            display->printf("%s", timePart.c_str());
        } else {
            display->printf("--:--:--");
        }

        // Network section - middle area
        display->setFont(u8g2_font_4x6_tf);  // Smaller font for IP
        display->drawFrame(2, 42, 124, 12);
        
        // Just show IP address - centered
        String ipText;
        if (manager->getWifiConnected()) {
            ipText = manager->getIpAddress();
        } else {
            ipText = "No IP";
        }
        int ipWidth = ipText.length() * 4;  // 4px per character for u8g2_font_4x6_tf
        int ipX = 2 + (124 - ipWidth) / 2;  // Center within the 124px wide frame (starts at x=2)
        display->setCursor(ipX, 50);
        display->printf("%s", ipText.c_str());
    }
};

#endif // MAIN_STATUS_DRAWER_H