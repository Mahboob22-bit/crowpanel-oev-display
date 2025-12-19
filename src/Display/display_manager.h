#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <GxEPD2_BW.h>

// Display Events
enum DisplayEvent {
    EVENT_BUTTON_MENU,
    EVENT_BUTTON_EXIT,
    EVENT_BUTTON_ROTARY,
    EVENT_INIT,
    EVENT_UPDATE,
    EVENT_WIFI_CONNECTED,
    EVENT_WIFI_LOST,
    EVENT_INTERNET_OK
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

private:
    static void taskCode(void* pvParameters);
    
    GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>* display;
    bool initialized;
    uint32_t updateCounter;
    TaskHandle_t taskHandle;
    QueueHandle_t eventQueue;

    void drawUI(DisplayEvent event);
};

#endif // DISPLAY_MANAGER_H
