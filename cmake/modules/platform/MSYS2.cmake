# MSYS2.cmake MSYS2 MinGW-specific build configurations for QtForge Provides
# MSYS2 MinGW64/MinGW32/UCRT64 platform-specific settings

include_guard(GLOBAL)

#[=======================================================================[.rst:
MSYS2 Platform Configuration
-----------------------------

This module provides MSYS2 MinGW-specific build configurations including:

- MinGW compiler settings
- MSYS2 subsystem detection (MINGW64, MINGW32, UCRT64, CLANG64, etc.)
- Windows-compatible path handling
- MinGW-specific library linking

This module is automatically included when building in MSYS2 environments.
#]=======================================================================]

# Only apply MSYS2-specific settings in MSYS2 environment
if(NOT QTFORGE_IS_MSYS2)
  return()
endif()

message(STATUS "Configuring MSYS2-specific build settings")
message(STATUS "MSYS2 Subsystem: ${QTFORGE_MSYS2_SUBSYSTEM}")

#[=======================================================================[.rst:
qtforge_configure_msys2_definitions
------------------------------------

Adds MSYS2-specific compile definitions.
#]=======================================================================]
function(qtforge_configure_msys2_definitions)
  # MSYS2 platform identifier
  add_compile_definitions(QTFORGE_MSYS2 # MSYS2 platform identifier
  )

  # Subsystem-specific definitions
  if(QTFORGE_IS_MINGW64)
    add_compile_definitions(QTFORGE_MINGW64)
  elseif(QTFORGE_IS_MINGW32)
    add_compile_definitions(QTFORGE_MINGW32)
  elseif(QTFORGE_IS_UCRT64)
    add_compile_definitions(QTFORGE_UCRT64)
  elseif(QTFORGE_IS_CLANG64)
    add_compile_definitions(QTFORGE_CLANG64)
  endif()

  # Windows compatibility definitions
  add_compile_definitions(WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE)
endfunction()

#[=======================================================================[.rst:
qtforge_configure_msys2_paths
------------------------------

Configures MSYS2-specific path settings.
#]=======================================================================]
function(qtforge_configure_msys2_paths)
  # Set output directories (Windows-style)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/lib"
      PARENT_SCOPE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/bin"
      PARENT_SCOPE)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/bin"
      PARENT_SCOPE)

  # Detect MSYS2 installation path
  if(DEFINED ENV{MSYSTEM_PREFIX})
    set(QTFORGE_MSYS2_PREFIX
        $ENV{MSYSTEM_PREFIX}
        PARENT_SCOPE)
    message(STATUS "MSYS2: Installation prefix: $ENV{MSYSTEM_PREFIX}")
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_configure_msys2_qt6
----------------------------

Configures Qt6 detection for MSYS2 environments.
#]=======================================================================]
function(qtforge_configure_msys2_qt6)
  # MSYS2 Qt6 is typically installed via pacman Qt6 packages:
  # mingw-w64-x86_64-qt6-base, etc.

  if(DEFINED ENV{MSYSTEM_PREFIX})
    # Add MSYS2 Qt6 paths to CMAKE_PREFIX_PATH
    list(APPEND CMAKE_PREFIX_PATH "$ENV{MSYSTEM_PREFIX}/lib/cmake")
    set(CMAKE_PREFIX_PATH
        ${CMAKE_PREFIX_PATH}
        PARENT_SCOPE)

    message(
      STATUS "MSYS2: Added Qt6 search path: $ENV{MSYSTEM_PREFIX}/lib/cmake")
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_configure_msys2_runtime
--------------------------------

Configures runtime library settings for MSYS2.
#]=======================================================================]
function(qtforge_configure_msys2_runtime)
  # MinGW uses its own runtime, not MSVC runtime Ensure we're linking against
  # the correct C++ standard library

  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Link against libstdc++ (GCC's C++ standard library)
    message(STATUS "MSYS2: Using libstdc++ (GCC)")
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Link against libc++ or libstdc++ depending on configuration
    message(STATUS "MSYS2: Using Clang C++ standard library")
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_configure_msys2_linking
--------------------------------

Configures linking settings for MSYS2.
#]=======================================================================]
function(qtforge_configure_msys2_linking)
  # Enable automatic DLL export/import for MinGW
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS
      ON
      PARENT_SCOPE)

  # MinGW-specific linker flags
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Avoid console window for GUI applications add_link_options(-mwindows)  #
    # Uncomment if building GUI apps

    # Enable large address aware (for 32-bit builds)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
      add_link_options(-Wl,--large-address-aware)
    endif()
  endif()

  message(STATUS "MSYS2: MinGW linking configured")
endfunction()

#[=======================================================================[.rst:
qtforge_configure_msys2_exceptions
-----------------------------------

Configures exception handling for MSYS2.
#]=======================================================================]
function(qtforge_configure_msys2_exceptions)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Use DWARF-2 exception handling for better compatibility This is the
    # default for MinGW-w64
    message(STATUS "MSYS2: Using DWARF-2 exception handling")
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_apply_msys2_configuration
----------------------------------

Applies all MSYS2-specific configurations.
This is the main entry point called from the build system.
#]=======================================================================]
function(qtforge_apply_msys2_configuration)
  qtforge_configure_msys2_definitions()
  qtforge_configure_msys2_paths()
  qtforge_configure_msys2_qt6()
  qtforge_configure_msys2_runtime()
  qtforge_configure_msys2_linking()
  qtforge_configure_msys2_exceptions()

  message(STATUS "MSYS2 platform configuration applied")
endfunction()

# Auto-apply MSYS2 configuration when module is included
qtforge_apply_msys2_configuration()
