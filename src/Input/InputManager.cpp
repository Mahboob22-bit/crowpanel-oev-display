#include "InputManager.h"
#include "crowpanel_pins.h"
#include "../Logger/Logger.h"
#include "../Transport/TransportModule.h" // Hier brauchen wir den vollen Header für den Methodenaufruf

InputManager::InputManager() : taskHandle(NULL), eventQueue(NULL), configStore(NULL), transportModule(NULL) {}

void InputManager::begin(QueueHandle_t queue, ConfigStore* config, TransportModule* transport) {
    this->eventQueue = queue;
    this->configStore = config;
    this->transportModule = transport;

    Logger::info("INPUT", "Configuring buttons (Polling Mode)...");
    pinMode(BTN_MENU, INPUT_PULLUP);
    pinMode(BTN_EXIT, INPUT_PULLUP);
    pinMode(BTN_ROTARY_SW, INPUT_PULLUP);

    xTaskCreatePinnedToCore(
        taskCode,
        "ButtonTask",
        4096,
        this,
        2,
        &taskHandle,
        0
    );
}

void InputManager::taskCode(void* pvParameters) {
    InputManager* instance = (InputManager*)pvParameters;
    
    // States für Entprellung und Long-Press
    // Wir nutzen einfache Zähler: Wenn Taste X Zyklen gedrückt ist, gilt sie als gedrückt.
    int menuCounter = 0;
    bool menuHandled = false;
    bool menuLongPressHandled = false;

    // Polling Intervall
    const int pollDelay = 50; 
    // Schwellwerte (50ms * x)
    const int shortPressThreshold = 1; // 50ms
    const int longPressThreshold = 60; // 3000ms / 50ms

    for(;;) {
        // --- MENU BUTTON ---
        if (digitalRead(BTN_MENU) == LOW) { // Active Low
            menuCounter++;
            
            // Long Press Detection
            if (menuCounter > longPressThreshold && !menuLongPressHandled) {
                Logger::info("BUTTON", "Menu LONG PRESS -> Factory Reset!");
                if (instance->configStore) {
                    instance->configStore->resetToFactory();
                    ESP.restart();
                }
                menuLongPressHandled = true;
                menuHandled = true; // Damit Short Press nicht feuert beim Loslassen
            }
        } else {
            // Taste losgelassen
            if (menuCounter > shortPressThreshold && !menuHandled) {
                // Short Press Action
                Logger::info("BUTTON", "Menu Short Press -> Trigger Update");
                if (instance->transportModule) {
                    instance->transportModule->triggerUpdate();
                }
                // Optional: Feedback auf Display
                /* if (instance->eventQueue) {
                     SystemEvent event = EVENT_UPDATE_TRIGGER;
                     xQueueSend(instance->eventQueue, &event, 0);
                } */
            }
            // Reset
            menuCounter = 0;
            menuHandled = false;
            menuLongPressHandled = false;
        }

        // --- EXIT BUTTON (Nur Beispiel, aktuell keine Funktion) ---
        /*
        if (digitalRead(BTN_EXIT) == LOW) {
            // ...
        }
        */

        vTaskDelay(pdMS_TO_TICKS(pollDelay));
    }
}
