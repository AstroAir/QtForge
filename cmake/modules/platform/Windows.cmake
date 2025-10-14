# Windows.cmake Windows-specific build configurations for QtForge Provides
# Windows platform-specific compiler flags, definitions, and settings

include_guard(GLOBAL)

#[=======================================================================[.rst:
Windows Platform Configuration
-------------------------------

This module provides Windows-specific build configurations including:

- MSVC compiler settings
- Windows-specific definitions
- Windows manifest and resource support
- DLL export/import handling

This module is automatically included when building on Windows platforms.
#]=======================================================================]

# Only apply Windows-specific settings on Windows
if(NOT WIN32)
  return()
endif()

message(STATUS "Configuring Windows-specific build settings")

#[=======================================================================[.rst:
qtforge_configure_windows_definitions
--------------------------------------

Adds Windows-specific compile definitions.
#]=======================================================================]
function(qtforge_configure_windows_definitions)
  # Windows API definitions
  add_compile_definitions(
    WIN32_LEAN_AND_MEAN # Exclude rarely-used Windows headers
    NOMINMAX # Prevent min/max macro conflicts
    UNICODE # Use Unicode character set
    _UNICODE # Use Unicode character set
  )

  # Suppress common Windows warnings
  if(MSVC)
    add_compile_definitions(
      _CRT_SECURE_NO_WARNINGS # Disable CRT security warnings
      _CRT_NONSTDC_NO_DEPRECATE # Disable POSIX function warnings
      _SCL_SECURE_NO_WARNINGS # Disable STL security warnings
    )
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_configure_windows_runtime
----------------------------------

Configures Windows runtime library settings.
#]=======================================================================]
function(qtforge_configure_windows_runtime)
  if(MSVC)
    # Use MultiThreaded DLL runtime by default
    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
        PARENT_SCOPE)

    message(STATUS "Windows: Using MultiThreaded DLL runtime")
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_configure_windows_exports
----------------------------------

Configures DLL export/import settings for Windows.
#]=======================================================================]
function(qtforge_configure_windows_exports)
  # Enable automatic DLL export/import
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
      ON
      PARENT_SCOPE)

  # Generate import libraries for DLLs
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/lib"
      PARENT_SCOPE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/bin"
      PARENT_SCOPE)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/bin"
      PARENT_SCOPE)

  message(STATUS "Windows: DLL export/import configured")
endfunction()

#[=======================================================================[.rst:
qtforge_configure_windows_paths
--------------------------------

Configures Windows-specific path settings.
#]=======================================================================]
function(qtforge_configure_windows_paths)
  # Set DLL search paths for runtime
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(QTFORGE_WINDOWS_ARCH
        "x64"
        PARENT_SCOPE)
  else()
    set(QTFORGE_WINDOWS_ARCH
        "x86"
        PARENT_SCOPE)
  endif()

  message(STATUS "Windows: Architecture ${QTFORGE_WINDOWS_ARCH}")
endfunction()

#[=======================================================================[.rst:
qtforge_configure_windows_manifest
-----------------------------------

Configures Windows application manifest support.
#]=======================================================================]
function(qtforge_configure_windows_manifest)
  if(QTFORGE_ENABLE_WINDOWS_MANIFEST)
    message(STATUS "Windows: Application manifest support enabled")
    # Manifest configuration is handled per-target in QtForgeTargets.cmake
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_configure_windows_resources
------------------------------------

Configures Windows resource file support.
#]=======================================================================]
function(qtforge_configure_windows_resources)
  if(QTFORGE_ENABLE_WINDOWS_RESOURCES)
    message(STATUS "Windows: Resource file support enabled")
    # Resource configuration is handled per-target in QtForgeTargets.cmake
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_apply_windows_configuration
------------------------------------

Applies all Windows-specific configurations.
This is the main entry point called from the build system.
#]=======================================================================]
function(qtforge_apply_windows_configuration)
  qtforge_configure_windows_definitions()
  qtforge_configure_windows_runtime()
  qtforge_configure_windows_exports()
  qtforge_configure_windows_paths()
  qtforge_configure_windows_manifest()
  qtforge_configure_windows_resources()

  message(STATUS "Windows platform configuration applied")
endfunction()

# Auto-apply Windows configuration when module is included
qtforge_apply_windows_configuration()
