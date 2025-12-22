# Core Module

Kern-Komponenten und Hilfsklassen für das System.

## ConfigStore

Die `ConfigStore` Klasse verwaltet die persistente Speicherung von Konfigurationsdaten im Non-Volatile Storage (NVS) des ESP32. Sie nutzt dafür die Arduino `Preferences` Library.

### Gespeicherte Daten (Namespace: `crowpanel`)

| Key | Typ | Beschreibung |
|-----|-----|--------------|
| `ssid` | String | WLAN SSID |
| `password` | String | WLAN Passwort |
| `st_name` | String | Name der Haltestelle |
| `st_id` | String | ID der Haltestelle (für API) |
| `l1_name` | String | Name Linie 1 |
| `l1_dir` | String | Richtung Linie 1 |
| `l2_name` | String | Name Linie 2 |
| `l2_dir` | String | Richtung Linie 2 |

### API

```cpp
// Initialisierung
void begin();

// WLAN
void setWifiCredentials(const String& ssid, const String& password);
String getWifiSSID();
String getWifiPassword();
bool hasWifiConfig();

// Station & Linien
void setStation(const String& name, const String& id);
StationConfig getStation();

void setLine1(const String& name, const String& direction);
LineConfig getLine1();
// ... (analog für Line 2)

// Reset
void resetToFactory(); // Löscht alle Keys im Namespace
```
