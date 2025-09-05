# ============================================================================
# QTFORGE LIBRARY - MAIN CMAKE CONFIGURATION
# ============================================================================
# Modern C++ Plugin System for Qt Applications with Modular Architecture
# Version: 3.0.0
# ============================================================================

cmake_minimum_required(VERSION 3.21)

# Set CMake policies to resolve warnings
if(POLICY CMP0156)
    cmake_policy(SET CMP0156 NEW)  # De-duplicate libraries on link lines based on linker capabilities
endif()

if(POLICY CMP0181)
    cmake_policy(SET CMP0181 NEW)  # Link command-line fragment variables are parsed and re-quoted
endif()

project(QtForge
    VERSION 3.0.0
    DESCRIPTION "Modern C++ Plugin System for Qt Applications with Modular Architecture"
    LANGUAGES CXX
)

# ============================================================================
# PROJECT SETUP AND BASIC CONFIGURATION
# ============================================================================

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

# ============================================================================
# FEATURE OPTIONS AND CONFIGURATION
# ============================================================================
# Feature options are defined in QtForgeOptions.cmake and include:
# - QTFORGE_BUILD_SHARED/QTFORGE_BUILD_STATIC: Library types
# - QTFORGE_BUILD_NETWORK: Network plugin support
# - QTFORGE_BUILD_UI: UI plugin support
# - QTFORGE_BUILD_SECURITY: Security component
# - QTFORGE_BUILD_PYTHON_BINDINGS: Python bindings
# - QTFORGE_BUILD_LUA_BINDINGS: Lua bindings
# - QTFORGE_BUILD_EXAMPLES: Example plugins
# - QTFORGE_BUILD_TESTS: Unit tests
# - QTFORGE_BUILD_DOCS: Documentation
# All features are enabled by default for full functionality
# ============================================================================

# ============================================================================
# CORE LIBRARY (ALWAYS REQUIRED)
# ============================================================================
# The core library contains essential plugin system functionality and is
# always built. It includes the plugin manager, loaders, communication
# systems, and all fundamental components.
# ============================================================================

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
    # Advanced plugin system sources (v3.1.0)
    src/communication/plugin_service_contracts.cpp
    src/core/advanced_plugin_interface.cpp
    src/orchestration/plugin_orchestrator.cpp
    src/composition/plugin_composition.cpp
    src/transactions/plugin_transaction_manager.cpp
    # Version management sources (v3.1.0)
    src/managers/plugin_version_manager.cpp
    # Dynamic plugin system sources (v3.2.0)
    src/core/dynamic_plugin_interface.cpp
    # Multi-language plugin bridges (v3.2.0)
    src/bridges/python_plugin_bridge.cpp
    # Note: lua_plugin_bridge.cpp and lua_plugin_loader.cpp are conditionally added below
    # Advanced security and sandboxing (v3.2.0)
    $<$<BOOL:${QTFORGE_ENABLE_SANDBOX}>:src/security/sandbox/plugin_sandbox.cpp>
    $<$<BOOL:${QTFORGE_ENABLE_SANDBOX}>:src/security/sandbox/resource_monitor.cpp>
    $<$<BOOL:${QTFORGE_ENABLE_SANDBOX}>:src/security/sandbox/security_enforcer.cpp>
    # Advanced plugin composition (v3.2.0)
    src/orchestration/advanced/plugin_orchestrator_v2.cpp
    # Plugin marketplace integration (v3.2.0)
    src/marketplace/plugin_marketplace.cpp
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
    # Advanced plugin system headers (v3.1.0)
    include/qtplugin/communication/plugin_service_contracts.hpp
    include/qtplugin/core/advanced_plugin_interface.hpp
    include/qtplugin/orchestration/plugin_orchestrator.hpp
    include/qtplugin/composition/plugin_composition.hpp
    include/qtplugin/transactions/plugin_transaction_manager.hpp
    # Version management headers (v3.1.0)
    include/qtplugin/managers/plugin_version_manager.hpp
    # Dynamic plugin system headers (v3.2.0)
    include/qtplugin/core/dynamic_plugin_interface.hpp
    # Multi-language plugin bridge headers (v3.2.0)
    include/qtplugin/bridges/python_plugin_bridge.hpp
    # Note: lua_plugin_bridge.hpp and lua_plugin_loader.hpp are conditionally added below
    # Advanced security and sandboxing headers (v3.2.0)
    $<$<BOOL:${QTFORGE_ENABLE_SANDBOX}>:include/qtplugin/security/sandbox/plugin_sandbox.hpp>
    # Enhanced plugin composition headers (v3.2.0)
    include/qtplugin/orchestration/advanced/plugin_orchestrator_v2.hpp
    # Plugin marketplace integration headers (v3.2.0)
    include/qtplugin/marketplace/plugin_marketplace.hpp
)

