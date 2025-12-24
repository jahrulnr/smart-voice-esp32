#ifndef BOOT_SPLASH_DRAWER_H
#define BOOT_SPLASH_DRAWER_H

#include "../interface.h"

class BootSplashDrawer : public DisplayDrawer {
public:
	BootSplashDrawer(U8G2* display = nullptr) : _startTime(millis()), _display(display), _taskHandle(nullptr) {}

		inline void start() {
			xTaskCreate([](void* arg) {
				BootSplashDrawer* drawer = static_cast<BootSplashDrawer*>(arg);
				for(;;){
					vTaskDelay(pdMS_TO_TICKS(33));
					drawer->draw();
				}
				vTaskDelete(nullptr);
			}, "bootSplash", 4096, this, 1, &_taskHandle);
		}

		inline void stop() {
			if(_taskHandle != nullptr){
				vTaskDelete(_taskHandle);
				_taskHandle = nullptr;
			}
		}
	
	inline void draw() override {
		// Clear display with black background
		_display->clearBuffer();
		
		// Set white text on black background
		_display->setDrawColor(1); // White
		_display->setFont(u8g2_font_helvB10_tr); // Bold font, sized for 128x64 display
		
		// Center the text "PioAssistant"
		const char* text = "PioAssistant";
		int textWidth = _display->getStrWidth(text);
		int textHeight = _display->getMaxCharHeight();
		int centerX = (_display->getDisplayWidth() - textWidth) / 2;
		int centerY = (_display->getDisplayHeight() + textHeight) / 2; // Center the text
		
		// Draw text (no fade for monochrome display)
		_display->setDrawColor(1);
		_display->drawStr(centerX, centerY, text);
		
		// Simple fade-in animation (0-255 over 2 seconds)
		unsigned long elapsed = millis() - _startTime;
		uint8_t brightness = _min(255, (elapsed * 255) / 2000); // 2 second fade-in
		
		// Optional: Add subtle animation dots below
		if (elapsed > 1000) { // Start dots after 1 second
			int dotY = centerY + textHeight + 5;
			int dotSpacing = 8;
			int numDots = 3;
			
			for (int i = 0; i < numDots; i++) {
				int dotX = centerX + textWidth/2 - (numDots-1)*dotSpacing/2 + i*dotSpacing;
				// Animate dots appearing one by one
				if (elapsed > 1000 + i*300) {
					_display->drawPixel(dotX, dotY);
				}
			}
		}
		
		_display->sendBuffer();
	}

private:
	unsigned long _startTime;
	TaskHandle_t _taskHandle;
	U8G2* _display;
};

#endif // BOOT_SPLASH_DRAWER_H