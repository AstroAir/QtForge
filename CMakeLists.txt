# QtPlugin Library CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(QtPlugin
    VERSION 3.0.0
    DESCRIPTION "Modern C++ Plugin System for Qt Applications"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find Qt6 Core (minimal dependency)
find_package(Qt6 REQUIRED COMPONENTS Core)

# Optional components
find_package(Qt6 COMPONENTS Network Widgets Sql StateMachine Concurrent QUIET)

# Library configuration options
option(QTPLUGIN_BUILD_NETWORK "Build network plugin support" OFF)
option(QTPLUGIN_BUILD_UI "Build UI plugin support" OFF)
option(QTPLUGIN_BUILD_EXAMPLES "Build example plugins" ON)
option(QTPLUGIN_BUILD_TESTS "Build unit tests" OFF)
option(QTPLUGIN_BUILD_COMPONENT_TESTS "Build component-specific tests" OFF)
option(QTPLUGIN_ENABLE_COMPONENT_LOGGING "Enable detailed component logging" OFF)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /permissive- /Zc:__cplusplus)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    # Enable coroutines for MSVC
    add_compile_options(/await)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    # Enable coroutines
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fcoroutines)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-fcoroutines-ts)
    endif()
endif()

# Core library sources
set(QTPLUGIN_CORE_SOURCES
    src/qtplugin.cpp
    src/core/plugin_interface.cpp
    src/core/plugin_manager.cpp
    src/core/plugin_loader.cpp
    src/core/plugin_registry.cpp
    src/core/plugin_dependency_resolver.cpp
    src/monitoring/plugin_hot_reload_manager.cpp
    src/monitoring/plugin_metrics_collector.cpp
    src/communication/message_bus.cpp
    src/communication/request_response_system.cpp
    src/utils/version.cpp
    src/utils/error_handling.cpp
    src/security/security_manager.cpp
    src/security/components/security_validator.cpp
    src/security/components/signature_verifier.cpp
    src/security/components/permission_manager.cpp
    src/security/components/security_policy_engine.cpp
    src/managers/configuration_manager.cpp
    src/managers/components/configuration_storage.cpp
    src/managers/components/configuration_validator.cpp
    src/managers/components/configuration_merger.cpp
    src/managers/components/configuration_watcher.cpp
    src/managers/logging_manager.cpp
    src/managers/resource_manager.cpp
    src/managers/resource_lifecycle.cpp
    src/managers/components/resource_pool.cpp
    src/managers/components/resource_allocator.cpp
    src/managers/resource_monitor.cpp
    # Enhanced plugin system sources (v3.1.0)
    src/communication/plugin_service_contracts.cpp
    src/core/enhanced_plugin_interface.cpp
    src/orchestration/plugin_orchestrator.cpp
    src/composition/plugin_composition.cpp
    src/transactions/plugin_transaction_manager.cpp
    # Version management sources (v3.1.0)
    src/managers/plugin_version_manager.cpp
)