# Conditionally add Lua-related sources if Lua bindings are enabled
if(QTFORGE_BUILD_LUA_BINDINGS AND QTFORGE_LUA_FOUND AND QTFORGE_SOL2_FOUND)
    list(APPEND QTFORGE_CORE_SOURCES
        src/bridges/lua_plugin_bridge.cpp
        src/core/lua_plugin_loader.cpp
    )
    list(APPEND QTFORGE_CORE_HEADERS
        include/qtplugin/bridges/lua_plugin_bridge.hpp
        include/qtplugin/core/lua_plugin_loader.hpp
    )
    message(STATUS "QtForge: Including Lua plugin sources in core library")
else()
    message(STATUS "QtForge: Excluding Lua plugin sources from core library")
endif()

# Prepare compile definitions based on available features
set(QTFORGE_CORE_DEFINITIONS)
if(QTFORGE_HAS_NETWORK)
    list(APPEND QTFORGE_CORE_DEFINITIONS QTFORGE_HAS_NETWORK)
endif()
if(QTFORGE_HAS_WIDGETS)
    list(APPEND QTFORGE_CORE_DEFINITIONS QTFORGE_HAS_WIDGETS)
endif()
if(QTFORGE_HAS_SQL)
    list(APPEND QTFORGE_CORE_DEFINITIONS QTFORGE_HAS_SQL)
endif()
if(QTFORGE_HAS_CONCURRENT)
    list(APPEND QTFORGE_CORE_DEFINITIONS QTFORGE_HAS_CONCURRENT)
endif()
if(QTFORGE_HAS_STATEMACHINE)
    list(APPEND QTFORGE_CORE_DEFINITIONS QTFORGE_HAS_STATEMACHINE)
endif()

# Add sandbox feature definitions
if(QTFORGE_ENABLE_SANDBOX)
    list(APPEND QTFORGE_CORE_DEFINITIONS QTFORGE_ENABLE_SANDBOX)
    if(QTFORGE_SANDBOX_PROCESS_ISOLATION)
        list(APPEND QTFORGE_CORE_DEFINITIONS QTFORGE_SANDBOX_PROCESS_ISOLATION)
    endif()
    if(QTFORGE_SANDBOX_RESOURCE_MONITORING)
        list(APPEND QTFORGE_CORE_DEFINITIONS QTFORGE_SANDBOX_RESOURCE_MONITORING)
    endif()
    if(QTFORGE_SANDBOX_SECURITY_ENFORCEMENT)
        list(APPEND QTFORGE_CORE_DEFINITIONS QTFORGE_SANDBOX_SECURITY_ENFORCEMENT)
    endif()
endif()

