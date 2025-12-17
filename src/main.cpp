#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "crowpanel_pins.h"
#include "display_manager.h"

// Display initialisieren (GYE042A87 für CrowPanel 4.2")
GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>
  display(GxEPD2_420_GYE042A87(EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN, EPD_BUSY_PIN));

// Display Manager
DisplayManager displayManager(&display);

// FreeRTOS Handles
TaskHandle_t TaskDisplay;
TaskHandle_t TaskSerial;
QueueHandle_t displayEventQueue;

// Interrupt Flags (volatile für ISR)
volatile bool menuPressed = false;
volatile bool exitPressed = false;
volatile bool rotaryPressed = false;

// Debounce
unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200; // ms

// Button Interrupt Handlers
void IRAM_ATTR handleMenuButton() {
    unsigned long currentTime = millis();
    if (currentTime - lastInterruptTime > debounceDelay) {
        menuPressed = true;
        lastInterruptTime = currentTime;
    }
}

void IRAM_ATTR handleExitButton() {
    unsigned long currentTime = millis();
    if (currentTime - lastInterruptTime > debounceDelay) {
        exitPressed = true;
        lastInterruptTime = currentTime;
    }
}

void IRAM_ATTR handleRotaryButton() {
    unsigned long currentTime = millis();
    if (currentTime - lastInterruptTime > debounceDelay) {
        rotaryPressed = true;
        lastInterruptTime = currentTime;
    }
}

// Display Task
void TaskDisplayCode(void *pvParameters) {
    DisplayEvent event;

    // Display initialisieren
    if (!displayManager.init()) {
        Serial.println("[ERROR] Display initialization failed!");
        vTaskDelete(NULL);
        return;
    }

    // Initiales Display-Update
    displayManager.update(EVENT_INIT);

    // Event Loop
    for(;;) {
        // Warte auf Events aus der Queue (blocking)
        if (xQueueReceive(displayEventQueue, &event, portMAX_DELAY) == pdTRUE) {
            Serial.printf("[TASK] Display event received: %d\n", event);
            displayManager.update(event);
        }
    }
}

// Button Monitor Task
void TaskButtonMonitorCode(void *pvParameters) {
    for(;;) {
        // Check button flags
        if (menuPressed) {
            menuPressed = false;
            Serial.println("[BUTTON] Menu pressed!");

            DisplayEvent event = EVENT_BUTTON_MENU;
            xQueueSend(displayEventQueue, &event, portMAX_DELAY);
        }

        if (exitPressed) {
            exitPressed = false;
            Serial.println("[BUTTON] Exit pressed!");

            DisplayEvent event = EVENT_BUTTON_EXIT;
            xQueueSend(displayEventQueue, &event, portMAX_DELAY);
        }

        if (rotaryPressed) {
            rotaryPressed = false;
            Serial.println("[BUTTON] Rotary pressed!");

            DisplayEvent event = EVENT_BUTTON_ROTARY;
            xQueueSend(displayEventQueue, &event, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms polling
    }
}

// Serial Monitoring Task
void TaskSerialCode(void *pvParameters) {
    for(;;) {
        Serial.printf("[SYSTEM] Core %d | Heap: %d KB | Stack: %d bytes\n",
                     xPortGetCoreID(),
                     ESP.getFreeHeap() / 1024,
                     uxTaskGetStackHighWaterMark(NULL));

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n\n====================================");
    Serial.println("   CrowPanel Swiss Transport Display");
    Serial.println("====================================\n");

    // Chip Info
    Serial.printf("Chip: ESP32-S3\n");
    Serial.printf("Cores: %d\n", ESP.getChipCores());
    Serial.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
    Serial.printf("PSRAM: %d KB\n\n", ESP.getPsramSize() / 1024);

    // Button Setup mit Interrupts
    Serial.println("[SETUP] Configuring buttons...");
    pinMode(BTN_MENU, INPUT_PULLUP);
    pinMode(BTN_EXIT, INPUT_PULLUP);
    pinMode(BTN_ROTARY_SW, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(BTN_MENU), handleMenuButton, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_EXIT), handleExitButton, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_ROTARY_SW), handleRotaryButton, FALLING);

    Serial.println("[SETUP] Buttons configured!");

    // Event Queue erstellen (max 10 Events)
    displayEventQueue = xQueueCreate(10, sizeof(DisplayEvent));
    if (displayEventQueue == NULL) {
        Serial.println("[ERROR] Failed to create event queue!");
        return;
    }
    Serial.println("[SETUP] Event queue created!");

    // FreeRTOS Tasks erstellen
    Serial.println("[SETUP] Creating tasks...");

    xTaskCreatePinnedToCore(
        TaskDisplayCode,
        "DisplayTask",
        10240,             // Mehr Stack für Display + GxEPD2
        NULL,
        2,                 // Höhere Priorität
        &TaskDisplay,
        0                  // Core 0
    );

    xTaskCreatePinnedToCore(
        TaskButtonMonitorCode,
        "ButtonTask",
        4096,
        NULL,
        2,                 // Hohe Priorität für schnelle Button-Response
        NULL,
        0                  // Core 0
    );

    xTaskCreatePinnedToCore(
        TaskSerialCode,
        "SerialTask",
        4096,
        NULL,
        1,                 // Niedrigere Priorität
        &TaskSerial,
        1                  // Core 1
    );

    Serial.println("[SETUP] Tasks created successfully!\n");
    Serial.println("====================================");
    Serial.println("System ready! Press buttons to update display.");
    Serial.println("====================================\n");
}

void loop() {
    // Alles läuft in FreeRTOS Tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}
