# Display Manager

Steuert das E-Paper Display und zeichnet die Benutzeroberfläche.

## Verantwortlichkeiten

1.  **Hardware-Abstraktion:** Kapselt die `GxEPD2` Bibliothek für das CrowPanel 4.2" E-Paper.
2.  **Power Management:** Verwaltet Deep-Sleep (Hibernate) des Displays zwischen den Updates.
3.  **UI Rendering:** Zeichnet verschiedene Screens basierend auf dem System-Status (`currentState`).
4.  **Event Handling:** Reagiert auf Events aus der Queue (z.B. Wifi Status, Button Inputs) und aktualisiert den State.

## Abhängigkeiten

*   `GxEPD2` (Hardware Treiber)
*   `DisplayEvent` Enum (Input Queue)

## States & Screens

*   `STATE_BOOT`: Initialer Boot-Screen.
*   `STATE_SETUP`: Wird bei `EVENT_WIFI_AP_MODE` aktiviert. Zeigt Instruktionen zum Verbinden mit dem "CrowPanel-Setup" WLAN und die URL.
*   `STATE_DASHBOARD`: Wird bei `EVENT_WIFI_CONNECTED` aktiviert. Zeigt:
    *   System Status (IP, Hostname).
    *   ÖV-Daten (sobald verfügbar).
    *   Logs/Status (aktuell implementiert).

## API

```cpp
void begin(QueueHandle_t eventQueue);
void update(DisplayEvent event);
bool init();
void hibernate();
void wakeup();
```
