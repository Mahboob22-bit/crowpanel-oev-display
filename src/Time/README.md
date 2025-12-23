# TimeModule

Das `TimeModule` ist verantwortlich für die Synchronisation der Systemzeit über NTP (Network Time Protocol) und die Bereitstellung von formatierten Zeitstempeln.

## Funktionen

*   **NTP Synchronisation:** Holt die aktuelle Zeit von NTP-Servern (`pool.ntp.org`, `time.nist.gov`).
*   **Zeitzonen-Management:** Konfiguriert die lokale Zeitzone (Standard: Schweizer Zeit `CET-1CEST,M3.5.0,M10.5.0/3`).
*   **Status-Überwachung:** Prüft periodisch, ob die Zeit synchronisiert wurde.
*   **Event-Signalisierung:** Feuert `EVENT_TIME_SYNCED`, sobald eine gültige Zeit verfügbar ist.
*   **Ressourcenschonend:** Nutzt einen eigenen FreeRTOS Task, der sich schlafen legt, wenn die Zeit synchronisiert ist.
*   **Race-Condition-Schutz:** Wartet mit der NTP-Konfiguration, bis eine aktive WLAN-Verbindung besteht.

## Abhängigkeiten

*   `WifiManager` (indirekt: benötigt aktive Internetverbindung)
*   `SystemEvents` (für `EVENT_TIME_SYNCED`)
*   `Logger`

## API

### `void begin(QueueHandle_t eventQueue)`
Initialisiert das Modul und startet den Hintergrund-Task.
*   `eventQueue`: Queue zum Senden von System-Events.

### `String getFormattedTime()`
Gibt die aktuelle lokale Zeit als String im Format `YYYY-MM-DD HH:MM:SS` zurück.
Gibt `"Not Synced"` zurück, wenn die Zeit noch nicht synchronisiert wurde.

## Interne Arbeitsweise

1.  Der Task wartet zunächst in einer Schleife, bis `WiFi.status() == WL_CONNECTED`.
2.  Sobald verbunden, wird `configTime()` aufgerufen, um den ESP32-NTP-Client zu starten.
3.  Der Task prüft dann periodisch (jede Sekunde), ob das Jahr > 2020 ist.
4.  Sobald dies der Fall ist, wird `isSynced = true` gesetzt, das Event gefeuert und das Abfrageintervall auf 60 Sekunden erhöht.

