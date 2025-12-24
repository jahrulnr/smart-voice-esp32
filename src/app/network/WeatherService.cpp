#include "WeatherService.h"
#include <HTTPClient.h>
#include <WiFi.h>

namespace Services {

const char* WeatherService::CACHE_FILE_PATH = "/cache/weather_cache.json";

WeatherService::WeatherService()
    : _lastCacheTime(0), _initialized(false) {
}

WeatherService::~WeatherService() {
}

bool WeatherService::init(const WeatherConfig& config) {
    _config = config;
    _initialized = true;

    // Try to load existing cache
    loadCache();

    ESP_LOGI("Weather", "Weather service initialized");
    return true;
}

void WeatherService::getCurrentWeather(WeatherCallback callback, bool forceRefresh) {
    if (!_initialized) {
        if (callback) {
            WeatherData errorData;
            callback(errorData, false);
        }
        return;
    }

    // Check if we should use cache
    if (!forceRefresh && isCacheValid()) {
        if (callback) {
            callback(_cachedData, true);
        }
        return;
    }

    // Fetch from API
    fetchFromAPI(callback);
}

void WeatherService::setLocation(const String& adm4Code) {
    _config.adm4Code = adm4Code;
    clearCache();
}

void WeatherService::setCacheExpiry(uint32_t minutes) {
    _config.cacheExpiryMinutes = minutes;
}

void WeatherService::clearCache() {
    _cachedData = WeatherData();
    _lastCacheTime = 0;

    if (LittleFS.exists(CACHE_FILE_PATH)) {
        LittleFS.remove(CACHE_FILE_PATH);
    }
}

bool WeatherService::isCacheValid() const {
    if (_lastCacheTime == 0 || !_cachedData.isValid) {
        return false;
    }

    unsigned long currentTime = getCurrentTimestamp();
    unsigned long cacheExpiryMs = _config.cacheExpiryMinutes * 60 * 1000;

    return (currentTime - _lastCacheTime) < cacheExpiryMs;
}

void WeatherService::fetchFromAPI(WeatherCallback callback) {
    HTTPClient http;
    String url = buildAPIUrl();

    http.begin(url);
    http.setReuse(true);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.setTimeout(10000); // 10 second timeout

    ESP_LOGI("Weather", "Fetching weather from: %s", url.c_str());

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        processAPIResponse(response, callback);
    } else {
        ESP_LOGE("Weather", "HTTP error: %d", httpCode);
        if (callback) {
            WeatherData errorData;
            callback(errorData, false);
        }
    }

    http.end();
}

