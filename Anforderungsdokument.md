# 📋 Anforderungsdokument: CrowPanel ÖV-Display

**Projekt:** Schweizer ÖV-Abfahrtsanzeige auf E-Paper Display  
**Hardware:** Elecrow CrowPanel 4.2" E-Paper HMI Display (ESP32-S3)  
**Datum:** Februar 2026  
**Version:** 2.0

---

## 1. Projektziel

Ein E-Paper Display, das aktuelle Abfahrtszeiten des öffentlichen Verkehrs in der Schweiz anzeigt. Das Gerät wird als fertiges Produkt verkauft und über eine Web-Oberfläche konfiguriert. Die Firmware ist Open Source, der kommerzielle Mehrwert liegt im vorkonfigurierten Gerät, dem OTA-Update-Service und dem Backend (API-Proxy, OTA-Server). Geräte im Feld werden über OTA-Updates (Over-The-Air) automatisch aktualisiert.

---

## 2. Funktionale Anforderungen

### 2.1 Anzeige der ÖV-Daten

**F-01:** Das System zeigt Abfahrtszeiten einer konfigurierbaren Haltestelle an.

**F-02:** Das System zeigt zwei verschiedene Linien (bzw. Fahrtrichtungen) gleichzeitig an.

**F-03:** Pro Linie werden die nächsten zwei Abfahrten angezeigt.

**F-04:** Für jede Abfahrt werden folgende Informationen angezeigt:
- Liniennummer/-bezeichnung
- Zielort
- Verbleibende Minuten bis zur Abfahrt

**F-05:** Das Display aktualisiert die Daten alle 30 Sekunden von der Datenquelle. (Hinweis: Benötigt API-Key von opentransportdata.swiss für dieses Intervall)

**F-06:** Das Display wird nur visuell aktualisiert, wenn sich die anzuzeigenden Informationen geändert haben.

**F-06b:** Der Benutzer kann durch kurzen Tastendruck eine manuelle Aktualisierung erzwingen.

### 2.2 Konfiguration

**F-07:** Die Haltestelle ist über eine Web-Oberfläche einstellbar.

**F-08:** Die anzuzeigenden Linien sind über eine Web-Oberfläche auswählbar.

**F-09:** Das System stellt beim ersten Start einen eigenen WLAN-Zugangspunkt bereit, über den die WLAN-Verbindung konfiguriert werden kann.

**F-10:** Nach erfolgreicher WLAN-Konfiguration verbindet sich das System automatisch mit dem konfigurierten WLAN.

**F-11:** Die Web-Oberfläche ist nach der WLAN-Verbindung über das lokale Netzwerk erreichbar.

**F-12:** Alle Einstellungen bleiben nach einem Neustart erhalten.

**F-13:** Es gibt eine Möglichkeit, das System auf Werkseinstellungen zurückzusetzen.

### 2.3 Darstellung

**F-14:** Die Anzeige erfolgt in Tabellenform ähnlich einer echten VBZ-Anzeige.

**F-15:** Die Darstellung ist klar strukturiert und gut lesbar.

**F-16:** Die Benutzeroberfläche (Web-Oberfläche und Display) ist in Deutsch verfügbar.

**F-17:** Die Benutzeroberfläche ist in Englisch verfügbar.

**F-18:** Das System ist so aufgebaut, dass weitere Sprachen hinzugefügt werden können.

### 2.4 Datenquelle

**F-19:** Das System bezieht die ÖV-Daten von der offiziellen Schweizer Transport-API.

**F-20:** Das System funktioniert mit allen Haltestellen, die in der Schweizer Transport-API verfügbar sind.

**F-21:** Das System synchronisiert die Uhrzeit automatisch über NTP.

**F-22:** Das Display zeigt die aktuelle WLAN-Signalstärke an.

**F-23:** Es gibt einen Info-Screen, der die Konfigurations-URL und (optional) einen QR-Code anzeigt.

**F-24:** Die Web-Oberfläche zeigt die gleichen Abfahrtszeiten wie das Display an (Live-Vorschau mit Auto-Refresh).

**F-25:** Umlaute werden auf dem E-Paper Display korrekt als ASCII-Äquivalente dargestellt (ä→ae, ö→oe, etc.).

**F-26:** Die Linienauswahl erfolgt über Dropdown-Menüs mit automatisch geladenen verfügbaren Linien.

