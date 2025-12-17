#!/usr/bin/env python3
"""
Fix compile_commands.json for host-based clangd usage.
Removes Docker container paths that don't exist on the host.
Ensures paths are absolute to avoid CWD ambiguity.
"""
import json
import os
import sys
from pathlib import Path

def fix_compile_commands():
    """Fix compile_commands.json for clangd on host"""
    
    project_root = Path.cwd().resolve()
    compile_commands = project_root / "compile_commands.json"

    if not compile_commands.exists():
        print(f"⚠ compile_commands.json not found at {compile_commands}")
        return 1

    print("🔧 Fixing compile_commands.json for host usage...")

    with open(compile_commands, 'r') as f:
        data = json.load(f)

    # Filter out our source files only (not libraries or framework)
    our_files = []
    for entry in data:
        file_path = entry.get('file', '')

        # Keep ONLY files from our src/ directory (not .pio, not libraries)
        # Must start with 'src/' or '/workspace/src/'
        if file_path.startswith('src/') or file_path.startswith('/workspace/src/'):
            our_files.append(entry)

    print(f"  Found {len(our_files)} project source files (from src/)")

    # Clean up each entry
    for entry in our_files:
        # Fix directory - make absolute
        entry['directory'] = str(project_root)

        # Get the command
        command = entry.get('command', '')

        # Remove all -I includes that point to Docker container paths
        # Keep only project-relative includes
        cleaned_command = []
        skip_next = False

        parts = command.split()
        for i, part in enumerate(parts):
            if skip_next:
                skip_next = False
                continue

            # Skip Docker container include paths
            if part.startswith('-I/home/devuser/'):
                continue
            if part.startswith('-I/home/devuser'):
                skip_next = True
                continue

            # Skip ESP32-specific compiler flags
            if part in ['-mlongcalls', '-mtext-section-literals', 
                       '-fstrict-volatile-bitfields', '-fno-tree-switch-conversion']:
                continue

            cleaned_command.append(part)

        # Build new command with proper includes
        # Use system g++ for clangd compatibility
        # Use absolute paths for includes to be safe
        include_path = project_root / "include"
        src_path = project_root / "src"
        stubs_path = project_root / "include" / "stubs"
        
        new_parts = [
            'g++',
            '-std=gnu++11',
            f'-I{include_path}',
            f'-I{src_path}',
            f'-I{stubs_path}',
            '-DARDUINO=10812',
            '-DESP32',
            '-DCORE_DEBUG_LEVEL=3',
            '-DARDUINO_USB_MODE=0',
            '-DARDUINO_USB_CDC_ON_BOOT=0',
            '-DBOARD_HAS_PSRAM',
            '-c',
            entry['file']
        ]

        entry['command'] = ' '.join(new_parts)

    # Write simplified compile_commands.json
    output_path = compile_commands
    with open(output_path, 'w') as f:
        json.dump(our_files, f, indent=2)

    print(f"✓ Fixed compile_commands.json")
    print(f"  - Kept {len(our_files)} project files")
    print(f"  - Set absolute directory paths: {project_root}")
    print(f"  - Added absolute include paths")

    return 0

if __name__ == "__main__":
    sys.exit(fix_compile_commands())
