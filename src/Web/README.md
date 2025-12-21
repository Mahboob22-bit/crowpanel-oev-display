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
| `GET` | `/api/status` | Liefert Systemstatus (IP, Mode, Heap, Config-Status). |
| `GET` | `/api/scan` | Startet einen WLAN-Scan und liefert gefundene Netzwerke. |
| `POST` | `/api/config` | Speichert neue Konfiguration (WLAN, Haltestelle, etc.) und startet neu. |
| `POST` | `/api/reset` | Führt einen Factory Reset durch. |

## Frontend

Das Frontend liegt im Ordner `data/` (bzw. `frontend/` im Repo Root) und ist eine Single Page Application (Vanilla JS).

*   **Setup Mode:** Zeigt nur WLAN-Konfiguration, wenn das Gerät im AP-Modus ist.
*   **Config Mode:** Zeigt vollständige Konfiguration (inkl. ÖV-Daten), wenn das Gerät mit einem Netzwerk verbunden ist.

## Abhängigkeiten

*   `ESPAsyncWebServer`
*   `AsyncTCP`
*   `ArduinoJson`
*   `LittleFS`
*   `ConfigStore`
*   `WifiManager`

