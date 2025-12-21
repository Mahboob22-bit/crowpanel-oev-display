# Logger

Zentrales Logging-Modul.

## Verantwortlichkeiten

1.  **Abstraktion:** Kapselt `Serial.print` Aufrufe.
2.  **Formatierung:** Fügt Tags und Zeilenumbrüche einheitlich hinzu.
3.  **Leveling:** Unterscheidet (semantisch) zwischen Info und Error.

## API

```cpp
static void init(unsigned long baudRate);
static void info(const char* tag, const char* message);
static void error(const char* tag, const char* message);
static void printf(const char* tag, const char* format, Args... args);
```
