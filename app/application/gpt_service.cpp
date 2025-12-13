#include "gpt_service.h"
#include "infrastructure/logger.h"
#include <WiFiClientSecure.h>
#include <WiFi.h>

namespace Services {

// Affordable GPT models sorted by cost (cheapest first)
static const GPTModel AFFORDABLE_MODELS[] = {
    {"gpt-5-nano", "GPT-5 Nano (Cheapest - $0.05/$0.40)", "$0.05/$0.40"},
    {"gpt-4.1-nano", "GPT-4.1 Nano ($0.10/$0.40)", "$0.10/$0.40"},
    {"gpt-4o-mini", "GPT-4o Mini (Balanced - $0.15/$0.60)", "$0.15/$0.60"},
    {"gpt-5-mini", "GPT-5 Mini ($0.25/$2.00)", "$0.25/$2.00"},
    {"gpt-4.1-mini", "GPT-4.1 Mini ($0.40/$1.60)", "$0.40/$1.60"},
    {"o3-mini", "O3 Mini ($1.10/$4.40)", "$1.10/$4.40"},
    {"o1-mini", "O1 Mini ($1.10/$4.40)", "$1.10/$4.40"}
};

static const size_t NUM_AFFORDABLE_MODELS = sizeof(AFFORDABLE_MODELS) / sizeof(AFFORDABLE_MODELS[0]);

GPTService::GPTService()
    : _model("gpt-5-nano")
    , _systemMessage("You are a helpful voice assistant on an ESP32 device. Keep responses concise and suitable for text-to-speech conversion.")
    , _maxTokens(150)
    , _temperature(0.7f)
    , _initialized(false)
    , _contextCache(10) // Keep last 10 messages
{
}

GPTService::~GPTService() {
    // Clean up if needed
}

bool GPTService::init(const String& apiKey) {
    if (apiKey.length() == 0) {
        Logger::error("GPT", "API key is empty");
        return false;
    }

    _apiKey = apiKey;
    _initialized = true;

    Logger::info("GPT", "GPT service initialized with model: %s", _model.c_str());
    return true;
}

void GPTService::sendPrompt(const String& prompt, ResponseCallback callback) {
    sendPromptWithContext(prompt, {}, callback);
}

void GPTService::sendPrompt(const String& prompt, const String& additionalContext, ResponseCallback callback) {
    std::vector<std::pair<String, String>> contextMessages;
    if (additionalContext.length() > 0) {
        contextMessages.push_back({"system", additionalContext});
    }
    sendPromptWithContext(prompt, contextMessages, callback);
}

void GPTService::sendPromptWithContext(const String& prompt,
                                      const std::vector<std::pair<String, String>>& contextMessages,
                                      ResponseCallback callback) {
    if (!_initialized) {
        Logger::error("GPT", "GPT service not initialized");
        callback("Error: GPT service not initialized");
        return;
    }

    if (!WiFi.isConnected()) {
        Logger::error("GPT", "No WiFi connection");
        callback("Error: No internet connection");
        return;
    }

    // Add user message to context cache
    _contextCache.addMessage("user", prompt);

    // Build JSON payload
    String jsonPayload = buildJsonPayload(prompt, contextMessages);

    // Create async task for HTTP request (since GPT API calls are slow)
    xTaskCreate([](void* param) {
        auto* params = static_cast<std::tuple<GPTService*, String, ResponseCallback>*>(param);
        auto& [service, payload, cb] = *params;

        HTTPClient http;
        WiFiClientSecure client;
        client.setInsecure(); // For HTTPS without certificate validation

        http.begin(client, "https://api.openai.com/v1/chat/completions");
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", "Bearer " + service->_apiKey);
        http.setTimeout(30000); // 30 second timeout

        Logger::info("GPT", "Sending request to OpenAI API...");

        int httpCode = http.POST(payload);

        if (httpCode > 0) {
            String response = http.getString();
            Logger::info("GPT", "API response received, code: %d", httpCode);
            service->processResponse(httpCode, response, payload, cb);
        } else {
            Logger::error("GPT", "HTTP request failed, error: %d", httpCode);
            cb("Error: Failed to connect to GPT API");
        }

        http.end();
        delete params;
        vTaskDelete(NULL);
    }, "GPT_Request", 8192, new std::tuple<GPTService*, String, ResponseCallback>(this, jsonPayload, callback), 1, NULL);
}

String GPTService::buildJsonPayload(const String& userPrompt, const std::vector<std::pair<String, String>>& contextMessages) {
    // Get recent conversation history
    auto recentMessages = _contextCache.getRecentMessages(8); // Last 8 messages to save tokens

    // Calculate required JSON size (conservative estimate)
    const size_t capacity = JSON_OBJECT_SIZE(5) + // root object
                           JSON_ARRAY_SIZE(10) +  // messages array (up to 10 messages)
                           JSON_OBJECT_SIZE(3) * 10 + // each message object
                           userPrompt.length() * 2 + _systemMessage.length() * 2 + 1024; // string content

    DynamicJsonDocument doc(capacity);

    doc["model"] = _model;
    doc["max_completion_tokens"] = _maxTokens;
    // doc["temperature"] = _temperature;

    JsonArray messages = doc.createNestedArray("messages");

    // Add system message
    JsonObject systemMsg = messages.createNestedObject();
    systemMsg["role"] = "system";
    systemMsg["content"] = _systemMessage;

    // Add context messages
    for (const auto& ctx : contextMessages) {
        JsonObject ctxMsg = messages.createNestedObject();
        ctxMsg["role"] = ctx.first;
        ctxMsg["content"] = ctx.second;
    }

    // Add conversation history (excluding the current user message to avoid duplication)
    for (const auto& msg : recentMessages) {
        JsonObject histMsg = messages.createNestedObject();
        histMsg["role"] = msg.role;
        histMsg["content"] = msg.content;
    }

    // Add current user message
    JsonObject userMsg = messages.createNestedObject();
    userMsg["role"] = "user";
    userMsg["content"] = userPrompt;

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

void GPTService::processResponse(int httpCode, const String& response, const String& userPrompt, ResponseCallback callback) {
    if (httpCode != 200) {
        Logger::error("GPT", "API returned error code: %d", httpCode);
        Logger::debug("GPT", "Response: %s", response.c_str());

        // Try to extract error message from JSON
        DynamicJsonDocument errorDoc(1024);
        if (deserializeJson(errorDoc, response) == DeserializationError::Ok) {
            if (errorDoc.containsKey("error")) {
                String errorMsg = errorDoc["error"]["message"] | "Unknown API error";
                callback("Error: " + errorMsg);
                return;
            }
        }

        callback("Error: GPT API returned code " + String(httpCode));
        return;
    }

    // Parse successful response
    String gptResponse = extractResponse(response);
    if (gptResponse.length() > 0) {
        // Add assistant response to context cache
        _contextCache.addMessage("assistant", gptResponse);

        Logger::info("GPT", "Response generated successfully (%d chars)", gptResponse.length());
        callback(gptResponse);
    } else {
        Logger::error("GPT", "Failed to extract response from API reply");
        callback("Error: Could not parse GPT response");
    }
}

String GPTService::extractResponse(const String& jsonResponse) {
    DynamicJsonDocument doc(4096); // ESP32 memory limit - keep small

    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error) {
        Logger::error("GPT", "JSON parse error: %s", error.c_str());
        return "";
    }

    // Navigate to the response content
    if (!doc.containsKey("choices") || doc["choices"].size() == 0) {
        Logger::error("GPT", "No choices in response");
        return "";
    }

    JsonObject choice = doc["choices"][0];
    if (!choice.containsKey("message") || !choice["message"].containsKey("content")) {
        Logger::error("GPT", "No content in response");
        return "";
    }

    String content = choice["message"]["content"];
    content.trim(); // Remove any leading/trailing whitespace
    if (content.length() == 0) {
        Logger::error("GPT", "Empty content in response. response: %s", jsonResponse.c_str());
        return "";
    };

    return content;
}

std::vector<GPTModel> GPTService::getAvailableModels() {
    return std::vector<GPTModel>(AFFORDABLE_MODELS, AFFORDABLE_MODELS + NUM_AFFORDABLE_MODELS);
}

} // namespace Services