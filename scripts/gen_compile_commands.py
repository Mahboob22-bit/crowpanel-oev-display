Import("env")

# Include toolchain paths for LSP
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)

# Override compilation DB path
env.Replace(COMPILATIONDB_PATH="compile_commands.json")

print("✓ compile_commands.json will be generated")
