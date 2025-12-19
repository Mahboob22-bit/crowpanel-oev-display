#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "crowpanel_pins.h"
#include "Display/display_manager.h"
#include "Logger/Logger.h"

// Display initialisieren (GYE042A87 für CrowPanel 4.2")
GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>
  display(GxEPD2_420_GYE042A87(EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN, EPD_BUSY_PIN));

// Display Manager
DisplayManager displayManager(&display);

// FreeRTOS Handles
TaskHandle_t TaskDisplay;
TaskHandle_t TaskSerial; // Name behalten, aber Code nutzt Logger
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
        Logger::error("TASK_DISPLAY", "Display initialization failed!");
        vTaskDelete(NULL);
        return;
    }

    // Initiales Display-Update
    displayManager.update(EVENT_INIT);

    // Event Loop
    for(;;) {
        // Warte auf Events aus der Queue (blocking)
        if (xQueueReceive(displayEventQueue, &event, portMAX_DELAY) == pdTRUE) {
            Logger::printf("TASK_DISPLAY", "Display event received: %d", event);
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
            Logger::info("BUTTON", "Menu pressed!");

            DisplayEvent event = EVENT_BUTTON_MENU;
            xQueueSend(displayEventQueue, &event, portMAX_DELAY);
        }

        if (exitPressed) {
            exitPressed = false;
            Logger::info("BUTTON", "Exit pressed!");

            DisplayEvent event = EVENT_BUTTON_EXIT;
            xQueueSend(displayEventQueue, &event, portMAX_DELAY);
        }

        if (rotaryPressed) {
            rotaryPressed = false;
            Logger::info("BUTTON", "Rotary pressed!");

            DisplayEvent event = EVENT_BUTTON_ROTARY;
            xQueueSend(displayEventQueue, &event, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // 50ms polling
    }
}

// System Monitoring Task
void TaskSystemMonitorCode(void *pvParameters) {
    for(;;) {
        Logger::printf("SYSTEM", "Core %d | Heap: %d KB | Stack: %d bytes",
                     xPortGetCoreID(),
                     ESP.getFreeHeap() / 1024,
                     uxTaskGetStackHighWaterMark(NULL));

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void setup() {
    Logger::init(115200);
    delay(2000);

    Logger::info("SETUP", "\n\n====================================");
    Logger::info("SETUP", "   CrowPanel Swiss Transport Display");
    Logger::info("SETUP", "====================================\n");

    // Chip Info
    Logger::printf("SETUP", "Chip: ESP32-S3");
    Logger::printf("SETUP", "Cores: %d", ESP.getChipCores());
    Logger::printf("SETUP", "CPU Freq: %d MHz", ESP.getCpuFreqMHz());
    Logger::printf("SETUP", "Flash: %d MB", ESP.getFlashChipSize() / (1024 * 1024));
    Logger::printf("SETUP", "PSRAM: %d KB\n", ESP.getPsramSize() / 1024);

    // Button Setup mit Interrupts
    Logger::info("SETUP", "Configuring buttons...");
    pinMode(BTN_MENU, INPUT_PULLUP);
    pinMode(BTN_EXIT, INPUT_PULLUP);
    pinMode(BTN_ROTARY_SW, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(BTN_MENU), handleMenuButton, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_EXIT), handleExitButton, FALLING);
    attachInterrupt(digitalPinToInterrupt(BTN_ROTARY_SW), handleRotaryButton, FALLING);

    Logger::info("SETUP", "Buttons configured!");

    // Event Queue erstellen (max 10 Events)
    displayEventQueue = xQueueCreate(10, sizeof(DisplayEvent));
    if (displayEventQueue == NULL) {
        Logger::error("SETUP", "Failed to create event queue!");
        return;
    }
    Logger::info("SETUP", "Event queue created!");

    // FreeRTOS Tasks erstellen
    Logger::info("SETUP", "Creating tasks...");

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
        TaskSystemMonitorCode,
        "SystemTask",
        4096,
        NULL,
        1,                 // Niedrigere Priorität
        &TaskSerial,
        1                  // Core 1
    );

    Logger::info("SETUP", "Tasks created successfully!\n");
    Logger::info("SETUP", "====================================");
    Logger::info("SETUP", "System ready! Press buttons to update display.");
    Logger::info("SETUP", "====================================\n");
}

void loop() {
    // Alles läuft in FreeRTOS Tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}