**F-27:** Die Web-Oberfläche unterstützt Keyboard-Navigation (Pfeiltasten, Enter, Escape) für die Haltestellensuche.

**F-28:** Die Web-Oberfläche speichert die letzten 5 Haltestellen als Favoriten im Browser (LocalStorage).

### 2.5 OTA-Updates (Over-The-Air)

**F-29:** Das System prüft automatisch nachts (Standard: 02:00–05:00 Uhr) auf neue Firmware-Versionen.

**F-30:** Firmware-Updates werden automatisch im Hintergrund heruntergeladen und installiert, ohne Benutzerinteraktion.

**F-31:** Bei einem fehlgeschlagenen Update führt das System automatisch einen Rollback auf die vorherige Firmware-Version durch.

**F-32:** Der OTA-Server unterstützt Staged Rollouts. Ausgewählte Test-Geräte erhalten Updates zuerst; nach erfolgreicher Validierung werden alle Geräte aktualisiert.

**F-33:** Das Update-Fenster (Uhrzeit, in der Updates geprüft und installiert werden) ist serverseitig konfigurierbar.

**F-34:** Die Web-Oberfläche zeigt die aktuelle Firmware-Version und den letzten Update-Status an (z.B. "Aktuell", "Update verfügbar", "Update fehlgeschlagen").

### 2.6 Geräte-Identität & Versionierung

**F-35:** Jedes Gerät besitzt eine eindeutige Device-ID, die aus der MAC-Adresse des ESP32 abgeleitet wird.

**F-36:** Die Firmware enthält eine semantische Versionsnummer (SemVer: MAJOR.MINOR.PATCH), die zur Laufzeit abfragbar ist.

**F-37:** Das Gerät meldet seine Device-ID und aktuelle Firmware-Version bei jedem Update-Check an den OTA-Server.

### 2.7 API-Proxy

**F-38:** Die ÖV-Daten werden über einen eigenen, selbst gehosteten Proxy-Server bezogen. Das Gerät kommuniziert nicht direkt mit der OJP-API.

**F-39:** Der API-Key für opentransportdata.swiss wird ausschliesslich auf dem Proxy-Server gespeichert und ist zu keinem Zeitpunkt auf dem Endgerät vorhanden.

### 2.8 Sicherheit

**F-40:** Alle Netzwerkkommunikation zwischen Gerät und Backend (OTA-Server, API-Proxy) erfolgt über HTTPS mit Zertifikatsvalidierung.

**F-41:** Firmware-Updates werden kryptographisch signiert. Das Gerät verifiziert die Signatur vor der Installation und lehnt unsignierte oder manipulierte Firmware ab.

**F-42:** Die Web-Oberfläche ist durch ein konfigurierbares Passwort geschützt, um unbefugten Zugriff im lokalen Netzwerk zu verhindern.

---

## 3. Optionale Anforderungen (Nice-to-Have)

**O-01:** Das System zeigt Störungsmeldungen des öffentlichen Verkehrs an.

---

## 4. Nicht-Funktionale Anforderungen

### 4.1 Benutzerfreundlichkeit

**NF-01:** Die Konfiguration über die Web-Oberfläche muss intuitiv bedienbar sein.

**NF-02:** Die Web-Oberfläche ist ohne Anleitung verwendbar.

**NF-03:** Die Anzeige auf dem Display ist auch aus mehreren Metern Entfernung gut lesbar.

### 4.2 Zuverlässigkeit

**NF-04:** Das System läuft stabil im Dauerbetrieb.

**NF-05:** Bei Verbindungsproblemen zur Datenquelle zeigt das System ein Warn-Icon über den bestehenden (veralteten) Daten an, anstatt die Anzeige zu löschen.

**NF-06:** Bei Verbindungsproblemen zum WLAN versucht das System automatisch eine Neuverbindung.

### 4.3 Wartbarkeit

**NF-07:** Die Konfiguration kann jederzeit über die Web-Oberfläche geändert werden.

**NF-08:** Das System kann über die Web-Oberfläche neu gestartet werden.

### 4.4 Technisch

**NF-09:** Das System wird über USB mit Strom versorgt.

**NF-10:** Die Anzeige nutzt die E-Paper Technologie des Displays.

**NF-11:** Das System ist für Dauerbetrieb ausgelegt.

