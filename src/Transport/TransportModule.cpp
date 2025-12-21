#include "TransportModule.h"
#include "OjpParser.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "secrets.h"

// Endpoint für OJP 2.0
const char* OJP_API_URL = "https://api.opentransportdata.swiss/ojp2020";

TransportModule::TransportModule() 
    : _updateInterval(30000), // 30 Sekunden
      taskHandle(NULL),
      eventQueue(NULL),
      _mutex(NULL)
{
    _mutex = xSemaphoreCreateMutex();
}

void TransportModule::begin(QueueHandle_t queue) {
    eventQueue = queue;
    setApiKey(OJP_API_KEY);
    setStationId(TEST_STATION_ID);
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

void TransportModule::setStationId(const String& id) {
    if (_mutex) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        _stationId = id;
        xSemaphoreGive(_mutex);
    }
}

void TransportModule::setApiKey(const String& key) {
    if (_mutex) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        _apiKey = key;
        xSemaphoreGive(_mutex);
    }
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
        
        // Prüfe Voraussetzungen
        bool ready = false;
        if (module->_mutex) {
            xSemaphoreTake(module->_mutex, portMAX_DELAY);
            ready = (module->_stationId.length() > 0 && module->_apiKey.length() > 0);
            xSemaphoreGive(module->_mutex);
        }
        
        if (ready) {
             module->fetchData();
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
        Serial.println("Wifi not connected, skipping transport update");
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
            Serial.println("Sending OJP Request...");
            
            int httpCode = http.POST(requestBody);
            
            if (httpCode > 0) {
                if (httpCode == HTTP_CODE_OK) {
                    String payload = http.getString();
                    Serial.println("OJP Response received");
                    
                    std::vector<Departure> newDepartures = OjpParser::parseResponse(payload);
                    Serial.printf("Parsed %d departures\n", newDepartures.size());
                    
                    if (_mutex) {
                        xSemaphoreTake(_mutex, portMAX_DELAY);
                        _departures = newDepartures;
                        xSemaphoreGive(_mutex);
                    }
                    
                    // TODO: Event feuern (EVENT_DATA_NEW)
                } else {
                    Serial.printf("HTTP Error: %d\n", httpCode);
                    // 403 Forbidden -> API Key invalid or not active
                    if (httpCode == 403) {
                         Serial.println("API Key invalid or not yet active. Please check your email/account.");
                    }
                }
            } else {
                Serial.printf("HTTP Connection failed: %s\n", http.errorToString(httpCode).c_str());
            }
            
            http.end();
        }
        delete client;
    }
}
