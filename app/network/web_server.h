#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WebServer.h>
#include <LittleFS.h>
#include "infrastructure/logger.h"
#include "application/gpt_service.h"

class WebServerService {
public:
    WebServerService();
    ~WebServerService();

    /**
     * Initialize web server service
     * @param server Pointer to WebServer instance (managed by WifiManager)
     * @param gptService Reference to GPT service for configuration
     * @return true if initialization successful
     */
    bool init(WebServer* server, Services::GPTService* gptService);

    /**
     * Setup web routes
     */
    void setupRoutes();

    /**
     * Handle web server requests (call in main loop)
     */
    void handle();

private:
    WebServer* _server;
    Services::GPTService* _gptService;

    // Web handlers
    void handleRoot();
    void handleConfig();
    void handleScan();
    void handleAddNetwork();
    void handleRemoveNetwork();
    void handleListNetworks();
    void handleGPTConfig();
    void handleGPTConfigPost();

    // Helper methods
    String getSavedNetworksHtml();
    String generateNetworkScanHtml(const std::vector<String>& networks);
    String generateGPTModelOptions();
};

#endif // WEB_SERVER_H