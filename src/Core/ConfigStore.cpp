#include "ConfigStore.h"
#include "../Logger/Logger.h"

ConfigStore::ConfigStore() {}

void ConfigStore::begin() {
    Logger::info("CONFIG", "Initializing ConfigStore...");
    preferences.begin(NAMESPACE, false); // false = read/write
}

// Wifi
void ConfigStore::setWifiCredentials(const String& ssid, const String& password) {
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    Logger::info("CONFIG", "Wifi credentials saved");
}

String ConfigStore::getWifiSSID() {
    return preferences.getString("ssid", "");
}

String ConfigStore::getWifiPassword() {
    return preferences.getString("password", "");
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

// Reset
void ConfigStore::resetToFactory() {
    Logger::info("CONFIG", "Factory Reset...");
    preferences.clear();
}

