# DeviceIdentity

Das `DeviceIdentity`-Modul stellt die eindeutige Geräte-Identifikation und Firmware-Versionsinformationen bereit.

## Funktionen

*   **Device-ID:** Generiert eine eindeutige Geräte-ID aus der MAC-Adresse im Format `CP-AABBCCDDEEFF`.
*   **Firmware-Version:** Stellt die aktuelle Firmware-Version (SemVer) bereit.
*   **Hardware-Info:** Liefert Flash-Grösse und PSRAM-Grösse.

## Abhängigkeiten

*   `Logger`
*   `include/version.h` (für `FW_VERSION`)

## API

### `void begin()`
Liest die MAC-Adresse aus den eFuses und generiert die Device-ID. Loggt Device-ID und FW-Version.

### `const char* getDeviceId()`
Gibt die eindeutige Device-ID zurück (z.B. `"CP-A1B2C3D4E5F6"`).

### `const char* getFirmwareVersion()`
Gibt die Firmware-Version als String zurück (z.B. `"1.3.0"`).

### `uint32_t getFlashSizeMB()`
Gibt die Flash-Grösse in Megabyte zurück.

### `uint32_t getPsramSizeKB()`
Gibt die PSRAM-Grösse in Kilobyte zurück.

## Hinweise

*   Dieses Modul benötigt keinen eigenen FreeRTOS-Task, da es nur statische Daten bereitstellt.
*   Die Device-ID ist stabil über Neustarts und Firmware-Updates hinweg, da sie auf der Hardware-MAC-Adresse basiert.
*   Die Device-ID wird vom `OtaManager` für Update-Checks und vom `WebConfigModule` für die `/api/device`-API verwendet.
