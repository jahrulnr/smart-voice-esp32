#ifndef MAIN_STATUS_DRAWER_H
#define MAIN_STATUS_DRAWER_H

#include "../interface.h"
#include "../icons.h"
#include <core/time.h>
#include "app/network/WiFiManager.h"
#include "app/network/WeatherService.h"

class MainStatusDrawer : public DisplayDrawer {
public:
	MainStatusDrawer(U8G2* display = nullptr): 
		_display(display), 
		_data(weatherData_t{}){}

	void updateData(weatherData_t* data) {
		if (!data) return;
		_data = *data;
		delete data;
	}

	void draw() override {
		_display->clearBuffer();
		_display->setFontMode(1);
		_display->setContrast(180);  // Higher contrast for better visibility

		// Draw main title with better styling - centered
		_display->setFont(u8g2_font_7x14B_tf);  // Bold font for title
		String title = "PioAssistant";
		int titleWidth = title.length() * 7;  // 7px per character
		int titleX = (128 - titleWidth) / 2;  // Center horizontally
		_display->setCursor(titleX, 10);
		_display->printf("%s", title.c_str());
		
		// Draw decorative line under title
		_display->drawHLine(0, 14, 128);
		_display->drawHLine(0, 15, 128);

		// Weather section - top left with icon and data
		_display->setFont(u8g2_font_4x6_tf);  // Smaller font for weather data
		
		// Weather box
		_display->drawFrame(2, 18, 60, 20);

		// Weather icon (17x16) - dynamic based on weather
		const unsigned char* weatherIcon = icon16::weather_sun; // default sunny
		String weatherDesc = _data.description;
		weatherDesc.toLowerCase();
		
		if (weatherDesc.indexOf("hujan") >= 0 || weatherDesc.indexOf("rain") >= 0 || weatherDesc.indexOf("shower") >= 0) {
			weatherIcon = icon16::weather_cloud_rain;
		} else if (weatherDesc.indexOf("thunder") >= 0 || weatherDesc.indexOf("storm") >= 0 || weatherDesc.indexOf("lightning") >= 0) {
			weatherIcon = icon16::weather_cloud_lightning;
		} else if (weatherDesc.indexOf("berawan") >= 0 || weatherDesc.indexOf("cloud") >= 0) {
			weatherIcon = icon16::weather_cloud_sunny;
		}
		// Default to weather_sun for "cerah" (clear/sunny) and other conditions
		
		_display->setBitmapMode(1);
		_display->drawXBM(6, 20, 17, 16, weatherIcon);
		
		// Weather data - smaller font
		_display->setCursor(26, 28);
		if (_data.description.length() > 0) {
			_display->printf("%dC", _data.temperature);
			_display->setCursor(26, 36);
			_display->printf("%s", _data.description.substring(0, 8).c_str());
		} else {
			_display->printf("Loading");
		}

		// Time section - top right
		_display->drawFrame(66, 18, 60, 20);
		
		// Time display with appropriately sized font
		_display->setFont(u8g2_font_7x14B_tf);  // 7px wide chars, fits 8 chars in 60px box
		_display->setCursor(68, 32);
		String currentTime = timeManager.getCurrentTime();
		// Extract just HH:MM:SS from the full timestamp
		int spaceIndex = currentTime.indexOf(' ');
		if (spaceIndex > 0 && currentTime != "Time not synced") {
			String timePart = currentTime.substring(spaceIndex + 1, spaceIndex + 9); // HH:MM:SS
			_display->printf("%s", timePart.c_str());
		} else {
			_display->printf("--:--:--");
		}

		// Network section - middle area
		_display->setFont(u8g2_font_4x6_tf);  // Smaller font for IP
		_display->drawFrame(2, 42, 124, 12);
		
		// Just show IP address - centered
		String ipText;
		if (wifiManager.isConnected()) {
			ipText = wifiManager.getIPAddress();
		} else {
			ipText = "No IP";
		}
		int ipWidth = ipText.length() * 4;  // 4px per character for u8g2_font_4x6_tf
		int ipX = 2 + (124 - ipWidth) / 2;  // Center within the 124px wide frame (starts at x=2)
		_display->setCursor(ipX, 50);
		_display->printf("%s", ipText.c_str());
		_display->sendBuffer();
	}

private:
	U8G2* _display;
	weatherData_t _data;
};

#endif // MAIN_STATUS_DRAWER_H