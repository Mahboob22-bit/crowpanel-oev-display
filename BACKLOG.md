# Backlog: Code-Review Findings

**Erstellt:** Februar 2026
**Quelle:** Automatisiertes Code-Review für kommerzielle Produktreife

Dieses Dokument listet alle im Code-Review gefundenen Probleme, priorisiert nach Dringlichkeit. Jedes Issue muss vor dem kommerziellen Release adressiert werden (je nach Priorität).

---

## Kritisch — Vor kommerziellem Release zwingend zu lösen

### BL-01: HTTPS-Zertifikatsvalidierung deaktiviert

| | |
|---|---|
| **Datei** | `src/Transport/TransportModule.cpp` (Zeilen 118, 172, 252) |
| **Problem** | `client->setInsecure()` deaktiviert die TLS-Zertifikatsvalidierung. Kommentar sagt "Für Development", aber es ist in allen Builds aktiv. |
| **Risiko** | Anfällig für Man-in-the-Middle-Angriffe. Ein Angreifer könnte manipulierte Daten oder Firmware einschleusen. |
| **Lösung** | Root-CA-Zertifikat einbetten und `client->setCACert()` verwenden. `setInsecure()` nur im Development-Build erlauben (via `#ifdef`). |

### BL-02: Keine Authentifizierung auf der Web-Oberfläche

| | |
|---|---|
| **Datei** | `src/Web/WebConfigModule.cpp` |
| **Problem** | Alle REST-Endpunkte (`/api/config`, `/api/reset`, etc.) sind ohne Authentifizierung zugänglich. |
| **Risiko** | Jeder im lokalen Netzwerk kann die Konfiguration ändern oder einen Factory-Reset auslösen. |
| **Lösung** | HTTP Basic Auth oder Token-basierte Authentifizierung einführen. Passwort konfigurierbar via Web-Oberfläche. |

### BL-03: WiFi-Passwörter im Klartext gespeichert

| | |
|---|---|
| **Datei** | `src/Core/ConfigStore.cpp` (Zeilen 21–24, 27–32) |
| **Problem** | WiFi-Passwörter werden unverschlüsselt in NVS (Preferences) gespeichert. |
| **Risiko** | Bei physischem Zugang zum Gerät können die WiFi-Zugangsdaten ausgelesen werden. |
| **Lösung** | WiFi-Passwörter mit AES-256 verschlüsseln bevor sie in NVS geschrieben werden. Alternativ: ESP32 NVS Encryption aktivieren. |

### BL-04: Fehlende Input-Validierung auf Server-Seite

| | |
|---|---|
| **Datei** | `src/Web/WebConfigModule.cpp` (Zeilen 181–204, 240–251) |
| **Problem** | Keine Längenbegrenzung bei SSID, Passwort und Konfigurationsdaten. Nur minimale Prüfung bei Haltestellensuche (min 2 Zeichen). |
| **Risiko** | Überlange Eingaben können Speicherprobleme verursachen (Heap-Overflow, OOM). |
| **Lösung** | Maximale Längen definieren und validieren: SSID (32), Passwort (64), Suchbegriff (50). JSON-Payload-Grösse begrenzen. |

### BL-05: Memory Leak in TransportModule

| | |
|---|---|
| **Datei** | `src/Transport/TransportModule.cpp` (Zeilen 116, 170, 249) |
| **Problem** | `new WiFiClientSecure` wird mit raw `new`/`delete` verwaltet. Bei Fehlern in `http.begin()` wird `client` nicht freigegeben. |
| **Risiko** | Schleichender Speicherverlust im Dauerbetrieb → Crash nach Stunden/Tagen. |
| **Lösung** | `std::unique_ptr<WiFiClientSecure>` verwenden (RAII), damit der Speicher bei Scope-Verlassen automatisch freigegeben wird. |

---

## Hoch — Sollte zeitnah gelöst werden

### BL-06: Inkonsistentes Logging in OjpParser

| | |
|---|---|
| **Datei** | `src/Transport/OjpParser.cpp` (Zeilen 89–127, 250–285) |
| **Problem** | Direkte `Serial.printf` und `Serial.println` Aufrufe statt der `Logger`-Klasse. |
| **Risiko** | Inkonsistente Log-Ausgabe, keine Log-Level-Filterung, potenzielle Ausgabe sensibler Daten. Verstösst gegen Architektur-Prinzip 5.1.5. |
| **Lösung** | Alle `Serial.*` Aufrufe durch `Logger::debug/info/error` ersetzen. |

### BL-07: Debug-Delay in main.cpp

| | |
|---|---|
| **Datei** | `src/main.cpp` (Zeile 35) |
| **Problem** | `delay(2000)` wartet auf den Serial Monitor — unnötig in Produktion. |
| **Risiko** | 2 Sekunden verzögerter Boot bei jedem Start, auch beim Kunden. |
| **Lösung** | Mit `#ifdef`-Guard nur im Development-Build einschliessen, oder durch `Serial.begin()` Timeout ersetzen. |

