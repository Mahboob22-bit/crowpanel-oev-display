#ifndef CONFIG_STORE_H
#define CONFIG_STORE_H

#include <Arduino.h>
#include <Preferences.h>

struct StationConfig {
    String name;
    String id;
};

struct LineConfig {
    String name;
    String direction;
};

class ConfigStore {
public:
    ConfigStore();
    
    void begin();
    
    // Wifi
    void setWifiCredentials(const String& ssid, const String& password);
    String getWifiSSID();
    String getWifiPassword();
    bool hasWifiConfig();
    
    // Transport API
    void setApiKey(const String& apiKey);
    String getApiKey();
    
    // Station & Lines
    void setStation(const String& name, const String& id);
    StationConfig getStation();
    
    void setLine1(const String& name, const String& direction);
    LineConfig getLine1();
    
    void setLine2(const String& name, const String& direction);
    LineConfig getLine2();
    
    // Reset
    void resetToFactory();

private:
    Preferences preferences;
    const char* NAMESPACE = "crowpanel";
};

#endif // CONFIG_STORE_H

