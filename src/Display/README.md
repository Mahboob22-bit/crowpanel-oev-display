# Display Manager

Steuert das E-Paper Display und zeichnet die Benutzeroberfläche.

## Verantwortlichkeiten

1.  **Hardware-Abstraktion:** Kapselt die `GxEPD2` Bibliothek.
2.  **Power Management:** Verwaltet Deep-Sleep (Hibernate) des Displays zwischen den Updates.
3.  **UI Rendering:** Zeichnet verschiedene Screens basierend auf dem System-Status (`STATE_BOOT`, `STATE_SETUP`, `STATE_DASHBOARD`).
4.  **Event Handling:** Reagiert auf Events aus der Queue (z.B. Wifi Status, Button Inputs).

## Abhängigkeiten

*   `GxEPD2` (Hardware Treiber)
*   `DisplayEvent` Enum (Input Queue)

## States

*   `STATE_BOOT`: Initialer Boot-Screen.
*   `STATE_SETUP`: Zeigt AP-Daten und IP-Adresse für die Erstkonfiguration.
*   `STATE_DASHBOARD`: Zeigt die Abfahrtszeiten (oder Status) im Normalbetrieb.

## API

```cpp
void begin(QueueHandle_t eventQueue);
void update(DisplayEvent event);
bool init();
```