### BL-08: Mutex-Erstellung nicht validiert

| | |
|---|---|
| **Datei** | `src/Transport/TransportModule.cpp` (Zeile 19) |
| **Problem** | `_mutex = xSemaphoreCreateMutex()` — Rückgabewert wird nicht geprüft. |
| **Risiko** | Falls die Erstellung fehlschlägt (z.B. bei Speichermangel), ist `_mutex` NULL. Nachfolgende `xSemaphoreTake` Aufrufe verursachen einen Crash. |
| **Lösung** | Rückgabewert prüfen und bei Fehler eine Fehlermeldung loggen / Modul deaktivieren. |

### BL-09: Kein CSRF-Schutz für POST-Endpunkte

| | |
|---|---|
| **Datei** | `src/Web/WebConfigModule.cpp`, `data/app.js` |
| **Problem** | POST-Requests (`/api/config`, `/api/reset`) haben keinen CSRF-Schutz. |
| **Risiko** | Eine bösartige Website könnte im Browser des Benutzers Konfigurationsänderungen am Gerät auslösen. |
| **Lösung** | CSRF-Token generieren und bei POST-Requests validieren. Alternativ: `SameSite`-Cookie + Origin-Header-Check. |

### BL-10: Kein Rate-Limiting auf API-Endpunkte

| | |
|---|---|
| **Datei** | `src/Web/WebConfigModule.cpp` |
| **Problem** | Alle API-Endpunkte können unbegrenzt oft aufgerufen werden. |
| **Risiko** | DoS-Angriff möglich — viele gleichzeitige Requests können den ESP32 überlasten (OOM, Watchdog-Timeout). |
| **Lösung** | Einfaches Rate-Limiting implementieren (z.B. max. 10 Requests/Sekunde pro Client-IP). |

---

## Mittel — Vor v2.0 lösen

### BL-11: Kein Produktions-Build-Profil

| | |
|---|---|
| **Datei** | `platformio.ini` |
| **Problem** | Nur ein Build-Profil (`[env:esp32s3]`) mit Debug-Konfiguration (`CORE_DEBUG_LEVEL=3`). |
| **Lösung** | Zweites Profil `[env:esp32s3_production]` mit `CORE_DEBUG_LEVEL=1`, OTA-Partitionstabelle und Signierung hinzufügen. |

### BL-12: Keine Firmware-Versionsnummer im Code

| | |
|---|---|
| **Datei** | Nicht vorhanden |
| **Problem** | Es gibt keine `FW_VERSION`-Konstante oder `version.h`. Das Gerät weiss nicht, welche Version es hat. |
| **Lösung** | `include/version.h` erstellen mit `#define FW_VERSION "x.y.z"`. Version per Build-Flag aus `platformio.ini` setzen oder aus Git-Tag ableiten. |

### BL-13: JSON.parse ohne Error-Handling in app.js

| | |
|---|---|
| **Datei** | `data/app.js` (Zeile 262) |
| **Problem** | `JSON.parse(select.value)` ohne try-catch. |
| **Risiko** | Bei ungültigem JSON crasht die Frontend-Logik ohne Fehlermeldung. |
| **Lösung** | In try-catch wrappen und bei Fehler eine Toast-Notification anzeigen. |

### BL-14: Queue-Erstellung schlägt fehl → System in inkonsistentem Zustand

| | |
|---|---|
| **Datei** | `src/main.cpp` (Zeilen 52–56) |
| **Problem** | Wenn die Event-Queue nicht erstellt werden kann, gibt `setup()` zurück, aber das System läuft weiter ohne Queue. |
| **Lösung** | Bei fatalen Fehlern in `setup()` eine Fehlermeldung auf dem Display anzeigen und in eine Endlosschleife gehen (oder `esp_restart()`). |

### BL-15: Sensitive Daten in Log-Ausgaben

| | |
|---|---|
| **Datei** | `src/Transport/TransportModule.cpp` (Zeile 50), `src/Core/ConfigStore.cpp` (Zeile 24) |
| **Problem** | API-Key-Nutzung wird geloggt. WiFi-SSID wird geloggt. Suchbegriffe der Benutzer werden geloggt. |
| **Risiko** | Bei physischem Zugang zum Serial-Port könnten sensible Informationen ausgelesen werden. |
| **Lösung** | Sensible Daten in Logs reduzieren oder maskieren. Im Produktions-Build Log-Level auf ERROR beschränken. |

### BL-16: Hardcodierter Internet-Check über HTTP

| | |
|---|---|
| **Datei** | `src/Wifi/WifiManager.cpp` (Zeile 149) |
| **Problem** | `http.begin("http://www.google.com")` — Unverschlüsselte Verbindung für Internet-Check. |
| **Lösung** | Durch einen Check gegen den eigenen Proxy-Server ersetzen (z.B. `GET /health`), oder HTTPS verwenden. |
