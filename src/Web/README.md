# Web Config Module

Stellt die Web-Oberfläche zur Konfiguration des Geräts bereit.

## Funktionalität

Das Modul startet einen asynchronen Webserver (`ESPAsyncWebServer`) auf Port 80 und dient als:
1.  **File Server:** Liefert die Frontend-Dateien (HTML, CSS, JS) aus dem LittleFS Dateisystem aus.
2.  **API Server:** Stellt REST-Endpunkte für das Frontend bereit.
3.  **mDNS Responder:** Macht das Gerät unter `http://crowpanel.local` im Netzwerk verfügbar.

## Authentifizierung

Endpunkte mit sensiblen Daten sind durch HTTP Basic Auth geschützt (Username: `admin`, Passwort: konfigurierbar via `/api/config` Feld `web_password`).

**Kein Schutz** wenn:
- Das Gerät im AP-Mode ist (Ersteinrichtung muss funktionieren)
- Kein Web-Passwort gesetzt ist (Standard nach Factory Reset)

| Endpunkt | Auth erforderlich |
|----------|------------------|
| `/api/status` | Ja (wenn Passwort gesetzt) |
| `/api/device` | Ja (wenn Passwort gesetzt) |
| `/api/config` (POST) | Ja |
| `/api/reset` (POST) | Ja |
| `/api/scan`, `/api/scan-results` | Nein |
| `/api/departures` | Nein |

## API Endpunkte

| Methode | Pfad | Beschreibung |
|---------|------|--------------|
| `GET` | `/api/status` | Systemstatus (IP, Mode, Heap, Config, `device_id`, `fw_version`). |
| `GET` | `/api/device` | Geräteinformationen (Device-ID, FW-Version, Flash, PSRAM, Uptime). |
| `GET` | `/api/scan` | Startet einen asynchronen WLAN-Scan. |
| `GET` | `/api/scan-results` | Liefert die Ergebnisse des WLAN-Scans. |
| `GET` | `/api/stops/search?q=...` | Sucht Haltestellen (min. 2, max. 50 Zeichen). |
| `GET` | `/api/lines?stopId=...` | Liefert verfügbare Linien einer Haltestelle (max. 20 Zeichen StopId). |
| `GET` | `/api/departures` | Liefert aktuelle Abfahrten (gleiche Daten wie auf dem Display). |
| `POST` | `/api/config` | Speichert neue Konfiguration und startet neu (max. 1024 Bytes). |
| `POST` | `/api/reset` | Führt einen Factory Reset durch. |

### `/api/config` — Akzeptierte Felder

```json
{
  "ssid": "...",           // max. 32 Zeichen
  "password": "...",       // max. 64 Zeichen
  "web_password": "...",   // max. 64 Zeichen (leer = Schutz deaktivieren)
  "station": { "name": "...", "id": "..." },
  "line1": { "name": "...", "dir": "..." },
  "line2": { "name": "...", "dir": "..." }
}
```

### `/api/device` — Response

```json
{
  "device_id": "CP-A1B2C3D4E5F6",
  "fw_version": "1.3.0",
  "hw": "ESP32-S3",
  "flash_mb": 8,
  "psram_kb": 8192,
  "heap_free": 245000,
  "uptime_s": 3600
}
```

### Live-Abfahrten

Der Endpunkt `/api/departures` liefert die aktuellen Abfahrten direkt vom `TransportModule`:

**Response:**
```json
{
  "departures": [
    {"line": "10", "direction": "Flueh, Station", "type": "tram", "minutes": 3},
    {"line": "10", "direction": "Dornach Bahnhof", "type": "tram", "minutes": 8}
  ],
  "count": 4,
  "timestamp": 1707000000
}
```

Dies sind dieselben Daten, die auch auf dem E-Paper Display angezeigt werden.

### Haltestellensuche

Der Endpunkt `/api/stops/search` ermöglicht die Suche nach Schweizer ÖV-Haltestellen:

**Request:**
```
GET /api/stops/search?q=Bern
```

**Response:**
```json
{
  "results": [
    {"id": "8507000", "name": "Bern", "location": "Bern"},
    {"id": "8507100", "name": "Bern, Bahnhof", "location": "Bern"}
  ]
}
```

Die Suche nutzt intern `TransportModule::searchStops()` und die OJP 2.0 LocationInformationRequest API.

## Frontend

Das Frontend liegt im Ordner `data/` und ist eine Single Page Application (Vanilla JS).

*   **Setup Mode:** Zeigt nur WLAN-Konfiguration, wenn das Gerät im AP-Modus ist.
*   **Config Mode:** Zeigt vollständige Konfiguration (inkl. ÖV-Daten), wenn das Gerät mit einem Netzwerk verbunden ist.

### Haltestellensuche im Frontend

Das Frontend bietet eine interaktive Haltestellensuche mit Autocomplete:

1. User tippt in das Suchfeld (mind. 2 Zeichen)
2. Nach 300ms Debouncing wird `/api/stops/search` aufgerufen
3. Ergebnisse werden als Dropdown angezeigt
4. Bei Auswahl werden Name und ID in versteckte Felder übernommen

## Abhängigkeiten

*   `ESPAsyncWebServer`
*   `AsyncTCP`
*   `ArduinoJson`
*   `LittleFS`
*   `ConfigStore`
*   `WifiManager`
*   `TransportModule` (für Haltestellensuche und Abfahrten)
*   `DeviceIdentity` (für Geräte-ID und FW-Version)