# Create core library using the modular target system
qtforge_add_library(
    NAME QtForgeCore
    SOURCES ${QTFORGE_CORE_SOURCES}
    HEADERS ${QTFORGE_CORE_HEADERS}
    PUBLIC_HEADERS ${QTFORGE_CORE_HEADERS}
    QT_COMPONENTS Core Network Widgets Sql Concurrent StateMachine
    DEFINITIONS ${QTFORGE_CORE_DEFINITIONS}
    EXPORT_NAME Core
    OUTPUT_NAME qtforge-core
    VERSION ${PROJECT_VERSION}
)

# Add platform-specific libraries for sandbox functionality
if(QTFORGE_ENABLE_SANDBOX)
    if(WIN32)
        target_link_libraries(QtForgeCore PRIVATE pdh psapi)
    endif()
endif()

# ============================================================================
# OPTIONAL COMPONENTS
# ============================================================================
# These components can be selectively enabled/disabled via CMake options.
# All are enabled by default to maintain full functionality.
# ============================================================================

# ----------------------------------------------------------------------------
# Network Component
# ----------------------------------------------------------------------------
# Network plugin support (optional) - disabled until sources are implemented
# if(QTFORGE_BUILD_NETWORK AND QTFORGE_HAS_NETWORK)
#     set(QTFORGE_NETWORK_SOURCES
#         # Network sources will be added when implemented
#     )
#
#     set(QTFORGE_NETWORK_HEADERS
#         # Network headers will be added when implemented
#     )
#
#     qtforge_add_library(
#         NAME QtForgeNetwork
#         SOURCES ${QTFORGE_NETWORK_SOURCES}
#         HEADERS ${QTFORGE_NETWORK_HEADERS}
#         QT_COMPONENTS Core Network
#         DEPENDENCIES QtForge::Core
#         EXPORT_NAME Network
#         OUTPUT_NAME qtforge-network
#     )
# endif()

# ----------------------------------------------------------------------------
# UI Component
# ----------------------------------------------------------------------------
# UI plugin support (optional) - disabled until sources are implemented
# if(QTFORGE_BUILD_UI AND QTFORGE_HAS_WIDGETS)
#     set(QTFORGE_UI_SOURCES
#         # UI sources will be added when implemented
#     )
#
#     set(QTFORGE_UI_HEADERS
#         include/qtplugin/ui/ui_plugin_interface.hpp
#     )
#
#     qtforge_add_library(
#         NAME QtForgeUI
#         SOURCES ${QTFORGE_UI_SOURCES}
#         HEADERS ${QTFORGE_UI_HEADERS}
#         QT_COMPONENTS Core Widgets
#         DEPENDENCIES QtForge::Core
#         EXPORT_NAME UI
#         OUTPUT_NAME qtforge-ui
#     )
# endif()

# ----------------------------------------------------------------------------
# Security Component
# ----------------------------------------------------------------------------
if(QTFORGE_BUILD_SECURITY)
    message(STATUS "QtForge: Building security component")

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
else()
    message(STATUS "QtForge: Security component disabled")
endif()

# ============================================================================
# INSTALLATION AND PACKAGING
# ============================================================================
# Configuration for installing libraries, headers, and creating packages.
# ============================================================================

# Installation configuration
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install targets using the modular system
set(QTFORGE_INSTALL_TARGETS QtForgeCore)

# Add optional component targets if they were built
if(TARGET QtForgeSecurity)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeSecurity)
endif()

if(TARGET QtForgeNetwork)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeNetwork)
endif()

if(TARGET QtForgeUI)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeUI)
endif()

# Add language binding targets if they were built
if(TARGET qtforge_python)
    list(APPEND QTFORGE_INSTALL_TARGETS qtforge_python)
endif()

if(TARGET qtforge_lua)
    list(APPEND QTFORGE_INSTALL_TARGETS qtforge_lua)
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

# ============================================================================
# LANGUAGE BINDINGS
# ============================================================================
# Optional language bindings for Python and Lua scripting support.
# These allow plugins to be written in scripting languages.
# ============================================================================