# Core library headers
set(QTPLUGIN_CORE_HEADERS
    include/qtplugin/core/plugin_interface.hpp
    include/qtplugin/core/plugin_manager.hpp
    include/qtplugin/core/plugin_loader.hpp
    include/qtplugin/core/plugin_registry.hpp
    include/qtplugin/core/plugin_dependency_resolver.hpp
    include/qtplugin/monitoring/plugin_hot_reload_manager.hpp
    include/qtplugin/monitoring/plugin_metrics_collector.hpp
    include/qtplugin/core/service_plugin_interface.hpp
    include/qtplugin/communication/message_bus.hpp
    include/qtplugin/communication/message_types.hpp
    include/qtplugin/communication/request_response_system.hpp
    include/qtplugin/utils/version.hpp
    include/qtplugin/utils/error_handling.hpp
    include/qtplugin/security/security_manager.hpp
    include/qtplugin/security/components/security_validator.hpp
    include/qtplugin/security/components/signature_verifier.hpp
    include/qtplugin/security/components/permission_manager.hpp
    include/qtplugin/security/components/security_policy_engine.hpp
    include/qtplugin/managers/configuration_manager.hpp
    include/qtplugin/managers/configuration_manager_impl.hpp
    include/qtplugin/managers/components/configuration_storage.hpp
    include/qtplugin/managers/components/configuration_validator.hpp
    include/qtplugin/managers/components/configuration_merger.hpp
    include/qtplugin/managers/components/configuration_watcher.hpp
    include/qtplugin/managers/logging_manager.hpp
    include/qtplugin/managers/logging_manager_impl.hpp
    include/qtplugin/managers/resource_manager.hpp
    include/qtplugin/managers/resource_manager_impl.hpp
    include/qtplugin/managers/resource_pools.hpp
    include/qtplugin/managers/resource_lifecycle.hpp
    include/qtplugin/managers/resource_lifecycle_impl.hpp
    include/qtplugin/managers/components/resource_pool.hpp
    include/qtplugin/managers/components/resource_allocator.hpp
    include/qtplugin/managers/resource_monitor_impl.hpp
    include/qtplugin/qtplugin.hpp
    include/qtplugin/components.hpp
    # Enhanced plugin system headers (v3.1.0)
    include/qtplugin/communication/plugin_service_contracts.hpp
    include/qtplugin/core/enhanced_plugin_interface.hpp
    include/qtplugin/orchestration/plugin_orchestrator.hpp
    include/qtplugin/composition/plugin_composition.hpp
    include/qtplugin/transactions/plugin_transaction_manager.hpp
    # Version management headers (v3.1.0)
    include/qtplugin/managers/plugin_version_manager.hpp
)

# Create core library
add_library(QtPluginCore ${QTPLUGIN_CORE_SOURCES} ${QTPLUGIN_CORE_HEADERS})
add_library(QtPlugin::Core ALIAS QtPluginCore)

# Set target properties
set_target_properties(QtPluginCore PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME Core
    OUTPUT_NAME qtplugin-core
)

