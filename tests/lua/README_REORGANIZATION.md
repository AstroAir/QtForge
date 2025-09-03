# Lua Tests Reorganization

This document describes the reorganization of Lua test files that were previously scattered in the project root directory.

## Changes Made

### Files Moved to `tests/lua/integration/`
These are comprehensive integration tests that test complete functionality:

- `test_lua_final.lua` - Final comprehensive test with success/failure indicators
- `test_lua_comprehensive.lua` - Comprehensive test using package.loadlib approach  
- `test_lua_bindings.lua` - Basic integration test for QtForge Lua bindings

### Files Moved to `tests/lua/unit/`
These are unit tests and debugging tools that test specific components:

- `test_lua_debug.lua` - Debug test with detailed property inspection
- `test_lua_module.lua` - Test using require() to load the module
- `test_lua_loadlib.lua` - Test using package.loadlib for loading library

### Files Moved to `tests/lua/`
These are C++ test files for Lua bindings:

- `test_lua_bindings_root.cpp` - C++ test using C-style API (renamed to avoid conflict)
- `test_lua_direct.cpp` - Direct test using sol2 library
- `test_lua_simple.cpp` - Simple C++ test using C++ API

### Files Moved to `tests/python/`
Non-Lua related test files:

- `test_minimal.cpp` - Minimal pybind11 test (Python-related)

## Path Updates

All moved Lua test files have been updated with corrected library paths:
- Changed from `./?.dll` to `../../?.dll;../../../build/?.dll`
- This allows tests to find the QtForge Lua library from their new locations

## Directory Structure

```
tests/lua/
├── integration/           # Integration tests (.lua files)
│   ├── test_lua_final.lua
│   ├── test_lua_comprehensive.lua
│   └── test_lua_bindings.lua
├── unit/                  # Unit tests (.lua files)
│   ├── test_lua_debug.lua
│   ├── test_lua_module.lua
│   └── test_lua_loadlib.lua
└── *.cpp                  # C++ test files
```

## Benefits

1. **Better Organization**: Tests are now logically grouped by type and purpose
2. **Cleaner Root Directory**: Removed clutter from the project root
3. **Consistent Structure**: Follows the existing pattern used by other test modules
4. **Easier Maintenance**: Related tests are grouped together
5. **Clear Separation**: Integration tests, unit tests, and C++ tests are clearly separated

## Running Tests

Tests can still be run using the existing CMake test infrastructure. The CMakeLists.txt files have been designed to work with the new organization.
