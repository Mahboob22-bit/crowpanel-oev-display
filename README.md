# CrowPanel ESP32-S3 4.2" E-Paper Display - ÖV Display Projekt

Ein smartes E-Paper Display für Schweizer ÖV-Abfahrtszeiten, basierend auf dem Elecrow CrowPanel 4.2".

## 🎯 Projektziel

Ein wartungsfreies Display, das die nächsten Abfahrten einer konfigurierten Haltestelle anzeigt.
Vollständig konfigurierbar über ein Web-Interface (WLAN, Haltestelle, Linien).

## ✨ Features

- **Echtzeit-Fahrplan:** Zeigt die nächsten Verbindungen von der Schweizer Transport API (opentransportdata.swiss).
- **Web-Konfiguration:** Keine Code-Änderung nötig! WLAN und Haltestelle einfach per Browser einstellen.
- **Automatische Updates:** Aktualisiert alle 30 Sekunden (konfigurierbar).
- **NTP-Zeitsynchronisation:** Immer die exakte Uhrzeit.
- **Smart Power Management:** Nutzt Deep-Sleep/Hibernate des E-Papers zwischen den Updates.
- **Robust:** Reconnect-Logik und visuelle Fehleranzeige bei Verbindungsproblemen.

## 🛠 Hardware

- **Board:** Elecrow CrowPanel ESP32-S3 HMI 4.2" E-Paper
- **Display:** 4.2" E-Paper (400x300px, schwarz/weiß)
- **Controller:** ESP32-S3 (8MB Flash, 8MB PSRAM)

## 🏗 Architektur

Das Projekt folgt einer modularen, Event-getriebenen Architektur basierend auf FreeRTOS Tasks.
Details siehe [ARCHITECTURE.md](ARCHITECTURE.md).

### Kern-Module
- **WifiModule:** Managed Connectivity & Access Point.
- **WebConfigModule:** Stellt die SPA (Single Page Application) zur Konfiguration bereit.
- **TransportModule:** Kommuniziert mit der Transport API.
- **DisplayModule:** Steuert das E-Paper.
- **TimeModule:** Synchronisiert die Zeit via NTP.

## 🚀 Setup & Development

Das Projekt nutzt **Docker** und **Make**, um eine konsistente Entwicklungsumgebung zu gewährleisten. Es ist keine lokale Installation von PlatformIO notwendig.

### Voraussetzungen
- Docker & Docker Compose
- Make
- API Key von [opentransportdata.swiss](https://opentransportdata.swiss/) (kostenlos registrieren)
- USB-Verbindung zum ESP32 (üblicherweise `/dev/ttyUSB0`)

### Befehle

```bash
# Projekt bauen
make build

# Auf das Board flashen
make upload

# Serial Monitor starten
make monitor

# Alles in einem: Build, Upload & Monitor
make flash

# Entwicklungsumgebung bereinigen
make clean

# Für IDE-Autocompletion (z.B. clangd)
make compiledb
```

### Konfiguration (Secrets)

Erstelle eine Datei `include/secrets.h` (basierend auf `secrets.example.h` falls vorhanden), um deine WLAN-Zugangsdaten einzutragen. Diese Datei wird von Git ignoriert.

```cpp
#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID_SECRET "DeinWLAN"
#define WIFI_PASSWORD_SECRET "DeinPasswort"

#endif // SECRETS_H
```

### Erste Einrichtung (Geplant)
1. Nach dem Start spannt das Gerät ein WLAN auf: `CrowPanel-OEV`
2. Verbinden und `http://192.168.4.1` aufrufen
3. Heim-WLAN und API-Key konfigurieren
4. Haltestelle suchen und speichern

## 🔌 Pin-Belegung

### E-Paper Display
| Pin | GPIO | Funktion |
|-----|------|----------|
| PWR | 7 | Power Enable |
| BUSY| 48 | Busy Status |
| RST | 47 | Reset |
| DC | 46 | Data/Command |
| CS | 45 | Chip Select |
| SCK | 39 | SPI Clock |
| MOSI| 40 | SPI MOSI |

### Buttons
| Button | GPIO | Funktion |
|--------|------|----------|
| MENU | 2 | Menu / Config |
| EXIT | 1 | Zurück |
| ROTARY | 5 | Navigation |

## 📝 Lizenz
MIT
