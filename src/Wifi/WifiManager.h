#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "../Core/SystemEvents.h"
#include "../Core/ConfigStore.h"

enum WifiState {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
    WIFI_AP_MODE
};

class WifiManager {
public:
    WifiManager();
    
    // Startet den Wifi-Task
    void begin(ConfigStore* configStore, QueueHandle_t eventQueue);
    
    WifiState getState();
    String getIpAddress();
    
private:
    static void taskCode(void* pvParameters);
    
    void init();
    void update();
    void connect();
    void startAP();
    void checkInternet();

    WifiState currentState;
    unsigned long lastCheckTime;
    unsigned long connectionStartTime;
    bool internetTested;
    
    ConfigStore* configStore;
    TaskHandle_t taskHandle;
    QueueHandle_t eventQueue;
    
    const unsigned long CONNECTION_TIMEOUT = 15000; 
    const unsigned long RECONNECT_INTERVAL = 30000; 
    const char* AP_SSID = "CrowPanel-Setup";
};

#endif // WIFI_MANAGER_H
