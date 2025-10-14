# QtForgeCompiler.cmake Compiler detection and configuration module for QtForge
# Provides comprehensive compiler detection and optimization settings

include_guard(GLOBAL)

# Include platform detection
include(${CMAKE_CURRENT_LIST_DIR}/QtForgePlatform.cmake)

# Compiler detection variables
set(QTFORGE_COMPILER_DETECTED FALSE)
set(QTFORGE_COMPILER_NAME "unknown")
set(QTFORGE_COMPILER_VERSION "unknown")

# Supported compilers
set(QTFORGE_SUPPORTED_COMPILERS msvc gcc clang intel mingw)

#[=======================================================================[.rst:
qtforge_detect_compiler
-----------------------

Detects the current compiler and sets compiler-specific variables.

Sets the following variables:
- QTFORGE_COMPILER_NAME: Compiler name (msvc, gcc, clang, etc.)
- QTFORGE_COMPILER_VERSION: Compiler version
- QTFORGE_IS_MSVC: TRUE if Microsoft Visual C++
- QTFORGE_IS_GCC: TRUE if GNU GCC
- QTFORGE_IS_CLANG: TRUE if Clang
- QTFORGE_IS_INTEL: TRUE if Intel C++ Compiler
- QTFORGE_IS_MINGW: TRUE if MinGW
#]=======================================================================]
function(qtforge_detect_compiler)
  if(QTFORGE_COMPILER_DETECTED)
    return()
  endif()

  # Reset compiler flags
  set(QTFORGE_IS_MSVC
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_GCC
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_CLANG
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_INTEL
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_MINGW
      FALSE
      PARENT_SCOPE)

  # Detect compiler
  if(MSVC)
    set(QTFORGE_COMPILER_NAME
        "msvc"
        PARENT_SCOPE)
    set(QTFORGE_IS_MSVC
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_COMPILER_VERSION
        "${MSVC_VERSION}"
        PARENT_SCOPE)

  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(MINGW OR QTFORGE_IS_MSYS2)
      set(QTFORGE_COMPILER_NAME
          "mingw"
          PARENT_SCOPE)
      set(QTFORGE_IS_MINGW
          TRUE
          PARENT_SCOPE)

      # Detect specific MSYS2 MinGW variant
      if(QTFORGE_IS_MSYS2)
        if(QTFORGE_IS_MINGW64)
          set(QTFORGE_MINGW_VARIANT
              "mingw64"
              PARENT_SCOPE)
        elseif(QTFORGE_IS_MINGW32)
          set(QTFORGE_MINGW_VARIANT
              "mingw32"
              PARENT_SCOPE)
        elseif(QTFORGE_IS_UCRT64)
          set(QTFORGE_MINGW_VARIANT
              "ucrt64"
              PARENT_SCOPE)
        endif()
      endif()
    else()
      set(QTFORGE_COMPILER_NAME
          "gcc"
          PARENT_SCOPE)
      set(QTFORGE_IS_GCC
          TRUE
          PARENT_SCOPE)
    endif()
    set(QTFORGE_COMPILER_VERSION
        "${CMAKE_CXX_COMPILER_VERSION}"
        PARENT_SCOPE)

  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(QTFORGE_COMPILER_NAME
        "clang"
        PARENT_SCOPE)
    set(QTFORGE_IS_CLANG
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_COMPILER_VERSION
        "${CMAKE_CXX_COMPILER_VERSION}"
        PARENT_SCOPE)

    # Detect specific MSYS2 Clang variant
    if(QTFORGE_IS_MSYS2)
      if(QTFORGE_IS_CLANG64)
        set(QTFORGE_CLANG_VARIANT
            "clang64"
            PARENT_SCOPE)
      elseif(QTFORGE_IS_CLANG32)
        set(QTFORGE_CLANG_VARIANT
            "clang32"
            PARENT_SCOPE)
      endif()
    endif()

  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(QTFORGE_COMPILER_NAME
        "intel"
        PARENT_SCOPE)
    set(QTFORGE_IS_INTEL
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_COMPILER_VERSION
        "${CMAKE_CXX_COMPILER_VERSION}"
        PARENT_SCOPE)

  else()
    set(QTFORGE_COMPILER_NAME
        "unknown"
        PARENT_SCOPE)
    message(WARNING "QtForge: Unknown compiler: ${CMAKE_CXX_COMPILER_ID}")
  endif()

  set(QTFORGE_COMPILER_DETECTED
      TRUE
      PARENT_SCOPE)

endfunction()

#[=======================================================================[.rst:
qtforge_configure_compiler
-------------------------

Configures compiler-specific settings including warnings, optimizations,
and language features.