### 4.5 Sicherheit

**NF-12:** Die Firmware wird mit Espressif Secure Boot V2 signiert, um die Integrität sicherzustellen.

**NF-13:** Alle HTTPS-Verbindungen nutzen Root-CA-Validierung. Selbstsignierte Zertifikate ohne Validierung (`setInsecure()`) sind im Produktions-Build nicht erlaubt.

**NF-14:** WiFi-Passwörter werden verschlüsselt im NVS (Non-Volatile Storage) gespeichert.

### 4.6 Entwicklung & Build

**NF-15:** Es gibt ein Entwicklungs-Build-Profil mit USB-Flash, Serial-Debug-Ausgabe und verbose Logging (CORE_DEBUG_LEVEL=3).

**NF-16:** Es gibt ein Produktions-Build-Profil mit OTA-Fähigkeit, reduziertem Logging (CORE_DEBUG_LEVEL=1) und Firmware-Signierung.

**NF-17:** Beide Build-Profile nutzen die gleiche Partitionstabelle, damit die Firmware im Feld sowohl via USB als auch via OTA aktualisiert werden kann.

---

## 5. Abgrenzungen (Explizit NICHT enthalten)

**A-01:** Keine Batterie-/Akkubetrieb im ersten Release.

**A-02:** Keine Touchscreen-Steuerung - Konfiguration nur über Web-Oberfläche.

**A-03:** Keine Anzeige von Fahrplänen - nur Echtzeit-Abfahrten.

**A-04:** Keine Routenplanung oder Verbindungssuche.

**A-05:** Keine Integration mit Smart Home Systemen.

**A-06:** Keine mobile App - nur Web-Oberfläche.

**A-07:** Kein Device-Management-Dashboard im ersten Release. Geräte werden manuell vom Hersteller registriert.

**A-08:** Keine automatische Crash-Telemetrie im ersten Release. Coredumps werden lokal gespeichert und können bei Bedarf manuell ausgelesen werden.

---

## 6. Anwendungsfälle

### UC-01: Erste Inbetriebnahme
1. Benutzer schließt das Gerät an Strom an
2. Gerät startet eigenen WLAN-Zugangspunkt
3. Benutzer verbindet sich mit dem WLAN des Geräts
4. Web-Oberfläche öffnet sich automatisch
5. Benutzer gibt WLAN-Zugangsdaten ein
6. Gerät verbindet sich mit dem WLAN

### UC-02: Haltestelle konfigurieren
1. Benutzer öffnet Web-Oberfläche im Browser
2. Benutzer wählt Haltestelle aus oder sucht danach
3. Benutzer wählt zwei Linien aus
4. Benutzer speichert die Einstellungen
5. Display zeigt die konfigurierten Abfahrtszeiten an

### UC-03: Normale Nutzung
1. Display zeigt aktuelle Abfahrtszeiten an
2. System aktualisiert Daten alle 30 Sekunden
3. Bei Änderungen wird das Display aktualisiert
4. Benutzer kann jederzeit die nächsten Abfahrten ablesen

### UC-04: Zurücksetzen
1. Benutzer möchte Gerät neu konfigurieren
2. Benutzer löst Reset aus
3. Gerät kehrt in Ersteinrichtungs-Modus zurück
4. Siehe UC-01

### UC-05: Automatisches OTA-Update
1. Gerät prüft nachts (innerhalb des Update-Fensters) den OTA-Server auf eine neue Version
2. OTA-Server antwortet mit Versionsinformationen und Download-URL
3. Gerät lädt die signierte Firmware herunter
4. Gerät verifiziert die Signatur der Firmware
5. Gerät flasht die neue Firmware auf die inaktive OTA-Partition
6. Gerät startet neu mit der neuen Firmware
7. Gerät meldet den erfolgreichen Update-Status an den OTA-Server

### UC-06: Fehlgeschlagenes Update mit Rollback
1. Gerät lädt ein Update herunter und flasht es (siehe UC-05, Schritte 1–5)
2. Gerät startet neu mit der neuen Firmware
3. Die neue Firmware erkennt einen Fehler (z.B. Boot-Loop, keine Netzwerkverbindung)
4. Nach dem fehlgeschlagenen Boot markiert das System die Partition als ungültig
5. Gerät startet automatisch mit der vorherigen (funktionierenden) Firmware neu
6. Gerät meldet den fehlgeschlagenen Update-Status an den OTA-Server

