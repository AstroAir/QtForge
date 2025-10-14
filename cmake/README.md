# QtForge CMake Build System

This directory contains the modular CMake build system for QtForge. The build system is organized into a hierarchical structure for maintainability, clarity, and cross-platform compatibility.

## Directory Structure

```
cmake/
├── modules/                    # Core CMake modules
│   ├── components/            # Component source configuration modules
│   │   ├── QtForgeCoreComponent.cmake
│   │   ├── QtForgeUtilsComponent.cmake
│   │   ├── QtForgeManagersComponent.cmake
│   │   ├── QtForgeCommunicationComponent.cmake
│   │   ├── QtForgeWorkflowComponent.cmake
│   │   ├── QtForgeMonitoringComponent.cmake
│   │   └── README.md
│   ├── platform/              # Platform-specific configurations
│   │   ├── Windows.cmake
│   │   ├── Unix.cmake
│   │   ├── MSYS2.cmake
│   │   └── README.md
│   ├── QtForgeOptions.cmake   # Build options and configuration
│   ├── QtForgeDependencies.cmake  # Dependency management
│   ├── QtForgeTargets.cmake   # Target creation and configuration
│   ├── QtForgeCompiler.cmake  # Compiler settings
│   ├── QtForgePlatform.cmake  # Platform detection
│   └── QtForgePackaging.cmake # CPack configuration
└── README.md                  # This file
```

## Build System Architecture

The QtForge build system follows a modular, layered architecture with 8 distinct phases:

1. **Platform Detection** - Identifies OS, compiler, and toolchain
2. **Platform Configuration** - Applies platform-specific settings
3. **Build Options** - Defines and validates build options
4. **Dependency Management** - Finds and configures dependencies
5. **Component Configuration** - Defines source lists for each component
6. **Target Creation** - Creates library and executable targets
7. **Compiler Configuration** - Sets compiler flags and options
8. **Packaging** - Configures installers and distribution

For detailed information on each phase, see the module-specific README files in `modules/components/` and `modules/platform/`.

## Quick Start

### Standard Build

```bash
# Configure
cmake -S . -B build -DQTFORGE_BUILD_TESTS=ON -DQTFORGE_BUILD_EXAMPLES=ON

# Build
cmake --build build --parallel

# Test
ctest --test-dir build --output-on-failure
```

### MSYS2 MinGW64 Build

```bash
# Install Qt6 via pacman (in MSYS2 MINGW64 shell)
pacman -S mingw-w64-x86_64-qt6-base mingw-w64-x86_64-qt6-tools

# Configure and build
cmake -S . -B build -G Ninja
cmake --build build
```

## Component Modules

Component modules (`cmake/modules/components/`) define source lists for each logical component:

- **QtForgeCoreComponent**: Core plugin system (interfaces, manager, registry, loader)
- **QtForgeUtilsComponent**: Utility functions (version, error handling)
- **QtForgeManagersComponent**: High-level managers (configuration, logging, resources)
- **QtForgeCommunicationComponent**: Inter-plugin communication (message bus, events)
- **QtForgeWorkflowComponent**: Workflow orchestration (composition, transactions)
- **QtForgeMonitoringComponent**: Runtime monitoring (hot reload, metrics)

Each component module sets variables like `QTFORGE_<COMPONENT>_SOURCES` and `QTFORGE_<COMPONENT>_MOC_HEADERS`.

## Platform Modules

Platform modules (`cmake/modules/platform/`) provide platform-specific configurations:

- **Windows.cmake**: MSVC settings, DLL export/import, Windows API definitions
- **Unix.cmake**: GCC/Clang settings, RPATH configuration, symbol visibility
- **MSYS2.cmake**: MinGW settings, Qt6 detection via pacman, exception handling

Platform modules are automatically included and applied based on platform detection.

## Adding New Components

1. Create `cmake/modules/components/QtForgeNewComponent.cmake`
2. Define source lists (e.g., `QTFORGE_NEW_SOURCES`)
3. Include the module in root `CMakeLists.txt`
4. Add sources to `QTFORGE_ALL_SOURCES`

See `cmake/modules/components/README.md` for detailed instructions.

## Cross-Platform Support

Supported platforms:

- Windows (MSVC 2019+, MinGW-w64 via MSYS2)
- Linux (GCC 9+, Clang 10+)
- macOS (Clang/Xcode 12+)
- Android (NDK r21+)
- iOS (Xcode 12+)

## Troubleshooting

### Qt6 Not Found

- **Windows (MSVC)**: Set `CMAKE_PREFIX_PATH` to Qt6 installation
- **MSYS2**: Install Qt6 via pacman: `pacman -S mingw-w64-x86_64-qt6-base`
- **Linux**: Install Qt6 dev packages or set `Qt6_DIR`

### Build Failures

- Check compiler version meets minimum requirements
- Ensure all dependencies are installed
- Review CMake configuration output for warnings

For platform-specific issues, see `cmake/modules/platform/README.md`.

## References

- [CMake Documentation](https://cmake.org/documentation/)
- [Qt6 CMake Manual](https://doc.qt.io/qt-6/cmake-manual.html)
- [Modern CMake](https://cliutils.gitlab.io/modern-cmake/)
