# Neovim LSP Setup (Fixed)

Ich habe dein Neovim-Setup für dieses Projekt repariert.

## Was wurde gemacht?

1. **Neovim Konfiguration repariert**:
   - Die Datei `~/.config/nvim/lua/plugins/clangd-direct.lua` wurde angepasst, damit sie korrekt mit LazyVim funktioniert.
   - Keybindings wie `gd` (Goto Definition) und `K` (Hover) funktionieren jetzt automatisch.

2. **Header-Probleme gelöst**:
   - `include/stubs/Arduino.h` korrigiert (konfliktierendes `#define NULL` entfernt).
   - Jetzt kann der Code auch auf dem Host (außerhalb von Docker) fehlerfrei geparst werden.

3. **Aufräumen**:
   - Unnötige Skripte und alte Konfigurationen wurden entfernt.

## Wie benutze ich es jetzt?

Einfach Neovim öffnen:

```bash
nvim src/main.cpp
```

### Verfügbare Funktionen

| Taste | Funktion |
|-------|----------|
| `gd` | **G**oto **D**efinition (Springe zur Definition) |
| `gr` | **G**oto **R**eferences (Zeige Verwendungen) |
| `K` | Hover Info (Zeige Typen/Doku) |
| `<leader>ca` | Code Actions |

### Wichtig

Wenn du neue Dateien hinzufügst oder das Projekt sich ändert, führe einmalig aus:

```bash
make compiledb
```

Das aktualisiert die `compile_commands.json`, damit `clangd` deine neuen Dateien findet.

## Testen

Du kannst das Setup jederzeit testen mit:

```bash
./test_lsp.sh
```

Viel Erfolg!
