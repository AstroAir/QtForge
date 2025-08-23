# iOS toolchain file for QtForge
# Cross-compilation toolchain for iOS platforms

# Ensure we're cross-compiling
set(CMAKE_SYSTEM_NAME iOS)

# Set minimum required CMake version
cmake_minimum_required(VERSION 3.21)

# iOS deployment target (default to 12.0)
if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "12.0")
endif()

# iOS SDK selection
if(NOT CMAKE_OSX_SYSROOT)
    if(QTFORGE_ENABLE_IOS_SIMULATOR OR CMAKE_GENERATOR MATCHES "Xcode")
        # Use simulator SDK for Xcode or when explicitly requested
        set(CMAKE_OSX_SYSROOT iphonesimulator)
        set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")  # Support both Intel and Apple Silicon simulators
    else()
        # Use device SDK
        set(CMAKE_OSX_SYSROOT iphoneos)
        set(CMAKE_OSX_ARCHITECTURES "arm64")  # Modern iOS devices are ARM64
    endif()
endif()

# Validate iOS SDK
execute_process(
    COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --show-sdk-path
    OUTPUT_VARIABLE IOS_SDK_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE IOS_SDK_RESULT
)

if(NOT IOS_SDK_RESULT EQUAL 0 OR NOT EXISTS "${IOS_SDK_PATH}")
    message(FATAL_ERROR "iOS SDK not found: ${CMAKE_OSX_SYSROOT}")
endif()

message(STATUS "Using iOS SDK: ${IOS_SDK_PATH}")

# Set compilers
execute_process(
    COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --find clang
    OUTPUT_VARIABLE CMAKE_C_COMPILER
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --find clang++
    OUTPUT_VARIABLE CMAKE_CXX_COMPILER
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set other tools
execute_process(
    COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --find ar
    OUTPUT_VARIABLE CMAKE_AR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --find ranlib
    OUTPUT_VARIABLE CMAKE_RANLIB
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND xcrun --sdk ${CMAKE_OSX_SYSROOT} --find strip
    OUTPUT_VARIABLE CMAKE_STRIP
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Set iOS-specific compiler flags
set(IOS_COMPILER_FLAGS "-fembed-bitcode")
set(IOS_COMPILER_FLAGS "${IOS_COMPILER_FLAGS} -fvisibility=hidden")
set(IOS_COMPILER_FLAGS "${IOS_COMPILER_FLAGS} -fvisibility-inlines-hidden")

# Architecture-specific flags
if(CMAKE_OSX_SYSROOT STREQUAL "iphoneos")
    set(IOS_COMPILER_FLAGS "${IOS_COMPILER_FLAGS} -miphoneos-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
elseif(CMAKE_OSX_SYSROOT STREQUAL "iphonesimulator")
    set(IOS_COMPILER_FLAGS "${IOS_COMPILER_FLAGS} -mios-simulator-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()

# Apply flags
set(CMAKE_C_FLAGS_INIT "${IOS_COMPILER_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${IOS_COMPILER_FLAGS}")

# Set library search paths
set(CMAKE_FIND_ROOT_PATH ${IOS_SDK_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# iOS-specific definitions
add_definitions(-DIOS)
add_definitions(-DQTFORGE_IOS)

# Disable shared libraries (iOS doesn't support them)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# Set install prefix for iOS
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install" CACHE PATH "Install prefix" FORCE)
endif()

# iOS bundle configuration
set(CMAKE_MACOSX_BUNDLE ON)
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")

# Qt for iOS specific settings
if(Qt6_DIR OR Qt5_DIR)
    # iOS deployment settings
    set(QT_IOS_DEPLOYMENT_TARGET ${CMAKE_OSX_DEPLOYMENT_TARGET})
    
    # Info.plist template
    if(EXISTS "${CMAKE_SOURCE_DIR}/ios/Info.plist.in")
        set(CMAKE_MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/ios/Info.plist.in")
    endif()
endif()

# Helper function for iOS-specific target configuration
function(configure_ios_target TARGET_NAME)
    # Set iOS-specific properties
    set_target_properties(${TARGET_NAME} PROPERTIES
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${CMAKE_OSX_DEPLOYMENT_TARGET}
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"  # iPhone and iPad
        XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH $<$<CONFIG:Debug>:YES>
    )
    
    # Add iOS-specific compile definitions
    target_compile_definitions(${TARGET_NAME} PRIVATE
        IOS
        QTFORGE_IOS
        TARGET_OS_IPHONE=1
    )
    
    # Link iOS frameworks
    target_link_libraries(${TARGET_NAME} PRIVATE
        "-framework Foundation"
        "-framework UIKit"
    )
endfunction()

# Print configuration summary
message(STATUS "iOS toolchain configuration:")
message(STATUS "  SDK: ${CMAKE_OSX_SYSROOT}")
message(STATUS "  SDK Path: ${IOS_SDK_PATH}")
message(STATUS "  Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
message(STATUS "  Architectures: ${CMAKE_OSX_ARCHITECTURES}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")

# Mark as configured
set(QTFORGE_IOS_TOOLCHAIN_CONFIGURED TRUE)
