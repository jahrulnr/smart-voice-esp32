#pragma once

/**
 * Display Drawer Interface - Defines drawing methods for different display states
 * Allows for consistent drawing implementations that can be swapped
 */
#include <U8g2lib.h>
#include "icons.h"
#include <app/network/WeatherService.h>

class DisplayDrawer {
public:
    DisplayDrawer() = default;
    virtual ~DisplayDrawer() = default;

    /**
     * Draw the display content
     * @param display U8G2 display instance
     * @param manager DisplayManager for accessing shared data
     */
    virtual void draw() = 0;

	void updateData(weatherData_t* data) {
		if (!data) return;
		_data = *data;
		delete data;
	}

protected:
	weatherData_t _data;
	
	const unsigned char* animateRain() {
		vTaskDelay(pdMS_TO_TICKS(300));
		static int frame = 0;
		if (frame > 3) frame = 0;
		switch (frame++) {
			case 0: return icon16::cloud_rain0;
			case 1: return icon16::cloud_rain1;
			case 2: return icon16::cloud_rain2;
			case 3: return icon16::cloud_rain3;
			default: return icon16::cloud_rain0;
		}
	}

	const unsigned char* animateCloudSunny() {
		vTaskDelay(pdMS_TO_TICKS(300));
		static int frame = 0;
		if (frame > 2) frame = 0;
		switch (frame++) {
			case 0: return icon16::cloud_sunny0;
			case 1: return icon16::cloud_sunny1;
			case 2: return icon16::cloud_sunny2;
			default: return icon16::cloud_rain0;
		}
	}

	const unsigned char* animateCloudLightning() {
		vTaskDelay(pdMS_TO_TICKS(250));
		static int frame = 0;
		if (frame > 5) frame = 0;
		switch (frame++) {
			case 0: return icon16::cloud_rain0;
			case 1: return icon16::cloud_rain1;
			case 2: return icon16::cloud_rain2;
			case 3: return icon16::cloud_rain3;
			case 4: return icon16::cloud_lightning0;
			case 5: return icon16::cloud_lightning1;
			default: return icon16::cloud_rain0;
		}
	}

	const unsigned char* animateSunny() {
		vTaskDelay(pdMS_TO_TICKS(300));
		static int frame = 0;
		if (frame > 1) frame = 0;
		switch (frame++) {
			case 0: return icon16::sun0;
			case 1: return icon16::sun1;
			default: return icon16::sun0;
		}
	}
};