#include "app/tasks.h"

struct NetworkInfo {
		String ssid;
		int32_t rssi;
		uint8_t encryptionType;
		String bssid;
		int32_t channel;
};

const char* wifiStatus() {
	switch (WiFi.status())
	{
	case WL_NO_SHIELD:
		return "WL_NO_SHIELD";
	case WL_STOPPED:
		return "WL_STOPPED";
	case WL_IDLE_STATUS:
		return "WL_IDLE_STATUS";
	case WL_NO_SSID_AVAIL:
		return "WL_NO_SSID_AVAIL";
	case WL_SCAN_COMPLETED:
		return "WL_SCAN_COMPLETED";
	case WL_CONNECTED:
		return "WL_CONNECTED";
	case WL_CONNECT_FAILED:
		return "WL_CONNECT_FAILED";
	case WL_CONNECTION_LOST:
		return "WL_CONNECTION_LOST";
	case WL_DISCONNECTED:
		return "WL_DISCONNECTED";
	}

	return "";
}

void networkTask(void *param) {
	const char* TAG = "networkTask";

  TickType_t lastWakeTime = xTaskGetTickCount();
  TickType_t updateFrequency = pdMS_TO_TICKS(100);
	size_t monitorCheck = millis();
	size_t timeCheck = millis();
	const char* lastEvent;

	wifiManager.init();
	wifiManager.addNetwork(WIFI_SSID, WIFI_PASS);
	wifiManager.begin();

	// wait notification initiate
	while (!notification)
		vTaskDelay(1);

	while(1) {
		vTaskDelayUntil(&lastWakeTime, updateFrequency);

		if (WiFi.isConnected() && millis() - timeCheck > 30000){
			timeManager.syncTime();
			ESP_LOGI(TAG, "Current Time: %s", timeManager.getCurrentTime());
			timeCheck = millis();
		}

		if(notification->hasSignal("WiFi check") && notification->signal("WiFi check") == 1){
			ESP_LOGI(TAG, "Wifi Status: %s", wifiStatus());
			std::vector<NetworkInfo> networks;
			int numNetworks = WiFi.scanNetworks();
			for (int i = 0; i < numNetworks; i++) {
				NetworkInfo network;
				network.ssid = WiFi.SSID(i);
				network.rssi = WiFi.RSSI(i);
				network.encryptionType = WiFi.encryptionType(i);
				network.bssid = WiFi.BSSIDstr(i);
				network.channel = WiFi.channel(i);
				networks.push_back(network);
					
				if (network.ssid == WIFI_SSID && !WiFi.isConnected() && wifiStatus() == "WL_DISCONNECTED") {
					WiFi.reconnect();
				}
			}

			for (auto network : networks) {
				ESP_LOGI(TAG, "Network: %s, RSSI: %d, Encryption: %d, BSSID: %s, Channel: %d", 
					network.ssid.c_str(), network.rssi, network.encryptionType, network.bssid.c_str(), network.channel);
			}
		}
	}
}