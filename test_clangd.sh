#!/bin/bash
# Clangd Test Script für ESP32-Projekt

echo "🔍 Testing clangd setup for ESP32 project"
echo "=========================================="
echo ""

# 1. Check if clangd is installed
echo "1. Checking clangd installation..."
if command -v clangd &> /dev/null; then
    echo "   ✅ clangd found: $(which clangd)"
    echo "   Version: $(clangd --version | head -1)"
else
    echo "   ❌ clangd not found in PATH"
    exit 1
fi
echo ""

# 2. Check if compile_commands.json exists
echo "2. Checking compile_commands.json..."
if [ -f "compile_commands.json" ]; then
    echo "   ✅ compile_commands.json found"
    echo "   Size: $(du -h compile_commands.json | cut -f1)"
    echo "   Entries: $(jq '. | length' compile_commands.json)"

    # Check directory field
    DIR_FIELD=$(jq -r '.[0].directory' compile_commands.json)
    echo "   Directory field: $DIR_FIELD"
    if [ "$DIR_FIELD" = "." ]; then
        echo "   ✅ Directory field is correct (.)"
    else
        echo "   ⚠️  Directory field should be '.' but is '$DIR_FIELD'"
    fi
else
    echo "   ❌ compile_commands.json not found"
    echo "   Run: make compiledb"
    exit 1
fi
echo ""

# 3. Check if .clangd config exists
echo "3. Checking .clangd configuration..."
if [ -f ".clangd" ]; then
    echo "   ✅ .clangd config found"
else
    echo "   ⚠️  .clangd config not found (optional)"
fi
echo ""

# 4. Check if src/main.cpp exists
echo "4. Checking source files..."
if [ -f "src/main.cpp" ]; then
    echo "   ✅ src/main.cpp found"
else
    echo "   ❌ src/main.cpp not found"
    exit 1
fi
echo ""

# 5. Test clangd with src/main.cpp
echo "5. Testing clangd with src/main.cpp..."
echo "   Running: clangd --check=src/main.cpp"
echo ""

# Capture output
CLANGD_OUTPUT=$(echo "#include <Arduino.h>" | clangd --check=src/main.cpp 2>&1)

# Check for errors
if echo "$CLANGD_OUTPUT" | grep -q "Error\|error:"; then
    echo "   ❌ Clangd reported errors:"
    echo "$CLANGD_OUTPUT" | grep -i "error" | head -10
else
    echo "   ✅ Clangd started successfully"
fi

# Check if compilation database was loaded
if echo "$CLANGD_OUTPUT" | grep -q "Loaded compilation database"; then
    echo "   ✅ Compilation database loaded"
else
    echo "   ⚠️  Compilation database not loaded"
fi

# Check for VFS errors
if echo "$CLANGD_OUTPUT" | grep -q "VFS:.*failed"; then
    echo "   ❌ VFS errors detected:"
    echo "$CLANGD_OUTPUT" | grep "VFS"
fi

echo ""
echo "=========================================="
echo "6. Next steps:"
echo ""
echo "   To test in Neovim:"
echo "   1. nvim src/main.cpp"
echo "   2. :LspInfo"
echo "   3. If clangd not attached: :LspStart clangd"
echo "   4. Check logs: :lua vim.cmd('edit ' .. vim.lsp.get_log_path())"
echo ""
echo "   To see clangd output in detail:"
echo "   echo '#include <Arduino.h>' | clangd --check=src/main.cpp 2>&1 | less"
echo ""
