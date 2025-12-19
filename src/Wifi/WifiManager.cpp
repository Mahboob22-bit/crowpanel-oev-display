#include "WifiManager.h"
#include "../Logger/Logger.h"

WifiManager::WifiManager() 
    : currentState(WIFI_DISCONNECTED), lastCheckTime(0), connectionStartTime(0), internetTested(false), taskHandle(NULL), eventQueue(NULL) {}

void WifiManager::begin(const char* ssid, const char* password, QueueHandle_t queue) {
    this->eventQueue = queue;
    this->_ssid = ssid;
    this->_password = password;

    xTaskCreatePinnedToCore(
        taskCode,
        "WifiTask",
        4096,
        this,              // "this" als Parameter übergeben
        1,                 // Normale Priorität
        &taskHandle,
        1                  // Core 1
    );
}

void WifiManager::taskCode(void* pvParameters) {
    WifiManager* instance = (WifiManager*)pvParameters;
    
    instance->init();
    instance->connect(instance->_ssid.c_str(), instance->_password.c_str());
    
    WifiState lastState = WIFI_DISCONNECTED;

    for(;;) {
        instance->update();
        
        WifiState currentState = instance->getState();
        
        // Statusänderung erkennen und an Display melden
        if (currentState != lastState) {
             if (currentState == WIFI_CONNECTED) {
                 Logger::info("TASK_WIFI", "Wifi connected -> Sending event");
                 if (instance->eventQueue != NULL) {
                     DisplayEvent event = EVENT_WIFI_CONNECTED;
                     xQueueSend(instance->eventQueue, &event, portMAX_DELAY);
                 }
             } else if (lastState == WIFI_CONNECTED && currentState == WIFI_DISCONNECTED) {
                 Logger::info("TASK_WIFI", "Wifi lost -> Sending event");
                 if (instance->eventQueue != NULL) {
                     DisplayEvent event = EVENT_WIFI_LOST;
                     xQueueSend(instance->eventQueue, &event, portMAX_DELAY);
                 }
             }
             lastState = currentState;
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 100ms polling
    }
}

void WifiManager::init() {
    Logger::info("WIFI", "Initializing Wifi Manager...");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    Logger::info("WIFI", "Wifi initialized in Station Mode");
}

void WifiManager::connect(const char* ssid, const char* password) {
    Logger::printf("WIFI", "Connecting to %s...", ssid);
    
    WiFi.begin(ssid, password);
    currentState = WIFI_CONNECTING;
    connectionStartTime = millis();
    internetTested = false;
}

void WifiManager::update() {
    switch (currentState) {
        case WIFI_DISCONNECTED:
            if (_ssid.length() > 0 && (millis() - lastCheckTime > RECONNECT_INTERVAL)) {
                Logger::info("WIFI", "Auto-reconnecting...");
                connect(_ssid.c_str(), _password.c_str());
                lastCheckTime = millis();
            }
            break;
            
        case WIFI_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                currentState = WIFI_CONNECTED;
                Logger::info("WIFI", "Connected successfully!");
                Logger::printf("WIFI", "IP Address: %s", WiFi.localIP().toString().c_str());
            } else if (millis() - connectionStartTime > CONNECTION_TIMEOUT) {
                currentState = WIFI_DISCONNECTED;
                Logger::error("WIFI", "Connection timed out!");
                WiFi.disconnect();
                lastCheckTime = millis();
            }
            break;
            
        case WIFI_CONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                currentState = WIFI_DISCONNECTED;
                Logger::error("WIFI", "Connection lost!");
                WiFi.disconnect();
                lastCheckTime = millis();
            } else {
                // Internet Check durchführen, falls noch nicht geschehen
                if (!internetTested) {
                    checkInternet();
                }
            }
            break;
            
        case WIFI_AP_MODE:
            break;
    }
}

void WifiManager::checkInternet() {
    internetTested = true; 
    
    Logger::info("WIFI", "Testing Internet connection...");
    
    HTTPClient http;
    // Wir nutzen google.com als Test-Target. 
    // http:// (ohne S) spart SSL Overhead und Zertifikats-Stress für diesen einfachen Check
    if (http.begin("http://www.google.com")) {
        int httpCode = http.GET();
        if (httpCode > 0) {
            Logger::printf("WIFI", "Internet Check: OK (Code %d)", httpCode);
            if (eventQueue) {
                DisplayEvent event = EVENT_INTERNET_OK;
                xQueueSend(eventQueue, &event, portMAX_DELAY);
            }
        } else {
             Logger::printf("WIFI", "Internet Check: Failed (Error: %s)", http.errorToString(httpCode).c_str());
        }
        http.end();
    } else {
        Logger::error("WIFI", "Unable to connect to test URL");
    }
}

WifiState WifiManager::getState() {
    return currentState;
}
