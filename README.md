# CrowPanel ESP32-S3 4.2" E-Paper Display - ÖV Display Projekt

Ein smartes E-Paper Display für Schweizer ÖV-Abfahrtszeiten, basierend auf dem Elecrow CrowPanel 4.2".

## 🎯 Projektziel

Ein wartungsfreies Display, das die nächsten Abfahrten einer konfigurierten Haltestelle anzeigt.
Vollständig konfigurierbar über ein Web-Interface (WLAN, Haltestelle, Linien).

## ✨ Features

- **Echtzeit-Fahrplan:** Zeigt die nächsten Verbindungen von der Schweizer Transport API (opentransportdata.swiss).
- **Web-Konfiguration:** Keine Code-Änderung nötig! WLAN und Haltestelle einfach per Browser einstellen.
- **Einfaches Setup:** Gerät erstellt bei Erstnutzung (oder Fehler) einen Hotspot.
- **Automatische Updates:** Aktualisiert alle 30 Sekunden (konfigurierbar).
- **Robust:** Reconnect-Logik und visuelle Fehleranzeige bei Verbindungsproblemen.

## 🛠 Hardware

- **Board:** Elecrow CrowPanel ESP32-S3 HMI 4.2" E-Paper
- **Display:** 4.2" E-Paper (400x300px, schwarz/weiß)
- **Controller:** ESP32-S3 (8MB Flash, 8MB PSRAM)

## 🚀 Erste Inbetriebnahme

### 1. API Key konfigurieren
Erstelle die Datei `include/secrets.h` und trage deinen API Key ein:
```cpp
#define OJP_API_KEY "DEIN_API_KEY"
```

### 2. Flashen & Verbinden
1.  Gerät mit Strom verbinden (USB).
2.  Auf dem Display erscheint: **"Verbinde mit WLAN: CrowPanel-Setup"**.
3.  Verbinde dein Handy oder Laptop mit diesem WLAN (Kein Passwort).
4.  Öffne im Browser: **http://192.168.4.1**.
5.  Wähle dein Heim-WLAN aus und gib das Passwort ein.
6.  Das Gerät startet neu und verbindet sich.

### Konfiguration ändern

Wenn das Gerät verbunden ist, zeigt es seine IP-Adresse und URL auf dem Display an (z.B. **http://crowpanel.local**).
Öffne diese Adresse im Browser, um Haltestelle und Linien zu ändern.

### Werkseinstellungen (Reset) & Manuelles Update

Das Gerät verfügt über Tasten (Menu = Oben), die folgende Funktionen haben:

-   **Kurzer Druck (Menu):** Erzwingt eine sofortige Aktualisierung der Abfahrtszeiten.
-   **Langer Druck (Menu > 3s):** Löscht alle Einstellungen und startet wieder im Setup-Modus.

Falls du das WLAN wechselst oder Fehler auftreten:
- **Hardware:** Halte die **MENU**-Taste (oben) für **3 Sekunden** gedrückt.
- **Software:** Über den "Reset"-Button im Web-Interface.

Das Gerät löscht alle Einstellungen und startet wieder im Setup-Modus.

## 🏗 Architektur

Das Projekt folgt einer modularen, Event-getriebenen Architektur basierend auf FreeRTOS Tasks.
Details siehe [ARCHITECTURE.md](ARCHITECTURE.md).

### Kern-Module
- **[WifiModule](src/Wifi/README.md):** Managed Connectivity & Access Point.
- **[WebConfigModule](src/Web/README.md):** Stellt die SPA (Single Page Application) zur Konfiguration bereit.
- **[TransportModule](src/Transport/README.md):** Kommuniziert mit der Transport API.
- **[DisplayModule](src/Display/README.md):** Steuert das E-Paper.
- **[ConfigStore](src/Core/README.md):** Speichert Einstellungen persistent im NVS.
- **[InputManager](src/Input/README.md):** Verarbeitet Tasten-Eingaben.
- **[SystemMonitor](src/System/README.md):** Überwacht Systemressourcen.
- **[Logger](src/Logger/README.md):** Zentrales Logging.

## 💻 Development

Voraussetzungen: **Docker** & **Make**.

```bash
# Bauen
make build

# Flashen (Firmware)
make upload

# Filesystem (Frontend) hochladen -> WICHTIG nach Änderungen im data/ Ordner!
make uploadfs

# Serial Monitor
make monitor
```

## 📝 Lizenz
MIT