void WeatherService::processAPIResponse(const String& response, WeatherCallback callback) {
    if (!callback) {
        ESP_LOGW("Weather", "No callback provided");
        return;
    }

    ESP_LOGI("Weather", "Processing API response");

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, response);

    if (error) {
        ESP_LOGE("Weather", "JSON parsing failed: %s", error.c_str());
        WeatherData errorData;
        callback(errorData, false);
        return;
    }

    WeatherData data;

    try {
        if (doc["lokasi"].isUnbound()) {
            ESP_LOGE("Weather", "No lokasi found in response");
            callback(data, false);
            return;
        }

        JsonObject lokasi = doc["lokasi"];
        data.location = String(lokasi["provinsi"]) + ", " +
                       String(lokasi["kotkab"]) + ", " +
                       String(lokasi["kecamatan"]) + ", " +
                       String(lokasi["desa"]);
        data.longitude = lokasi["lon"];
        data.latitude = lokasi["lat"];
        data.timezone = String(lokasi["timezone"]);

        ESP_LOGI("Weather", "Location: %s (Lat: %.6f, Lon: %.6f)", data.location.c_str(), data.latitude, data.longitude);

        if (doc["data"].isUnbound() || doc["data"].size() == 0) {
            ESP_LOGE("Weather", "No data array found");
            callback(data, false);
            return;
        }

        JsonArray dataArray = doc["data"];
        if (dataArray.size() == 0) {
            ESP_LOGE("Weather", "Empty data array");
            callback(data, false);
            return;
        }

        JsonObject locationData = dataArray[0];
        if (locationData["cuaca"].isUnbound() || locationData["cuaca"].size() == 0) {
            ESP_LOGE("Weather", "No cuaca data found");
            callback(data, false);
            return;
        }

        JsonArray cuacaArray = locationData["cuaca"];
        if (cuacaArray.size() == 0) {
            ESP_LOGE("Weather", "No current weather data");
            callback(data, false);
            return;
        }

        JsonArray timePeriod = cuacaArray[0];
        if (timePeriod.size() == 0) {
            ESP_LOGE("Weather", "No weather data in time period");
            callback(data, false);
            return;
        }

        JsonObject currentWeather = timePeriod[0];

        data.temperature = currentWeather["t"] | 0;
        data.humidity = currentWeather["hu"] | 0;
        data.windSpeed = (int)((currentWeather["ws"] | 0.0) * 3.6); // m/s to km/h
        data.windDirection = String(currentWeather["wd"] | "");
        data.description = String(currentWeather["weather_desc"] | "");
        data.imageUrl = String(currentWeather["image"] | "");
        data.lastUpdated = String(currentWeather["local_datetime"] | "");

        int weatherCode = currentWeather["weather"] | 0;
        data.condition = getConditionFromCode(weatherCode);

        ESP_LOGI("Weather", "Weather: %s (Code: %d)", data.description.c_str(), weatherCode);
        ESP_LOGI("Weather", "Temp: %d°C, Humidity: %d%%, Wind: %d km/h %s",
                    data.temperature, data.humidity, data.windSpeed, data.windDirection.c_str());

        data.isValid = true;

        // Cache the data
        _cachedData = data;
        _lastCacheTime = getCurrentTimestamp();
        saveCache(data);

        callback(data, true);

    } catch (...) {
        ESP_LOGE("Weather", "Exception during processing");
        WeatherData errorData;
        callback(errorData, false);
    }
}

bool WeatherService::loadCache() {
    if (!LittleFS.exists(CACHE_FILE_PATH)) {
        return false;
    }

    File file = LittleFS.open(CACHE_FILE_PATH, "r");
    if (!file) {
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        return false;
    }

    _cachedData.location = String(doc["location"]);
    _cachedData.description = String(doc["description"]);
    _cachedData.condition = static_cast<WeatherCondition>((int)doc["condition"]);
    _cachedData.temperature = doc["temperature"];
    _cachedData.humidity = doc["humidity"];
    _cachedData.windSpeed = doc["windSpeed"];
    _cachedData.windDirection = String(doc["windDirection"]);
    _cachedData.lastUpdated = String(doc["lastUpdated"]);
    _cachedData.imageUrl = String(doc["imageUrl"]);
    _cachedData.longitude = doc["longitude"];
    _cachedData.latitude = doc["latitude"];
    _cachedData.timezone = String(doc["timezone"]);
    _cachedData.isValid = doc["isValid"];
    _lastCacheTime = doc["cacheTime"];

    return true;
}

bool WeatherService::saveCache(const WeatherData& data) {
    JsonDocument doc;

    doc["location"] = data.location;
    doc["description"] = data.description;
    doc["condition"] = static_cast<int>(data.condition);
    doc["temperature"] = data.temperature;
    doc["humidity"] = data.humidity;
    doc["windSpeed"] = data.windSpeed;
    doc["windDirection"] = data.windDirection;
    doc["lastUpdated"] = data.lastUpdated;
    doc["imageUrl"] = data.imageUrl;
    doc["longitude"] = data.longitude;
    doc["latitude"] = data.latitude;
    doc["timezone"] = data.timezone;
    doc["isValid"] = data.isValid;
    doc["cacheTime"] = getCurrentTimestamp();

    File file = LittleFS.open(CACHE_FILE_PATH, "w");
    if (!file) {
        ESP_LOGE("Weather", "Failed to open cache file for writing");
        return false;
    }

    serializeJson(doc, file);
    file.close();

    ESP_LOGD("Weather", "Weather data cached");
    return true;
}

String WeatherService::buildAPIUrl() const {
    String url = "https://api.bmkg.go.id/publik/prakiraan-cuaca?adm4=";
    url += _config.adm4Code;
    return url;
}

unsigned long WeatherService::getCurrentTimestamp() const {
    return millis();
}