Options:
- ENABLE_WARNINGS: Enable comprehensive warnings (default: ON)
- ENABLE_WERROR: Treat warnings as errors (default: OFF)
- ENABLE_SANITIZERS: Enable sanitizers in debug builds (default: OFF)
- ENABLE_LTO: Enable Link Time Optimization (default: OFF)
- ENABLE_FAST_MATH: Enable fast math optimizations (default: OFF)
#]=======================================================================]
function(qtforge_configure_compiler)
  cmake_parse_arguments(
    QTFORGE_COMPILER
    "ENABLE_WARNINGS;ENABLE_WERROR;ENABLE_SANITIZERS;ENABLE_LTO;ENABLE_FAST_MATH"
    ""
    ""
    ${ARGN})

  # Default options
  if(NOT DEFINED QTFORGE_COMPILER_ENABLE_WARNINGS)
    set(QTFORGE_COMPILER_ENABLE_WARNINGS ON)
  endif()

  # Configure C++ standard
  set(CMAKE_CXX_STANDARD
      20
      PARENT_SCOPE)
  set(CMAKE_CXX_STANDARD_REQUIRED
      ON
      PARENT_SCOPE)
  set(CMAKE_CXX_EXTENSIONS
      OFF
      PARENT_SCOPE)

  # Enable position independent code
  set(CMAKE_POSITION_INDEPENDENT_CODE
      ON
      PARENT_SCOPE)

  # MSVC-specific configuration
  if(QTFORGE_IS_MSVC)
    qtforge_configure_msvc(
      ENABLE_WARNINGS
      ${QTFORGE_COMPILER_ENABLE_WARNINGS}
      ENABLE_WERROR
      ${QTFORGE_COMPILER_ENABLE_WERROR}
      ENABLE_LTO
      ${QTFORGE_COMPILER_ENABLE_LTO}
      ENABLE_FAST_MATH
      ${QTFORGE_COMPILER_ENABLE_FAST_MATH})
  endif()

  # GCC-specific configuration
  if(QTFORGE_IS_GCC OR QTFORGE_IS_MINGW)
    qtforge_configure_gcc(
      ENABLE_WARNINGS
      ${QTFORGE_COMPILER_ENABLE_WARNINGS}
      ENABLE_WERROR
      ${QTFORGE_COMPILER_ENABLE_WERROR}
      ENABLE_SANITIZERS
      ${QTFORGE_COMPILER_ENABLE_SANITIZERS}
      ENABLE_LTO
      ${QTFORGE_COMPILER_ENABLE_LTO}
      ENABLE_FAST_MATH
      ${QTFORGE_COMPILER_ENABLE_FAST_MATH})
  endif()

  # Clang-specific configuration
  if(QTFORGE_IS_CLANG)
    qtforge_configure_clang(
      ENABLE_WARNINGS
      ${QTFORGE_COMPILER_ENABLE_WARNINGS}
      ENABLE_WERROR
      ${QTFORGE_COMPILER_ENABLE_WERROR}
      ENABLE_SANITIZERS
      ${QTFORGE_COMPILER_ENABLE_SANITIZERS}
      ENABLE_LTO
      ${QTFORGE_COMPILER_ENABLE_LTO}
      ENABLE_FAST_MATH
      ${QTFORGE_COMPILER_ENABLE_FAST_MATH})
  endif()

  # Intel compiler configuration
  if(QTFORGE_IS_INTEL)
    qtforge_configure_intel(
      ENABLE_WARNINGS
      ${QTFORGE_COMPILER_ENABLE_WARNINGS}
      ENABLE_WERROR
      ${QTFORGE_COMPILER_ENABLE_WERROR}
      ENABLE_LTO
      ${QTFORGE_COMPILER_ENABLE_LTO}
      ENABLE_FAST_MATH
      ${QTFORGE_COMPILER_ENABLE_FAST_MATH})
  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_configure_msvc
---------------------

Configures MSVC-specific compiler settings.
#]=======================================================================]
function(qtforge_configure_msvc)
  cmake_parse_arguments(
    MSVC "ENABLE_WARNINGS;ENABLE_WERROR;ENABLE_LTO;ENABLE_FAST_MATH" "" ""
    ${ARGN})

  # Basic MSVC flags
  add_compile_options(/permissive- /Zc:__cplusplus)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

  # Enable coroutines
  add_compile_options(/await)

  # Warning configuration
  if(MSVC_ENABLE_WARNINGS)
    add_compile_options(/W4)

    if(MSVC_ENABLE_WERROR)
      add_compile_options(/WX)
    endif()
  endif()

  # Runtime library configuration
  set(CMAKE_MSVC_RUNTIME_LIBRARY
      "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
      PARENT_SCOPE)

  # Link Time Optimization
  if(MSVC_ENABLE_LTO)
    add_compile_options($<$<CONFIG:Release>:/GL>)
    add_link_options($<$<CONFIG:Release>:/LTCG>)
  endif()

  # Fast math
  if(MSVC_ENABLE_FAST_MATH)
    add_compile_options(/fp:fast)
  endif()

  # Debug information
  add_compile_options($<$<CONFIG:Debug>:/Zi>)
  add_link_options($<$<CONFIG:Debug>:/DEBUG>)

