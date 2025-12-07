#include "web_server.h"
#include "network/wifi_manager.h"

WebServerService::WebServerService()
    : _server(nullptr)
    , _gptService(nullptr)
{
}

WebServerService::~WebServerService() {
    if (_server) {
        _server->stop();
        delete _server;
        _server = nullptr;
    }
}

bool WebServerService::init(WebServer* server, Services::GPTService* gptService) {
    _server = server;
    _gptService = gptService;

    if (!_server) {
        Logger::error("WEB", "WebServer instance is null");
        return false;
    }

    setupRoutes();
    Logger::info("WEB", "Web server service initialized");
    return true;
}

void WebServerService::setupRoutes() {
    if (!_server) return;

    // WiFi management routes
    _server->on("/", HTTP_GET, std::bind(&WebServerService::handleRoot, this));
    _server->on("/config", HTTP_POST, std::bind(&WebServerService::handleConfig, this));
    _server->on("/scan", HTTP_GET, std::bind(&WebServerService::handleScan, this));
    _server->on("/add", HTTP_POST, std::bind(&WebServerService::handleAddNetwork, this));
    _server->on("/remove", HTTP_POST, std::bind(&WebServerService::handleRemoveNetwork, this));
    _server->on("/networks", HTTP_GET, std::bind(&WebServerService::handleListNetworks, this));

    // GPT configuration routes
    _server->on("/gpt-config", HTTP_GET, std::bind(&WebServerService::handleGPTConfig, this));
    _server->on("/gpt-config", HTTP_POST, std::bind(&WebServerService::handleGPTConfigPost, this));

    // Static file serving
    _server->serveStatic("/css/", LittleFS, "/assets/css/");
    _server->serveStatic("/js/", LittleFS, "/assets/js/");

    Logger::info("WEB", "Web routes configured");
}

void WebServerService::handle() {
    if (_server) {
        _server->handleClient();
    }
}

