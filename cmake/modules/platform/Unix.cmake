# Unix.cmake Unix-specific build configurations for QtForge Provides Unix/Linux
# platform-specific compiler flags, definitions, and settings

include_guard(GLOBAL)

#[=======================================================================[.rst:
Unix Platform Configuration
----------------------------

This module provides Unix/Linux-specific build configurations including:

- GCC/Clang compiler settings
- Unix-specific definitions
- RPATH configuration
- Shared library settings

This module is automatically included when building on Unix/Linux platforms.
#]=======================================================================]

# Only apply Unix-specific settings on Unix platforms
if(NOT UNIX OR APPLE)
  return()
endif()

message(STATUS "Configuring Unix-specific build settings")

#[=======================================================================[.rst:
qtforge_configure_unix_definitions
-----------------------------------

Adds Unix-specific compile definitions.
#]=======================================================================]
function(qtforge_configure_unix_definitions)
  # Unix API definitions
  add_compile_definitions(_GNU_SOURCE # Enable GNU extensions
  )

  # Linux-specific definitions
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_compile_definitions(_LINUX # Linux platform identifier
    )
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_configure_unix_rpath
-----------------------------

Configures RPATH settings for Unix shared libraries.
#]=======================================================================]
function(qtforge_configure_unix_rpath)
  # Use RPATH for finding shared libraries
  set(CMAKE_SKIP_BUILD_RPATH
      FALSE
      PARENT_SCOPE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH
      FALSE
      PARENT_SCOPE)
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH
      TRUE
      PARENT_SCOPE)

  # Set RPATH to $ORIGIN for relative library paths
  set(CMAKE_INSTALL_RPATH
      "$ORIGIN:$ORIGIN/../lib"
      PARENT_SCOPE)

  message(STATUS "Unix: RPATH configured for shared libraries")
endfunction()

#[=======================================================================[.rst:
qtforge_configure_unix_visibility
----------------------------------

Configures symbol visibility for Unix shared libraries.
#]=======================================================================]
function(qtforge_configure_unix_visibility)
  # Hide symbols by default, export only what's needed
  set(CMAKE_CXX_VISIBILITY_PRESET
      hidden
      PARENT_SCOPE)
  set(CMAKE_VISIBILITY_INLINES_HIDDEN
      ON
      PARENT_SCOPE)

  message(STATUS "Unix: Symbol visibility set to hidden by default")
endfunction()

#[=======================================================================[.rst:
qtforge_configure_unix_threading
---------------------------------

Configures threading support for Unix platforms.
#]=======================================================================]
function(qtforge_configure_unix_threading)
  # Enable pthread support
  set(CMAKE_THREAD_PREFER_PTHREAD
      TRUE
      PARENT_SCOPE)
  set(THREADS_PREFER_PTHREAD_FLAG
      TRUE
      PARENT_SCOPE)

  message(STATUS "Unix: pthread threading support enabled")
endfunction()

#[=======================================================================[.rst:
qtforge_configure_unix_paths
-----------------------------

Configures Unix-specific path settings.
#]=======================================================================]
function(qtforge_configure_unix_paths)
  # Set standard Unix installation paths
  include(GNUInstallDirs)

  # Set output directories
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/lib"
      PARENT_SCOPE)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/lib"
      PARENT_SCOPE)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
      "${CMAKE_BINARY_DIR}/bin"
      PARENT_SCOPE)

  message(STATUS "Unix: Standard installation paths configured")
endfunction()

#[=======================================================================[.rst:
qtforge_configure_unix_pic
---------------------------

Configures Position Independent Code for Unix platforms.
#]=======================================================================]
function(qtforge_configure_unix_pic)
  # Enable Position Independent Code for shared libraries
  set(CMAKE_POSITION_INDEPENDENT_CODE
      ON
      PARENT_SCOPE)

  message(STATUS "Unix: Position Independent Code enabled")
endfunction()

#[=======================================================================[.rst:
qtforge_apply_unix_configuration
---------------------------------

Applies all Unix-specific configurations.
This is the main entry point called from the build system.
#]=======================================================================]
function(qtforge_apply_unix_configuration)
  qtforge_configure_unix_definitions()
  qtforge_configure_unix_rpath()
  qtforge_configure_unix_visibility()
  qtforge_configure_unix_threading()
  qtforge_configure_unix_paths()
  qtforge_configure_unix_pic()

  message(STATUS "Unix platform configuration applied")
endfunction()

# Auto-apply Unix configuration when module is included
qtforge_apply_unix_configuration()
