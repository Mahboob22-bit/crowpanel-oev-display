# Changelog

## [1.3.0] - 2026-02-04
### Added
- **Intelligente Linienauswahl:** Automatische Abfrage verfügbarer Linien für ausgewählte Haltestelle via neuer API `/api/lines?stopId=...`.
- **Live-Abfahrtsanzeige:** Neuer `/api/departures` Endpunkt zeigt aktuelle Abfahrten (gleiche Daten wie E-Paper Display).
- **Web-Live-View:** Zeigt konfigurierte Linien und alle Abfahrten in Echtzeit mit Auto-Refresh (30s).
- **Dropdown-basierte Linienauswahl:** Ersetzt manuelle Texteingabe durch gruppierte Dropdown-Menüs mit Verkehrsmittel-Icons.
- **Keyboard-Navigation:** Pfeiltasten (↑/↓), Enter und Escape für die Haltestellensuche.
- **Favoriten-Feature:** Speichert die letzten 5 Haltestellen in LocalStorage mit Schnellauswahl-Chips (⭐).
- **Verkehrsmittel-Icons:** Visuelle Unterscheidung durch Emojis und Farben (🚋 Tram rot, 🚌 Bus blau, 🚆 Zug grün).
- **Toast-Notifications:** Moderne Benachrichtigungen statt Browser-Alerts.
- **Mobile Optimierung:** Touch-freundliche Buttons (min 44px), größere Input-Felder, verbesserte Scroll-Performance.
- **Loading-Spinner:** CSS-Animation während Linien geladen werden.
- **Bestätigungsdialoge:** Verbesserte Nutzerführung beim Speichern und Reset mit Countdown.

### Changed
- **TransportTypes:** Neue `LineInfo` Struktur für Linien-Metadaten (line, direction, type).
- **TransportModule:** Neue Methode `getAvailableLines(stopId)` für synchrone Linienabfrage.
- **WebConfigModule:** Neue Routes `/api/lines` (Query-Parameter) und `/api/departures` (Echtzeit-Daten).
- **Frontend-Struktur:** Dropdowns statt manuelle Eingabe, gruppiert nach Verkehrsmittel-Typ.
- **UX-Flow:** Linien werden automatisch nach Haltestellenauswahl geladen.
- **Live-Daten:** WebApp zeigt nun dieselben Abfahrten wie das E-Paper Display.

### Improved
- **Benutzerfreundlichkeit:** Keine manuellen Fehleingaben mehr bei Linien und Richtungen.
- **Accessibility:** Vollständige Keyboard-Steuerung für Haltestellensuche.
- **Responsive Design:** Media Queries für Bildschirme < 600px.
- **Error Handling:** Bessere Fehlermeldungen mit Retry-Möglichkeit.
- **API-Robustheit:** Query-Parameter statt Regex-Routes für bessere Kompatibilität.

### Fixed
- **API-Route:** `/api/lines` verwendet nun Query-Parameter statt URL-Pattern (ESPAsyncWebServer Kompatibilität).
- **Linien-Dropdown:** Wird nun korrekt nach Haltestellenauswahl angezeigt.

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
