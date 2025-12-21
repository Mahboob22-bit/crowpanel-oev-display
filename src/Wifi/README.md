# Wifi Manager

Verwaltet die WLAN-Verbindung und den Access Point Modus.

## Verantwortlichkeiten

1.  **Station Mode (STA):** Verbindet sich mit einem konfigurierten WLAN (SSID/Password aus `ConfigStore`).
2.  **Access Point Mode (AP):** Startet einen AP ("CrowPanel-Setup", IP 192.168.4.1), wenn:
    *   Keine Konfiguration im `ConfigStore` gefunden wird.
    *   Die Verbindung zum gespeicherten WLAN fehlschlägt (Timeout).
3.  **Auto-Reconnect:** Versucht bei Verbindungsabbruch automatisch eine Wiederherstellung.
4.  **Internet Check:** Prüft nach erfolgreicher Verbindung einmalig die Internet-Konnektivität (via HTTP Request zu Google).

## Abhängigkeiten

*   `WiFi.h`, `HTTPClient.h` (ESP32 Core)
*   `ConfigStore` (für SSID/Password)
*   `Logger` (für Ausgaben)
*   `DisplayEvent` (Queue für Status-Meldungen an Display)

## Events

Das Modul sendet folgende Events an die `displayEventQueue`:

*   `EVENT_WIFI_CONNECTED`: Erfolgreich mit WLAN verbunden.
*   `EVENT_WIFI_LOST`: Verbindung verloren.
*   `EVENT_WIFI_AP_MODE`: Access Point gestartet (Setup erforderlich).
*   `EVENT_INTERNET_OK`: Internet-Verbindung bestätigt.

## API

```cpp
void begin(ConfigStore* configStore, QueueHandle_t eventQueue);
WifiState getState();
String getIpAddress(); // Gibt IP (Station) oder 192.168.4.1 (AP) zurück
```
