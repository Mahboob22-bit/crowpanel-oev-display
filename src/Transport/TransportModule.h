#ifndef TRANSPORT_MODULE_H
#define TRANSPORT_MODULE_H

#include <Arduino.h>
#include <vector>
#include "TransportTypes.h"

// Events (vorläufig hier definiert, sollten in eine zentrale Event-Datei)
#define EVENT_DATA_NEW "DATA_NEW"
#define EVENT_DATA_ERROR "DATA_ERROR"

class TransportModule {
public:
    TransportModule();
    
    void begin(QueueHandle_t eventQueue);
    
    void setStationId(const String& id);
    void setApiKey(const String& key);
    
    // Gibt die letzten geladenen Abfahrten zurück (Thread-safe Access nötig!)
    std::vector<Departure> getDepartures();

private:
    static void taskCode(void* pvParameters);

    String _stationId;
    String _apiKey;
    unsigned long _updateInterval; // ms
    
    std::vector<Departure> _departures;
    SemaphoreHandle_t _mutex; // Für Thread-safe Zugriff auf Daten
    
    TaskHandle_t taskHandle;
    QueueHandle_t eventQueue;
    
    void fetchData();
    String buildRequestXml();
};

#endif // TRANSPORT_MODULE_H


