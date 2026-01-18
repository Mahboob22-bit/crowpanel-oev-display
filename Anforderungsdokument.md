# 📋 Anforderungsdokument: CrowPanel ÖV-Display

**Projekt:** Schweizer ÖV-Abfahrtsanzeige auf E-Paper Display  
**Hardware:** Elecrow CrowPanel 4.2" E-Paper HMI Display (ESP32-S3)  
**Datum:** Dezember 2024  
**Version:** 1.0

---

## 1. Projektziel

Ein E-Paper Display, das aktuelle Abfahrtszeiten des öffentlichen Verkehrs in der Schweiz anzeigt. Die Anzeige soll über eine Web-Oberfläche konfigurierbar sein und sich automatisch aktualisieren.

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

**(Geplant für Serie):** Der API-Zugriff erfolgt später über einen eigenen Proxy-Server, um den API-Key nicht auf dem Endgerät speichern zu müssen. Vorerst hardcodiert.

**F-21:** Das System synchronisiert die Uhrzeit automatisch über NTP.

**F-22:** Das Display zeigt die aktuelle WLAN-Signalstärke an.

**F-23:** Es gibt einen Info-Screen, der die Konfigurations-URL und (optional) einen QR-Code anzeigt.

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

---

## 5. Abgrenzungen (Explizit NICHT enthalten)

**A-01:** Keine Batterie-/Akkubetrieb im ersten Release.

**A-02:** Keine Touchscreen-Steuerung - Konfiguration nur über Web-Oberfläche.

**A-03:** Keine Anzeige von Fahrplänen - nur Echtzeit-Abfahrten.

**A-04:** Keine Routenplanung oder Verbindungssuche.

**A-05:** Keine Integration mit Smart Home Systemen.

**A-06:** Keine mobile App - nur Web-Oberfläche.

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

---

## 7. Datenmodell (Logisch)

### Konfiguration
- WLAN-Zugangsdaten (SSID, Passwort)
- Haltestelle (Name, ID)
- API Key (für opentransportdata.swiss)
- Linie 1 (Bezeichnung, Richtung)
- Linie 2 (Bezeichnung, Richtung)
- Sprache
- Aktualisierungsintervall

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

---

## 9. Offene Fragen

_Keine offenen Fragen - alle Anforderungen sind geklärt._

---

## 10. Änderungshistorie

| Version | Datum | Änderung | Autor |
|---------|-------|----------|-------|
| 1.0 | 2024-12 | Initiale Version | Mahboob |
| 1.1 | 2024-12 | Ergänzung NTP, Warn-Icon | Assistant |
| 1.2 | 2024-12 | Manuelles Update per Taster, Input Refactoring | Assistant |
| 1.3 | 2024-12 | Display Layout (Tabelle, Signalstärke, Info Screen) | Assistant |
| 1.4 | 2026-01 | Haltestellensuche im Web-Interface, OJP 2.0 Parser-Fix, Zeitzonenkorrektur | Assistant |

