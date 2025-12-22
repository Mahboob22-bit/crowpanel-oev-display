#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <vector>
#include <functional>
#include "../Transport/TransportTypes.h"

// Display Events
enum DisplayEvent {
    EVENT_BUTTON_MENU,
    EVENT_BUTTON_EXIT,
    EVENT_BUTTON_ROTARY,
    EVENT_INIT,
    EVENT_UPDATE,
    EVENT_DATA_AVAILABLE,
    EVENT_WIFI_CONNECTED,
    EVENT_WIFI_LOST,
    EVENT_WIFI_AP_MODE,
    EVENT_INTERNET_OK
};

enum DisplayState {
    STATE_BOOT,
    STATE_SETUP,
    STATE_DASHBOARD,
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
    void update(DisplayEvent event);

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
    void drawUI(DisplayEvent event);
    
    // Screens
    void drawBootScreen();
    void drawSetupScreen();
    void drawDashboard(DisplayEvent event);
    void drawErrorScreen();

    // Helpers
    void drawHeader(String title, String rightText);
    void drawFooter(String status);
    void drawInvertedBadge(int x, int y, int w, int h, String text);
    void drawDepartureRow(int y, const Departure& dep);
};

#endif // DISPLAY_MANAGER_H
