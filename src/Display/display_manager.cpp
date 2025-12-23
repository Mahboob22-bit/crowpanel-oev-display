#include <WiFi.h>
#include "display_manager.h"
#include "crowpanel_pins.h"
#include "../Logger/Logger.h"
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>

DisplayManager::DisplayManager(GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>* disp)
    : display(disp), initialized(false), updateCounter(0), taskHandle(NULL), eventQueue(NULL), currentState(STATE_BOOT) {
    stationName = "Station";
}

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
    SystemEvent event;

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
    display->setRotation(0);
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

void DisplayManager::setDepartures(const std::vector<Departure>& departures) {
    this->currentDepartures = departures;
}

void DisplayManager::setStationName(String name) {
    this->stationName = name;
}

void DisplayManager::setErrorMessage(String msg) {
    this->errorMessage = msg;
    this->currentState = STATE_ERROR;
}

void DisplayManager::setDataProvider(DataProvider provider) {
    this->dataProvider = provider;
}

void DisplayManager::update(SystemEvent event) {
    if (!initialized) {
        Logger::error("DISPLAY", "Not initialized!");
        return;
    }

    // Handle Data Update
    if (event == EVENT_DATA_AVAILABLE && dataProvider) {
        Logger::info("DISPLAY", "Fetching new data from provider...");
        currentDepartures = dataProvider();
        currentState = STATE_DASHBOARD; // Switch to dashboard if we get data
    }

    // State Machine Transition
    switch(event) {
        case EVENT_WIFI_AP_MODE:
            currentState = STATE_SETUP;
            break;
        case EVENT_WIFI_CONNECTED:
            // Don't force dashboard if we are already there or in error
            if (currentState == STATE_BOOT || currentState == STATE_SETUP) {
                currentState = STATE_DASHBOARD;
            }
            break;
        case EVENT_WIFI_LOST:
            currentState = STATE_ERROR;
            errorMessage = "WLAN Verbindung verloren!";
            break;
        case EVENT_INIT:
            currentState = STATE_BOOT;
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

void DisplayManager::drawUI(SystemEvent event) {
    display->fillScreen(GxEPD_WHITE);

    switch(currentState) {
        case STATE_BOOT:
            drawBootScreen();
            break;
        case STATE_SETUP:
            drawSetupScreen();
            break;
        case STATE_ERROR:
            drawErrorScreen();
            break;
        case STATE_DASHBOARD:
        default:
            drawDashboard(event);
            break;
    }
}

void DisplayManager::drawBootScreen() {
    display->setFont(&FreeMonoBold12pt7b);
    
    int16_t tbx, tby; uint16_t tbw, tbh;
    String title = "CROWPANEL OEV";
    display->getTextBounds(title, 0, 0, &tbx, &tby, &tbw, &tbh);
    display->setCursor((400 - tbw) / 2, 140);
    display->println(title);

    display->setFont(&FreeSans9pt7b);
    String sub = "v1.0 - Starting...";
    display->getTextBounds(sub, 0, 0, &tbx, &tby, &tbw, &tbh);
    display->setCursor((400 - tbw) / 2, 170);
    display->println(sub);
}

void DisplayManager::drawSetupScreen() {
    drawHeader("SETUP ERFORDERLICH", "WiFi");

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

void DisplayManager::drawErrorScreen() {
    display->drawRect(10, 10, 380, 280, GxEPD_BLACK);
    
    display->setFont(&FreeMonoBold12pt7b);
    String title = "FEHLER";
    int16_t tbx, tby; uint16_t tbw, tbh;
    display->getTextBounds(title, 0, 0, &tbx, &tby, &tbw, &tbh);
    display->setCursor((400 - tbw) / 2, 60);
    display->print(title);

    display->setFont(&FreeSans9pt7b);
    
    // Split message if too long (simple approach)
    display->setCursor(30, 120);
    display->println(errorMessage);
    
    display->setCursor(30, 200);
    display->println("Versuche Neustart...");
}

void DisplayManager::drawDashboard(SystemEvent event) {
    // Header
    struct tm timeinfo;
    char timeStr[6] = "00:00";
    if(getLocalTime(&timeinfo)){
        strftime(timeStr, 6, "%H:%M", &timeinfo);
    }
    
    drawHeader(stationName, String(timeStr));

    // Departures
    int y = 50; // Start Y position
    if (currentDepartures.empty()) {
        display->setFont(&FreeSans9pt7b);
        display->setCursor(10, 100);
        display->println("Keine Abfahrten verfuegbar...");
    } else {
        for (const auto& dep : currentDepartures) {
            if (y > 240) break; // Don't draw outside
            drawDepartureRow(y, dep);
            y += 55;
        }
    }

    // Footer
    String status;
    if (event == EVENT_WIFI_LOST) {
        status = "Offline / Verbindungsfehler";
    } else {
        char updateTimeStr[10];
        strftime(updateTimeStr, 10, "%H:%M:%S", &timeinfo);
        status = "Aktualisiert: " + String(updateTimeStr);
    }
    
    drawFooter(status);
}

// --- Helpers ---

void DisplayManager::drawHeader(String title, String rightText) {
    // Left: Title
    display->setFont(&FreeMonoBold12pt7b);
    display->setCursor(5, 30);
    display->print(title.substring(0, 15)); // Basic truncate

    // Right: Text
    display->setFont(&FreeSansBold9pt7b);
    int16_t tbx, tby; uint16_t tbw, tbh;
    display->getTextBounds(rightText, 0, 0, &tbx, &tby, &tbw, &tbh);
    display->setCursor(400 - tbw - 5, 30);
    display->print(rightText);

    // Thick Line
    display->fillRect(0, 40, 400, 3, GxEPD_BLACK);
}

void DisplayManager::drawFooter(String status) {
    // Thin Line
    display->drawLine(0, 275, 400, 275, GxEPD_BLACK);

    display->setFont(&FreeSans9pt7b);
    display->setCursor(5, 295);
    display->print(status);
}

void DisplayManager::drawInvertedBadge(int x, int y, int w, int h, String text) {
    display->fillRect(x, y, w, h, GxEPD_BLACK);
    display->setTextColor(GxEPD_WHITE);
    
    // Center text
    display->setFont(&FreeSansBold9pt7b);
    int16_t tbx, tby; uint16_t tbw, tbh;
    display->getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);
    int textX = x + (w - tbw) / 2;
    int textY = y + (h + tbh) / 2; // Approximate center

    display->setCursor(textX, textY);
    display->print(text);
    display->setTextColor(GxEPD_BLACK); // Reset
}

void DisplayManager::drawDepartureRow(int y, const Departure& dep) {
    // Line Badge
    drawInvertedBadge(10, y + 5, 50, 40, dep.line);

    // Destination
    display->setFont(&FreeSansBold9pt7b);
    display->setCursor(70, y + 30);
    display->print(dep.direction.substring(0, 18)); // Truncate

    // Time
    // Calculate minutes difference
    time_t now;
    time(&now);
    time_t depTime = dep.getEffectiveTime();
    double diffSeconds = difftime(depTime, now);
    int diffMin = (int)(diffSeconds / 60);

    String timeStr;
    if (diffMin <= 0) timeStr = "0'";
    else if (diffMin > 60) timeStr = ">1h";
    else timeStr = String(diffMin) + "'";

    display->setFont(&FreeMonoBold12pt7b);
    int16_t tbx, tby; uint16_t tbw, tbh;
    display->getTextBounds(timeStr, 0, 0, &tbx, &tby, &tbw, &tbh);
    display->setCursor(400 - tbw - 10, y + 30);
    display->print(timeStr);

    // Separator
    display->drawLine(0, y + 50, 400, y + 50, GxEPD_BLACK);
}
