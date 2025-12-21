# System Monitor

Überwacht den Gesundheitszustand des Systems.

## Verantwortlichkeiten

1.  **Ressourcen-Monitoring:** Loggt periodisch (alle 5s) den Zustand von:
    *   Free Heap
    *   Min Free Heap (High Water Mark)
    *   Stack Usage
2.  **Watchdog:** (Optional/Geplant) Könnte System resetten bei Hängern.

## API

```cpp
void begin();
```
