# QtForge Remote Plugin System CMake Configuration
# Version 3.2.0

cmake_minimum_required(VERSION 3.21)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
    Concurrent
    Test
)

# Find OpenSSL for cryptographic operations (still needed for HTTPS)
find_package(OpenSSL REQUIRED)

# === Remote Plugin System Libraries ===

# Security Manager - Removed (SHA256 verification preserved in core PluginManager)

# Remote Plugin Manager
add_library(QtForgeRemoteManager SHARED
    core/remote_plugin_manager.cpp
    core/remote_plugin_cache.cpp
    core/remote_plugin_loader.cpp
    ../../include/qtplugin/remote/core/remote_plugin_manager.hpp
)

target_link_libraries(QtForgeRemoteManager
    Qt6::Core
    Qt6::Network
    Qt6::Concurrent
    QtForgeCore
)

# Unified Integration Layer
add_library(QtForgeRemoteIntegration SHARED
    integration/unified_plugin_manager.cpp
    integration/unified_repository_manager.cpp
    ../../include/qtplugin/remote/integration/unified_plugin_manager.hpp
)

target_link_libraries(QtForgeRemoteIntegration
    Qt6::Core
    Qt6::Network
    Qt6::Concurrent
    QtForgeCore
    QtForgeRemoteManager
)

# All-in-One Remote Library
add_library(QtForgeRemote SHARED)

target_link_libraries(QtForgeRemote
    QtForgeRemoteManager
    QtForgeRemoteIntegration
)

set_target_properties(QtForgeRemote PROPERTIES
    EXPORT_NAME Remote
)

# === Installation ===
if(QTFORGE_INSTALL)
    install(TARGETS
        QtForgeRemoteManager
        QtForgeRemoteIntegration
        QtForgeRemote
        EXPORT QtForgeRemoteTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(DIRECTORY ../../include/qtplugin/remote/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtplugin/remote
        FILES_MATCHING PATTERN "*.hpp"
    )
endif()

message(STATUS "QtForge Remote Plugin System configured successfully")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
    Concurrent
    Test
)

# Find OpenSSL for cryptographic operations
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include/qtplugin/remote
)

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3 -DNDEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-g -O0)
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2 /DNDEBUG)
    elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/Od /Zi)
    endif()
endif()

# === Remote Security Manager - Removed ===
# Security components have been removed from QtForge
# SHA256 verification is preserved in the core PluginManager

# === Remote Plugin Manager ===
set(REMOTE_MANAGER_SOURCES
    core/remote_plugin_manager.cpp
    core/remote_plugin_cache.cpp
    core/remote_plugin_loader.cpp
    core/remote_plugin_repository.cpp
)

set(REMOTE_MANAGER_HEADERS
    ../../include/qtplugin/remote/core/remote_plugin_manager.hpp
)

add_library(QtForgeRemoteManager SHARED
    ${REMOTE_MANAGER_SOURCES}
    ${REMOTE_MANAGER_HEADERS}
)

target_link_libraries(QtForgeRemoteManager
    Qt6::Core
    Qt6::Network
    Qt6::Concurrent
    QtForgeCore
)

target_compile_definitions(QtForgeRemoteManager PRIVATE
    QTFORGE_REMOTE_MANAGER_LIBRARY
)

set_target_properties(QtForgeRemoteManager PROPERTIES
    VERSION ${QtForge_VERSION}
    SOVERSION ${QtForge_VERSION_MAJOR}
    EXPORT_NAME RemoteManager
)

# === Unified Integration Layer ===
set(INTEGRATION_SOURCES
    integration/unified_plugin_manager.cpp
    integration/unified_repository_manager.cpp
    integration/load_strategy_manager.cpp
)

set(INTEGRATION_HEADERS
    ../../include/qtplugin/remote/integration/unified_plugin_manager.hpp
)

add_library(QtForgeRemoteIntegration SHARED
    ${INTEGRATION_SOURCES}
    ${INTEGRATION_HEADERS}
)

