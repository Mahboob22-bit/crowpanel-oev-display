#include "display_manager.h"
#include "crowpanel_pins.h"
#include "../Logger/Logger.h"
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

DisplayManager::DisplayManager(GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>* disp)
    : display(disp), initialized(false), updateCounter(0) {}

bool DisplayManager::init() {
    // Display Power ON
    pinMode(EPD_PWR_PIN, OUTPUT);
    digitalWrite(EPD_PWR_PIN, HIGH);
    delay(100);

    Logger::info("DISPLAY", "Initializing E-Paper Display...");

    // Display initialisieren
    display->init(115200, true, 2, false); // initial, reset, delay, pulldown
    display->setRotation(1);  // Landscape
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

    wakeup();

    Logger::printf("DISPLAY", "Updating (Event: %d, Count: %d)...", event, updateCounter);

    display->setFullWindow();
    display->firstPage();
    do {
        drawUI(event);
    } while (display->nextPage());

    updateCounter++;

    Logger::info("DISPLAY", "Update complete!");

    // Nach Update wieder in Hibernate
    hibernate();
}

void DisplayManager::drawUI(DisplayEvent event) {
    display->fillScreen(GxEPD_WHITE);

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
        case EVENT_BUTTON_MENU:
            display->println("MENU");
            break;
        case EVENT_BUTTON_EXIT:
            display->println("EXIT");
            break;
        case EVENT_BUTTON_ROTARY:
            display->println("ROTARY");
            break;
        case EVENT_INIT:
            display->println("INIT");
            break;
        case EVENT_UPDATE:
            display->println("UPDATE");
            break;
        default:
            display->println("UNKNOWN");
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

    display->setCursor(10, 165);
    display->print("PSRAM: ");
    display->print(ESP.getPsramSize() / (1024 * 1024));
    display->println(" MB");

    // Status
    display->setFont(&FreeMonoBold12pt7b);
    display->setCursor(10, 220);
    display->println("Bereit!");

    // Footer
    display->setFont(&FreeSans9pt7b);
    display->setCursor(10, 285);
    display->println("Taste druecken zum Aktualisieren");
}
