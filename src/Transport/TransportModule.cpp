#include "TransportModule.h"
#include "OjpParser.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "../Logger/Logger.h"
#include "secrets.h"
// #include "../Display/display_manager.h" // Entfernt, da wir jetzt SystemEvents nutzen

// Endpoint für OJP 2.0 (Korrektur: ojp20 statt ojp2020)
const char* OJP_API_URL = "https://api.opentransportdata.swiss/ojp20";

TransportModule::TransportModule() 
    : _updateInterval(30000), // 30 Sekunden
      taskHandle(NULL),
      eventQueue(NULL),
      _mutex(NULL),
      configStore(NULL)
{
    _mutex = xSemaphoreCreateMutex();
}

void TransportModule::begin(QueueHandle_t queue, ConfigStore* store) {
    eventQueue = queue;
    configStore = store;
    
    // Initiale Config laden
    updateConfig();
    
    // Starte Task
    xTaskCreate(
        taskCode,          // Task Funktion
        "TransportTask",   // Name
        8192,              // Stack Größe (XML Parsing braucht RAM)
        this,              // Parameter (Pointer auf this Instanz)
        1,                 // Priorität (Niedrig, da Netzwerklastig)
        &taskHandle        // Handle
    );
}

void TransportModule::updateConfig() {
    if (!configStore || !_mutex) return;
    
    xSemaphoreTake(_mutex, portMAX_DELAY);
    
    _apiKey = OJP_API_KEY;
    StationConfig station = configStore->getStation();
    _stationId = station.id;
    
    Logger::info("TRANSPORT", "Config updated from Store");
    Logger::info("TRANSPORT", "API Key used from secrets.h");
    Logger::printf("TRANSPORT", "Station ID: %s", _stationId.c_str());
    
    xSemaphoreGive(_mutex);
}

std::vector<Departure> TransportModule::getDepartures() {
    std::vector<Departure> deps;
    if (_mutex) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        deps = _departures;
        xSemaphoreGive(_mutex);
    }
    return deps;
}

void TransportModule::taskCode(void* pvParameters) {
    TransportModule* module = (TransportModule*)pvParameters;
    
    // Initialer Fetch
    const TickType_t xFrequency = pdMS_TO_TICKS(module->_updateInterval);
    
    for (;;) {
        // 1. Config laden & Fetch ausführen
        module->updateConfig();
        
        bool ready = false;
        if (module->_mutex) {
            xSemaphoreTake(module->_mutex, portMAX_DELAY);
            ready = (module->_stationId.length() > 0 && module->_apiKey.length() > 0);
            xSemaphoreGive(module->_mutex);
        }
        
        if (ready) {
             module->fetchData();
        } else {
             Logger::info("TRANSPORT", "Missing configuration (API Key or Station ID)");
        }

        // 2. Warten: Entweder Timeout (30s) abgelaufen ODER Signal bekommen (triggerUpdate)
        // ulTaskNotifyTake gibt > 0 zurück, wenn ein Signal kam, 0 bei Timeout
        if (ulTaskNotifyTake(pdTRUE, xFrequency) > 0) {
            Logger::info("TRANSPORT", "Update triggered manually!");
        }
    }
}

void TransportModule::triggerUpdate() {
    if (taskHandle != NULL) {
        xTaskNotifyGive(taskHandle);
    }
}

void TransportModule::fetchData() {
    if (WiFi.status() != WL_CONNECTED) {
        Logger::info("TRANSPORT", "Wifi not connected, skipping update");
        return;
    }

    // Thread-safe copy of API Key
    String key;
    if (_mutex) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        key = _apiKey;
        xSemaphoreGive(_mutex);
    }

    WiFiClientSecure *client = new WiFiClientSecure;
    if(client) {
        // Für Development akzeptieren wir unsichere Zertifikate, später Root CA setzen
        client->setInsecure(); 
        
        HTTPClient http;
        
        if (http.begin(*client, OJP_API_URL)) {
            http.addHeader("Content-Type", "application/xml");
            http.addHeader("Authorization", "Bearer " + key);
            http.addHeader("User-Agent", "CrowPanel-OEV-Display/1.0");
            
            String sId;
            if (_mutex) {
                xSemaphoreTake(_mutex, portMAX_DELAY);
                sId = _stationId;
                xSemaphoreGive(_mutex);
            }
            
            String requestBody = OjpParser::buildRequestXml(sId, "CrowPanelDisplay");
            Logger::info("TRANSPORT", "Sending OJP Request...");
            
            int httpCode = http.POST(requestBody);
            
            if (httpCode > 0) {
                if (httpCode == HTTP_CODE_OK) {
                    String payload = http.getString();
                    Logger::info("TRANSPORT", "OJP Response received");
                    
                    std::vector<Departure> newDepartures = OjpParser::parseResponse(payload);
                    Logger::printf("TRANSPORT", "Parsed %d departures", newDepartures.size());
                    
                    if (_mutex) {
                        xSemaphoreTake(_mutex, portMAX_DELAY);
                        _departures = newDepartures;
                        xSemaphoreGive(_mutex);
                    }
                    
                    // Fire Event
                    if (eventQueue) {
                        SystemEvent event = EVENT_DATA_AVAILABLE;
                        xQueueSend(eventQueue, &event, 0);
                    }
                } else {
                    Logger::printf("TRANSPORT", "HTTP Error: %d", httpCode);
                    // 403 Forbidden -> API Key invalid or not active
                    if (httpCode == 403) {
                         Logger::error("TRANSPORT", "API Key invalid or not yet active. Please check your email/account.");
                    }
                }
            } else {
                Logger::printf("TRANSPORT", "HTTP Connection failed: %s", http.errorToString(httpCode).c_str());
            }
            
            http.end();
        }
        delete client;
    }
}
