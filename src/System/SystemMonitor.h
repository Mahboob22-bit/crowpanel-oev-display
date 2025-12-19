#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <Arduino.h>

class SystemMonitor {
public:
    void begin();
private:
    static void taskCode(void* pvParameters);
    TaskHandle_t taskHandle;
};

#endif // SYSTEM_MONITOR_H
