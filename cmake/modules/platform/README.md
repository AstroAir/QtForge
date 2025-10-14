# QtForge Platform Modules

This directory contains CMake modules that provide platform-specific build configurations for QtForge.

## Purpose

Platform modules encapsulate platform-specific compiler flags, definitions, path configurations, and build settings. This separation improves maintainability and makes it easier to support multiple platforms and toolchains.

## Available Platforms

### Windows (`Windows.cmake`)

Windows-specific configurations for MSVC and native Windows builds:

- **Compiler Settings**: MSVC-specific flags and definitions
- **Runtime Library**: MultiThreaded DLL runtime configuration
- **DLL Export/Import**: Automatic symbol export/import handling
- **Windows API**: WIN32_LEAN_AND_MEAN, NOMINMAX, UNICODE definitions
- **Manifest Support**: Application manifest configuration
- **Resource Files**: Windows resource file support

**Key Functions:**

- `qtforge_configure_windows_definitions()`
- `qtforge_configure_windows_runtime()`
- `qtforge_configure_windows_exports()`
- `qtforge_apply_windows_configuration()`

### Unix (`Unix.cmake`)

Unix/Linux-specific configurations for GCC and Clang:

- **Compiler Settings**: GCC/Clang-specific flags
- **RPATH Configuration**: Relative library path handling
- **Symbol Visibility**: Hidden visibility by default
- **Threading**: pthread support
- **Position Independent Code**: PIC enabled for shared libraries
- **Installation Paths**: Standard GNU installation directories

**Key Functions:**

- `qtforge_configure_unix_definitions()`
- `qtforge_configure_unix_rpath()`
- `qtforge_configure_unix_visibility()`
- `qtforge_apply_unix_configuration()`

### MSYS2 (`MSYS2.cmake`)

MSYS2 MinGW-specific configurations:

- **Subsystem Detection**: MINGW64, MINGW32, UCRT64, CLANG64 support
- **Qt6 Integration**: MSYS2 pacman-installed Qt6 detection
- **MinGW Runtime**: libstdc++ and libc++ support
- **Windows Compatibility**: Windows API definitions for MinGW
- **Exception Handling**: DWARF-2 exception handling
- **DLL Handling**: MinGW-specific DLL export/import

**Key Functions:**

- `qtforge_configure_msys2_definitions()`
- `qtforge_configure_msys2_qt6()`
- `qtforge_configure_msys2_linking()`
- `qtforge_apply_msys2_configuration()`

## Usage

Platform modules are automatically included based on platform detection in `QtForgePlatform.cmake`:

```cmake
# Platform modules are included automatically
include(QtForgePlatform)

# Platform-specific configuration is applied automatically
# No manual inclusion needed
```

## Platform Detection

Platform detection is handled by `QtForgePlatform.cmake`, which sets:

- `QTFORGE_IS_WINDOWS` - TRUE on Windows (non-MSYS2)
- `QTFORGE_IS_MSYS2` - TRUE in MSYS2 environments
- `QTFORGE_IS_LINUX` - TRUE on Linux
- `QTFORGE_IS_MACOS` - TRUE on macOS
- `QTFORGE_IS_ANDROID` - TRUE on Android
- `QTFORGE_IS_IOS` - TRUE on iOS

## Adding New Platforms

To add support for a new platform:

1. Create a new `.cmake` file in this directory
2. Follow the naming convention: `<PlatformName>.cmake`
3. Implement platform-specific configuration functions
4. Add platform detection logic to `QtForgePlatform.cmake`
5. Include the new module conditionally based on platform flags

## Best Practices

- **Use `include_guard(GLOBAL)`** to prevent multiple inclusions
- **Check platform flags** before applying configurations
- **Provide configuration functions** for each aspect (definitions, paths, linking, etc.)
- **Auto-apply configuration** when the module is included
- **Document all functions** using RST-style comments
- **Test on target platform** before committing changes

## Integration with Build System

Platform modules integrate with the main build system through:

1. **QtForgePlatform.cmake**: Detects platform and sets flags
2. **QtForgeCompiler.cmake**: Uses platform flags for compiler configuration
3. **QtForgeTargets.cmake**: Applies platform-specific target settings
4. **Root CMakeLists.txt**: Orchestrates platform detection and configuration

## Troubleshooting

### Windows Issues

- Ensure MSVC runtime library matches Qt6 build
- Check DLL export/import symbols are correct
- Verify Windows SDK version compatibility

### Unix Issues

- Check RPATH is set correctly for shared libraries
- Verify pthread linking for threaded applications
- Ensure symbol visibility is configured properly

### MSYS2 Issues

- Verify MSYSTEM environment variable is set
- Check Qt6 is installed via pacman (e.g., `mingw-w64-x86_64-qt6-base`)
- Ensure CMAKE_PREFIX_PATH includes MSYS2 Qt6 paths
- Verify MinGW toolchain is in PATH
