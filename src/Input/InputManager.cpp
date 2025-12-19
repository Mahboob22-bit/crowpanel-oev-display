#include "InputManager.h"
#include "crowpanel_pins.h"
#include "../Logger/Logger.h"

// Statische Member initialisieren
volatile bool InputManager::menuPressed = false;
volatile bool InputManager::exitPressed = false;
volatile bool InputManager::rotaryPressed = false;
unsigned long InputManager::lastInterruptTime = 0;
const unsigned long debounceDelay = 200;

InputManager::InputManager() : taskHandle(NULL), eventQueue(NULL) {}

void InputManager::begin(QueueHandle_t queue) {
    this->eventQueue = queue;

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
    
    for(;;) {
        if (menuPressed) {
            menuPressed = false;
            Logger::info("BUTTON", "Menu pressed!");
            if (instance->eventQueue) {
                DisplayEvent event = EVENT_BUTTON_MENU;
                xQueueSend(instance->eventQueue, &event, portMAX_DELAY);
            }
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
