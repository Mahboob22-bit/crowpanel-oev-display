#ifndef TIME_MODULE_H
#define TIME_MODULE_H

#include <Arduino.h>
#include <time.h>
#include "../Core/SystemEvents.h"

class TimeModule {
public:
    TimeModule();
    void begin(QueueHandle_t eventQueue);
    
    String getFormattedTime();

private:
    static void taskCode(void* pvParameters);
    
    QueueHandle_t eventQueue;
    TaskHandle_t taskHandle;
    bool isSynced;
    bool isConfigured;
    
    const char* NTP_SERVER_1 = "pool.ntp.org";
    const char* NTP_SERVER_2 = "time.nist.gov";
    // Schweizer Zeitzone
    const char* TIMEZONE = "CET-1CEST,M3.5.0,M10.5.0/3"; 
};

#endif // TIME_MODULE_H
