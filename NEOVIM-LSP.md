# Neovim LSP für ESP32-Entwicklung

## ✅ Setup abgeschlossen!

Clangd ist jetzt konfiguriert und sollte automatisch beim Öffnen von C/C++-Dateien starten.

## 🚀 LSP-Funktionen (Tastenkombinationen)

| Funktion | Tastenkombination | Beschreibung |
|----------|-------------------|--------------|
| **Goto Definition** | `gd` | Springe zur Definition |
| **Goto Declaration** | `gD` | Springe zur Deklaration |
| **Goto Implementation** | `gI` | Springe zur Implementierung |
| **Goto Type Definition** | `gy` | Springe zur Typ-Definition |
| **Find References** | `gr` | Zeige alle Verwendungen |
| **Hover Documentation** | `K` | Zeige Dokumentation |
| **Signature Help** | `gK` | Zeige Funktionssignatur |
| **Rename** | `<leader>cr` | Variable/Funktion umbenennen |
| **Code Action** | `<leader>ca` | Zeige Code-Aktionen |
| **LSP Info** | `<leader>cl` oder `:LspInfo` | Zeige LSP-Status |

## 🔧 Workflow

### 1. Nach jedem Build
```bash
make build
# → compile_commands.json wird automatisch korrigiert
```

### 2. Compile Commands manuell generieren
```bash
make compiledb
# → Generiert und korrigiert compile_commands.json
```

### 3. Neovim starten
```bash
nvim src/main.cpp
# → Clangd startet automatisch
```

### 4. Falls clangd nicht startet
```vim
:LspStart clangd
```

### 5. Debug-Info
```vim
:luafile ~/.config/nvim/lua/debug-lsp.lua
# Oder
:LspDebug
```

## 🐛 Troubleshooting

### Problem: "No active clients"

**Lösung 1: Filetype überprüfen**
```vim
:set filetype?
# Sollte zeigen: filetype=cpp

# Falls nicht:
:set filetype=cpp
```

**Lösung 2: Clangd manuell starten**
```vim
:LspStart clangd
```

**Lösung 3: Logs überprüfen**
```vim
:lua vim.cmd('edit ' .. vim.lsp.get_log_path())
```

### Problem: compile_commands.json hat falsche Pfade

**Lösung:**
```bash
# Im Projektverzeichnis:
sed -i 's|"directory": "/workspace"|"directory": "."|g' compile_commands.json
```

### Problem: Header nicht gefunden

**Erwartetes Verhalten:** Da die ESP32-Toolchain nur im Docker-Container existiert, werden manche SDK-Header nicht gefunden. Das ist normal.

**Was funktioniert:**
- ✅ Eigener Code (src/, include/)
- ✅ Syntax-Highlighting
- ✅ Goto Definition (für eigenen Code)
- ✅ Autocomplete (mit Einschränkungen)

**Was eingeschränkt ist:**
- ⚠️ ESP32-SDK-Header (z.B. `freertos/FreeRTOS.h`)
- ⚠️ Toolchain-spezifische Header

## 📁 Wichtige Dateien

| Datei | Zweck |
|-------|-------|
| `~/.config/nvim/lua/plugins/clangd-autostart.lua` | Clangd-Konfiguration |
| `~/.config/nvim/lua/debug-lsp.lua` | Debug-Helper |
| `.clangd` | Clangd-Projekt-Konfiguration |
| `compile_commands.json` | Compile-Commands-Datenbank |

## 🧪 Test clangd (Terminal)

```bash
# Im Projektverzeichnis:
./test_clangd.sh
```

## 📝 Notizen

- Nach jedem `make build` oder `make compiledb` wird compile_commands.json automatisch korrigiert
- Clangd sollte beim Öffnen von C/C++-Dateien automatisch starten
- Bei Problemen: `:LspDebug` in Neovim ausführen
