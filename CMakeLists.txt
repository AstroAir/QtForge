# QtForge Library CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(QtForge
    VERSION 3.0.0
    DESCRIPTION "Modern C++ Plugin System for Qt Applications with Modular Architecture"
    LANGUAGES CXX
)

# Add cmake modules directory to module path
list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Include modular build system components
include(QtForgePlatform)    # Platform and architecture detection
include(QtForgeCompiler)    # Compiler detection and configuration
include(QtForgeOptions)     # Build options and feature flags
include(QtForgeDependencies) # Dependency management
include(QtForgeTargets)     # Target creation utilities
include(QtForgePackaging)   # Packaging configuration

# Setup dependencies after options are configured
qtforge_setup_dependencies()

# Configure compiler with enhanced options
qtforge_configure_compiler(
    ENABLE_WARNINGS ${QTFORGE_ENABLE_WARNINGS}
    ENABLE_WERROR ${QTFORGE_ENABLE_WERROR}
    ENABLE_SANITIZERS ${QTFORGE_ENABLE_SANITIZERS}
    ENABLE_LTO ${QTFORGE_ENABLE_LTO}
    ENABLE_FAST_MATH ${QTFORGE_ENABLE_FAST_MATH}
)

# Core library sources
set(QTFORGE_CORE_SOURCES
    src/qtplugin.cpp
    src/core/plugin_interface.cpp
    src/core/plugin_manager.cpp
    src/core/plugin_loader.cpp
    src/core/plugin_registry.cpp
    src/core/plugin_dependency_resolver.cpp
    src/core/plugin_lifecycle_manager.cpp
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
set(QTFORGE_CORE_HEADERS
    include/qtplugin/core/plugin_interface.hpp
    include/qtplugin/core/plugin_manager.hpp
    include/qtplugin/core/plugin_loader.hpp
    include/qtplugin/core/plugin_registry.hpp
    include/qtplugin/core/plugin_dependency_resolver.hpp
    include/qtplugin/core/plugin_lifecycle_manager.hpp
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

# Create core library using the modular target system
qtforge_add_library(
    NAME QtForgeCore
    SOURCES ${QTFORGE_CORE_SOURCES}
    HEADERS ${QTFORGE_CORE_HEADERS}
    PUBLIC_HEADERS ${QTFORGE_CORE_HEADERS}
    QT_COMPONENTS Core
    EXPORT_NAME Core
    OUTPUT_NAME qtforge-core
    VERSION ${PROJECT_VERSION}
)

# Network plugin support (optional)
if(QTFORGE_BUILD_NETWORK AND QTFORGE_HAS_NETWORK)
    set(QTFORGE_NETWORK_SOURCES
        # Network sources will be added when implemented
    )

    set(QTFORGE_NETWORK_HEADERS
        # Network headers will be added when implemented
    )

    qtforge_add_library(
        NAME QtForgeNetwork
        SOURCES ${QTFORGE_NETWORK_SOURCES}
        HEADERS ${QTFORGE_NETWORK_HEADERS}
        QT_COMPONENTS Core Network
        DEPENDENCIES QtForge::Core
        EXPORT_NAME Network
        OUTPUT_NAME qtforge-network
    )
endif()

# UI plugin support (optional)
if(QTFORGE_BUILD_UI AND QTFORGE_HAS_WIDGETS)
    set(QTFORGE_UI_SOURCES
        # UI sources will be added when implemented
    )

    set(QTFORGE_UI_HEADERS
        include/qtplugin/ui/ui_plugin_interface.hpp
    )

    qtforge_add_library(
        NAME QtForgeUI
        SOURCES ${QTFORGE_UI_SOURCES}
        HEADERS ${QTFORGE_UI_HEADERS}
        QT_COMPONENTS Core Widgets
        DEPENDENCIES QtForge::Core
        EXPORT_NAME UI
        OUTPUT_NAME qtforge-ui
    )
endif()

# Security component (always built)
set(QTFORGE_SECURITY_SOURCES
    src/security/security_manager.cpp
)

set(QTFORGE_SECURITY_HEADERS
    include/qtplugin/security/security_manager.hpp
)

qtforge_add_library(
    NAME QtForgeSecurity
    SOURCES ${QTFORGE_SECURITY_SOURCES}
    HEADERS ${QTFORGE_SECURITY_HEADERS}
    QT_COMPONENTS Core
    DEPENDENCIES QtForge::Core
    EXPORT_NAME Security
    OUTPUT_NAME qtforge-security
)

# Installation configuration
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install targets using the modular system
set(QTFORGE_INSTALL_TARGETS QtForgeCore QtForgeSecurity)

if(TARGET QtForgeNetwork)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeNetwork)
endif()

if(TARGET QtForgeUI)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeUI)
endif()

# Add static targets if they exist
if(TARGET QtForgeCoreStatic)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeCoreStatic)
endif()

if(TARGET QtForgeSecurityStatic)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeSecurityStatic)
endif()

if(TARGET QtForgeNetworkStatic)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeNetworkStatic)
endif()

if(TARGET QtForgeUIStatic)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeUIStatic)
endif()

# Install all targets
qtforge_install_targets(TARGETS ${QTFORGE_INSTALL_TARGETS} COMPONENT Runtime)

# Export targets for find_package support
install(EXPORT QtForgeTargets
    FILE QtForgeTargets.cmake
    NAMESPACE QtForge::
    DESTINATION ${QTFORGE_INSTALL_CMAKE_DIR}
)

# Create package config file
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtForgeConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfig.cmake
    INSTALL_DESTINATION ${QTFORGE_INSTALL_CMAKE_DIR}
)

# Create version file
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Install config files
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfigVersion.cmake
    DESTINATION ${QTFORGE_INSTALL_CMAKE_DIR}
)

# Create pkg-config file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/qtforge.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/qtforge.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtforge.pc
    DESTINATION ${QTFORGE_INSTALL_PKGCONFIG_DIR}
)

# Add subdirectories
if(QTFORGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(QTFORGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Documentation target
if(QTFORGE_BUILD_DOCS AND DOXYGEN_FOUND)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Configure packaging using the modular system
qtforge_configure_packaging()

# Add package targets
qtforge_add_package_target()

# Install headers
install(DIRECTORY include/qtplugin
    DESTINATION ${QTFORGE_INSTALL_INCLUDE_DIR}
    COMPONENT Development
    FILES_MATCHING PATTERN "*.hpp"
)

# Install documentation
install(FILES
    "${CMAKE_SOURCE_DIR}/README.md"
    "${CMAKE_SOURCE_DIR}/LICENSE"
    "${CMAKE_SOURCE_DIR}/CHANGELOG.md"
    "${CMAKE_SOURCE_DIR}/CODE_OF_CONDUCT.md"
    "${CMAKE_SOURCE_DIR}/CONTRIBUTING.md"
    DESTINATION ${QTFORGE_INSTALL_DOC_DIR}
    COMPONENT Documentation
)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/docs/"
    DESTINATION ${QTFORGE_INSTALL_DOC_DIR}
    COMPONENT Documentation
    PATTERN "*.md"
)

# Print build configuration summary
message(STATUS "")
message(STATUS "QtForge Build System Summary:")
message(STATUS "=============================")
message(STATUS "Modular build system successfully configured!")
message(STATUS "")
