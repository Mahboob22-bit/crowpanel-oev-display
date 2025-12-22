#include "TransportModule.h"
#include "OjpParser.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "../Logger/Logger.h"
#include "secrets.h"
#include "../Display/display_manager.h" // For DisplayEvent enum

// Endpoint für OJP 2.0
const char* OJP_API_URL = "https://api.opentransportdata.swiss/ojp2020";

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
    
    // Setup Loop
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = pdMS_TO_TICKS(module->_updateInterval);
    xLastWakeTime = xTaskGetTickCount();
    
    for (;;) {
        // Warte bis zum nächsten Zyklus
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        
        // Config kann sich geändert haben, wir laden sie neu vor jedem Fetch
        // (Effizienter wäre nur bei Änderung, aber für jetzt ok)
        module->updateConfig();
        
        // Prüfe Voraussetzungen
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
    }
}

String TransportModule::buildRequestXml() {
    // Aktuelle Zeit für Request (in UTC)
    time_t now;
    time(&now);
    struct tm* timeinfo = gmtime(&now);
    char timeStr[30];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%SZ", timeinfo);
    
    // Copy members thread-safe for local usage
    String sId;
    if (_mutex) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        sId = _stationId;
        xSemaphoreGive(_mutex);
    }

    // Einfacher String-Builder für XML Request
    // Wir fragen 4 Resultate ab, um Puffer zu haben
    String xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    xml += "<OJP xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns=\"http://www.siri.org.uk/siri\" version=\"1.0\" xmlns:ojp=\"http://www.vdv.de/ojp\" xsi:schemaLocation=\"http://www.siri.org.uk/siri ../ojp-xsd-v1.0/OJP.xsd\">";
    xml += "<OJPRequest>";
    xml += "<ServiceRequest>";
    xml += "<RequestTimestamp>" + String(timeStr) + "</RequestTimestamp>";
    xml += "<RequestorRef>CrowPanelDisplay</RequestorRef>";
    xml += "<ojp:OJPStopEventRequest>";
    xml += "<RequestTimestamp>" + String(timeStr) + "</RequestTimestamp>";
    xml += "<ojp:Location>";
    xml += "<ojp:PlaceRef>";
    xml += "<ojp:StopPlaceRef>" + sId + "</ojp:StopPlaceRef>";
    xml += "<ojp:LocationName>";
    xml += "<ojp:Text>Station</ojp:Text>"; 
    xml += "</ojp:LocationName>";
    xml += "</ojp:PlaceRef>";
    xml += "<ojp:DepArrTime>" + String(timeStr) + "</ojp:DepArrTime>";
    xml += "</ojp:Location>";
    xml += "<ojp:Params>";
    xml += "<ojp:NumberOfResults>4</ojp:NumberOfResults>";
    xml += "<ojp:StopEventType>departure</ojp:StopEventType>";
    xml += "<ojp:IncludeRealtimeData>true</ojp:IncludeRealtimeData>";
    xml += "</ojp:Params>";
    xml += "</ojp:OJPStopEventRequest>";
    xml += "</ServiceRequest>";
    xml += "</OJPRequest>";
    xml += "</OJP>";
    
    return xml;
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
            http.addHeader("Content-Type", "text/xml");
            http.addHeader("Authorization", "Bearer " + key);
            
            String requestBody = buildRequestXml();
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
                        DisplayEvent event = EVENT_DATA_AVAILABLE;
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
