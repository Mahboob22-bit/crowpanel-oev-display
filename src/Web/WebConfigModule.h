#ifndef WEB_CONFIG_MODULE_H
#define WEB_CONFIG_MODULE_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "../Core/ConfigStore.h"
#include "../Wifi/WifiManager.h"

class WebConfigModule {
public:
    WebConfigModule();
    
    void begin(ConfigStore* configStore, WifiManager* wifiManager);
    
private:
    AsyncWebServer server;
    ConfigStore* configStore;
    WifiManager* wifiManager;
    
    void setupRoutes();
    void handleScan(AsyncWebServerRequest *request);
    void handleScanResults(AsyncWebServerRequest *request);
    void handleStatus(AsyncWebServerRequest *request);
    void handleConfigSave(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
};

#endif // WEB_CONFIG_MODULE_H

