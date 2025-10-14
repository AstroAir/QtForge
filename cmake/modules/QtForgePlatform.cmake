# QtForgePlatform.cmake Platform and architecture detection module for QtForge
# Provides comprehensive platform detection and configuration

include_guard(GLOBAL)

# Platform detection variables
set(QTFORGE_PLATFORM_DETECTED FALSE)
set(QTFORGE_PLATFORM_NAME "unknown")
set(QTFORGE_PLATFORM_VERSION "unknown")
set(QTFORGE_ARCH_NAME "unknown")
set(QTFORGE_ARCH_BITS 0)

# Supported platforms
set(QTFORGE_SUPPORTED_PLATFORMS
    windows
    msys2
    macos
    linux
    android
    ios
    freebsd
    openbsd
    netbsd
    solaris
    qnx
    embedded)

# Supported architectures
set(QTFORGE_SUPPORTED_ARCHITECTURES
    x86
    x64
    arm
    arm64
    armv6
    armv7
    armv8
    mips
    mips64
    mipsel
    mips64el
    riscv32
    riscv64
    ppc
    ppc64
    ppc64le
    s390x
    sparc
    sparc64
    loongarch64
    alpha
    hppa
    m68k
    sh4)

#[=======================================================================[.rst:
qtforge_detect_platform
-----------------------

Detects the current platform and sets platform-specific variables.