# ----------------------------------------------------------------------------
# Python Bindings
# ----------------------------------------------------------------------------
if(QTFORGE_BUILD_PYTHON_BINDINGS AND QTFORGE_PYTHON_FOUND AND QTFORGE_PYBIND11_FOUND)
    message(STATUS "QtForge: Building Python bindings")

    # Python binding sources - core components (always included)
    set(QTFORGE_PYTHON_SOURCES
        src/python/qtforge_python.cpp
        # src/python/qt_conversions.cpp  # Temporarily disabled - pybind11 API compatibility issues
        src/python/core/core_bindings_minimal.cpp  # Using minimal version to avoid missing method errors
        src/python/utils/utils_bindings.cpp
    )

    # Add optional binding sources based on available Qt components
    # Temporarily disabled all optional bindings until pybind11 and C++ API issues are resolved
    # list(APPEND QTFORGE_PYTHON_SOURCES
    #     src/python/security/security_bindings.cpp      # Basic security enums and interfaces
    #     # Temporarily disabled due to compilation issues:
    #     # src/python/monitoring/monitoring_bindings.cpp  # Uses incomplete IPluginRegistry type
    #     # src/python/managers/managers_bindings.cpp      # Missing concrete manager classes
    #     # src/python/orchestration/orchestration_bindings.cpp  # Missing orchestrator methods
    #     # src/python/transactions/transaction_bindings.cpp     # Missing transaction classes
    #     # src/python/composition/composition_bindings.cpp      # Missing composition classes
    #     # src/python/marketplace/marketplace_bindings.cpp      # Missing marketplace classes
    #     # src/python/threading/threading_bindings.cpp          # Missing PluginTask struct and interfaces
    # )

    # Communication bindings (requires Qt6::Network for some features)
    # Temporarily disabled due to missing C++ service contract implementations
    # if(Qt6Network_FOUND OR NOT QTFORGE_REQUIRE_NETWORK_FOR_COMMUNICATION)
    #     list(APPEND QTFORGE_PYTHON_SOURCES
    #         src/python/communication/communication_bindings.cpp
    #     )
    #     message(STATUS "QtForge: Including communication bindings")
    # else()
    #     message(STATUS "QtForge: Skipping communication bindings (Qt6::Network not found)")
    # endif()
    message(STATUS "QtForge: Communication bindings temporarily disabled - missing ServiceMethodDescriptor and related classes")

    # Create Python module (minimal version without Qt dependencies for now)
    pybind11_add_module(qtforge_python ${QTFORGE_PYTHON_SOURCES})

    # Set module properties
    set_target_properties(qtforge_python PROPERTIES
        OUTPUT_NAME qtforge
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    # Configure include directories
    target_include_directories(qtforge_python PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/python
    )

    # Link Qt Core library (required for Qt type conversions and bindings)
    target_link_libraries(qtforge_python PRIVATE
        Qt6::Core
    )

    # Link optional Qt components if available
    if(Qt6Network_FOUND)
        target_link_libraries(qtforge_python PRIVATE Qt6::Network)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_NETWORK_AVAILABLE)
    endif()

    if(Qt6Widgets_FOUND)
        target_link_libraries(qtforge_python PRIVATE Qt6::Widgets)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_WIDGETS_AVAILABLE)
    endif()

    if(Qt6Sql_FOUND)
        target_link_libraries(qtforge_python PRIVATE Qt6::Sql)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_SQL_AVAILABLE)
    endif()

    # Link QtForge core library
    target_link_libraries(qtforge_python PRIVATE QtForgeCore)

    # Add compile definitions
    target_compile_definitions(qtforge_python PRIVATE
        QTPLUGIN_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        QTPLUGIN_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        QTPLUGIN_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    )

    # Create Python package structure
    if(QTFORGE_PYTHON_BINDINGS_INSTALL)
        # Install __init__.py for the package
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/python/__init__.py.in
            ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py
            @ONLY
        )

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py
            DESTINATION ${PYTHON_INSTALL_DIR}/qtforge
            COMPONENT PythonBindings
        )
    endif()
