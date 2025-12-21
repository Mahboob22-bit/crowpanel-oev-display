# Transport Module

Verantwortlich für die Kommunikation mit der OpenTransportData Swiss API (OJP 2.0).

## Funktionalität

Das Modul fragt periodisch (alle 30s) die API nach aktuellen Abfahrten für eine konfigurierte Haltestelle ab.

1.  **XML Request Builder:** Erstellt valide OJP 2.0 XML Anfragen.
2.  **HTTPS Client:** Sendet POST Requests an `https://api.opentransportdata.swiss/ojp2020`.
3.  **Parsing:** Nutzt `tinyxml2` (via `OjpParser`), um die XML-Antwort zu parsen und in `Departure` Objekte zu wandeln.
4.  **Config Integration:** Liest API-Key und Haltestellen-ID dynamisch aus dem `ConfigStore`.

## Thread-Safety

Da das Modul in einem eigenen Task läuft und von anderen Tasks (z.B. Display) Daten gelesen werden, sind die internen Datenstrukturen (`_departures`, `_apiKey`, `_stationId`) durch einen **Mutex** (`xSemaphoreCreateMutex`) geschützt.

## Abhängigkeiten

*   `WiFiClientSecure`
*   `HTTPClient`
*   `tinyxml2`
*   `ConfigStore`

## API

```cpp
void begin(QueueHandle_t eventQueue, ConfigStore* configStore);

// Liefert die Liste der letzten geparsten Abfahrten (Thread-safe)
std::vector<Departure> getDepartures();

// Lädt Konfiguration neu aus dem Store
void updateConfig();
```

