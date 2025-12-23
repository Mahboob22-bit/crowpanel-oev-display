#include "TimeModule.h"
#include "../Logger/Logger.h"
#include <WiFi.h>

TimeModule::TimeModule() : taskHandle(NULL), isSynced(false), isConfigured(false) {}

void TimeModule::begin(QueueHandle_t eventQueue) {
    this->eventQueue = eventQueue;
    
    // Wir konfigurieren NTP noch nicht hier, um Race-Conditions mit dem Wifi-Stack Init zu vermeiden.
    // Das passiert im Task sobald Wifi connected ist.
    
    Logger::info("TIME", "Starting Time Task...");
    
    // Task starten, der auf Zeit-Sync prüft
    xTaskCreate(
        taskCode,
        "TimeTask",
        4096,
        this,
        1,
        &taskHandle
    );
}

void TimeModule::taskCode(void* pvParameters) {
    TimeModule* module = (TimeModule*)pvParameters;
    
    for (;;) {
        // 1. Konfiguration (erst wenn Wifi da ist)
        if (!module->isConfigured) {
            if (WiFi.status() == WL_CONNECTED) {
                Logger::info("TIME", "Wifi connected. Configuring NTP...");
                
                configTime(0, 0, module->NTP_SERVER_1, module->NTP_SERVER_2);
                setenv("TZ", module->TIMEZONE, 1);
                tzset();
                
                module->isConfigured = true;
            } else {
                // Warten auf Wifi...
                vTaskDelay(pdMS_TO_TICKS(1000));
                continue;
            }
        }

        // 2. Sync Check
        if (!module->isSynced) {
            time_t now;
            time(&now);
            struct tm timeinfo;
            localtime_r(&now, &timeinfo);
            
            // Wenn Jahr > 2020 (also nicht 1970), dann ist Zeit da
            if (timeinfo.tm_year > (2020 - 1900)) {
                module->isSynced = true;
                Logger::printf("TIME", "Time synchronized: %s", module->getFormattedTime().c_str());
                
                // Event feuern
                if (module->eventQueue != NULL) {
                    SystemEvent event = EVENT_TIME_SYNCED;
                    xQueueSend(module->eventQueue, &event, 0);
                }
            }
        }
        
        // Jede Sekunde prüfen wenn nicht gesynct, sonst jede Minute
        vTaskDelay(pdMS_TO_TICKS(module->isSynced ? 60000 : 1000)); 
    }
}

String TimeModule::getFormattedTime() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return "Not Synced";
    }
    char timeStringBuff[50];
    strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(timeStringBuff);
}