endfunction()

#[=======================================================================[.rst:
qtforge_configure_gcc
--------------------

Configures GCC-specific compiler settings.
#]=======================================================================]
function(qtforge_configure_gcc)
  cmake_parse_arguments(
    GCC
    "ENABLE_WARNINGS;ENABLE_WERROR;ENABLE_SANITIZERS;ENABLE_LTO;ENABLE_FAST_MATH"
    ""
    ""
    ${ARGN})

  # Enable coroutines
  add_compile_options(-fcoroutines)

  # Warning configuration
  if(GCC_ENABLE_WARNINGS)
    add_compile_options(-Wall -Wextra -Wpedantic)

    # Only enable -Werror if explicitly requested and QTFORGE_ENABLE_WERROR is
    # ON
    if(GCC_ENABLE_WERROR AND QTFORGE_ENABLE_WERROR)
      add_compile_options(-Werror)
    endif()

    # Disable specific warnings that are treated as errors during migration
    add_compile_options(-Wno-error=deprecated-declarations)
  endif()

  # Sanitizers (Debug builds only)
  if(GCC_ENABLE_SANITIZERS AND QTFORGE_ENABLE_SANITIZERS)
    add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
  endif()

  # Link Time Optimization Temporarily disabled due to linking issues with GCC
  # 15.2.0 if(GCC_ENABLE_LTO) add_compile_options($<$<CONFIG:Release>:-flto>)
  # add_link_options($<$<CONFIG:Release>:-flto>) endif()

  # Fast math
  if(GCC_ENABLE_FAST_MATH)
    add_compile_options($<$<CONFIG:Release>:-ffast-math>)
  endif()

  # Debug information
  add_compile_options($<$<CONFIG:Debug>:-g3>)

  # Optimization
  add_compile_options($<$<CONFIG:Release>:-O3>)

endfunction()

#[=======================================================================[.rst:
qtforge_configure_clang
----------------------

Configures Clang-specific compiler settings.
#]=======================================================================]
function(qtforge_configure_clang)
  cmake_parse_arguments(
    CLANG
    "ENABLE_WARNINGS;ENABLE_WERROR;ENABLE_SANITIZERS;ENABLE_LTO;ENABLE_FAST_MATH"
    ""
    ""
    ${ARGN})

  # Enable coroutines
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "14.0")
    add_compile_options(-fcoroutines)
  else()
    add_compile_options(-fcoroutines-ts)
  endif()

  # Warning configuration
  if(CLANG_ENABLE_WARNINGS)
    add_compile_options(-Wall -Wextra -Wpedantic)

    # Only enable -Werror if explicitly requested and QTFORGE_ENABLE_WERROR is
    # ON
    if(CLANG_ENABLE_WERROR AND QTFORGE_ENABLE_WERROR)
      add_compile_options(-Werror)
    endif()

    # Disable specific warnings that are treated as errors during migration
    add_compile_options(-Wno-error=deprecated-declarations)
  endif()

  # Sanitizers (Debug builds only)
  if(CLANG_ENABLE_SANITIZERS AND QTFORGE_ENABLE_SANITIZERS)
    add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
  endif()

  # Link Time Optimization
  if(CLANG_ENABLE_LTO)
    add_compile_options($<$<CONFIG:Release>:-flto>)
    add_link_options($<$<CONFIG:Release>:-flto>)
  endif()

  # Fast math
  if(CLANG_ENABLE_FAST_MATH)
    add_compile_options($<$<CONFIG:Release>:-ffast-math>)
  endif()

  # Debug information
  add_compile_options($<$<CONFIG:Debug>:-g>)

  # Optimization
  add_compile_options($<$<CONFIG:Release>:-O3>)

endfunction()

#[=======================================================================[.rst:
qtforge_configure_intel
----------------------

Configures Intel C++ Compiler settings.
#]=======================================================================]
function(qtforge_configure_intel)
  cmake_parse_arguments(
    INTEL "ENABLE_WARNINGS;ENABLE_WERROR;ENABLE_LTO;ENABLE_FAST_MATH" "" ""
    ${ARGN})

  # Warning configuration
  if(INTEL_ENABLE_WARNINGS)
    add_compile_options(-Wall)

    if(INTEL_ENABLE_WERROR)
      add_compile_options(-Werror)
    endif()
  endif()

  # Link Time Optimization
  if(INTEL_ENABLE_LTO)
    add_compile_options($<$<CONFIG:Release>:-ipo>)
  endif()

  # Fast math
  if(INTEL_ENABLE_FAST_MATH)
    add_compile_options($<$<CONFIG:Release>:-fast>)
  endif()

  # Optimization
  add_compile_options($<$<CONFIG:Release>:-O3>)

endfunction()

# Initialize compiler detection
qtforge_detect_compiler()
