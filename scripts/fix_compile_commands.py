#!/usr/bin/env python3
"""
Post-build script to fix compile_commands.json for use with clangd on the host.
Replaces container paths with host-compatible paths.
"""
Import("env")
import json
import os

def fix_compile_commands(source, target, env):
    """Fix compile_commands.json after build"""

    compile_commands_path = os.path.join(env.get("PROJECT_DIR"), "compile_commands.json")

    if not os.path.exists(compile_commands_path):
        print("⚠ compile_commands.json not found, skipping fix")
        return

    try:
        # Read compile_commands.json
        with open(compile_commands_path, 'r') as f:
            data = json.load(f)

        # Fix each entry
        modified = False
        for entry in data:
            # Replace /workspace with current directory
            if entry.get('directory') == '/workspace':
                entry['directory'] = '.'
                modified = True

        # Write back if modified
        if modified:
            with open(compile_commands_path, 'w') as f:
                json.dump(data, f, indent=2)
            print("✓ Fixed compile_commands.json for host usage")
        else:
            print("✓ compile_commands.json already correct")

    except Exception as e:
        print(f"⚠ Error fixing compile_commands.json: {e}")

# Register the callback to run after build
env.AddPostAction("$BUILD_DIR/${PROGNAME}.elf", fix_compile_commands)
print("✓ Post-build fix for compile_commands.json registered")
