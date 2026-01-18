# Web Config Module

Stellt die Web-Oberfläche zur Konfiguration des Geräts bereit.

## Funktionalität

Das Modul startet einen asynchronen Webserver (`ESPAsyncWebServer`) auf Port 80 und dient als:
1.  **File Server:** Liefert die Frontend-Dateien (HTML, CSS, JS) aus dem LittleFS Dateisystem aus.
2.  **API Server:** Stellt REST-Endpunkte für das Frontend bereit.
3.  **mDNS Responder:** Macht das Gerät unter `http://crowpanel.local` im Netzwerk verfügbar.

## API Endpunkte

| Methode | Pfad | Beschreibung |
|---------|------|--------------|
| `GET` | `/api/status` | Liefert Systemstatus (IP, Mode, Heap, Config-Status, aktuelle Konfiguration). |
| `GET` | `/api/scan` | Startet einen asynchronen WLAN-Scan. |
| `GET` | `/api/scan-results` | Liefert die Ergebnisse des WLAN-Scans. |
| `GET` | `/api/stops/search?q=...` | Sucht Haltestellen via OJP API (min. 2 Zeichen). |
| `POST` | `/api/config` | Speichert neue Konfiguration (WLAN, Haltestelle, etc.) und startet neu. |
| `POST` | `/api/reset` | Führt einen Factory Reset durch. |

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
*   `TransportModule` (für Haltestellensuche)

