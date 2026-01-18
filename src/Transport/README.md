# Transport Module

Verantwortlich für die Kommunikation mit der OpenTransportData Swiss API (OJP 2.0).

## Funktionalität

Das Modul fragt periodisch (alle 30s) die API nach aktuellen Abfahrten für eine konfigurierte Haltestelle ab.

1.  **XML Request Builder:** Erstellt valide OJP 2.0 XML Anfragen.
2.  **HTTPS Client:** Sendet POST Requests an `https://api.opentransportdata.swiss/ojp20`.
3.  **Parsing:** Nutzt `tinyxml2` (via `OjpParser`), um die XML-Antwort zu parsen und in `Departure` Objekte zu wandeln.
4.  **Haltestellensuche:** Bietet synchrone Suche nach Haltestellen via OJP LocationInformationRequest.
5.  **Config Integration:** 
    *   **Haltestelle:** Dynamisch aus `ConfigStore`.
    *   **API Key:** Hardcoded in `secrets.h`.

## OJP 2.0 Struktur

Die OJP 2.0 API Response für StopEventRequest hat folgende Struktur:

```
StopEventResult
└── StopEvent
    ├── ThisCall
    │   └── CallAtStop
    │       └── ServiceDeparture
    │           ├── TimetabledTime (UTC)
    │           └── EstimatedTime (UTC, optional)
    │
    └── Service
        ├── PublishedServiceName → Text (Linienname)
        ├── DestinationText → Text (Ziel)
        └── Mode → PtMode (tram, bus, rail, etc.)
```

**Wichtig:** Die Zeiten werden in UTC zurückgegeben (mit `Z` Suffix). Der Parser konvertiert diese automatisch in die lokale Zeitzone des ESP32.

## Thread-Safety

Da das Modul in einem eigenen Task läuft und von anderen Tasks (z.B. Display) Daten gelesen werden, sind die internen Datenstrukturen (`_departures`, `_apiKey`, `_stationId`) durch einen **Mutex** (`xSemaphoreCreateMutex`) geschützt.

## Abhängigkeiten

*   `WiFiClientSecure`
*   `HTTPClient`
*   `tinyxml2`
*   `ConfigStore`
*   `secrets.h` (Muss `OJP_API_KEY` definieren)

## API

```cpp
void begin(QueueHandle_t eventQueue, ConfigStore* configStore);

// Liefert die Liste der letzten geparsten Abfahrten (Thread-safe)
std::vector<Departure> getDepartures();

// Lädt Konfiguration neu aus dem Store
void updateConfig();

// Weckt den Task für sofortiges Update auf
void triggerUpdate();

// Synchrone Haltestellensuche (blockiert bis Antwort da)
std::vector<StopSearchResult> searchStops(const String& query);
```

## Datentypen

```cpp
struct Departure {
    String line;          // Liniennummer (z.B. "10")
    String direction;     // Zielort (z.B. "Dornach Bahnhof")
    time_t departureTime; // Geplante Abfahrtszeit
    time_t estimatedTime; // Prognostizierte Zeit (falls verfügbar)
    String type;          // Verkehrsmittel (tram, bus, rail, etc.)
};

struct StopSearchResult {
    String id;                // z.B. "8588764"
    String name;              // z.B. "Arlesheim, Im Lee"
    String topographicPlace;  // z.B. "Arlesheim"
};
```
