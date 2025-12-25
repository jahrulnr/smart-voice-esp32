#include "app/tasks.h"
#include <FTPServer.h>

const char* topic = "pioassistant/audio";

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
	TickType_t updateFrequency = pdMS_TO_TICKS(1);
	unsigned long monitorCheck = millis();
	unsigned long timeCheck = 0;
	unsigned long weatherCheck = 0;
	unsigned long mqttCheck = 0;
	const char* lastEvent;

	wifiManager.init();
	wifiManager.addNetwork(WIFI_SSID, WIFI_PASS);
	wifiManager.begin();
	WiFiClient wifiClient;
	
	String mac = WiFi.macAddress();
	mac.replace(":", "");
	String mqttClientId = String(MQTT_CLIENT_ID) + "-" + mac.substring(6);

	FTPServer ftpServer(LittleFS);
	ftpServer.begin(FTP_USER, FTP_PASS);

	mqttClient.setClient(wifiClient);
	mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
	mqttClient.setBufferSize(16 * 1030);
	mqttClient.setCallback(mqttCallback);
	bool hasSubsribe = false;
	AudioSamples audioSamples;

	weatherConfig_t weatherConfig;
	weatherConfig.adm4Code = "31.73.05.1001"; // Default Jakarta Barat location
	weatherConfig.cacheExpiryMinutes = 60;
	weatherService.init(weatherConfig);

	// wait notification initiate
	while (!notification)
		vTaskDelay(1);

	while(1) {
		vTaskDelayUntil(&lastWakeTime, updateFrequency);

		if(notification->hasSignal("WiFi check") && notification->signal("WiFi check") == 1){
			ESP_LOGI(TAG, "Wifi Status: %s", wifiStatus());
			ESP_LOGI(TAG, "MQTT Status: %s; clientId: %s", mqttClient.connected() ? "Connected" : "Disconnected", mqttClientId.c_str());
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

				if (network.ssid == WIFI_SSID && wifiStatus() == "WL_DISCONNECTED") {
					WiFi.reconnect();
				}
			}

			for (auto network : networks) {
				ESP_LOGI(TAG, "Network: %s, RSSI: %d, Encryption: %d, BSSID: %s, Channel: %d",
					network.ssid.c_str(), network.rssi, network.encryptionType, network.bssid.c_str(), network.channel);
			}
		}

		if (wifiManager.isConnected() && millis() - timeCheck > 30000){
			timeManager.syncTime();
			ESP_LOGI(TAG, "Current Time: %s", timeManager.getCurrentTime());
			timeCheck = millis();
		}

		// Periodic weather updates (every 5 minutes)
		if (wifiManager.isConnected() && millis() - weatherCheck > (weatherService.isCacheValid() ? 300000 : 10000)) {
			ESP_LOGI(TAG, "Fetching weather update...");
			weatherService.getCurrentWeather([TAG](const weatherData_t& data, bool success) {
				if (success) {
					// Send weather update event
					ESP_LOGI(TAG, "Weather updated: %s, %d°C", data.description.c_str(), data.temperature);
					
					weatherData_t* newData = new weatherData_t(data);
					if (!notification->send(NOTIFICATION_WEATHER, (void*)newData)){
						delete newData;
					}
				} else {
					ESP_LOGE(TAG, "Weather update failed");
				}
			});
			weatherCheck = millis();
		}

		bool connected = mqttClient.loop();
		if (!connected && hasSubsribe) {
			mqttClient.unsubscribe(MQTT_TOPIC_STT);
		} else if (connected && !hasSubsribe) {
			mqttClient.subscribe(MQTT_TOPIC_STT);
			hasSubsribe = true;
		}
		
		if (wifiManager.isConnected()) {
			ftpServer.handleFTP();

			if (xQueueReceive(audioQueue, &audioSamples, 10) == pdTRUE) {
				if (audioSamples.data != nullptr && audioSamples.length > 0){
					mqttClient.publish(audioSamples.topic, audioSamples.data, audioSamples.length);
					delete[] audioSamples.data;
					audioSamples.data = nullptr;
				}
			}

			// handle loop
			try {
				if (!connected && millis() - mqttCheck > 5000) {
					if (strlen(MQTT_USER) > 0) {
						mqttClient.connect(mqttClientId.c_str(), MQTT_USER, MQTT_PASS);
					} else {
						mqttClient.connect(mqttClientId.c_str());
					}

					mqttCheck = millis();
				}
			}
			catch(const std::exception& e)
			{
				ESP_LOGW(TAG, "MQTT loop error: %s", e.what());
			}
			catch(...) {
				ESP_LOGW(TAG, "MQTT loop unknown error");
			}
		}
	}

	ftpServer.stop();
}