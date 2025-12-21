# Input Manager

Verwaltet die physischen Eingabemöglichkeiten des Geräts.

## Verantwortlichkeiten

1.  **Interrupt Handling:** Überwacht GPIOs für Menu, Exit und Rotary-Encoder Buttons.
2.  **Debouncing:** Entprellt die Signale per Software, um Mehrfachauslösungen zu verhindern.
3.  **Event Dispatching:** Sendet bei Tastendruck entsprechende Events in die zentrale Queue.

## Hardware

*   **Menu Button:** GPIO 2
*   **Exit Button:** GPIO 1
*   **Rotary Switch:** GPIO 5

## Abhängigkeiten

*   `DisplayEvent` (Queue)

## API

```cpp
void begin(QueueHandle_t eventQueue);
```
