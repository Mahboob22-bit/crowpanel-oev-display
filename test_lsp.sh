#!/bin/bash
# Test LSP Setup

echo "🔍 Testing LSP Setup..."
echo ""

# 1. Check if compile_commands.json exists
if [ ! -f "compile_commands.json" ]; then
    echo "❌ compile_commands.json not found!"
    echo "   Run: make compiledb"
    exit 1
fi
echo "✅ compile_commands.json exists"

# 2. Check compile_commands.json content
entries=$(jq length compile_commands.json 2>/dev/null)
if [ "$entries" = "2" ]; then
    echo "✅ compile_commands.json has 2 entries (correct)"
else
    echo "⚠️  compile_commands.json has $entries entries (expected 2)"
fi

# 3. Check if clangd is installed
if command -v clangd &> /dev/null; then
    version=$(clangd --version | head -1)
    echo "✅ clangd installed: $version"
else
    echo "❌ clangd not installed!"
    echo "   Run: sudo pacman -S clang (Arch) or sudo apt install clangd (Ubuntu)"
    exit 1
fi

# 4. Check .clangd config
if [ -f ".clangd" ]; then
    echo "✅ .clangd config exists"
else
    echo "❌ .clangd config not found!"
    exit 1
fi

# 5. Check stub headers
if [ -f "include/stubs/Arduino.h" ]; then
    echo "✅ Stub headers exist"
else
    echo "❌ Stub headers missing!"
    exit 1
fi

# 6. Quick clangd check
echo ""
echo "🧪 Testing clangd on src/main.cpp..."
errors=$(clangd --check=src/main.cpp 2>&1 | grep -c "undeclared_var_use\|unknown_typename")
if [ "$errors" = "0" ]; then
    echo "✅ No syntax errors! clangd can parse the code."
else
    echo "⚠️  Found $errors syntax errors"
fi

echo ""
echo "📝 Next steps:"
echo "1. Close Neovim completely"
echo "2. Clear clangd cache: rm -rf ~/.cache/clangd"
echo "3. Open Neovim: nvim src/main.cpp"
echo "4. Wait for LSP to start (check with :LspInfo)"
echo "5. Test: Put cursor on 'displayManager' and press 'gd'"
echo ""
echo "💡 If 'gd' still doesn't work, your Neovim LSP keybindings might not be set up."
echo "   Check: ~/.config/nvim/lua/plugins/clangd-direct.lua"
