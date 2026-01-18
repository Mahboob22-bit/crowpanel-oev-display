# Changelog

## [1.2.0] - 2026-01-16
### Added
- **Haltestellensuche:** Neuer API-Endpunkt `/api/stops/search` für OJP LocationInformationRequest.
- **Frontend Autocomplete:** Interaktive Haltestellensuche mit Dropdown im Web-Interface.
- **Standardwerte:** Automatische Konfiguration von "Arlesheim, Im Lee" (Tram 10) bei Erststart.
- **Status API erweitert:** `/api/status` liefert jetzt auch die aktuelle Stations- und Linien-Konfiguration.

### Fixed
- **OJP 2.0 Parser:** Komplette Überarbeitung für korrekte XML-Struktur (CallAtStop, PublishedServiceName).
- **Zeitzonenkorrektur:** UTC-Zeiten aus der API werden jetzt korrekt in lokale Zeit konvertiert.
- **Config-Überschreibung:** Leere Felder im Frontend überschreiben nicht mehr die vorhandene Konfiguration.
- **API-Route Reihenfolge:** API-Endpunkte werden jetzt vor serveStatic registriert.
- **NULL-Pointer Crashes:** Alle verketteten XML-Aufrufe haben jetzt NULL-Checks.

### Changed
- **OJP Endpoint:** Wechsel von `/ojp2020` zu `/ojp20` (OJP 2.0).
- **WebConfigModule:** Erhält jetzt auch TransportModule per Dependency Injection.

## [1.1.0] - 2024-12-21
### Added
- **Web Interface:** Vollständige Konfiguration via Web-Browser.
- **Access Point Mode:** Startet automatisch `CrowPanel-Setup` AP, wenn keine Verbindung möglich ist.
- **mDNS Support:** Erreichbar unter `http://crowpanel.local`.
- **Factory Reset:** 
  - Via Web-Interface Button.
  - Via Hardware Button: MENU-Taste 3 Sekunden gedrückt halten.
- **Display Feedback:**
  - Zeigt IP-Adresse und Hostname im Dashboard an.
  - Zeigt Setup-Anweisungen im AP-Modus an.

## [1.0.0] - 2024-12-10
### Initiale Version
- Anzeige von ÖV-Daten auf E-Paper.
- Modulare Architektur mit FreeRTOS.