# Include directories
target_include_directories(QtPluginCore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link Qt6 Core and optional components
target_link_libraries(QtPluginCore
    PUBLIC
        Qt6::Core
)

# Link optional components if available
if(Qt6Network_FOUND)
    target_link_libraries(QtPluginCore PUBLIC Qt6::Network)
endif()

if(Qt6Sql_FOUND)
    target_link_libraries(QtPluginCore PUBLIC Qt6::Sql)
endif()

if(Qt6Concurrent_FOUND)
    target_link_libraries(QtPluginCore PUBLIC Qt6::Concurrent)
endif()

# Enable Qt MOC for core library
set_target_properties(QtPluginCore PROPERTIES
    AUTOMOC ON
)

# Add compile definitions for build options
if(QTPLUGIN_ENABLE_COMPONENT_LOGGING)
    target_compile_definitions(QtPluginCore PRIVATE QTPLUGIN_ENABLE_COMPONENT_LOGGING)
endif()

if(QTPLUGIN_BUILD_COMPONENT_TESTS)
    target_compile_definitions(QtPluginCore PRIVATE QTPLUGIN_BUILD_COMPONENT_TESTS)
endif()

# Network plugin support (optional)
if(QTPLUGIN_BUILD_NETWORK)
    set(QTPLUGIN_NETWORK_SOURCES
        # Network sources will be added later
    )

    set(QTPLUGIN_NETWORK_HEADERS
        # Network headers will be added later
    )

    add_library(QtPluginNetwork ${QTPLUGIN_NETWORK_SOURCES} ${QTPLUGIN_NETWORK_HEADERS})
    add_library(QtPlugin::Network ALIAS QtPluginNetwork)

    set_target_properties(QtPluginNetwork PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        EXPORT_NAME Network
        OUTPUT_NAME qtplugin-network
        AUTOMOC ON
    )

    target_include_directories(QtPluginNetwork
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(QtPluginNetwork
        PUBLIC
            QtPlugin::Core
            Qt6::Network
    )
endif()

# UI plugin support (optional)
if(QTPLUGIN_BUILD_UI)
    set(QTPLUGIN_UI_SOURCES
        # UI sources will be added later
    )

    set(QTPLUGIN_UI_HEADERS
        include/qtplugin/ui/ui_plugin_interface.hpp
    )

    add_library(QtPluginUI ${QTPLUGIN_UI_SOURCES} ${QTPLUGIN_UI_HEADERS})
    add_library(QtPlugin::UI ALIAS QtPluginUI)

    set_target_properties(QtPluginUI PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        EXPORT_NAME UI
        OUTPUT_NAME qtplugin-ui
        AUTOMOC ON
    )

    target_include_directories(QtPluginUI
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
    )

    target_link_libraries(QtPluginUI
        PUBLIC
            QtPlugin::Core
            Qt6::Widgets
    )
endif()

# Security component
set(QTPLUGIN_SECURITY_SOURCES
    src/security/security_manager.cpp
)

set(QTPLUGIN_SECURITY_HEADERS
    include/qtplugin/security/security_manager.hpp
)

add_library(QtPluginSecurity ${QTPLUGIN_SECURITY_SOURCES} ${QTPLUGIN_SECURITY_HEADERS})
add_library(QtPlugin::Security ALIAS QtPluginSecurity)

set_target_properties(QtPluginSecurity PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    EXPORT_NAME Security
    OUTPUT_NAME qtplugin-security
    AUTOMOC ON
)

target_include_directories(QtPluginSecurity
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(QtPluginSecurity
    PUBLIC
        QtPlugin::Core
)

# Installation configuration
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Note: Install rules are now defined in the CPack section below with component support

# Create and install package config files
set(QTPLUGIN_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/QtPlugin)

# Export targets
install(EXPORT QtPluginCoreTargets
    FILE QtPluginCoreTargets.cmake
    NAMESPACE QtPlugin::
    DESTINATION ${QTPLUGIN_CMAKE_DIR}
)

if(QTPLUGIN_BUILD_NETWORK)
    install(EXPORT QtPluginNetworkTargets
        FILE QtPluginNetworkTargets.cmake
        NAMESPACE QtPlugin::
        DESTINATION ${QTPLUGIN_CMAKE_DIR}
    )
endif()

if(QTPLUGIN_BUILD_UI)
    install(EXPORT QtPluginUITargets
        FILE QtPluginUITargets.cmake
        NAMESPACE QtPlugin::
        DESTINATION ${QTPLUGIN_CMAKE_DIR}
    )
endif()

install(EXPORT QtPluginSecurityTargets
    FILE QtPluginSecurityTargets.cmake
    NAMESPACE QtPlugin::
    DESTINATION ${QTPLUGIN_CMAKE_DIR}
)

# Create package config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtPluginConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QtPluginConfig.cmake
    INSTALL_DESTINATION ${QTPLUGIN_CMAKE_DIR}
)

# Create version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QtPluginConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QtPluginConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/QtPluginConfigVersion.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtPluginComponents.cmake
    DESTINATION ${QTPLUGIN_CMAKE_DIR}
)

# Create pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/qtplugin.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/qtplugin.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtplugin.pc
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# Add subdirectories
if(QTPLUGIN_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(QTPLUGIN_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Doxygen documentation target (optional)
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

# CPack configuration for multi-platform packaging
include(CPack)

# Basic package information
set(CPACK_PACKAGE_NAME "QtPlugin")
set(CPACK_PACKAGE_VENDOR "QtForge")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern C++ Plugin System for Qt Applications")
set(CPACK_PACKAGE_DESCRIPTION "QtPlugin is a modern, type-safe plugin system for Qt applications with modular component architecture, security features, and comprehensive plugin management capabilities.")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_CONTACT "support@qtforge.dev")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/QtForge/QtPlugin")

# Package files and directories
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/packages")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

# Resource files
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/README.md")

# Component definitions for selective installation
set(CPACK_COMPONENTS_ALL Core Security Network UI Examples Documentation)
set(CPACK_COMPONENT_CORE_DISPLAY_NAME "QtPlugin Core Library")
set(CPACK_COMPONENT_CORE_DESCRIPTION "Core plugin system with essential functionality")
set(CPACK_COMPONENT_CORE_REQUIRED TRUE)

set(CPACK_COMPONENT_SECURITY_DISPLAY_NAME "Security Module")
set(CPACK_COMPONENT_SECURITY_DESCRIPTION "Security validation and permission management")
set(CPACK_COMPONENT_SECURITY_DEPENDS Core)

if(QTPLUGIN_BUILD_NETWORK)
    set(CPACK_COMPONENT_NETWORK_DISPLAY_NAME "Network Support")
    set(CPACK_COMPONENT_NETWORK_DESCRIPTION "Network plugin support and communication")
    set(CPACK_COMPONENT_NETWORK_DEPENDS Core)
endif()

if(QTPLUGIN_BUILD_UI)
    set(CPACK_COMPONENT_UI_DISPLAY_NAME "UI Support")
    set(CPACK_COMPONENT_UI_DESCRIPTION "User interface plugin support")
    set(CPACK_COMPONENT_UI_DEPENDS Core)
endif()

if(QTPLUGIN_BUILD_EXAMPLES)
    set(CPACK_COMPONENT_EXAMPLES_DISPLAY_NAME "Example Plugins")
    set(CPACK_COMPONENT_EXAMPLES_DESCRIPTION "Sample plugins and demonstrations")
    set(CPACK_COMPONENT_EXAMPLES_DEPENDS Core)
endif()

set(CPACK_COMPONENT_DOCUMENTATION_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_DOCUMENTATION_DESCRIPTION "API documentation and guides")

# Platform-specific packaging configuration
if(WIN32)
    # Windows-specific packaging
    set(CPACK_GENERATOR "NSIS;ZIP;WIX")

    # NSIS (Nullsoft Scriptable Install System) configuration
    set(CPACK_NSIS_DISPLAY_NAME "QtPlugin Library")
    set(CPACK_NSIS_PACKAGE_NAME "QtPlugin")
    set(CPACK_NSIS_CONTACT "${CPACK_PACKAGE_CONTACT}")
    set(CPACK_NSIS_URL_INFO_ABOUT "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_NSIS_HELP_LINK "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

    # Create desktop shortcut for documentation
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\QtPlugin Documentation.lnk' '$INSTDIR\\\\share\\\\doc\\\\qtplugin\\\\README.md'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\QtPlugin Documentation.lnk'"
    )

    # WiX (Windows Installer XML) configuration for MSI packages
    set(CPACK_WIX_UPGRADE_GUID "12345678-1234-1234-1234-123456789012")
    set(CPACK_WIX_PRODUCT_GUID "87654321-4321-4321-4321-210987654321")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/resources/qtplugin.ico")
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/resources/banner.bmp")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/resources/dialog.bmp")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "QtPlugin")
    set(CPACK_WIX_PROPERTY_ARPHELPLINK "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_WIX_PROPERTY_ARPURLINFOABOUT "${CPACK_PACKAGE_HOMEPAGE_URL}")

elseif(APPLE)
    # macOS-specific packaging
    set(CPACK_GENERATOR "DragNDrop;TGZ;productbuild")

    # DragNDrop (DMG) configuration
    set(CPACK_DMG_VOLUME_NAME "QtPlugin ${PROJECT_VERSION}")
    set(CPACK_DMG_FORMAT "UDBZ")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_SOURCE_DIR}/resources/dmg_background.png")
    set(CPACK_DMG_DS_STORE_SETUP_SCRIPT "${CMAKE_SOURCE_DIR}/packaging/macos/setup_dmg.applescript")

    # ProductBuild (PKG) configuration
    set(CPACK_PRODUCTBUILD_IDENTITY_NAME "Developer ID Installer: QtForge")
    set(CPACK_PRODUCTBUILD_KEYCHAIN_PATH "")

