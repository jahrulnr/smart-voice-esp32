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
    , _maxTokens(300)
    , _temperature(0.7f)
    , _initialized(false)
    , _contextCache(10) // Keep last 10 messages
    , _previousResponseId("") // Initialize empty for first conversation
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

        http.begin(client, "https://api.openai.com/v1/responses");
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
    JsonDocument doc;

    doc["model"] = _model;
    doc["input"] = userPrompt;
    doc["instructions"] = _systemMessage;
    doc["max_output_tokens"] = _maxTokens;

    JsonObject reasoning = doc["reasoning"].to<JsonObject>();
    reasoning["effort"] = "low";

    // Include previous response ID for conversation continuity
    if (_previousResponseId.length() > 0) {
        doc["previous_response_id"] = _previousResponseId;
    }

    doc["store"] = false;

    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

void GPTService::processResponse(int httpCode, const String& response, const String& userPrompt, ResponseCallback callback) {
    if (httpCode != 200) {
        Logger::error("GPT", "API returned error code: %d", httpCode);
        Logger::debug("GPT", "Response: %s", response.c_str());

        // Try to extract error message from JSON
        JsonDocument errorDoc;
        if (deserializeJson(errorDoc, response) == DeserializationError::Ok) {
            if (errorDoc["error"].is<JsonObject>()) {
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
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, jsonResponse);
    if (error) {
        Logger::error("GPT", "JSON parse error: %s", error.c_str());
        return "";
    }

    // Extract and store response ID for conversation continuity
    if (doc["id"].is<String>()) {
        _previousResponseId = doc["id"].as<String>();
        Logger::debug("GPT", "Stored response ID: %s", _previousResponseId.c_str());
    }

    // Navigate to the response content (Responses API format)
    if (!doc["output"].is<JsonArray>() || doc["output"].size() == 0) {
        Logger::error("GPT", "No output in response. response: %s", doc.as<String>().c_str());
        return "";
    }

    JsonDocument outputItem;
    for(auto outputI : doc["output"].as<JsonArray>()){
        if(outputI["type"] == "message") {
            outputItem.set(outputI);
            Logger::info("GPT", "Found message in output item. message: %s", outputI.as<String>().c_str());
            break;
        }
    }

    if (!outputItem["content"].is<JsonArray>() || outputItem["content"].size() == 0) {
        Logger::error("GPT", "No content in output item. response: %s", doc.as<String>().c_str());
        return "";
    }

    JsonObject contentItem = outputItem["content"][0];
    if (!contentItem["text"].is<String>()) {
        Logger::error("GPT", "No text in content item. response: %s", doc.as<String>().c_str());
        return "";
    }

    String content = contentItem["text"];
    content.trim(); // Remove any leading/trailing whitespace
    if (content.length() == 0) {
        Logger::error("GPT", "Empty content in response. response: %s", doc.as<String>().c_str());
        return "";
    };

    return content;
}

std::vector<GPTModel> GPTService::getAvailableModels() {
    return std::vector<GPTModel>(AFFORDABLE_MODELS, AFFORDABLE_MODELS + NUM_AFFORDABLE_MODELS);
}

void GPTService::resetConversation() {
    _previousResponseId = "";
    _contextCache.clear();
    Logger::info("GPT", "Conversation state reset");
}

} // namespace Services