target_link_libraries(QtForgeRemoteIntegration
    Qt6::Core
    Qt6::Network
    Qt6::Concurrent
    QtForgeCore
    QtForgeRemoteManager
)

target_compile_definitions(QtForgeRemoteIntegration PRIVATE
    QTFORGE_REMOTE_INTEGRATION_LIBRARY
)

set_target_properties(QtForgeRemoteIntegration PROPERTIES
    VERSION ${QtForge_VERSION}
    SOVERSION ${QtForge_VERSION_MAJOR}
    EXPORT_NAME RemoteIntegration
)

# === All-in-One Remote Library ===
add_library(QtForgeRemote SHARED)

target_link_libraries(QtForgeRemote
    QtForgeRemoteManager
    QtForgeRemoteIntegration
)

set_target_properties(QtForgeRemote PROPERTIES
    VERSION ${QtForge_VERSION}
    SOVERSION ${QtForge_VERSION_MAJOR}
    EXPORT_NAME Remote
)

# === Installation ===
if(QTFORGE_INSTALL)
    # Install libraries
    install(TARGETS
        QtForgeRemoteManager
        QtForgeRemoteIntegration
        QtForgeRemote
        EXPORT QtForgeRemoteTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    # Install headers
    install(DIRECTORY ../../include/qtplugin/remote/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtplugin/remote
        FILES_MATCHING PATTERN "*.hpp"
    )

    # Export targets
    install(EXPORT QtForgeRemoteTargets
        FILE QtForgeRemoteTargets.cmake
        NAMESPACE QtForge::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtForge
    )

    # Create package config files
    include(CMakePackageConfigHelpers)
    
    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/QtForgeRemoteConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/QtForgeRemoteConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtForge
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/QtForgeRemoteConfigVersion.cmake"
        VERSION ${QtForge_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/QtForgeRemoteConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/QtForgeRemoteConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QtForge
    )
endif()

# === Examples ===
if(QTFORGE_BUILD_EXAMPLES AND QTFORGE_BUILD_REMOTE_EXAMPLES)
    add_subdirectory(../../examples/remote_plugins examples)
endif()

# === Tests ===
if(QTFORGE_BUILD_TESTS AND QTFORGE_BUILD_REMOTE_TESTS)
    enable_testing()
    add_subdirectory(../../tests/remote tests)
endif()

# === Tools ===
if(QTFORGE_BUILD_TOOLS AND QTFORGE_BUILD_REMOTE_TOOLS)
    add_subdirectory(tools)
endif()

# === Summary ===
message(STATUS "QtForge Remote Plugin System Configuration:")
message(STATUS "  - Remote Security Library: Removed (SHA256 preserved in core)")
message(STATUS "  - Remote Manager Library: QtForgeRemoteManager")
message(STATUS "  - Integration Library: QtForgeRemoteIntegration")
message(STATUS "  - Unified Library: QtForgeRemote")
message(STATUS "  - Examples: ${QTFORGE_BUILD_REMOTE_EXAMPLES}")
message(STATUS "  - Tests: ${QTFORGE_BUILD_REMOTE_TESTS}")
message(STATUS "  - Tools: ${QTFORGE_BUILD_REMOTE_TOOLS}")
message(STATUS "  - Installation: ${QTFORGE_INSTALL}")

# === CMake Package Configuration Template ===
# Create QtForgeRemoteConfig.cmake.in
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/QtForgeRemoteConfig.cmake.in" "
@PACKAGE_INIT@

# QtForge Remote Plugin System Package Configuration
# Version @QtForge_VERSION@

include(CMakeFindDependencyMacro)

# Find required dependencies
find_dependency(Qt6 REQUIRED COMPONENTS Core Network Concurrent)
find_dependency(OpenSSL REQUIRED)

# Include target definitions
include(\"\${CMAKE_CURRENT_LIST_DIR}/QtForgeRemoteTargets.cmake\")

# Define convenience variables
set(QtForgeRemote_LIBRARIES
    QtForge::RemoteManager
    QtForge::RemoteIntegration
    QtForge::Remote
)

set(QtForgeRemote_INCLUDE_DIRS \"\${PACKAGE_PREFIX_DIR}/include\")

# Compatibility aliases
add_library(QtForge::RemotePluginSystem ALIAS QtForge::Remote)

check_required_components(QtForgeRemote)

# Additional configuration
set(QTFORGE_REMOTE_VERSION \"@QtForge_VERSION@\")
set(QTFORGE_REMOTE_VERSION_MAJOR @QtForge_VERSION_MAJOR@)
set(QTFORGE_REMOTE_VERSION_MINOR @QtForge_VERSION_MINOR@)
set(QTFORGE_REMOTE_VERSION_PATCH @QtForge_VERSION_PATCH@)

# Helper macro for adding remote plugins
macro(qtforge_add_remote_plugin target)
    cmake_parse_arguments(PLUGIN
        \"SIGNATURE_REQUIRED\"
        \"PUBLISHER_ID;REPOSITORY_URL;CATEGORY\"
        \"SOURCES;HEADERS;METADATA;TAGS\"
        \${ARGN}
    )
    
    # Create the plugin target
    add_library(\${target} SHARED \${PLUGIN_SOURCES} \${PLUGIN_HEADERS})
    
    # Link with remote plugin system
    target_link_libraries(\${target} 
        QtForge::Remote
        Qt6::Core
    )
    
    # Set plugin properties
    set_target_properties(\${target} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY \"\${CMAKE_BINARY_DIR}/plugins\"
        RUNTIME_OUTPUT_DIRECTORY \"\${CMAKE_BINARY_DIR}/plugins\"
    )
    
    # Add metadata if provided
    if(PLUGIN_METADATA)
        configure_file(\${PLUGIN_METADATA} 
                      \"\${CMAKE_BINARY_DIR}/plugins/\${target}.json\"
                      COPYONLY)
    endif()
    
    # Generate signature if required
    if(PLUGIN_SIGNATURE_REQUIRED)
        find_program(QTFORGE_SIGN_TOOL qtforge-sign)
        if(QTFORGE_SIGN_TOOL)
            add_custom_command(TARGET \${target} POST_BUILD
                COMMAND \${QTFORGE_SIGN_TOOL} 
                    --plugin $<TARGET_FILE:\${target}>
                    --publisher \${PLUGIN_PUBLISHER_ID}
                    --output $<TARGET_FILE:\${target}>.sig
                COMMENT \"Signing plugin \${target}\"
            )
        endif()
    endif()
endmacro()

message(STATUS \"Found QtForge Remote Plugin System version \${QTFORGE_REMOTE_VERSION}\")
")

# === Build Options ===
option(QTFORGE_BUILD_REMOTE_EXAMPLES "Build remote plugin examples" ${QTFORGE_BUILD_EXAMPLES})
option(QTFORGE_BUILD_REMOTE_TESTS "Build remote plugin tests" ${QTFORGE_BUILD_TESTS})
option(QTFORGE_BUILD_REMOTE_TOOLS "Build remote plugin tools" ${QTFORGE_BUILD_TOOLS})
option(QTFORGE_REMOTE_ENABLE_OPENSSL "Enable OpenSSL support" ON)
option(QTFORGE_REMOTE_ENABLE_SANDBOX "Enable plugin sandboxing" ON)

# === Feature Configuration ===
# Security features removed - QtForgeRemoteSecurity library no longer exists
# OpenSSL and sandbox features have been removed

# === pkg-config Support ===
if(QTFORGE_INSTALL AND NOT WIN32)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/QtForgeRemote.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/QtForgeRemote.pc"
        @ONLY
    )
    
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/QtForgeRemote.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
    )
endif()

# Create QtForgeRemote.pc.in for pkg-config
file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/QtForgeRemote.pc.in" "
prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=\${prefix}
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

Name: QtForge Remote Plugin System
Description: Remote plugin system for QtForge applications
Version: @QtForge_VERSION@
Requires: Qt6Core Qt6Network Qt6Concurrent
Libs: -L\${libdir} -lQtForgeRemote
Cflags: -I\${includedir}
")
