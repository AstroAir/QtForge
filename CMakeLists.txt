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
find_package(Qt6 COMPONENTS Network Widgets Sql StateMachine QUIET)

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

# Install libraries
install(TARGETS QtPluginCore
    EXPORT QtPluginCoreTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QTPLUGIN_BUILD_NETWORK)
    install(TARGETS QtPluginNetwork
        EXPORT QtPluginNetworkTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

if(QTPLUGIN_BUILD_UI)
    install(TARGETS QtPluginUI
        EXPORT QtPluginUITargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

install(TARGETS QtPluginSecurity
    EXPORT QtPluginSecurityTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(DIRECTORY include/qtplugin
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)

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