void WebServerService::handleRoot() {
    String networkList = getSavedNetworksHtml();

    String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Voice Assistant - WiFi Setup</title>
    <link href="/css/bootstrap.min.css" rel="stylesheet">
    <style>
        body { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; }
        .card { border: none; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); }
        .btn-primary { background: linear-gradient(45deg, #667eea, #764ba2); border: none; }
        .btn-primary:hover { transform: translateY(-2px); box-shadow: 0 5px 15px rgba(0,0,0,0.2); }
        .form-control:focus { border-color: #667eea; box-shadow: 0 0 0 0.2rem rgba(102, 126, 234, 0.25); }
        .network-item { background: rgba(255,255,255,0.1); padding: 10px; border-radius: 8px; margin-bottom: 8px; }
    </style>
</head>
<body>
    <div class="container d-flex align-items-center justify-content-center min-vh-100">
        <div class="card col-md-8 col-lg-6">
            <div class="card-body p-5">
                <div class="text-center mb-4">
                    <h2 class="card-title text-primary mb-2">🎙️ ESP32 Voice Assistant</h2>
                    <p class="text-muted">WiFi Network Management</p>
                </div>

                <div class="mb-4">
                    <h5 class="mb-3">Saved Networks</h5>
                    <div class="network-item">
)" + networkList + R"(
                    </div>
                </div>

                <div class="mb-4">
                    <h5 class="mb-3">Add New Network</h5>
                    <form action="/config" method="POST">
                        <div class="mb-3">
                            <label for="ssid" class="form-label fw-bold">WiFi Network</label>
                            <input type="text" class="form-control form-control-lg" id="ssid" name="ssid"
                                   placeholder="Enter network name" required>
                        </div>

                        <div class="mb-3">
                            <label for="password" class="form-label fw-bold">Password</label>
                            <input type="password" class="form-control form-control-lg" id="password" name="password"
                                   placeholder="Enter password">
                        </div>

                        <button type="submit" class="btn btn-primary btn-lg w-100">
                            <i class="me-2">📡</i>Add Network
                        </button>
                    </form>
                </div>

                <div class="text-center">
                    <a href="/scan" class="btn btn-outline-secondary me-2">
                        <i class="me-2">🔍</i>Scan Networks
                    </a>
                    <a href="/gpt-config" class="btn btn-outline-primary">
                        <i class="me-2">🤖</i>GPT Settings
                    </a>
                </div>

                <div class="text-center mt-4">
                    <small class="text-muted">
                        Connect to <strong>)" + String(HOTSPOT_SSID) + R"(</strong> network
                    </small>
                </div>
            </div>
        </div>
    </div>

    <script src="/js/bootstrap.bundle.js"></script>
</body>
</html>
)";
    _server->send(200, "text/html", html);
}

void WebServerService::handleConfig() {
    extern WifiManager wifiManager;

    String ssid = _server->arg("ssid");
    String password = _server->arg("password");

    if (ssid.length() > 0) {
        if (wifiManager.addNetwork(ssid, password)) {
            Logger::info("WEB", "New network added: %s", ssid.c_str());

            String response = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Network Added - ESP32 Setup</title>
    <link href="/css/bootstrap.min.css" rel="stylesheet">
    <style>
        body { background: linear-gradient(135deg, #28a745 0%, #20c997 100%); min-height: 100vh; }
        .card { border: none; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); }
        .success-icon { font-size: 4rem; color: #28a745; }
    </style>
</head>
<body>
    <div class="container d-flex align-items-center justify-content-center min-vh-100">
        <div class="card col-md-6 col-lg-5">
            <div class="card-body p-5 text-center">
                <div class="success-icon mb-4">✅</div>
                <h2 class="card-title text-success mb-3">Network Added!</h2>
                <p class="text-muted mb-4">Network <strong>)HTML" + ssid + R"HTML(</strong> has been saved.</p>
                <div class="d-grid gap-2">
                    <a href="/" class="btn btn-success">Add Another Network</a>
                    <a href="/networks" class="btn btn-outline-secondary">View Saved Networks</a>
                </div>
            </div>
        </div>
    </div>
    <script src="/js/bootstrap.bundle.js"></script>
</body>
</html>
)HTML";
            _server->send(200, "text/html", response);
        } else {
            _server->send(400, "text/plain", "Failed to add network (storage full or invalid)");
        }
    } else {
        _server->send(400, "text/plain", "Invalid SSID");
    }
}

void WebServerService::handleScan() {
    extern WifiManager wifiManager;

    std::vector<String> networks = wifiManager.scanNetworks();
    String networkList = generateNetworkScanHtml(networks);

    String html = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Available Networks - ESP32 Setup</title>
    <link href="/css/bootstrap.min.css" rel="stylesheet">
    <style>
        body { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; }
        .card { border: none; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); }
        .btn-primary { background: linear-gradient(45deg, #667eea, #764ba2); border: none; }
    </style>
</head>
<body>
    <div class="container d-flex align-items-center justify-content-center min-vh-100">
        <div class="card col-md-8 col-lg-6">
            <div class="card-body p-5">
                <div class="text-center mb-4">
                    <h2 class="card-title text-primary mb-2">📡 Available Networks</h2>
                    <p class="text-muted">Select your WiFi network</p>
                </div>

                <div class="mb-4">
                    <ul class="list-group list-group-flush">
)HTML" + networkList + R"HTML(
                    </ul>
                </div>

                <div class="text-center">
                    <a href="/" class="btn btn-outline-secondary me-2">
                        <i class="me-2">⬅️</i>Back to Setup
                    </a>
                    <button onclick="location.reload()" class="btn btn-primary">
                        <i class="me-2">🔄</i>Scan Again
                    </button>
                </div>
            </div>
        </div>
    </div>

    <script src="/js/bootstrap.bundle.js"></script>
</body>
</html>
)HTML";
    _server->send(200, "text/html", html);
}

void WebServerService::handleAddNetwork() {
    extern WifiManager wifiManager;

    String ssid = _server->arg("ssid");
    String password = _server->arg("password");

    if (ssid.length() > 0) {
        if (wifiManager.addNetwork(ssid, password)) {
            Logger::info("WEB", "Network added via /add: %s", ssid.c_str());
            _server->send(200, "text/plain", "Network added successfully");
        } else {
            _server->send(400, "text/plain", "Failed to add network");
        }
    } else {
        _server->send(400, "text/plain", "Invalid SSID");
    }
}

void WebServerService::handleRemoveNetwork() {
    extern WifiManager wifiManager;

    String ssid = _server->arg("ssid");

    if (ssid.length() > 0) {
        if (wifiManager.removeNetwork(ssid)) {
            Logger::info("WEB", "Network removed: %s", ssid.c_str());

            String response = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Network Removed - ESP32 Setup</title>
    <link href="/css/bootstrap.min.css" rel="stylesheet">
    <style>
        body { background: linear-gradient(135deg, #dc3545 0%, #fd7e14 100%); min-height: 100vh; }
        .card { border: none; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); }
        .success-icon { font-size: 4rem; color: #dc3545; }
    </style>
</head>
<body>
    <div class="container d-flex align-items-center justify-content-center min-vh-100">
        <div class="card col-md-6 col-lg-5">
            <div class="card-body p-5 text-center">
                <div class="success-icon mb-4">🗑️</div>
                <h2 class="card-title text-danger mb-3">Network Removed!</h2>
                <p class="text-muted mb-4">Network <strong>)HTML" + ssid + R"HTML(</strong> has been removed.</p>
                <div class="d-grid gap-2">
                    <a href="/" class="btn btn-danger">Back to Networks</a>
                    <a href="/scan" class="btn btn-outline-secondary">Scan Networks</a>
                </div>
            </div>
        </div>
    </div>
    <script src="/js/bootstrap.bundle.js"></script>
</body>
</html>
)HTML";
            _server->send(200, "text/html", response);
        } else {
            _server->send(400, "text/plain", "Failed to remove network");
        }
    } else {
        _server->send(400, "text/plain", "Invalid SSID");
    }
}

void WebServerService::handleListNetworks() {
    extern WifiManager wifiManager;

    auto savedNetworks = wifiManager.getSavedNetworks();
    String json = "[";

    for (size_t i = 0; i < savedNetworks.size(); i++) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"" + savedNetworks[i] + "\"}";
    }
    json += "]";

    _server->send(200, "application/json", json);
}

void WebServerService::handleGPTConfig() {
    String html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GPT Configuration - ESP32 Setup</title>
    <link href="/css/bootstrap.min.css" rel="stylesheet">
    <style>
        body { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; }
        .card { border: none; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); }
        .btn-primary { background: linear-gradient(45deg, #667eea, #764ba2); border: none; }
        .btn-primary:hover { transform: translateY(-2px); box-shadow: 0 5px 15px rgba(0,0,0,0.2); }
        .form-control:focus { border-color: #667eea; box-shadow: 0 0 0 0.2rem rgba(102, 126, 234, 0.25); }
    </style>
</head>
<body>
    <div class="container d-flex align-items-center justify-content-center min-vh-100">
        <div class="card col-md-8 col-lg-6">
            <div class="card-body p-5">
                <div class="text-center mb-4">
                    <h2 class="card-title text-primary mb-2">🤖 GPT Configuration</h2>
                    <p class="text-muted">Configure OpenAI GPT integration</p>
                </div>

                <form action="/gpt-config" method="POST">
                    <div class="mb-3">
                        <label for="apiKey" class="form-label fw-bold">OpenAI API Key</label>
                        <input type="password" class="form-control form-control-lg" id="apiKey" name="apiKey"
                               placeholder="sk-..." required>
                        <div class="form-text">Get your API key from <a href="https://platform.openai.com/api-keys" target="_blank">OpenAI Platform</a></div>
                    </div>

                    <div class="mb-3">
                        <label for="model" class="form-label fw-bold">GPT Model</label>
                        <select class="form-control form-control-lg" id="model" name="model">
)HTML" + generateGPTModelOptions() + R"HTML(
                        </select>
                        <div class="form-text">Pricing: Input/Output tokens per 1M tokens (sorted by cost)</div>
                    </div>

                    <div class="mb-3">
                        <label for="maxTokens" class="form-label fw-bold">Max Response Length</label>
                        <input type="number" class="form-control form-control-lg" id="maxTokens" name="maxTokens"
                               value="150" min="50" max="500">
                        <div class="form-text">Keep low to save memory (50-500 tokens)</div>
                    </div>

                    <div class="mb-4">
                        <label for="temperature" class="form-label fw-bold">Creativity Level</label>
                        <input type="range" class="form-range" id="temperature" name="temperature"
                               min="0" max="1" step="0.1" value="0.7">
                        <div class="d-flex justify-content-between">
                            <small class="text-muted">Focused</small>
                            <small class="text-muted">Creative</small>
                        </div>
                    </div>

                    <button type="submit" class="btn btn-primary btn-lg w-100 mb-3">
                        <i class="me-2">💾</i>Save GPT Settings
                    </button>
                </form>

                <div class="text-center">
                    <a href="/" class="btn btn-outline-secondary">
                        <i class="me-2">⬅️</i>Back to WiFi Settings
                    </a>
                </div>
            </div>
        </div>
    </div>

    <script src="/js/bootstrap.bundle.js"></script>
</body>
</html>
)";
    _server->send(200, "text/html", html);
}

void WebServerService::handleGPTConfigPost() {
    String apiKey = _server->arg("apiKey");
    String model = _server->arg("model");
    String maxTokensStr = _server->arg("maxTokens");
    String temperatureStr = _server->arg("temperature");

    // Basic validation
    if (apiKey.length() < 20) {
        _server->send(400, "text/plain", "Invalid API key");
        return;
    }

    // Configure GPT service
    if (_gptService) {
        _gptService->setModel(model);
        _gptService->setMaxTokens(maxTokensStr.toInt());
        _gptService->setTemperature(temperatureStr.toFloat());

        // Try to initialize with the API key
        if (_gptService->init(apiKey)) {
            Logger::info("WEB", "GPT service configured successfully");
        } else {
            Logger::warn("WEB", "GPT service configuration failed");
        }
    }

    String response = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GPT Settings Saved - ESP32 Setup</title>
    <link href="/css/bootstrap.min.css" rel="stylesheet">
    <style>
        body { background: linear-gradient(135deg, #28a745 0%, #20c997 100%); min-height: 100vh; }
        .card { border: none; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.1); }
        .success-icon { font-size: 4rem; color: #28a745; }
    </style>
</head>
<body>
    <div class="container d-flex align-items-center justify-content-center min-vh-100">
        <div class="card col-md-6 col-lg-5">
            <div class="card-body p-5 text-center">
                <div class="success-icon mb-4">🤖</div>
                <h2 class="card-title text-success mb-3">GPT Configured!</h2>
                <p class="text-muted mb-4">GPT integration has been set up.</p>
                <div class="d-grid gap-2">
                    <a href="/" class="btn btn-success">Back to Main</a>
                    <a href="/gpt-config" class="btn btn-outline-secondary">Modify Settings</a>
                </div>
            </div>
        </div>
    </div>
    <script src="/js/bootstrap.bundle.js"></script>
</body>
</html>
)HTML";
    _server->send(200, "text/html", response);
}

// Helper methods
String WebServerService::getSavedNetworksHtml() {
    extern WifiManager wifiManager;
    auto savedNetworks = wifiManager.getSavedNetworks();

    if (savedNetworks.empty()) {
        return "<p class='text-muted text-center'>No saved networks yet.</p>";
    }

    String html = "";
    for (size_t i = 0; i < savedNetworks.size(); i++) {
        html += "<div class='d-flex justify-content-between align-items-center mb-2'>";
        html += "<span class='fw-bold'>" + savedNetworks[i] + "</span>";
        html += "<form action='/remove' method='POST' class='d-inline'>";
        html += "<input type='hidden' name='ssid' value='" + savedNetworks[i] + "'>";
        html += "<button type='submit' class='btn btn-outline-danger btn-sm'>";
        html += "<i class='me-1'>🗑️</i>Remove</button></form></div>";
    }

    return html;
}

String WebServerService::generateNetworkScanHtml(const std::vector<String>& networks) {
    if (networks.empty()) {
        return "<li class='list-group-item text-center text-muted'>No networks found</li>";
    }

    String html = "";
    for (size_t i = 0; i < networks.size(); i++) {
        html += "<li class='list-group-item d-flex justify-content-between align-items-center'>";
        html += networks[i];
        html += "<form action='/config' method='POST' class='d-inline'>";
        html += "<input type='hidden' name='ssid' value='" + networks[i] + "'>";
        html += "<input type='password' class='form-control form-control-sm d-inline-block w-auto me-2' name='password' placeholder='Password'>";
        html += "<button type='submit' class='btn btn-primary btn-sm'>Connect</button>";
        html += "</form></li>";
    }

    return html;
}

String WebServerService::generateGPTModelOptions() {
    String html = "";
    if (_gptService) {
        auto models = _gptService->getAvailableModels();
        for (const auto& model : models) {
            html += "<option value='" + String(model.id) + "'>" + String(model.displayName) + "</option>";
        }
    }
    return html;
}