else()
    if(NOT QTFORGE_BUILD_PYTHON_BINDINGS)
        message(STATUS "QtForge: Python bindings disabled (QTFORGE_BUILD_PYTHON_BINDINGS=OFF)")
    elseif(NOT QTFORGE_PYTHON_FOUND)
        message(STATUS "QtForge: Python bindings disabled - Python interpreter not found")
    elseif(NOT QTFORGE_PYBIND11_FOUND)
        message(STATUS "QtForge: Python bindings disabled - pybind11 not found")
    else()
        message(STATUS "QtForge: Python bindings disabled - unknown reason")
    endif()
endif()

# ----------------------------------------------------------------------------
# Lua Bindings
# ----------------------------------------------------------------------------
if(QTFORGE_BUILD_LUA_BINDINGS AND QTFORGE_LUA_FOUND AND QTFORGE_SOL2_FOUND)
    message(STATUS "QtForge: Building Lua bindings")

    # Lua binding sources - all components now enabled
    set(QTFORGE_LUA_SOURCES
        src/lua/qtforge_lua.cpp
        src/lua/core/core_bindings.cpp
        src/lua/utils/utils_bindings.cpp
        src/lua/security/security_bindings.cpp
        src/lua/managers/managers_bindings.cpp
        src/lua/orchestration/orchestration_bindings.cpp
        src/lua/monitoring/monitoring_bindings.cpp
        src/lua/transactions/transaction_bindings.cpp
        src/lua/composition/composition_bindings.cpp
        src/lua/marketplace/marketplace_bindings.cpp
        src/lua/threading/threading_bindings.cpp
    )

    # Communication bindings (requires Qt6::Network for some features)
    if(Qt6Network_FOUND OR NOT QTFORGE_REQUIRE_NETWORK_FOR_COMMUNICATION)
        list(APPEND QTFORGE_LUA_SOURCES
            src/lua/communication/communication_bindings.cpp
        )
        message(STATUS "QtForge: Including Lua communication bindings")
    else()
        message(STATUS "QtForge: Skipping Lua communication bindings (Qt6::Network not found)")
    endif()

    # Create Lua module
    add_library(qtforge_lua SHARED ${QTFORGE_LUA_SOURCES})

    # Set module properties
    set_target_properties(qtforge_lua PROPERTIES
        OUTPUT_NAME qtforge_lua
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    # Configure include directories
    target_include_directories(qtforge_lua PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua
        ${QTFORGE_LUA_INCLUDE_DIR}
    )

    # Add sol2 include directory if found manually
    if(QTFORGE_SOL2_INCLUDE_DIR)
        target_include_directories(qtforge_lua PRIVATE ${QTFORGE_SOL2_INCLUDE_DIR})
    endif()

    # Ultra-minimal bindings - only link Lua libraries for now
    target_link_libraries(qtforge_lua PRIVATE ${QTFORGE_LUA_LIBRARIES})

    # # Link Qt components required by bindings
    # target_link_libraries(qtforge_lua PRIVATE Qt6::Core)

    # # Link optional Qt components if available
    # if(Qt6Network_FOUND)
    #     target_link_libraries(qtforge_lua PRIVATE Qt6::Network)
    #     target_compile_definitions(qtforge_lua PRIVATE QTFORGE_NETWORK_AVAILABLE)
    # endif()

    # if(Qt6Widgets_FOUND)
    #     target_link_libraries(qtforge_lua PRIVATE Qt6::Widgets)
    #     target_compile_definitions(qtforge_lua PRIVATE QTFORGE_WIDGETS_AVAILABLE)
    # endif()

    # if(Qt6Sql_FOUND)
    #     target_link_libraries(qtforge_lua PRIVATE Qt6::Sql)
    #     target_compile_definitions(qtforge_lua PRIVATE QTFORGE_SQL_AVAILABLE)
    # endif()

    # # Link QtForge core library
    # target_link_libraries(qtforge_lua PRIVATE QtForgeCore)

    # Add compile definitions
    target_compile_definitions(qtforge_lua PRIVATE
        QTPLUGIN_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        QTPLUGIN_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        QTPLUGIN_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        QTFORGE_LUA_BINDINGS
    )

    # Add sandboxing support if enabled
    if(QTFORGE_LUA_ENABLE_SANDBOX)
        target_compile_definitions(qtforge_lua PRIVATE QTFORGE_LUA_SANDBOX_ENABLED)
    endif()

    # Installation
    if(QTFORGE_LUA_BINDINGS_INSTALL)
        install(TARGETS qtforge_lua
            LIBRARY DESTINATION ${QTFORGE_INSTALL_LIBRARY_DIR}
            RUNTIME DESTINATION ${QTFORGE_INSTALL_RUNTIME_DIR}
            COMPONENT LuaBindings
        )
    endif()
else()
    if(NOT QTFORGE_BUILD_LUA_BINDINGS)
        message(STATUS "QtForge: Lua bindings disabled (QTFORGE_BUILD_LUA_BINDINGS=OFF)")
    elseif(NOT QTFORGE_LUA_FOUND)
        message(STATUS "QtForge: Lua bindings disabled - Lua interpreter not found")
    elseif(NOT QTFORGE_SOL2_FOUND)
        message(STATUS "QtForge: Lua bindings disabled - sol2 library not found")
    else()
        message(STATUS "QtForge: Lua bindings disabled - unknown reason")
    endif()
endif()

# ============================================================================
# DEVELOPMENT FEATURES
# ============================================================================
# Optional development and testing features that can be enabled/disabled.
# ============================================================================

# ----------------------------------------------------------------------------
# Examples
# ----------------------------------------------------------------------------
if(QTFORGE_BUILD_EXAMPLES)
    message(STATUS "QtForge: Building examples")
    add_subdirectory(examples)
else()
    message(STATUS "QtForge: Examples disabled")
endif()

# ----------------------------------------------------------------------------
# Tests
# ----------------------------------------------------------------------------
if(QTFORGE_BUILD_TESTS)
    message(STATUS "QtForge: Building tests")
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "QtForge: Tests disabled")
endif()

