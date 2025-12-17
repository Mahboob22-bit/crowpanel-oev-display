#!/usr/bin/env python3
"""
Patch compile_commands.json to include stub headers and fix paths.
Run after: make compiledb or make build
"""
import json
import os
import sys

def patch_compile_commands():
    """Patch compile_commands.json for host clangd usage"""

    compile_commands_path = "compile_commands.json"

    if not os.path.exists(compile_commands_path):
        print("⚠ compile_commands.json not found")
        return 1

    try:
        # Read
        with open(compile_commands_path, 'r') as f:
            data = json.load(f)

        modified = False
        stub_include = "-Iinclude/stubs"

        for entry in data:
            # Fix directory
            if entry.get('directory') == '/workspace':
                entry['directory'] = '.'
                modified = True

            # Add stub includes to command
            if 'command' in entry:
                command = entry['command']

                # Check if stub include is already there
                if stub_include not in command and 'src/main.cpp' in command:
                    # Insert stub include right after the compiler command
                    parts = command.split(' ', 1)
                    if len(parts) == 2:
                        compiler, rest = parts
                        entry['command'] = f"{compiler} {stub_include} {rest}"
                        modified = True

        if modified:
            # Write back
            with open(compile_commands_path, 'w') as f:
                json.dump(data, f, indent=2)

            print("✓ Patched compile_commands.json:")
            print("  - Fixed directory: /workspace -> .")
            print("  - Added stub includes: -Iinclude/stubs")
            return 0
        else:
            print("✓ compile_commands.json already correct")
            return 0

    except Exception as e:
        print(f"✗ Error: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(patch_compile_commands())
