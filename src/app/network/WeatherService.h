#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <functional>
#include <LittleFS.h>

// BMKG Weather Service for Indonesian weather data
// API: https://api.bmkg.go.id/publik/prakiraan-cuaca

namespace Services {

class WeatherService {
public:
    // Indonesian Provinces (BMKG codes)
    enum class Province {
        ACEH = 11,
        SUMATERA_UTARA = 12,
        SUMATERA_BARAT = 13,
        RIAU = 14,
        JAMBI = 15,
        SUMATERA_SELATAN = 16,
        BENGKULU = 17,
        LAMPUNG = 18,
        KEP_BANGKA_BELITUNG = 19,
        KEP_RIAU = 21,
        DKI_JAKARTA = 31,
        JAWA_BARAT = 32,
        JAWA_TENGAH = 33,
        DI_YOGYAKARTA = 34,
        JAWA_TIMUR = 35,
        BANTEN = 36,
        BALI = 51,
        NUSA_TENGGARA_BARAT = 52,
        NUSA_TENGGARA_TIMUR = 53,
        KALIMANTAN_BARAT = 61,
        KALIMANTAN_TENGAH = 62,
        KALIMANTAN_SELATAN = 63,
        KALIMANTAN_TIMUR = 64,
        KALIMANTAN_UTARA = 65,
        SULAWESI_UTARA = 71,
        SULAWESI_TENGAH = 72,
        SULAWESI_SELATAN = 73,
        SULAWESI_TENGGARA = 74,
        GORONTALO = 75,
        SULAWESI_BARAT = 76,
        MALUKU = 81,
        MALUKU_UTARA = 82,
        PAPUA_BARAT = 91,
        PAPUA = 94
    };

    // Weather Condition Categories
    enum class WeatherCondition {
        CLEAR = 0,
        PARTLY_CLOUDY,
        CLOUDY,
        OVERCAST,
        LIGHT_RAIN,
        MODERATE_RAIN,
        HEAVY_RAIN,
        THUNDERSTORM,
        FOG,
        MIST,
        UNKNOWN
    };

    struct WeatherData {
        String location;
        String description;
        WeatherCondition condition;
        int temperature;    // in Celsius
        int humidity;       // in percentage
        int windSpeed;      // in km/h
        String windDirection;
        String lastUpdated;
        String imageUrl;
        float longitude;
        float latitude;
        String timezone;
        bool isValid;

        WeatherData() : condition(WeatherCondition::UNKNOWN), temperature(0), humidity(0),
                       windSpeed(0), longitude(0.0), latitude(0.0), isValid(false) {}
    };

    struct WeatherConfig {
        String adm4Code;           // Administrative level 4 code (village/kelurahan)
        uint32_t cacheExpiryMinutes;

        WeatherConfig() : adm4Code("31.73.05.1001"), cacheExpiryMinutes(60) {}
    };

    using WeatherCallback = std::function<void(const WeatherData& data, bool success)>;

    WeatherService();
    ~WeatherService();

    bool init(const WeatherConfig& config);
    bool isInitialized() const { return _initialized; }

    void getCurrentWeather(WeatherCallback callback, bool forceRefresh = false);
    void setLocation(const String& adm4Code);
    void setCacheExpiry(uint32_t minutes);
    WeatherConfig getConfig() const { return _config; }
    void clearCache();
    bool isCacheValid() const;
    WeatherData getCachedData();

    static WeatherCondition getConditionFromCode(int weatherCode);
    static String conditionToString(WeatherCondition condition);
    static String getProvinceName(Province province);

private:
    WeatherConfig _config;
    WeatherData _cachedData;
    unsigned long _lastCacheTime;
    bool _initialized;

    static const char* CACHE_FILE_PATH;

    void fetchFromAPI(WeatherCallback callback);
    void processAPIResponse(const String& response, WeatherCallback callback);
    bool loadCache();
    bool saveCache(const WeatherData& data);
    String buildAPIUrl() const;
    unsigned long getCurrentTimestamp() const;
};

} // namespace Services

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_LITTLEFS)
extern Services::WeatherService weatherService;
typedef Services::WeatherService::WeatherConfig weatherConfig_t;
typedef Services::WeatherService::WeatherData weatherData_t;
#endif