#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <Arduino.h>
#include "../Display/display_manager.h" // Für Events
#include "../Core/ConfigStore.h"

class InputManager {
public:
    InputManager();
    void begin(QueueHandle_t eventQueue, ConfigStore* configStore);

private:
    static void taskCode(void* pvParameters);
    
    ConfigStore* configStore;

    // ISRs
    static void IRAM_ATTR handleMenuButton();
    static void IRAM_ATTR handleExitButton();
    static void IRAM_ATTR handleRotaryButton();
    
    // Flags
    static volatile bool menuPressed;
    static volatile bool exitPressed;
    static volatile bool rotaryPressed;
    static unsigned long lastInterruptTime;
    
    TaskHandle_t taskHandle;
    QueueHandle_t eventQueue;
};

#endif // INPUT_MANAGER_H
