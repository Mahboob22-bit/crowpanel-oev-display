# Systemarchitektur: CrowPanel ÖV-Display

## 1. Übersicht

Das System ist modular aufgebaut. Die Firmware auf dem ESP32 besteht aus unabhängigen Modulen (Tasks), die lose gekoppelt sind und über definierte Events oder Queues kommunizieren. Die Benutzeroberfläche zur Konfiguration ist als Single Page Application (SPA) realisiert, die vom ESP32 ausgeliefert wird.

## 2. Firmware Architektur (ESP32)

Die Firmware ist in funktionale Module unterteilt. Jedes Modul kapselt seine Logik und läuft (wo sinnvoll) in einem eigenen FreeRTOS Task. `main.cpp` dient nur zur Initialisierung und "Verdrahtung".

### 2.1 Module

| Modul | Verantwortung | Interaktion |
|-------|---------------|-------------|
| **WifiManager** | Verwaltet WLAN-Verbindung (Station Mode) und Access Point (AP Mode). Reconnect-Logik. | Meldet: `EVENT_WIFI_CONNECTED`, `EVENT_WIFI_LOST`. |
| **InputManager** | Verwaltet Buttons (Menu, Exit, Rotary). Entprellt Signale (Debounce) und feuert Events. | Meldet: `EVENT_BUTTON_MENU`, `EVENT_BUTTON_EXIT`, `EVENT_BUTTON_ROTARY`. |
| **SystemMonitor** | Überwacht Systemressourcen (Heap, Stack, Uptime) und loggt diese periodisch. | Loggt via `Logger`. |
| **TimeModule** | Synchronisiert Systemzeit via NTP. | Meldet: `EVENT_TIME_SYNCED`. Stellt `getLocalTime()` bereit. |
| **WebConfigModule** | Startet Webserver. Stellt REST-API bereit. Liefert Frontend-Files aus. | Liest/Schreibt: `ConfigStore`. Triggered: `EVENT_CONFIG_CHANGED`. |
| **TransportModule** | Fragt periodisch (oder bei Event) die Transport-API ab. Parst JSON. | Trigger: Timer (30s). Meldet: `EVENT_DATA_NEW`, `EVENT_DATA_ERROR`. Liest: `ConfigStore`. |
| **DisplayManager** | Verwaltet E-Paper Hardware. Zeichnet UI basierend auf Status. | Hört auf: `EVENT_DATA_NEW`, `EVENT_DATA_ERROR`, `EVENT_WIFI_...`. Verwaltet Power-Modes. |
| **ConfigStore** | Persistente Speicherung (NVS/Preferences). | Wird von allen Modulen gelesen. Geschrieben von `WebConfigModule`. |

### 2.2 Datenfluss & Kommunikation

Die Kommunikation erfolgt primär über eine zentrale **Event Queue** oder direkte Task-Notifications, um Thread-Safety zu gewährleisten.

```mermaid
graph TD
    %% Events
    EventQ{Event Queue}

    %% Modules
    Wifi[WifiManager] -->|Wifi Status| EventQ
    Input[InputManager] -->|Button Press| EventQ
    Web[WebConfigModule] -->|Config Changed| EventQ
    Time[TimeModule] -->|Time Synced| EventQ
    
    Transport[TransportModule]
    EventQ -->|Trigger Update| Transport
    Transport -->|New Data / Error| EventQ
    
    Display[DisplayManager]
    EventQ -->|All Events| Display
```

## 3. Web Frontend Architektur

Die Konfigurationsoberfläche ist eine leichtgewichtige Web-App (Vanilla JS), die im Flash-Speicher des ESP32 (LittleFS) liegt.

### 3.1 Struktur

*   **API Kommunikation:** Direkte `fetch` Calls an die REST-API (`/api/config`, `/api/scan`, `/api/status`, `/api/reset`).
*   **Logik:** `app.js` steuert den Ablauf.
*   **Views:** Dynamische DOM-Manipulation in `index.html` basierend auf dem Status (AP-Mode vs. Connected).

## 4. Technologie-Stack

*   **Firmware:** C++17, PlatformIO, Arduino Framework, FreeRTOS.
*   **Libraries:** ArduinoJson (Parsing), GxEPD2 (Display), ESPAsyncWebServer (Web/API), LittleFS (Dateisystem).
*   **Frontend:** HTML5, CSS3, Modern JS (ES6+). Keine Build-Tools notwendig.

## 5. Entwicklung neuer Module

Um die Wartbarkeit und Stabilität zu gewährleisten, müssen neue Module folgenden Richtlinien folgen:

### 5.1 Prinzipien
1.  **Kapselung:** Jedes Modul erhält einen eigenen Ordner in `src/`. Die gesamte Logik muss innerhalb der Klasse gekapselt sein. Globale Variablen sind zu vermeiden.
2.  **Autonomie:** Module sollten eigenständig funktionieren. Abhängigkeiten zu anderen Modulen sollten minimiert und explizit (z.B. per Dependency Injection im `begin()`) übergeben werden.
3.  **Minimaler Main-Code:** Die `main.cpp` dient **ausschließlich** der Instanziierung und dem Starten (`begin()`) der Module. Keine Logik in `setup()` oder `loop()`.
4.  **FreeRTOS Tasks:** Wenn ein Modul dauerhaft laufen oder periodisch Aufgaben erledigen muss, startet es intern seinen eigenen FreeRTOS Task. Die `taskCode` Methode muss `static` sein.
5.  **Logging:** Verwende ausschließlich die `Logger`-Klasse, keine direkten `Serial.print`.
6.  **Dokumentation:** Jedes Modul muss eine `README.md` in seinem Ordner enthalten, die die Verantwortlichkeiten, Abhängigkeiten und public API beschreibt.

### 5.2 Template für ein neues Modul
