# Network Infrastructure 🌐

This folder contains low-level network infrastructure components for connectivity, hardware management, and transport layer protocols.

## 📋 Network Components

### WiFi Manager (`wifi_manager.h/.cpp`)
**Purpose**: Core WiFi connectivity management, hardware control, and network infrastructure.

**Key Features**:
- **WiFi Hardware Management**: ESP32 WiFi stack initialization and control
- **Multi-Network Storage**: Save up to 5 WiFi networks using ESP32 Preferences NVS
- **Auto-Roaming**: Automatically connects to the first available saved network
- **AP Hotspot Mode**: Configuration portal when no networks are available
- **DNS Server**: Captive portal for initial setup
- **Web Server Provisioning**: Creates WebServer instance for application services
- **Persistent Storage**: Networks survive reboots and firmware updates

**API Methods**:
```cpp
// Core lifecycle
bool init();                          // Initialize WiFi stack
void begin();                         // Start connection process
void handle();                        // Periodic tasks (reconnect, DNS processing)

// Network management
bool addNetwork(String ssid, String password);    // Add WiFi network
bool removeNetwork(String ssid);                  // Remove WiFi network
std::vector<String> getSavedNetworks();           // Get saved network list

// Connection management
bool connect(const String& ssid, const String& password);  // Connect to specific network
bool connectToAvailableNetwork();               // Auto-connect to best available
bool isConnected();                             // Check connection status
String getIPAddress();                          // Get current IP address

// Hotspot mode
void startHotspot();                            // Start AP configuration mode
void stopHotspot();                             // Stop AP mode

// Infrastructure
WebServer* getWebServer();                      // Get WebServer instance for services
std::vector<String> scanNetworks();             // Scan available WiFi networks
```

**Configuration**:
```cpp
#define HOTSPOT_SSID "ESP32-Config"
#define HOTSPOT_PASSWORD "password123"
#define WIFI_WEB_SERVER_PORT 80
#define WIFI_DNS_SERVER_PORT 53
#define WIFI_RECONNECT_INTERVAL 30000  // 30 seconds
```

**Usage Example**:
```cpp
#include "network/wifi_manager.h"

WifiManager wifiManager;

// Initialize WiFi infrastructure
if (!wifiManager.init()) {
    Logger::error("WIFI", "Failed to initialize WiFi");
    return;
}

// Start connection process
wifiManager.begin();

// Get WebServer instance for application services
WebServer* server = wifiManager.getWebServer();
if (server) {
    // Application services can now register routes
    webServerService.init(server, &gptService);
}

// Handle periodic tasks
wifiManager.handle();
```

### Network Architecture

**Infrastructure Layer** (This folder):
- WiFi hardware abstraction and connectivity
- Network protocol stack management
- DNS server for captive portal
- WebServer instance creation and management

**Application Layer** (`app/services/`):
- Web interface and user interaction
- FTP file transfer service
- GPT AI integration
- Business logic using network infrastructure

### Dependencies

**Required Libraries**:
- `WiFi.h` - ESP32 WiFi stack
- `WiFiAP.h` - Access point functionality
- `DNSServer.h` - DNS server for captive portal
- `WebServer.h` - HTTP server infrastructure
- `Preferences.h` - NVS storage for network credentials

**Integration Points**:
- **WebServerService**: Uses `getWebServer()` to register application routes
- **TaskScheduler**: Calls `handle()` in main loop for reconnection logic
- **Main.cpp**: Initializes WiFi infrastructure before application services

### Error Handling

**Common Issues**:
- **WiFi Connection Failures**: Check credentials, signal strength, network compatibility
- **Hotspot Not Starting**: Verify AP configuration, check for IP conflicts
- **DNS Resolution**: Ensure DNS server is properly initialized in AP mode

**Debug Information**:
```cpp
// Enable WiFi event logging
WiFi.onEvent([](WiFiEvent_t event) {
    Logger::info("WIFI", "Event: %d", event);
});
```

### Performance Considerations

**Memory Usage**:
- WiFi stack: ~20KB RAM
- Network storage: ~1KB NVS
- WebServer instance: ~8KB RAM

**Timing Constraints**:
- Connection timeout: 20 seconds
- Reconnection interval: 30 seconds
- DNS processing: Called in main loop

This infrastructure provides the foundation for all network-dependent services in the ESP32 voice assistant.