Sets the following variables:
- QTFORGE_PLATFORM_NAME: Platform name (windows, macos, linux, etc.)
- QTFORGE_PLATFORM_VERSION: Platform version if detectable
- QTFORGE_IS_WINDOWS: TRUE if Windows
- QTFORGE_IS_MACOS: TRUE if macOS
- QTFORGE_IS_LINUX: TRUE if Linux
- QTFORGE_IS_ANDROID: TRUE if Android
- QTFORGE_IS_IOS: TRUE if iOS
- QTFORGE_IS_MOBILE: TRUE if mobile platform
- QTFORGE_IS_DESKTOP: TRUE if desktop platform
- QTFORGE_IS_EMBEDDED: TRUE if embedded platform
#]=======================================================================]
function(qtforge_detect_platform)
  if(QTFORGE_PLATFORM_DETECTED)
    return()
  endif()

  # Reset platform flags
  set(QTFORGE_IS_WINDOWS
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_MSYS2
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_MACOS
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_LINUX
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_ANDROID
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_IOS
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_FREEBSD
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_OPENBSD
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_NETBSD
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_SOLARIS
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_QNX
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_EMBEDDED
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_MOBILE
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_DESKTOP
      FALSE
      PARENT_SCOPE)

  # Detect platform
  if(WIN32)
    # Check if we're running in MSYS2 environment
    if(DEFINED ENV{MSYSTEM} OR DEFINED ENV{MSYS2_PATH_TYPE})
      set(QTFORGE_PLATFORM_NAME
          "msys2"
          PARENT_SCOPE)
      set(QTFORGE_IS_MSYS2
          TRUE
          PARENT_SCOPE)
      set(QTFORGE_IS_DESKTOP
          TRUE
          PARENT_SCOPE)

      # Detect MSYS2 subsystem type
      if(DEFINED ENV{MSYSTEM})
        set(QTFORGE_MSYS2_SUBSYSTEM
            $ENV{MSYSTEM}
            PARENT_SCOPE)
        if($ENV{MSYSTEM} STREQUAL "MINGW64")
          set(QTFORGE_IS_MINGW64
              TRUE
              PARENT_SCOPE)
        elseif($ENV{MSYSTEM} STREQUAL "MINGW32")
          set(QTFORGE_IS_MINGW32
              TRUE
              PARENT_SCOPE)
        elseif($ENV{MSYSTEM} STREQUAL "UCRT64")
          set(QTFORGE_IS_UCRT64
              TRUE
              PARENT_SCOPE)
        elseif($ENV{MSYSTEM} STREQUAL "CLANG64")
          set(QTFORGE_IS_CLANG64
              TRUE
              PARENT_SCOPE)
        elseif($ENV{MSYSTEM} STREQUAL "CLANG32")
          set(QTFORGE_IS_CLANG32
              TRUE
              PARENT_SCOPE)
        elseif($ENV{MSYSTEM} STREQUAL "MSYS")
          set(QTFORGE_IS_MSYS
              TRUE
              PARENT_SCOPE)
        endif()
      endif()

      # Detect MSYS2 version
      if(EXISTS "/usr/share/msys2-runtime/VERSION")
        file(READ "/usr/share/msys2-runtime/VERSION" MSYS2_VERSION)
        string(STRIP "${MSYS2_VERSION}" MSYS2_VERSION)
        set(QTFORGE_PLATFORM_VERSION
            ${MSYS2_VERSION}
            PARENT_SCOPE)
      endif()
    else()
      set(QTFORGE_PLATFORM_NAME
          "windows"
          PARENT_SCOPE)
      set(QTFORGE_IS_WINDOWS
          TRUE
          PARENT_SCOPE)
      set(QTFORGE_IS_DESKTOP
          TRUE
          PARENT_SCOPE)

      # Detect Windows version
      if(CMAKE_SYSTEM_VERSION)
        set(QTFORGE_PLATFORM_VERSION
            ${CMAKE_SYSTEM_VERSION}
            PARENT_SCOPE)
      endif()
    endif()

  elseif(APPLE)
    if(IOS)
      set(QTFORGE_PLATFORM_NAME
          "ios"
          PARENT_SCOPE)
      set(QTFORGE_IS_IOS
          TRUE
          PARENT_SCOPE)
      set(QTFORGE_IS_MOBILE
          TRUE
          PARENT_SCOPE)
    else()
      set(QTFORGE_PLATFORM_NAME
          "macos"
          PARENT_SCOPE)
      set(QTFORGE_IS_MACOS
          TRUE
          PARENT_SCOPE)
      set(QTFORGE_IS_DESKTOP
          TRUE
          PARENT_SCOPE)
    endif()

    # Detect macOS/iOS version
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
      set(QTFORGE_PLATFORM_VERSION
          ${CMAKE_OSX_DEPLOYMENT_TARGET}
          PARENT_SCOPE)
    endif()

  elseif(ANDROID)
    set(QTFORGE_PLATFORM_NAME
        "android"
        PARENT_SCOPE)
    set(QTFORGE_IS_ANDROID
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_MOBILE
        TRUE
        PARENT_SCOPE)

    # Detect Android API level
    if(ANDROID_PLATFORM)
      string(REGEX MATCH "[0-9]+" ANDROID_API_LEVEL ${ANDROID_PLATFORM})
      set(QTFORGE_PLATFORM_VERSION
          ${ANDROID_API_LEVEL}
          PARENT_SCOPE)
    endif()

  elseif(UNIX)
    # Detect specific Unix variants
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
      set(QTFORGE_PLATFORM_NAME
          "linux"
          PARENT_SCOPE)
      set(QTFORGE_IS_LINUX
          TRUE
          PARENT_SCOPE)
      set(QTFORGE_IS_DESKTOP
          TRUE
          PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
      set(QTFORGE_PLATFORM_NAME
          "freebsd"
          PARENT_SCOPE)
      set(QTFORGE_IS_FREEBSD
          TRUE
          PARENT_SCOPE)
      set(QTFORGE_IS_DESKTOP
          TRUE
          PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
      set(QTFORGE_PLATFORM_NAME
          "openbsd"
          PARENT_SCOPE)
      set(QTFORGE_IS_OPENBSD
          TRUE
          PARENT_SCOPE)
      set(QTFORGE_IS_DESKTOP
          TRUE
          PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
      set(QTFORGE_PLATFORM_NAME
          "netbsd"
          PARENT_SCOPE)
      set(QTFORGE_IS_NETBSD
          TRUE
          PARENT_SCOPE)
      set(QTFORGE_IS_DESKTOP
          TRUE
          PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
      set(QTFORGE_PLATFORM_NAME
          "solaris"
          PARENT_SCOPE)
      set(QTFORGE_IS_SOLARIS
          TRUE
          PARENT_SCOPE)
      set(QTFORGE_IS_DESKTOP
          TRUE
          PARENT_SCOPE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "QNX")
      set(QTFORGE_PLATFORM_NAME
          "qnx"
          PARENT_SCOPE)
      set(QTFORGE_IS_QNX
          TRUE
          PARENT_SCOPE)
      set(QTFORGE_IS_EMBEDDED
          TRUE
          PARENT_SCOPE)
    else()
      # Generic Unix - might be embedded
      set(QTFORGE_PLATFORM_NAME
          "unix"
          PARENT_SCOPE)
      set(QTFORGE_IS_EMBEDDED
          TRUE
          PARENT_SCOPE)
    endif()

    # Detect Linux distribution
    if(QTFORGE_IS_LINUX)
      qtforge_detect_linux_distribution()
    endif()
  endif()

  set(QTFORGE_PLATFORM_DETECTED
      TRUE
      PARENT_SCOPE)

  if(NOT QTFORGE_PLATFORM_VERSION STREQUAL "unknown")

  endif()
endfunction()

#[=======================================================================[.rst:
qtforge_detect_architecture
---------------------------

Detects the target architecture and sets architecture-specific variables.

Sets the following variables:
- QTFORGE_ARCH_NAME: Architecture name (x86, x64, arm, arm64, etc.)
- QTFORGE_ARCH_BITS: Architecture bit width (32, 64)
- QTFORGE_IS_X86: TRUE if x86 architecture
- QTFORGE_IS_X64: TRUE if x64 architecture
- QTFORGE_IS_ARM: TRUE if ARM architecture
- QTFORGE_IS_ARM64: TRUE if ARM64 architecture
#]=======================================================================]
function(qtforge_detect_architecture)
  # Reset architecture flags
  set(QTFORGE_IS_X86
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_X64
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_ARM
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_ARM64
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_MIPS
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_MIPS64
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_RISCV
      FALSE
      PARENT_SCOPE)
  set(QTFORGE_IS_PPC
      FALSE
      PARENT_SCOPE)

  # Detect architecture from CMAKE_SYSTEM_PROCESSOR
  string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" PROCESSOR_LOWER)

  if(PROCESSOR_LOWER MATCHES "^(x86_64|amd64|x64)$")
    set(QTFORGE_ARCH_NAME
        "x64"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_X64
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(i[3-6]86|x86)$")
    set(QTFORGE_ARCH_NAME
        "x86"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_X86
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(aarch64|arm64)$")
    set(QTFORGE_ARCH_NAME
        "arm64"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_ARM64
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(armv8|armv8-a)$")
    set(QTFORGE_ARCH_NAME
        "armv8"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_ARM64
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_ARMV8
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(armv7|armv7-a|armv7l)$")
    set(QTFORGE_ARCH_NAME
        "armv7"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_ARM
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_ARMV7
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(armv6|armv6l)$")
    set(QTFORGE_ARCH_NAME
        "armv6"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_ARM
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_ARMV6
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(arm|armv[4-5])$")
    set(QTFORGE_ARCH_NAME
        "arm"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_ARM
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(mips64el)$")
    set(QTFORGE_ARCH_NAME
        "mips64el"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_MIPS64
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_LITTLE_ENDIAN
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(mips64)$")
    set(QTFORGE_ARCH_NAME
        "mips64"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_MIPS64
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(mipsel)$")
    set(QTFORGE_ARCH_NAME
        "mipsel"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_MIPS
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_LITTLE_ENDIAN
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(mips)$")
    set(QTFORGE_ARCH_NAME
        "mips"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_MIPS
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(riscv64|rv64)$")
    set(QTFORGE_ARCH_NAME
        "riscv64"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_RISCV
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_RISCV64
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(riscv32|rv32)$")
    set(QTFORGE_ARCH_NAME
        "riscv32"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_RISCV
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_RISCV32
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(riscv64)$")
    set(QTFORGE_ARCH_NAME
        "riscv64"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_RISCV
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(riscv32|riscv)$")
    set(QTFORGE_ARCH_NAME
        "riscv32"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_RISCV
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(ppc64le|powerpc64le)$")
    set(QTFORGE_ARCH_NAME
        "ppc64le"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_PPC
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_PPC64
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_LITTLE_ENDIAN
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(ppc64|powerpc64)$")
    set(QTFORGE_ARCH_NAME
        "ppc64"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_PPC
        TRUE
        PARENT_SCOPE)
    set(QTFORGE_IS_PPC64
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(ppc|powerpc)$")
    set(QTFORGE_ARCH_NAME
        "ppc"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_PPC
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(loongarch64)$")
    set(QTFORGE_ARCH_NAME
        "loongarch64"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_LOONGARCH
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(s390x)$")
    set(QTFORGE_ARCH_NAME
        "s390x"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_S390X
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(alpha)$")
    set(QTFORGE_ARCH_NAME
        "alpha"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        64
        PARENT_SCOPE)
    set(QTFORGE_IS_ALPHA
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(hppa|parisc)$")
    set(QTFORGE_ARCH_NAME
        "hppa"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_HPPA
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(m68k)$")
    set(QTFORGE_ARCH_NAME
        "m68k"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_M68K
        TRUE
        PARENT_SCOPE)
  elseif(PROCESSOR_LOWER MATCHES "^(sh4)$")
    set(QTFORGE_ARCH_NAME
        "sh4"
        PARENT_SCOPE)
    set(QTFORGE_ARCH_BITS
        32
        PARENT_SCOPE)
    set(QTFORGE_IS_SH4
        TRUE
        PARENT_SCOPE)
  else()
    # Fallback to CMAKE_SIZEOF_VOID_P for bit detection
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(QTFORGE_ARCH_BITS
          64
          PARENT_SCOPE)
    else()
      set(QTFORGE_ARCH_BITS
          32
          PARENT_SCOPE)
    endif()
    set(QTFORGE_ARCH_NAME
        "unknown"
        PARENT_SCOPE)
    message(WARNING "QtForge: Unknown architecture: ${CMAKE_SYSTEM_PROCESSOR}")
  endif()

endfunction()

#[=======================================================================[.rst:
qtforge_detect_linux_distribution
---------------------------------

Detects the Linux distribution and version.
Sets QTFORGE_LINUX_DISTRO and QTFORGE_LINUX_DISTRO_VERSION variables.
#]=======================================================================]
function(qtforge_detect_linux_distribution)
  if(NOT QTFORGE_IS_LINUX)
    return()
  endif()

  set(QTFORGE_LINUX_DISTRO
      "unknown"
      PARENT_SCOPE)
  set(QTFORGE_LINUX_DISTRO_VERSION
      "unknown"
      PARENT_SCOPE)

  # Try to read /etc/os-release
  if(EXISTS "/etc/os-release")
    file(READ "/etc/os-release" OS_RELEASE_CONTENT)

    # Extract ID
    if(OS_RELEASE_CONTENT MATCHES "ID=([^\n]*)")
      string(STRIP "${CMAKE_MATCH_1}" DISTRO_ID)
      string(REPLACE "\"" "" DISTRO_ID "${DISTRO_ID}")
      set(QTFORGE_LINUX_DISTRO
          "${DISTRO_ID}"
          PARENT_SCOPE)
    endif()

    # Extract VERSION_ID
    if(OS_RELEASE_CONTENT MATCHES "VERSION_ID=([^\n]*)")
      string(STRIP "${CMAKE_MATCH_1}" VERSION_ID)
      string(REPLACE "\"" "" VERSION_ID "${VERSION_ID}")
      set(QTFORGE_LINUX_DISTRO_VERSION
          "${VERSION_ID}"
          PARENT_SCOPE)
    endif()
  endif()

endfunction()

# Initialize platform detection
qtforge_detect_platform()
qtforge_detect_architecture()