# ----------------------------------------------------------------------------
# Documentation
# ----------------------------------------------------------------------------
if(QTFORGE_BUILD_DOCS AND DOXYGEN_FOUND)
    message(STATUS "QtForge: Building documentation")
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
elseif(QTFORGE_BUILD_DOCS)
    message(WARNING "QtForge: Documentation requested but Doxygen not found")
else()
    message(STATUS "QtForge: Documentation disabled")
endif()

# ----------------------------------------------------------------------------
# Packaging Configuration
# ----------------------------------------------------------------------------
# Configure packaging using the modular system
qtforge_configure_packaging()

# Add package targets
qtforge_add_package_target()

# ----------------------------------------------------------------------------
# Header Installation
# ----------------------------------------------------------------------------
# Install headers
install(DIRECTORY include/qtplugin
    DESTINATION ${QTFORGE_INSTALL_INCLUDE_DIR}
    COMPONENT Development
    FILES_MATCHING PATTERN "*.hpp"
)

# ----------------------------------------------------------------------------
# Documentation Installation
# ----------------------------------------------------------------------------
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

# ============================================================================
# BUILD CONFIGURATION SUMMARY
# ============================================================================

# Print build configuration summary
message(STATUS "")
message(STATUS "QtForge Build System Summary:")
message(STATUS "=============================")
message(STATUS "Modular build system successfully configured!")
message(STATUS "All feature toggles are available via CMake options.")
message(STATUS "Use -DQTFORGE_BUILD_<FEATURE>=OFF to disable specific features.")
message(STATUS "")