### UC-07: Gerät erstmalig provisionieren (Hersteller)
1. Hersteller flasht die Firmware und das LittleFS-Dateisystem via USB
2. Gerät startet und generiert automatisch eine eindeutige Device-ID
3. Hersteller notiert die Device-ID (z.B. via Serial Monitor oder Display)
4. Hersteller registriert die Device-ID auf dem OTA-Server
5. Gerät ist bereit für den Versand an den Kunden
6. Kunde führt Ersteinrichtung durch (siehe UC-01)

---

## 7. Datenmodell (Logisch)

### Konfiguration
- WLAN-Zugangsdaten (SSID, Passwort — verschlüsselt gespeichert)
- Haltestelle (Name, ID)
- Linie 1 (Bezeichnung, Richtung)
- Linie 2 (Bezeichnung, Richtung)
- Sprache
- Aktualisierungsintervall
- Web-Passwort (für Zugangsschutz)

### Geräte-Identität
- Device-ID (eindeutig, abgeleitet aus MAC-Adresse)
- Firmware-Version (SemVer: MAJOR.MINOR.PATCH)
- Update-Channel (z.B. "test" oder "stable")
- Letzter Update-Status (Erfolg/Fehlgeschlagen/Kein Update)
- Letzter Update-Zeitpunkt

### Anzuzeigende Daten (pro Linie)
- Linienbezeichnung
- Ziel
- Abfahrtszeit 1 (in Minuten)
- Abfahrtszeit 2 (in Minuten)

---

## 8. Erfolgskriterien

Das Projekt gilt als erfolgreich, wenn:

✅ **K-01:** Eine beliebige Schweizer ÖV-Haltestelle über die Web-Oberfläche konfiguriert werden kann.

✅ **K-02:** Die Abfahrtszeiten korrekt und aktuell auf dem Display angezeigt werden.

✅ **K-03:** Die Konfiguration von einer technisch nicht versierten Person ohne Anleitung durchgeführt werden kann.

✅ **K-04:** Das System mindestens 24 Stunden ohne Unterbrechung stabil läuft.

✅ **K-05:** Die Web-Oberfläche auf verschiedenen Geräten (Smartphone, Tablet, PC) funktioniert.

**K-06:** Ein OTA-Update kann erfolgreich vom Server auf das Gerät übertragen, verifiziert und installiert werden.

**K-07:** Bei einem fehlgeschlagenen Update führt das Gerät einen automatischen Rollback auf die vorherige Firmware durch und bleibt funktionsfähig.

**K-08:** Das Gerät bezieht ÖV-Daten ausschliesslich über den API-Proxy, ohne dass ein API-Key auf dem Gerät gespeichert ist.

---

## 9. Offene Fragen

- **Q-01:** Welche E-Mail-Adresse wird als Support-Kanal für Kunden bei Update-Problemen eingerichtet? (Platzhalter: support@example.com)
- **Q-02:** Welcher Signatur-Algorithmus soll für die Firmware-Signierung verwendet werden? (Empfehlung: ECDSA mit secp256r1 oder RSA-2048)
- **Q-03:** Soll Flash Encryption (Verschlüsselung des gesamten Flash-Speichers) zusätzlich zu Secure Boot aktiviert werden?

---

## 10. Änderungshistorie

| Version | Datum | Änderung | Autor |
|---------|-------|----------|-------|
| 1.0 | 2024-12 | Initiale Version | Mahboob |
| 1.1 | 2024-12 | Ergänzung NTP, Warn-Icon | Assistant |
| 1.2 | 2024-12 | Manuelles Update per Taster, Input Refactoring | Assistant |
| 1.3 | 2024-12 | Display Layout (Tabelle, Signalstärke, Info Screen) | Assistant |
| 1.4 | 2026-01 | Haltestellensuche im Web-Interface, OJP 2.0 Parser-Fix, Zeitzonenkorrektur | Assistant |
| 1.5 | 2026-02 | WebApp Live-Abfahrten, Umlaute-Konvertierung, Stationsname-Bereinigung | Assistant |
| 2.0 | 2026-02 | OTA-Updates, API-Proxy, Geräte-Identität, Sicherheit, Build-Profile, Provisionierung | Assistant |

