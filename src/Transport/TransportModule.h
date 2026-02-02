#ifndef TRANSPORT_MODULE_H
#define TRANSPORT_MODULE_H

#include <Arduino.h>
#include <vector>
#include "TransportTypes.h"
#include "../Core/ConfigStore.h"
#include "../Core/SystemEvents.h"

class TransportModule {
public:
    TransportModule();
    
    void begin(QueueHandle_t eventQueue, ConfigStore* configStore);
    
    // Config wird jetzt intern aus dem Store geholt
    void updateConfig();
    
    // Weckt den Task auf für ein sofortiges Update
    void triggerUpdate();

    std::vector<Departure> getDepartures();
    
    // Synchrone Haltestellensuche (blockiert bis Antwort da)
    std::vector<StopSearchResult> searchStops(const String& query);
    
    // Synchrone Linienabfrage für eine Haltestelle (blockiert bis Antwort da)
    std::vector<LineInfo> getAvailableLines(const String& stopId);

private:
    static void taskCode(void* pvParameters);
    
    ConfigStore* configStore;
    
    String _stationId;
    String _apiKey;
    unsigned long _updateInterval; // ms
    
    std::vector<Departure> _departures;
    SemaphoreHandle_t _mutex; // Für Thread-safe Zugriff auf Daten
    
    TaskHandle_t taskHandle;
    QueueHandle_t eventQueue;
    
    void fetchData();
};

#endif // TRANSPORT_MODULE_H