else()
    # Linux and other Unix-like systems
    set(CPACK_GENERATOR "DEB;RPM;TGZ;STGZ")

    # DEB package configuration
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "${CPACK_PACKAGE_CONTACT}")
    set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6core6 (>= 6.0.0)")
    set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "libqt6network6, libqt6widgets6")
    set(CPACK_DEBIAN_PACKAGE_SUGGESTS "qtplugin-examples, qtplugin-doc")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
        "${CMAKE_SOURCE_DIR}/packaging/debian/postinst"
        "${CMAKE_SOURCE_DIR}/packaging/debian/prerm"
    )

    # RPM package configuration
    set(CPACK_RPM_PACKAGE_SUMMARY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
    set(CPACK_RPM_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}")
    set(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
    set(CPACK_RPM_PACKAGE_LICENSE "MIT")
    set(CPACK_RPM_PACKAGE_URL "${CPACK_PACKAGE_HOMEPAGE_URL}")
    set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase >= 6.0.0")
    set(CPACK_RPM_PACKAGE_SUGGESTS "qtplugin-examples, qtplugin-doc")
    set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/packaging/rpm/postinstall.sh")
    set(CPACK_RPM_PRE_UNINSTALL_SCRIPT_FILE "${CMAKE_SOURCE_DIR}/packaging/rpm/preuninstall.sh")

    # Set architecture
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
        set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i[3-6]86")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
        set(CPACK_RPM_PACKAGE_ARCHITECTURE "i686")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
        set(CPACK_RPM_PACKAGE_ARCHITECTURE "aarch64")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7")
        set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "armhf")
        set(CPACK_RPM_PACKAGE_ARCHITECTURE "armv7hl")
    endif()
endif()

# Component-specific install rules
install(TARGETS QtPluginCore
    EXPORT QtPluginCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Core
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Core
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Core
)

install(TARGETS QtPluginSecurity
    EXPORT QtPluginSecurityTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Security
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Security
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Security
)

if(QTPLUGIN_BUILD_NETWORK)
    install(TARGETS QtPluginNetwork
        EXPORT QtPluginNetworkTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Network
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT Network
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT Network
    )
endif()

if(QTPLUGIN_BUILD_UI)
    install(TARGETS QtPluginUI
        EXPORT QtPluginUITargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT UI
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT UI
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT UI
    )
endif()

# Install headers with component
install(DIRECTORY include/qtplugin
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT Core
    FILES_MATCHING PATTERN "*.hpp"
)

# Install documentation
install(FILES
    "${CMAKE_SOURCE_DIR}/README.md"
    "${CMAKE_SOURCE_DIR}/LICENSE"
    "${CMAKE_SOURCE_DIR}/CHANGELOG.md"
    "${CMAKE_SOURCE_DIR}/CODE_OF_CONDUCT.md"
    "${CMAKE_SOURCE_DIR}/CONTRIBUTING.md"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT Documentation
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/docs/"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT Documentation
    PATTERN "*.md"
)

# Print configuration summary
message(STATUS "")
message(STATUS "QtPlugin Library Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Component architecture: ENABLED")
message(STATUS "  Build network support: ${QTPLUGIN_BUILD_NETWORK}")
message(STATUS "  Build UI support: ${QTPLUGIN_BUILD_UI}")
message(STATUS "  Build examples: ${QTPLUGIN_BUILD_EXAMPLES}")
message(STATUS "  Build tests: ${QTPLUGIN_BUILD_TESTS}")
message(STATUS "  Build component tests: ${QTPLUGIN_BUILD_COMPONENT_TESTS}")
message(STATUS "  Enable component logging: ${QTPLUGIN_ENABLE_COMPONENT_LOGGING}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
message(STATUS "Packaging Configuration:")
message(STATUS "  Package generators: ${CPACK_GENERATOR}")
message(STATUS "  Package directory: ${CPACK_PACKAGE_DIRECTORY}")
message(STATUS "  Package filename: ${CPACK_PACKAGE_FILE_NAME}")
message(STATUS "  Components: ${CPACK_COMPONENTS_ALL}")
message(STATUS "")
