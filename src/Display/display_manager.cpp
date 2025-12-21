#include "display_manager.h"
#include "crowpanel_pins.h"
#include "../Logger/Logger.h"
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

DisplayManager::DisplayManager(GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>* disp)
    : display(disp), initialized(false), updateCounter(0), taskHandle(NULL), eventQueue(NULL), currentState(STATE_BOOT) {}

void DisplayManager::begin(QueueHandle_t queue) {
    this->eventQueue = queue;

    xTaskCreatePinnedToCore(
        taskCode,
        "DisplayTask",
        10240,
        this,
        2,
        &taskHandle,
        0
    );
}

void DisplayManager::taskCode(void* pvParameters) {
    DisplayManager* instance = (DisplayManager*)pvParameters;
    DisplayEvent event;

    if (!instance->init()) {
        Logger::error("TASK_DISPLAY", "Display initialization failed!");
        vTaskDelete(NULL);
        return;
    }

    instance->update(EVENT_INIT);

    for(;;) {
        if (xQueueReceive(instance->eventQueue, &event, portMAX_DELAY) == pdTRUE) {
            Logger::printf("TASK_DISPLAY", "Display event received: %d", event);
            instance->update(event);
        }
    }
}

bool DisplayManager::init() {
    pinMode(EPD_PWR_PIN, OUTPUT);
    digitalWrite(EPD_PWR_PIN, HIGH);
    delay(100);

    Logger::info("DISPLAY", "Initializing E-Paper Display...");

    display->init(115200, true, 2, false);
    display->setRotation(1);
    display->setTextColor(GxEPD_BLACK);

    initialized = true;
    Logger::info("DISPLAY", "Initialization successful!");

    return true;
}

void DisplayManager::hibernate() {
    if (!initialized) return;

    display->hibernate();
    digitalWrite(EPD_PWR_PIN, LOW);
    Logger::info("DISPLAY", "Hibernating...");
}

void DisplayManager::wakeup() {
    if (!initialized) return;

    digitalWrite(EPD_PWR_PIN, HIGH);
    delay(10);
    Logger::info("DISPLAY", "Waking up...");
}

void DisplayManager::update(DisplayEvent event) {
    if (!initialized) {
        Logger::error("DISPLAY", "Not initialized!");
        return;
    }

    // State Machine Transition
    switch(event) {
        case EVENT_WIFI_AP_MODE:
            currentState = STATE_SETUP;
            break;
        case EVENT_WIFI_CONNECTED:
            currentState = STATE_DASHBOARD;
            break;
        default:
            // Bleibe im aktuellen State
            break;
    }

    wakeup();

    Logger::printf("DISPLAY", "Updating (Event: %d, State: %d)...", event, currentState);

    display->setFullWindow();
    display->firstPage();
    do {
        drawUI(event);
    } while (display->nextPage());

    updateCounter++;

    Logger::info("DISPLAY", "Update complete!");

    hibernate();
}

void DisplayManager::drawUI(DisplayEvent event) {
    display->fillScreen(GxEPD_WHITE);

    if (currentState == STATE_SETUP) {
        drawSetupScreen();
    } else {
        drawDashboard(event);
    }
}

void DisplayManager::drawSetupScreen() {
    display->setFont(&FreeMonoBold12pt7b);
    display->setCursor(10, 30);
    display->println("Einrichtung erforderlich");
    
    display->drawLine(0, 40, 400, 40, GxEPD_BLACK);
    
    display->setFont(&FreeSans9pt7b);
    display->setCursor(10, 80);
    display->println("1. Verbinde mit WLAN:");
    
    display->setFont(&FreeSansBold9pt7b);
    display->setCursor(30, 110);
    display->println("CrowPanel-Setup");
    
    display->setFont(&FreeSans9pt7b);
    display->setCursor(10, 150);
    display->println("2. Oeffne im Browser:");
    
    display->setFont(&FreeSansBold9pt7b);
    display->setCursor(30, 180);
    display->println("http://192.168.4.1");
    
    display->setFont(&FreeSans9pt7b);
    display->setCursor(10, 240);
    display->println("Folge den Anweisungen auf dem");
    display->setCursor(10, 260);
    display->println("Bildschirm zur Konfiguration.");
}

void DisplayManager::drawDashboard(DisplayEvent event) {
    // Header
    display->setFont(&FreeMonoBold12pt7b);
    display->setCursor(10, 30);
    display->println("CrowPanel OeV Display");

    // Trennlinie
    display->drawLine(0, 40, 400, 40, GxEPD_BLACK);

    // Event Info
    display->setFont(&FreeSans9pt7b);
    display->setCursor(10, 65);
    display->print("Letztes Event: ");

    switch(event) {
        case EVENT_BUTTON_MENU: display->println("MENU"); break;
        case EVENT_BUTTON_EXIT: display->println("EXIT"); break;
        case EVENT_BUTTON_ROTARY: display->println("ROTARY"); break;
        case EVENT_INIT: display->println("INIT"); break;
        case EVENT_UPDATE: display->println("UPDATE"); break;
        case EVENT_WIFI_CONNECTED: display->println("WIFI CONNECTED"); break;
        case EVENT_WIFI_LOST: display->println("WIFI LOST"); break;
        case EVENT_INTERNET_OK: display->println("INTERNET OK!"); break;
        default: display->println("UNKNOWN");
    }

    // Update Counter
    display->setCursor(10, 90);
    display->print("Update #");
    display->println(updateCounter);

    // System Info
    display->setCursor(10, 115);
    display->print("CPU: ");
    display->print(ESP.getCpuFreqMHz());
    display->println(" MHz");

    display->setCursor(10, 140);
    display->print("Free Heap: ");
    display->print(ESP.getFreeHeap() / 1024);
    display->println(" KB");
    
    // Status
    display->setFont(&FreeMonoBold12pt7b);
    display->setCursor(10, 220);
    
    if (currentState == STATE_DASHBOARD) {
        display->println("Online");
    } else {
        display->println("Booting...");
    }

    // Footer
    display->setFont(&FreeSans9pt7b);
    display->setCursor(10, 285);
    display->println("Taste druecken zum Aktualisieren");
}
