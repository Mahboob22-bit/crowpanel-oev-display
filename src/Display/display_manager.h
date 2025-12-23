#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <vector>
#include <functional>
#include "../Transport/TransportTypes.h"
#include "../Core/SystemEvents.h"

enum DisplayState {
    STATE_BOOT,
    STATE_SETUP,
    STATE_DASHBOARD,
    STATE_INFO,
    STATE_ERROR
};

// Display Manager Class
class DisplayManager {
public:
    DisplayManager(GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>* disp);

    // Startet den Display-Task
    void begin(QueueHandle_t eventQueue);

    bool init();
    void hibernate();
    void wakeup();
    void update(SystemEvent event);

    // Data Setters
    void setDepartures(const std::vector<Departure>& departures);
    void setStationName(String name);
    void setErrorMessage(String msg);
    
    using DataProvider = std::function<std::vector<Departure>()>;
    void setDataProvider(DataProvider provider);

private:
    static void taskCode(void* pvParameters);
    
    GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>* display;
    bool initialized;
    uint32_t updateCounter;
    TaskHandle_t taskHandle;
    QueueHandle_t eventQueue;
    DisplayState currentState;

    // Data
    std::vector<Departure> currentDepartures;
    String stationName;
    String errorMessage;
    DataProvider dataProvider;

    // Drawing Methods
    void drawUI(SystemEvent event);
    
    // Screens
    void drawBootScreen();
    void drawSetupScreen();
    void drawDashboard(SystemEvent event);
    void drawInfoScreen();
    void drawErrorScreen();

    // Helpers
    void drawHeader(String title, String rightText);
    void drawFooter(String status);
    void drawInvertedBadge(int x, int y, int w, int h, String text);
    void drawDepartureRow(int y, const Departure& dep);
    void drawWifiSignal(int x, int y, int rssi);
};

#endif // DISPLAY_MANAGER_H
