# CrowPanel ESP32-S3 4.2" E-Paper Display - ÖV Display Projekt

Projekt für das Elecrow CrowPanel 4.2" E-Paper HMI Display mit ESP32-S3.

## Hardware

- **Board**: ESP32-S3-WROOM-1-N8R8
- **Display**: 4.2" E-Paper (400x300px, schwarz/weiß)
- **Flash**: 8 MB
- **PSRAM**: 8 MB (OPI Mode)
- **CPU**: Dual-Core @ 240 MHz

## Features

✅ Event-driven Display Updates (nur bei Button-Press)
✅ Power-Management für E-Paper (Hibernate zwischen Updates)
✅ FreeRTOS Multi-Tasking (3 Tasks auf 2 Cores)
✅ Button-Interrupt-System mit Debouncing
✅ Hardware-UART für stabile Serial-Kommunikation
✅ Strukturierter Code mit DisplayManager-Klasse

## Architektur

### FreeRTOS Tasks

| Task | Core | Stack | Priority | Funktion |
|------|------|-------|----------|----------|
| DisplayTask | 0 | 10KB | 2 | E-Paper Display Updates |
| ButtonTask | 0 | 4KB | 2 | Button Monitoring & Events |
| SerialTask | 1 | 4KB | 1 | System Monitoring |

### Event-System

- Button-Interrupts → Flags setzen
- ButtonTask überprüft Flags → Events in Queue
- DisplayTask wartet auf Queue → Update Display
- Display geht in Hibernate → Stromsparen!

## Pin-Belegung

### E-Paper Display

| Pin | GPIO | Funktion |
|-----|------|----------|
| PWR | 7 | Power Enable |
| BUSY | 48 | Busy Status |
| RST | 47 | Reset |
| DC | 46 | Data/Command |
| CS | 45 | Chip Select |
| SCK | 39 | SPI Clock |
| MOSI | 40 | SPI MOSI |

### Buttons

| Button | GPIO | Funktion |
|--------|------|----------|
| MENU | 2 | Menu-Taste |
| EXIT | 1 | Exit-Taste |
| ROTARY_SW | 5 | Rotary Switch |

## Development

### Voraussetzungen

- Docker & Docker Compose
- USB-Verbindung zum ESP32-S3

### Build & Upload

```bash
# Projekt bauen
make build

# Auf Board hochladen
make upload

# Serial Monitor starten
make monitor

# Alles zusammen
make flash
```

### Entwicklungsumgebung

Das Projekt nutzt ein Docker-basiertes PlatformIO-Setup für konsistente Builds.

## Verwendung

1. Board mit USB verbinden
2. `make flash` ausführen
3. Buttons drücken um Display zu aktualisieren:
   - **MENU** (GPIO 2): Menu-Event
   - **EXIT** (GPIO 1): Exit-Event
   - **ROTARY** (GPIO 5): Rotary-Event

Das Display zeigt:
- Letztes Button-Event
- Update-Counter
- System-Informationen (CPU, RAM, PSRAM)
- Status-Nachricht

## Code-Struktur

```
├── include/
│   ├── crowpanel_pins.h      # Pin-Definitionen
│   ├── display_manager.h     # Display Manager Header
│   └── stubs/                # LSP Stubs
├── src/
│   ├── main.cpp              # Hauptprogramm
│   └── display_manager.cpp   # Display Manager Implementation
├── scripts/                   # Build-Scripts
├── platformio.ini            # PlatformIO-Konfiguration
└── docker-compose.yml        # Docker-Setup
```

## Wichtige Hinweise

### E-Paper Display Lebensdauer

⚠️ **E-Paper Displays haben eine begrenzte Anzahl von Refresh-Zyklen!**

- Dieses Setup aktualisiert nur bei Button-Press
- Display geht zwischen Updates in Hibernate
- Kein kontinuierliches Refresh wie bei LCD/OLED

### Power-Management

Das Display wird automatisch:
- Eingeschaltet vor dem Update
- Aktualisiert mit Full-Refresh
- In Hibernate versetzt (Strom aus)

### Serial-Kommunikation

Das Board nutzt **Hardware-UART** (nicht USB-CDC):
- Stabiler als USB-CDC auf ESP32-S3
- 115200 Baud
- `/dev/ttyUSB0` (Linux)

## Troubleshooting

### Display zeigt nichts

1. SPI-Pins überprüfen (evtl. GPIO 12/11 statt 39/40)
2. Display-Power-Pin prüfen (GPIO 7)
3. Serial-Log auf Fehler checken

### Buttons reagieren nicht

1. Pull-Up-Widerstände aktiviert? (INPUT_PULLUP)
2. Richtige GPIOs? (siehe crowpanel_pins.h)
3. Debounce-Delay evtl. erhöhen

### Stack-Overflow

1. Stack-Größen in main.cpp erhöhen
2. Debug-Level reduzieren (CORE_DEBUG_LEVEL)
3. Serial-Monitor auf Backtrace prüfen

## Nächste Schritte

- [ ] Swiss Transport API Integration
- [ ] WiFi-Verbindung
- [ ] Echtzeit-ÖV-Daten anzeigen
- [ ] Rotary Encoder für Navigation
- [ ] Konfiguration über Buttons

## Lizenz

Dieses Projekt ist Open Source.

## Credits

- Hardware: Elecrow CrowPanel
- Display Library: GxEPD2
- Framework: Arduino ESP32 + FreeRTOS
