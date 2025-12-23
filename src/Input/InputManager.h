#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>
#include "../Core/SystemEvents.h" 
#include "../Core/ConfigStore.h"

// Forward Declaration, um Zirkuläre Abhängigkeit im Header zu vermeiden
class TransportModule; 

class InputManager {
public:
    InputManager();
    // Wir injizieren jetzt auch das TransportModule
    void begin(QueueHandle_t eventQueue, ConfigStore* configStore, TransportModule* transportModule);

private:
    static void taskCode(void* pvParameters);
    
    ConfigStore* configStore;
    TransportModule* transportModule;
    
    TaskHandle_t taskHandle;
    QueueHandle_t eventQueue;
};

#endif // INPUT_MANAGER_H
