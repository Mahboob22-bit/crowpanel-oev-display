#include "InputManager.h"
#include "crowpanel_pins.h"
#include "../Logger/Logger.h"

// Statische Member initialisieren
volatile bool InputManager::menuPressed = false;
volatile bool InputManager::exitPressed = false;
volatile bool InputManager::rotaryPressed = false;
unsigned long InputManager::lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

InputManager::InputManager() : taskHandle(NULL), eventQueue(NULL), configStore(NULL) {}

void InputManager::begin(QueueHandle_t queue, ConfigStore* config) {
    this->eventQueue = queue;
    this->configStore = config;

    Logger::info("INPUT", "Configuring buttons...");
    pinMode(BTN_MENU, INPUT_PULLUP);
    pinMode(BTN_EXIT, INPUT_PULLUP);
    pinMode(BTN_ROTARY_SW, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(BTN_MENU), handleMenuButton, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_EXIT), handleExitButton, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_ROTARY_SW), handleRotaryButton, FALLING);

    Logger::info("INPUT", "Buttons configured!");

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

void IRAM_ATTR InputManager::handleMenuButton() {
    unsigned long currentTime = millis();
    if (currentTime - lastInterruptTime > debounceDelay) {
        menuPressed = true;
        lastInterruptTime = currentTime;
    }
}

void IRAM_ATTR InputManager::handleExitButton() {
    unsigned long currentTime = millis();
    if (currentTime - lastInterruptTime > debounceDelay) {
        exitPressed = true;
        lastInterruptTime = currentTime;
    }
}

void IRAM_ATTR InputManager::handleRotaryButton() {
    unsigned long currentTime = millis();
    if (currentTime - lastInterruptTime > debounceDelay) {
        rotaryPressed = true;
        lastInterruptTime = currentTime;
    }
}

void InputManager::taskCode(void* pvParameters) {
    InputManager* instance = (InputManager*)pvParameters;
    
    // Für Long-Press Erkennung
    unsigned long menuPressStart = 0;
    bool menuLongPressHandled = false;

    for(;;) {
        // Menu Button Logic (Short & Long Press)
        if (digitalRead(BTN_MENU) == LOW) { // Gedrückt gehalten
            if (menuPressStart == 0) {
                menuPressStart = millis();
                menuLongPressHandled = false;
            } else if (millis() - menuPressStart > 3000 && !menuLongPressHandled) { // 3 Sekunden
                // Long Press -> Factory Reset
                Logger::info("BUTTON", "Menu LONG PRESS -> Factory Reset!");
                if (instance->configStore) {
                    instance->configStore->resetToFactory();
                    // Feedback Event? Vorerst Restart
                    ESP.restart();
                }
                menuLongPressHandled = true;
            }
        } else { // Losgelassen
            if (menuPressStart != 0 && !menuLongPressHandled) {
                // Short Press
                if (millis() - menuPressStart > 50) { // Entprellung
                     Logger::info("BUTTON", "Menu pressed!");
                     if (instance->eventQueue) {
                         DisplayEvent event = EVENT_BUTTON_MENU;
                         xQueueSend(instance->eventQueue, &event, portMAX_DELAY);
                     }
                }
            }
            menuPressStart = 0;
            menuPressed = false; // Reset ISR Flag (wir nutzen hier Polling für LongPress)
        }

        if (exitPressed) {
            exitPressed = false;
            Logger::info("BUTTON", "Exit pressed!");
            if (instance->eventQueue) {
                DisplayEvent event = EVENT_BUTTON_EXIT;
                xQueueSend(instance->eventQueue, &event, portMAX_DELAY);
            }
        }

        if (rotaryPressed) {
            rotaryPressed = false;
            Logger::info("BUTTON", "Rotary pressed!");
            if (instance->eventQueue) {
                DisplayEvent event = EVENT_BUTTON_ROTARY;
                xQueueSend(instance->eventQueue, &event, portMAX_DELAY);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
