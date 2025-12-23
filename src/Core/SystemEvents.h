#ifndef SYSTEM_EVENTS_H
#define SYSTEM_EVENTS_H

/**
 * Zentrale Definition aller System-Events.
 * Dient der Entkopplung: Module müssen nicht den DisplayManager kennen,
 * um Events auszulösen.
 */
enum SystemEvent {
    // User Input
    EVENT_BUTTON_MENU,
    EVENT_BUTTON_EXIT,
    EVENT_BUTTON_ROTARY,
    
    // System Lifecycle
    EVENT_INIT,
    
    // Data & Transport
    EVENT_UPDATE_TRIGGER, // Expliziter Trigger für Update
    EVENT_DATA_AVAILABLE, // Neue Daten liegen bereit
    
    // Connectivity
    EVENT_WIFI_CONNECTED,
    EVENT_WIFI_LOST,
    EVENT_WIFI_AP_MODE,
    EVENT_INTERNET_OK,

    // Time
    EVENT_TIME_SYNCED
};

#endif // SYSTEM_EVENTS_H

