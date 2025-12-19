#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "crowpanel_pins.h"
#include "secrets.h"
#include "Display/display_manager.h"
#include "Logger/Logger.h"
#include "Wifi/WifiManager.h"
#include "Input/InputManager.h"
#include "System/SystemMonitor.h"

// Display Treiber Instanz (muss global oder langliebend sein) (GYE042A87 für CrowPanel 4.2")
GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>
  display(GxEPD2_420_GYE042A87(EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN, EPD_BUSY_PIN));

// Module
DisplayManager displayManager(&display);
WifiManager wifiManager;
InputManager inputManager;
SystemMonitor systemMonitor;

// Globale Event Queue
QueueHandle_t displayEventQueue;

void setup() {
    Logger::init(115200);
    delay(2000); // Warten auf Serial Monitor

    Logger::info("SETUP", "\n\n====================================");
    Logger::info("SETUP", "   CrowPanel Swiss Transport Display");
    Logger::info("SETUP", "====================================\n");

    // Chip Info
    Logger::printf("SETUP", "Chip: ESP32-S3");
    Logger::printf("SETUP", "Cores: %d", ESP.getChipCores());
    Logger::printf("SETUP", "CPU Freq: %d MHz", ESP.getCpuFreqMHz());
    Logger::printf("SETUP", "Flash: %d MB", ESP.getFlashChipSize() / (1024 * 1024));
    Logger::printf("SETUP", "PSRAM: %d KB\n", ESP.getPsramSize() / 1024);

    // 1. Event Queue
    displayEventQueue = xQueueCreate(10, sizeof(DisplayEvent));
    if (displayEventQueue == NULL) {
        Logger::error("SETUP", "Failed to create event queue!");
        return; // Fatal Error
    }
    Logger::info("SETUP", "Event queue created");

    // 2. Module starten
    Logger::info("SETUP", "Starting modules...");

    // Input (Buttons)
    inputManager.begin(displayEventQueue);

    // Display
    displayManager.begin(displayEventQueue);

    // Wifi
    // Nutzt WIFI_SSID_SECRET aus secrets.h
    wifiManager.begin(WIFI_SSID_SECRET, WIFI_PASSWORD_SECRET, displayEventQueue);

    // System Monitor
    systemMonitor.begin();

    Logger::info("SETUP", "All modules started!");
    Logger::info("SETUP", "====================================\n");
}

void loop() {
    // Main Loop ist leer, alles läuft in FreeRTOS Tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}
