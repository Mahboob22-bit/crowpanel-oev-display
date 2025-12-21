# Input Manager

Verwaltet die physischen Eingabemöglichkeiten des Geräts.

## Verantwortlichkeiten

1.  **Interrupt Handling:** Überwacht GPIOs für Menu, Exit und Rotary-Encoder Buttons via ISR.
2.  **Debouncing:** Entprellt die Signale per Software.
3.  **Event Dispatching:** Sendet bei Tastendruck entsprechende Events in die zentrale Queue.
4.  **Long-Press Erkennung:** Erkennt langes Drücken der MENU-Taste (> 3s) für Factory Reset.

## Hardware

*   **Menu Button:** GPIO 2 (Active Low)
*   **Exit Button:** GPIO 1 (Active Low)
*   **Rotary Switch:** GPIO 5 (Active Low)

## Abhängigkeiten

*   `DisplayEvent` (Queue)
*   `ConfigStore` (für Factory Reset)

## API

```cpp
void begin(QueueHandle_t eventQueue, ConfigStore* configStore);
```
