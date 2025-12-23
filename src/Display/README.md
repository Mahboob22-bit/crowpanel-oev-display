# Display Manager

Steuert das E-Paper Display und zeichnet die Benutzeroberfläche.

## Verantwortlichkeiten

1.  **Hardware-Abstraktion:** Kapselt die `GxEPD2` Bibliothek für das CrowPanel 4.2" E-Paper.
2.  **Power Management:** Verwaltet Deep-Sleep (Hibernate) des Displays zwischen den Updates.
3.  **UI Rendering:** Zeichnet verschiedene Screens basierend auf dem System-Status (`currentState`).
4.  **Event Handling:** Reagiert auf Events aus der Queue (z.B. Wifi Status, Button Inputs) und aktualisiert den State.

## Abhängigkeiten

*   `GxEPD2` (Hardware Treiber)
*   `SystemEvent` Enum (via `src/Core/SystemEvents.h`)
*   `TransportTypes.h` (für Fahrplandaten)

## States & Screens

*   `STATE_BOOT`: Initialer Boot-Screen mit Logo.
*   `STATE_SETUP`: Wird bei `EVENT_WIFI_AP_MODE` aktiviert. Zeigt Instruktionen zum Verbinden mit dem "CrowPanel-Setup" WLAN und die URL.
*   `STATE_DASHBOARD`: Die Hauptansicht. Zeigt:
    *   **Header:** Haltestellenname, Uhrzeit, WLAN-Signalstärke.
    *   **Tabelle:** Die nächsten 4 Abfahrten (Linie invertiert, Ziel, Minuten).
    *   **Footer:** Update-Zeitpunkt.
*   `STATE_INFO`: Informations-Screen mit URL zur Konfiguration und Platzhalter für QR-Code.
*   `STATE_ERROR`: Zeigt kritische Fehler (z.B. WLAN verloren) groß an.

## API

```cpp
void begin(QueueHandle_t eventQueue);
void update(SystemEvent event);
bool init();
void hibernate();
void wakeup();

// Data Setters
void setDepartures(const std::vector<Departure>& departures);
void setStationName(String name);
void setDataProvider(DataProvider provider);
```
