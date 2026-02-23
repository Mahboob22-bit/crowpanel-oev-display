#include "ConfigStore.h"
#include "../Logger/Logger.h"
#include <mbedtls/base64.h>

ConfigStore::ConfigStore() : _macKey(0) {}

void ConfigStore::begin() {
    Logger::info("CONFIG", "Initializing ConfigStore...");
    preferences.begin(NAMESPACE, false);
    
    _macKey = ESP.getEfuseMac();
    
    migratePassword();
    
    StationConfig station = getStation();
    if (station.id.length() == 0) {
        Logger::info("CONFIG", "No station configured, setting defaults (Arlesheim, Im Lee)...");
        setStation("Arlesheim, Im Lee", "8588764");
        setLine1("10", "Flüh, Bahnhof");
        setLine2("10", "Dornach Bahnhof");
    }
}

// Wifi
void ConfigStore::setWifiCredentials(const String& ssid, const String& password) {
    preferences.putString("ssid", ssid);
    preferences.putString("password", obfuscate(password));
    preferences.putBool("pw_obf", true);
    Logger::info("CONFIG", "Wifi credentials saved");
}

String ConfigStore::getWifiSSID() {
    return preferences.getString("ssid", "");
}

String ConfigStore::getWifiPassword() {
    String stored = preferences.getString("password", "");
    if (stored.length() == 0) return stored;
    
    if (preferences.getBool("pw_obf", false)) {
        return deobfuscate(stored);
    }
    return stored;
}

bool ConfigStore::hasWifiConfig() {
    String ssid = getWifiSSID();
    return ssid.length() > 0;
}

// Transport API
void ConfigStore::setApiKey(const String& apiKey) {
    preferences.putString("apikey", apiKey);
    Logger::info("CONFIG", "API Key saved");
}

String ConfigStore::getApiKey() {
    return preferences.getString("apikey", "");
}

// Station
void ConfigStore::setStation(const String& name, const String& id) {
    preferences.putString("st_name", name);
    preferences.putString("st_id", id);
    Logger::info("CONFIG", ("Station saved: " + name).c_str());
}

StationConfig ConfigStore::getStation() {
    StationConfig config;
    config.name = preferences.getString("st_name", "");
    config.id = preferences.getString("st_id", "");
    return config;
}

// Lines
void ConfigStore::setLine1(const String& name, const String& direction) {
    preferences.putString("l1_name", name);
    preferences.putString("l1_dir", direction);
    Logger::info("CONFIG", "Line 1 saved");
}

LineConfig ConfigStore::getLine1() {
    LineConfig config;
    config.name = preferences.getString("l1_name", "");
    config.direction = preferences.getString("l1_dir", "");
    return config;
}

void ConfigStore::setLine2(const String& name, const String& direction) {
    preferences.putString("l2_name", name);
    preferences.putString("l2_dir", direction);
    Logger::info("CONFIG", "Line 2 saved");
}

LineConfig ConfigStore::getLine2() {
    LineConfig config;
    config.name = preferences.getString("l2_name", "");
    config.direction = preferences.getString("l2_dir", "");
    return config;
}

// Web Password
void ConfigStore::setWebPassword(const String& password) {
    preferences.putString("web_pw", password);
    Logger::info("CONFIG", "Web password saved");
}

String ConfigStore::getWebPassword() {
    return preferences.getString("web_pw", "");
}

bool ConfigStore::hasWebPassword() {
    return getWebPassword().length() > 0;
}

// Reset
void ConfigStore::resetToFactory() {
    Logger::info("CONFIG", "Factory Reset...");
    preferences.clear();
}

String ConfigStore::obfuscate(const String& input) {
    if (input.length() == 0) return input;
    
    uint8_t* keyBytes = reinterpret_cast<uint8_t*>(&_macKey);
    size_t keyLen = sizeof(_macKey);
    
    size_t inputLen = input.length();
    uint8_t* xored = new uint8_t[inputLen];
    for (size_t i = 0; i < inputLen; i++) {
        xored[i] = input[i] ^ keyBytes[i % keyLen];
    }
    
    size_t b64Len = 0;
    mbedtls_base64_encode(NULL, 0, &b64Len, xored, inputLen);
    
    uint8_t* b64 = new uint8_t[b64Len + 1];
    mbedtls_base64_encode(b64, b64Len + 1, &b64Len, xored, inputLen);
    b64[b64Len] = '\0';
    
    String result = String(reinterpret_cast<char*>(b64));
    delete[] xored;
    delete[] b64;
    return result;
}

String ConfigStore::deobfuscate(const String& input) {
    if (input.length() == 0) return input;
    
    size_t decodedLen = 0;
    mbedtls_base64_decode(NULL, 0, &decodedLen,
        reinterpret_cast<const uint8_t*>(input.c_str()), input.length());
    
    if (decodedLen == 0) return "";
    
    uint8_t* decoded = new uint8_t[decodedLen];
    size_t actualLen = 0;
    int ret = mbedtls_base64_decode(decoded, decodedLen, &actualLen,
        reinterpret_cast<const uint8_t*>(input.c_str()), input.length());
    
    if (ret != 0) {
        delete[] decoded;
        return input;
    }
    
    uint8_t* keyBytes = reinterpret_cast<uint8_t*>(&_macKey);
    size_t keyLen = sizeof(_macKey);
    
    char* result = new char[actualLen + 1];
    for (size_t i = 0; i < actualLen; i++) {
        result[i] = decoded[i] ^ keyBytes[i % keyLen];
    }
    result[actualLen] = '\0';
    
    String output = String(result);
    delete[] decoded;
    delete[] result;
    return output;
}

void ConfigStore::migratePassword() {
    if (preferences.getBool("pw_obf", false)) return;
    
    String plainPw = preferences.getString("password", "");
    if (plainPw.length() == 0) return;
    
    Logger::info("CONFIG", "Migrating WiFi password to obfuscated storage");
    preferences.putString("password", obfuscate(plainPw));
    preferences.putBool("pw_obf", true);
}

