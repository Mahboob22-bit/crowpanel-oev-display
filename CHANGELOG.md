# Changelog

## [Unreleased]

### Added
- **WifiManager**: Initial implementation with Station Mode and Reconnect logic.
- **InputManager**: Encapsulated button handling (Menu, Exit, Rotary) with interrupts and debouncing.
- **SystemMonitor**: Periodic logging of system stats (Heap, Stack, CPU).
- **Logger**: Centralized logging module with tags and levels, replacing raw `Serial.print`.
- **Secrets Management**: `secrets.h` introduced for sensitive credentials (gitignored).

### Changed
- **Refactoring**: 
    - Moved `DisplayManager` to its own module `src/Display`.
    - Moved task logic from `main.cpp` into respective modules (`begin()` starts the task).
    - `main.cpp` is now minimal and only orchestrates module initialization.
- **Events**: Extended `DisplayEvent` with Wifi status (`EVENT_WIFI_CONNECTED`, `EVENT_WIFI_LOST`).

### Fixed
- Code structure modularized for better maintainability and testability.
