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
    EVENT_UPDATE
};

// Display Manager Class
class DisplayManager {
public:
    DisplayManager(GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>* disp);

    bool init();
    void hibernate();
    void wakeup();
    void update(DisplayEvent event);

private:
    GxEPD2_BW<GxEPD2_420_GYE042A87, GxEPD2_420_GYE042A87::HEIGHT>* display;
    bool initialized;
    uint32_t updateCounter;

    void drawUI(DisplayEvent event);
};

#endif // DISPLAY_MANAGER_H
