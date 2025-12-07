#include "wifi_manager.h"
#include <Preferences.h>
#include <LittleFS.h>

WifiManager::WifiManager() : webServer(nullptr), dnsServer(nullptr), apMode(false), lastReconnectAttempt(0) {}

WifiManager::~WifiManager() {
    stopHotspot();
    if (webServer) delete webServer;
    if (dnsServer) delete dnsServer;
}

bool WifiManager::init() {
    Logger::info("WIFI", "Initializing WiFi manager");
    WiFi.onEvent(onWiFiEvent);
    return true;
}

void WifiManager::begin() {
    Logger::info("WIFI", "Starting WiFi connection process");

    // Try to connect to any available saved network
    if (connectToAvailableNetwork()) {
        Logger::info("WIFI", "Connected to WiFi successfully");
        return;
    }

    Logger::warn("WIFI", "No saved networks available or connection failed, starting hotspot");
    startHotspot();
}

bool WifiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String WifiManager::getIPAddress() {
    if (apMode) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

WebServer* WifiManager::getWebServer() {
    return webServer;
}

std::vector<String> WifiManager::scanNetworks() {
    std::vector<String> networks;
    int numNetworks = WiFi.scanNetworks();
    for (int i = 0; i < numNetworks; i++) {
        networks.push_back(WiFi.SSID(i));
    }
    return networks;
}

bool WifiManager::connect(const String& ssid, const String& password) {
    Logger::info("WIFI", "Connecting to: %s", ssid.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
    }

    return WiFi.status() == WL_CONNECTED;
}

bool WifiManager::addNetwork(const String& ssid, const String& password) {
    if (ssid.length() == 0) return false;
    
    Preferences preferences;
    preferences.begin("wifi", false);
    
    // Check if network already exists
    std::vector<String> savedNetworks = getSavedNetworks();
    for (const auto& savedSsid : savedNetworks) {
        if (savedSsid == ssid) {
            // Update password for existing network
            String key = "pwd_" + ssid;
            preferences.putString(key.c_str(), password);
            preferences.end();
            Logger::info("WIFI", "Updated password for network: %s", ssid.c_str());
            return true;
        }
    }
    
    // Check if we have space for new network
    if (savedNetworks.size() >= MAX_SAVED_NETWORKS) {
        preferences.end();
        Logger::warn("WIFI", "Maximum number of saved networks reached (%d)", MAX_SAVED_NETWORKS);
        return false;
    }
    
    // Add new network
    String networksKey = "networks";
    String currentNetworks = preferences.getString(networksKey.c_str(), "");
    
    if (currentNetworks.length() > 0) {
        currentNetworks += ",";
    }
    currentNetworks += ssid;
    
    preferences.putString(networksKey.c_str(), currentNetworks);
    
    String pwdKey = "pwd_" + ssid;
    preferences.putString(pwdKey.c_str(), password);
    
    preferences.end();
    Logger::info("WIFI", "Added new network: %s", ssid.c_str());
    return true;
}

bool WifiManager::removeNetwork(const String& ssid) {
    Preferences preferences;
    preferences.begin("wifi", false);
    
    String networksKey = "networks";
    String currentNetworks = preferences.getString(networksKey.c_str(), "");
    
    if (currentNetworks.length() == 0) {
        preferences.end();
        return false;
    }
    
    // Remove from networks list
    std::vector<String> networks;
    int start = 0;
    int end = currentNetworks.indexOf(',');
    while (end != -1) {
        String network = currentNetworks.substring(start, end);
        if (network != ssid) {
            networks.push_back(network);
        }
        start = end + 1;
        end = currentNetworks.indexOf(',', start);
    }
    // Handle last network
    String lastNetwork = currentNetworks.substring(start);
    if (lastNetwork != ssid && lastNetwork.length() > 0) {
        networks.push_back(lastNetwork);
    }
    
    // Rebuild networks string
    String newNetworks = "";
    for (size_t i = 0; i < networks.size(); i++) {
        if (i > 0) newNetworks += ",";
        newNetworks += networks[i];
    }
    
    preferences.putString(networksKey.c_str(), newNetworks);
    
    // Remove password
    String pwdKey = "pwd_" + ssid;
    preferences.remove(pwdKey.c_str());
    
    preferences.end();
    Logger::info("WIFI", "Removed network: %s", ssid.c_str());
    return true;
}

std::vector<String> WifiManager::getSavedNetworks() {
    std::vector<String> networks;
    Preferences preferences;
    preferences.begin("wifi", true);
    
    String networksStr = preferences.getString("networks", "");
    preferences.end();
    
    if (networksStr.length() == 0) return networks;
    
    int start = 0;
    int end = networksStr.indexOf(',');
    while (end != -1) {
        networks.push_back(networksStr.substring(start, end));
        start = end + 1;
        end = networksStr.indexOf(',', start);
    }
    // Handle last network
    String lastNetwork = networksStr.substring(start);
    if (lastNetwork.length() > 0) {
        networks.push_back(lastNetwork);
    }
    
    return networks;
}

bool WifiManager::connectToAvailableNetwork() {
    // Get available networks
    std::vector<String> availableNetworks = scanNetworks();
    std::vector<String> savedNetworks = getSavedNetworks();
    
    if (availableNetworks.empty() || savedNetworks.empty()) {
        return false;
    }
    
    // Try to connect to any saved network that's available
    for (const auto& savedNetwork : savedNetworks) {
        for (const auto& availableNetwork : availableNetworks) {
            if (savedNetwork == availableNetwork) {
                Logger::info("WIFI", "Found saved network: %s", savedNetwork.c_str());
                
                // Get password for this network
                Preferences preferences;
                preferences.begin("wifi", true);
                String pwdKey = "pwd_" + savedNetwork;
                String password = preferences.getString(pwdKey.c_str(), "");
                preferences.end();
                
                if (password.length() > 0 && connect(savedNetwork, password)) {
                    Logger::info("WIFI", "Successfully connected to: %s", savedNetwork.c_str());
                    return true;
                }
                break;
            }
        }
    }
    
    return false;
}

void WifiManager::startHotspot() {
    if (apMode) return;

    Logger::info("WIFI", "Starting AP hotspot");
    WiFi.softAP(HOTSPOT_SSID, HOTSPOT_PASSWORD);
    apMode = true;

    // Start DNS server for captive portal
    dnsServer = new DNSServer();
    dnsServer->start(WIFI_DNS_SERVER_PORT, "*", WiFi.softAPIP());

    // Start web server (routes will be registered by WebServerService)
    webServer = new WebServer(WIFI_WEB_SERVER_PORT);
    webServer->begin();

    Logger::info("WIFI", "Hotspot started. Connect to ESP32-Config and visit %s", WiFi.softAPIP().toString().c_str());

    Logger::info("WIFI", "Hotspot started. Connect to ESP32-Config and visit %s", WiFi.softAPIP().toString().c_str());
}

void WifiManager::stopHotspot() {
    if (!apMode) return;

    Logger::info("WIFI", "Stopping hotspot");
    if (webServer) {
        webServer->stop();
        delete webServer;
        webServer = nullptr;
    }
    if (dnsServer) {
        dnsServer->stop();
        delete dnsServer;
        dnsServer = nullptr;
    }
    WiFi.softAPdisconnect(true);
    apMode = false;
}

void WifiManager::handle() {
    if (apMode) {
        dnsServer->processNextRequest();
        // webServer->handleClient() is now handled by WebServerService
    } else if (!isConnected()) {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > WIFI_RECONNECT_INTERVAL) {
            Logger::info("WIFI", "Attempting to reconnect...");
            connectToAvailableNetwork();
            lastReconnectAttempt = now;
        }
    }
}


void WifiManager::onWiFiEvent(WiFiEvent_t event) {
    switch (event) {
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Logger::info("WIFI", "Connected to AP");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Logger::warn("WIFI", "Disconnected from AP");
            break;
        case ARDUINO_EVENT_WIFI_AP_START:
            Logger::info("WIFI", "AP started");
            break;
        case ARDUINO_EVENT_WIFI_AP_STOP:
            Logger::info("WIFI", "AP stopped");
            break;
        default:
            break;
    }
}