#include "WifiManager.h"
#include "../Logger/Logger.h"

WifiManager::WifiManager() 
    : currentState(WIFI_DISCONNECTED), lastCheckTime(0), connectionStartTime(0), internetTested(false), configStore(NULL), taskHandle(NULL), eventQueue(NULL) {}

void WifiManager::begin(ConfigStore* config, QueueHandle_t queue) {
    this->configStore = config;
    this->eventQueue = queue;

    xTaskCreatePinnedToCore(
        taskCode,
        "WifiTask",
        4096,
        this,
        1,
        &taskHandle,
        1
    );
}

void WifiManager::taskCode(void* pvParameters) {
    WifiManager* instance = (WifiManager*)pvParameters;
    
    instance->init();
    
    // Check if we have credentials
    if (instance->configStore->hasWifiConfig()) {
        instance->connect();
    } else {
        Logger::info("TASK_WIFI", "No Wifi config found -> Starting AP");
        instance->startAP();
    }
    
    WifiState lastState = WIFI_DISCONNECTED;

    for(;;) {
        instance->update();
        
        WifiState currentState = instance->getState();
        
        if (currentState != lastState) {
             if (currentState == WIFI_CONNECTED) {
                 Logger::info("TASK_WIFI", "Wifi connected -> Sending event");
                 if (instance->eventQueue != NULL) {
                     SystemEvent event = EVENT_WIFI_CONNECTED;
                     xQueueSend(instance->eventQueue, &event, portMAX_DELAY);
                 }
             } else if (currentState == WIFI_AP_MODE) {
                 Logger::info("TASK_WIFI", "AP Mode started -> Sending event");
                 if (instance->eventQueue != NULL) {
                     SystemEvent event = EVENT_WIFI_AP_MODE;
                     xQueueSend(instance->eventQueue, &event, portMAX_DELAY);
                 }
             } else if (lastState == WIFI_CONNECTED && currentState == WIFI_DISCONNECTED) {
                 Logger::info("TASK_WIFI", "Wifi lost -> Sending event");
                 if (instance->eventQueue != NULL) {
                     SystemEvent event = EVENT_WIFI_LOST;
                     xQueueSend(instance->eventQueue, &event, portMAX_DELAY);
                 }
             }
             lastState = currentState;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void WifiManager::init() {
    Logger::info("WIFI", "Initializing Wifi Manager...");
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
}

void WifiManager::connect() {
    String ssid = configStore->getWifiSSID();
    String password = configStore->getWifiPassword();

    Logger::printf("WIFI", "Connecting to %s...", ssid.c_str());
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    
    currentState = WIFI_CONNECTING;
    connectionStartTime = millis();
    internetTested = false;
}

void WifiManager::startAP() {
    Logger::info("WIFI", "Starting Access Point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID);
    
    currentState = WIFI_AP_MODE;
    
    IPAddress IP = WiFi.softAPIP();
    Logger::printf("WIFI", "AP Started: %s", AP_SSID);
    Logger::printf("WIFI", "AP IP: %s", IP.toString().c_str());
}

void WifiManager::update() {
    switch (currentState) {
        case WIFI_DISCONNECTED:
            // Auto-reconnect nur wenn Config da ist
            if (configStore->hasWifiConfig() && (millis() - lastCheckTime > RECONNECT_INTERVAL)) {
                Logger::info("WIFI", "Auto-reconnecting...");
                connect();
                lastCheckTime = millis();
            }
            break;
            
        case WIFI_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                currentState = WIFI_CONNECTED;
                Logger::info("WIFI", "Connected successfully!");
                Logger::printf("WIFI", "IP Address: %s", WiFi.localIP().toString().c_str());
            } else if (millis() - connectionStartTime > CONNECTION_TIMEOUT) {
                Logger::error("WIFI", "Connection timed out -> Switching to AP Mode");
                startAP(); // Fallback to AP
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
                if (!internetTested) {
                    checkInternet();
                }
            }
            break;
            
        case WIFI_AP_MODE:
            // Im AP Mode prüfen wir, ob wir vielleicht neue Config haben (z.B. durch User Eingabe via Web)
            // Das WebConfigModule könnte einen Reconnect triggern, indem es den State resetet oder restart called.
            // Vorerst bleiben wir im AP Mode.
            break;
    }
}

void WifiManager::checkInternet() {
    internetTested = true; 
    Logger::info("WIFI", "Testing Internet connection...");
    HTTPClient http;
    if (http.begin("http://www.google.com")) {
        int httpCode = http.GET();
        if (httpCode > 0) {
            Logger::printf("WIFI", "Internet Check: OK (Code %d)", httpCode);
            if (eventQueue) {
                SystemEvent event = EVENT_INTERNET_OK;
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

String WifiManager::getIpAddress() {
    if (currentState == WIFI_AP_MODE) {
        return WiFi.softAPIP().toString();
    } else {
        return WiFi.localIP().toString();
    }
}
