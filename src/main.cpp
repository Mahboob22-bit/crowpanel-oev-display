#include <Arduino.h>
#include <GxEPD2_BW.h>
#include "crowpanel_pins.h"
#include "Display/display_manager.h"
#include "Logger/Logger.h"
#include "Wifi/WifiManager.h"
#include "Input/InputManager.h"
#include "System/SystemMonitor.h"
#include "Transport/TransportModule.h"
#include "Core/ConfigStore.h"
#include "Core/StringUtils.h"
#include "Web/WebConfigModule.h"
#include "Time/TimeModule.h"
#include "Core/SystemEvents.h"

// Display Treiber Instanz (GYE042A87 für CrowPanel 4.2")
GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>
  display(GxEPD2_420_GYE042A87(EPD_CS_PIN, EPD_DC_PIN, EPD_RST_PIN, EPD_BUSY_PIN));

// Module
DisplayManager displayManager(&display);
WifiManager wifiManager;
InputManager inputManager;
SystemMonitor systemMonitor;
TransportModule transportModule;
ConfigStore configStore;
WebConfigModule webConfigModule;
TimeModule timeModule;

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

    // 0. Config Store
    configStore.begin();

    // 1. Event Queue
    displayEventQueue = xQueueCreate(10, sizeof(SystemEvent));
    if (displayEventQueue == NULL) {
        Logger::error("SETUP", "Failed to create event queue!");
        return; // Fatal Error
    }
    Logger::info("SETUP", "Event queue created");

    // 2. Module starten
    Logger::info("SETUP", "Starting modules...");

    // Input (Buttons)
    inputManager.begin(displayEventQueue, &configStore, &transportModule);

    // Display
    displayManager.begin(displayEventQueue);
    
    // Data Provider verknüpfen
    displayManager.setDataProvider([]() -> std::vector<Departure> {
        return transportModule.getDepartures();
    });
    
    // Initialen Stationsnamen setzen
    StationConfig station = configStore.getStation();
    if (station.name.length() > 0) {
        // Nur Stationsname ohne Ortsangabe
        String shortName = StringUtils::getStationNameOnly(station.name);
        displayManager.setStationName(shortName);
    } else {
        displayManager.setStationName("Nicht konfiguriert");
    }

    // Wifi
    wifiManager.begin(&configStore, displayEventQueue);

    // Time Module (NTP)
    timeModule.begin(displayEventQueue);

    // Web Config
    webConfigModule.begin(&configStore, &wifiManager, &transportModule);

    // System Monitor
    systemMonitor.begin();

    // Transport Module (Test)
    transportModule.begin(displayEventQueue, &configStore);


    Logger::info("SETUP", "All modules started!");
    Logger::info("SETUP", "====================================\n");
}

void loop() {
    // Main Loop ist leer, alles läuft in FreeRTOS Tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}
