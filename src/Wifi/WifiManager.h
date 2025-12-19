#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "../Display/display_manager.h" // Für DisplayEvent Enum

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
    void begin(const char* ssid, const char* password, QueueHandle_t eventQueue);
    
    WifiState getState();
    
private:
    // Task Funktion (muss static sein für FreeRTOS)
    static void taskCode(void* pvParameters);
    
    void init();
    void update();
    void connect(const char* ssid, const char* password);

    WifiState currentState;
    unsigned long lastCheckTime;
    unsigned long connectionStartTime;
    
    String _ssid;
    String _password;
    
    TaskHandle_t taskHandle;
    QueueHandle_t eventQueue;
    
    const unsigned long CONNECTION_TIMEOUT = 10000; 
    const unsigned long RECONNECT_INTERVAL = 30000; 
};

#endif // WIFI_MANAGER_H
