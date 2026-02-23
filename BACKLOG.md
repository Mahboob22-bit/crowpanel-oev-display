# Backlog: Code-Review Findings

**Erstellt:** Februar 2026
**Quelle:** Automatisiertes Code-Review für kommerzielle Produktreife

Dieses Dokument listet alle im Code-Review gefundenen Probleme, priorisiert nach Dringlichkeit. Jedes Issue muss vor dem kommerziellen Release adressiert werden (je nach Priorität).

---

## Kritisch — Vor kommerziellem Release zwingend zu lösen

### ✅ BL-01: HTTPS-Zertifikatsvalidierung deaktiviert — BEHOBEN (2026-02)

| | |
|---|---|
| **Datei** | `src/Transport/TransportModule.cpp`, `include/certs.h` (neu) |
| **Lösung umgesetzt** | ISRG Root X1 Zertifikat in `include/certs.h` eingebettet. `configureTLS()`-Methode entscheidet per `#ifdef DEV_BUILD`: im Dev-Build `setInsecure()`, im Production-Build `setCACert(ROOT_CA_CERT)`. Dev-Profil in `platformio.ini` erhält `-DDEV_BUILD`. |

### ✅ BL-02: Keine Authentifizierung auf der Web-Oberfläche — BEHOBEN (2026-02)

| | |
|---|---|
| **Datei** | `src/Web/WebConfigModule.cpp/.h`, `src/Core/ConfigStore.cpp/.h` |
| **Lösung umgesetzt** | HTTP Basic Auth via `checkAuth()`. Passwort konfigurierbar im ConfigStore (`web_pw`). Im AP-Mode und ohne gesetztes Passwort kein Schutz (Ersteinrichtung). Geschützt: `/api/status`, `/api/device`, `/api/config`, `/api/reset`. |

### ✅ BL-03: WiFi-Passwörter im Klartext gespeichert — TEILWEISE BEHOBEN (2026-02)

| | |
|---|---|
| **Datei** | `src/Core/ConfigStore.cpp/.h` |
| **Lösung umgesetzt** | XOR-Verschleierung mit MAC-Adresse als Key, Base64-kodiert im NVS. Automatische Migration bestehender Klartext-Werte. **Hinweis:** Dies ist keine echte Verschlüsselung. Die finale Lösung ist NVS Encryption mit Espressif Flash Encryption — wird mit Secure Boot V2 umgesetzt. |

### ✅ BL-04: Fehlende Input-Validierung auf Server-Seite — BEHOBEN (2026-02)

| | |
|---|---|
| **Datei** | `src/Web/WebConfigModule.cpp` |
| **Lösung umgesetzt** | Payload max 1024 Bytes, SSID max 32, Passwort max 64, Suchbegriff max 50, StopId max 20, Station/Line-Felder begrenzt. Alle als `static constexpr` Konstanten definiert. |

### ✅ BL-05: Memory Leak in TransportModule — BEHOBEN (2026-02)

| | |
|---|---|
| **Datei** | `src/Transport/TransportModule.cpp` |
| **Lösung umgesetzt** | Alle 3 `new WiFiClientSecure` durch `std::make_unique<WiFiClientSecure>()` ersetzt. Kein `delete` mehr nötig — automatische Freigabe via RAII. |

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
| **Hinweis** | Grundlage geschaffen: `-DDEV_BUILD` Flag vorhanden, `partitions_ota.csv` erstellt. Das zweite `[env:...]` Profil folgt mit Phase 3 (Secure Boot). |

### ✅ BL-12: Keine Firmware-Versionsnummer im Code — BEHOBEN (2026-02)

| | |
|---|---|
| **Datei** | `include/version.h` (neu), `src/main.cpp`, `src/DeviceIdentity/DeviceIdentity.cpp` |
| **Lösung umgesetzt** | `include/version.h` mit `FW_VERSION "1.3.0"` sowie `FW_VERSION_MAJOR/MINOR/PATCH`. Version wird im Boot-Banner geloggt und via `/api/device` und `/api/status` bereitgestellt. |

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