WeatherService::WeatherCondition WeatherService::getConditionFromCode(int weatherCode) {
    switch (weatherCode) {
        case 0: return WeatherCondition::CLEAR;
        case 1:
        case 2: return WeatherCondition::PARTLY_CLOUDY;
        case 3: return WeatherCondition::CLOUDY;
        case 4: return WeatherCondition::OVERCAST;
        case 60:
        case 61: return WeatherCondition::LIGHT_RAIN;
        case 63: return WeatherCondition::MODERATE_RAIN;
        case 65: return WeatherCondition::HEAVY_RAIN;
        case 95:
        case 97: return WeatherCondition::THUNDERSTORM;
        case 45:
        case 48: return WeatherCondition::FOG;
        default: return WeatherCondition::UNKNOWN;
    }
}

String WeatherService::conditionToString(WeatherCondition condition) {
    switch (condition) {
        case WeatherCondition::CLEAR: return "Clear";
        case WeatherCondition::PARTLY_CLOUDY: return "Partly Cloudy";
        case WeatherCondition::CLOUDY: return "Cloudy";
        case WeatherCondition::OVERCAST: return "Overcast";
        case WeatherCondition::LIGHT_RAIN: return "Light Rain";
        case WeatherCondition::MODERATE_RAIN: return "Moderate Rain";
        case WeatherCondition::HEAVY_RAIN: return "Heavy Rain";
        case WeatherCondition::THUNDERSTORM: return "Thunderstorm";
        case WeatherCondition::FOG: return "Fog";
        case WeatherCondition::MIST: return "Mist";
        default: return "Unknown";
    }
}

String WeatherService::getProvinceName(Province province) {
    switch (province) {
        case Province::ACEH: return "Aceh";
        case Province::SUMATERA_UTARA: return "Sumatera Utara";
        case Province::SUMATERA_BARAT: return "Sumatera Barat";
        case Province::RIAU: return "Riau";
        case Province::JAMBI: return "Jambi";
        case Province::SUMATERA_SELATAN: return "Sumatera Selatan";
        case Province::BENGKULU: return "Bengkulu";
        case Province::LAMPUNG: return "Lampung";
        case Province::KEP_BANGKA_BELITUNG: return "Kepulauan Bangka Belitung";
        case Province::KEP_RIAU: return "Kepulauan Riau";
        case Province::DKI_JAKARTA: return "DKI Jakarta";
        case Province::JAWA_BARAT: return "Jawa Barat";
        case Province::JAWA_TENGAH: return "Jawa Tengah";
        case Province::DI_YOGYAKARTA: return "DI Yogyakarta";
        case Province::JAWA_TIMUR: return "Jawa Timur";
        case Province::BANTEN: return "Banten";
        case Province::BALI: return "Bali";
        case Province::NUSA_TENGGARA_BARAT: return "Nusa Tenggara Barat";
        case Province::NUSA_TENGGARA_TIMUR: return "Nusa Tenggara Timur";
        case Province::KALIMANTAN_BARAT: return "Kalimantan Barat";
        case Province::KALIMANTAN_TENGAH: return "Kalimantan Tengah";
        case Province::KALIMANTAN_SELATAN: return "Kalimantan Selatan";
        case Province::KALIMANTAN_TIMUR: return "Kalimantan Timur";
        case Province::KALIMANTAN_UTARA: return "Kalimantan Utara";
        case Province::SULAWESI_UTARA: return "Sulawesi Utara";
        case Province::SULAWESI_TENGAH: return "Sulawesi Tenggara";
        case Province::SULAWESI_SELATAN: return "Sulawesi Selatan";
        case Province::SULAWESI_TENGGARA: return "Sulawesi Tenggara";
        case Province::GORONTALO: return "Gorontalo";
        case Province::SULAWESI_BARAT: return "Sulawesi Barat";
        case Province::MALUKU: return "Maluku";
        case Province::MALUKU_UTARA: return "Maluku Utara";
        case Province::PAPUA_BARAT: return "Papua Barat";
        case Province::PAPUA: return "Papua";
        default: return "Unknown Province";
    }
}

} // namespace Services

Services::WeatherService weatherService;