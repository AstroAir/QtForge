cmake_minimum_required(VERSION 3.21)

if(POLICY CMP0156)
    cmake_policy(SET CMP0156 NEW)
endif()

if(POLICY CMP0181)
    cmake_policy(SET CMP0181 NEW)
endif()

project(QtForge
    VERSION 3.2.0
    DESCRIPTION "Modern C++ Plugin System for Qt Applications with Modular Architecture"
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(QtForgePlatform)
include(QtForgeCompiler)
include(QtForgeOptions)
include(QtForgeDependencies)
include(QtForgeTargets)
include(QtForgePackaging)

qtforge_setup_dependencies()

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
    # Advanced plugin system sources (v3.1.0)
    src/communication/plugin_service_contracts.cpp
    src/core/advanced_plugin_interface.cpp
    src/orchestration/plugin_orchestrator.cpp
    src/composition/plugin_composition.cpp
    src/transactions/plugin_transaction_manager.cpp
    src/managers/plugin_version_manager.cpp
    src/core/dynamic_plugin_interface.cpp
    src/bridges/python_plugin_bridge.cpp
    $<$<BOOL:${QTFORGE_ENABLE_SANDBOX}>:src/security/sandbox/plugin_sandbox.cpp>
    $<$<BOOL:${QTFORGE_ENABLE_SANDBOX}>:src/security/sandbox/resource_monitor.cpp>
    $<$<BOOL:${QTFORGE_ENABLE_SANDBOX}>:src/security/sandbox/security_enforcer.cpp>
    src/orchestration/advanced/plugin_orchestrator_v2.cpp
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
    include/qtplugin/communication/plugin_service_contracts.hpp
    include/qtplugin/core/advanced_plugin_interface.hpp
    include/qtplugin/orchestration/plugin_orchestrator.hpp
    include/qtplugin/composition/plugin_composition.hpp
    include/qtplugin/transactions/plugin_transaction_manager.hpp
    include/qtplugin/managers/plugin_version_manager.hpp
    include/qtplugin/core/dynamic_plugin_interface.hpp
    include/qtplugin/bridges/python_plugin_bridge.hpp
    $<$<BOOL:${QTFORGE_ENABLE_SANDBOX}>:include/qtplugin/security/sandbox/plugin_sandbox.hpp>
    include/qtplugin/orchestration/advanced/plugin_orchestrator_v2.hpp
    include/qtplugin/marketplace/plugin_marketplace.hpp
)

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

if(QTFORGE_ENABLE_SANDBOX)
    if(WIN32)
        target_link_libraries(QtForgeCore PRIVATE pdh psapi)
    endif()
endif()
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

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(QTFORGE_INSTALL_TARGETS QtForgeCore)

if(TARGET QtForgeSecurity)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeSecurity)
endif()

if(TARGET QtForgeNetwork)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeNetwork)
endif()

if(TARGET QtForgeUI)
    list(APPEND QTFORGE_INSTALL_TARGETS QtForgeUI)
endif()

if(TARGET qtforge_python)
    list(APPEND QTFORGE_INSTALL_TARGETS qtforge_python)
endif()

if(TARGET qtforge_lua)
    list(APPEND QTFORGE_INSTALL_TARGETS qtforge_lua)
endif()

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
qtforge_install_targets(TARGETS ${QTFORGE_INSTALL_TARGETS} COMPONENT Runtime)

install(EXPORT QtForgeTargets
    FILE QtForgeTargets.cmake
    NAMESPACE QtForge::
    DESTINATION ${QTFORGE_INSTALL_CMAKE_DIR}
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QtForgeConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfig.cmake
    INSTALL_DESTINATION ${QTFORGE_INSTALL_CMAKE_DIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/QtForgeConfigVersion.cmake
    DESTINATION ${QTFORGE_INSTALL_CMAKE_DIR}
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/qtforge.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/qtforge.pc
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtforge.pc
    DESTINATION ${QTFORGE_INSTALL_PKGCONFIG_DIR}
)

# Python Bindings
if(QTFORGE_BUILD_PYTHON_BINDINGS AND QTFORGE_PYTHON_FOUND AND QTFORGE_PYBIND11_FOUND)
    message(STATUS "QtForge: Building Python bindings")

    # Progressive Python bindings - start with working modules, add more as they're fixed
    set(QTFORGE_PYTHON_SOURCES
        src/python/qtforge_python.cpp
        src/python/core/core_bindings_basic.cpp
        src/python/core/plugin_manager_simple.cpp
        src/python/utils/utils_bindings.cpp
    )

    # Additional modules to be enabled as compilation issues are resolved
    if(QTFORGE_PYTHON_ENABLE_SECURITY_MODULE)
        list(APPEND QTFORGE_PYTHON_SOURCES src/python/security/security_bindings.cpp)
        message(STATUS "QtForge: Including security bindings")
    endif()

    if(QTFORGE_PYTHON_ENABLE_MANAGERS_MODULE)
        list(APPEND QTFORGE_PYTHON_SOURCES src/python/managers/managers_bindings.cpp)
        message(STATUS "QtForge: Including managers bindings")
    endif()

    if(QTFORGE_PYTHON_ENABLE_COMMUNICATION_MODULE AND Qt6Network_FOUND)
        list(APPEND QTFORGE_PYTHON_SOURCES src/python/communication/communication_bindings.cpp)
        message(STATUS "QtForge: Including communication bindings (Qt Network available)")
    endif()

    if(QTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE)
        list(APPEND QTFORGE_PYTHON_SOURCES src/python/orchestration/orchestration_bindings.cpp)
        message(STATUS "QtForge: Including orchestration bindings")
    endif()

    if(QTFORGE_PYTHON_ENABLE_MONITORING_MODULE)
        list(APPEND QTFORGE_PYTHON_SOURCES src/python/monitoring/monitoring_bindings.cpp)
        message(STATUS "QtForge: Including monitoring bindings")
    endif()

    if(QTFORGE_PYTHON_ENABLE_THREADING_MODULE)
        list(APPEND QTFORGE_PYTHON_SOURCES src/python/threading/threading_bindings.cpp)
        message(STATUS "QtForge: Including threading bindings")
    endif()

    if(QTFORGE_PYTHON_ENABLE_TRANSACTIONS_MODULE)
        list(APPEND QTFORGE_PYTHON_SOURCES src/python/transactions/transaction_bindings.cpp)
        message(STATUS "QtForge: Including transactions bindings")
    endif()

    if(QTFORGE_PYTHON_ENABLE_COMPOSITION_MODULE)
        list(APPEND QTFORGE_PYTHON_SOURCES src/python/composition/composition_bindings.cpp)
        message(STATUS "QtForge: Including composition bindings")
    endif()

    if(QTFORGE_PYTHON_ENABLE_MARKETPLACE_MODULE)
        list(APPEND QTFORGE_PYTHON_SOURCES src/python/marketplace/marketplace_bindings.cpp)
        message(STATUS "QtForge: Including marketplace bindings")
    endif()

    # Communication module is already included above if Qt Network is available

    # Additional modules can be enabled by setting the appropriate CMake variables

    message(STATUS "QtForge: Building Python bindings with core modules (${QTFORGE_PYTHON_SOURCES})")

    pybind11_add_module(qtforge_python ${QTFORGE_PYTHON_SOURCES})

    set_target_properties(qtforge_python PROPERTIES
        OUTPUT_NAME qtforge
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    target_include_directories(qtforge_python PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/python
    )

    # Link against Qt and QtForge libraries for complete functionality
    target_link_libraries(qtforge_python PRIVATE
        QtForgeCore
        Qt6::Core
    )

    if(Qt6Network_FOUND)
        target_link_libraries(qtforge_python PRIVATE Qt6::Network)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_NETWORK_AVAILABLE)
    endif()

    if(Qt6Widgets_FOUND)
        target_link_libraries(qtforge_python PRIVATE Qt6::Widgets)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_WIDGETS_AVAILABLE)
    endif()

    if(Qt6Widgets_FOUND)
        target_link_libraries(qtforge_python PRIVATE Qt6::Widgets)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_WIDGETS_AVAILABLE)
    endif()

    if(Qt6Sql_FOUND)
        target_link_libraries(qtforge_python PRIVATE Qt6::Sql)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_SQL_AVAILABLE)
    endif()

    target_link_libraries(qtforge_python PRIVATE QtForgeCore)

    target_compile_definitions(qtforge_python PRIVATE
        QTPLUGIN_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        QTPLUGIN_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        QTPLUGIN_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        QTFORGE_PYTHON_BINDINGS
        QTFORGE_PYTHON_ALL_MODULES_ENABLED
    )

    # Add compile definitions for enabled modules
    if(QTFORGE_PYTHON_ENABLE_SECURITY_MODULE)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_PYTHON_ENABLE_SECURITY_MODULE)
    endif()

    if(QTFORGE_PYTHON_ENABLE_MANAGERS_MODULE)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_PYTHON_ENABLE_MANAGERS_MODULE)
    endif()

    if(QTFORGE_PYTHON_ENABLE_COMMUNICATION_MODULE AND Qt6Network_FOUND)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_PYTHON_ENABLE_COMMUNICATION_MODULE)
    endif()

    if(QTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE)
    endif()

    if(QTFORGE_PYTHON_ENABLE_MONITORING_MODULE)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_PYTHON_ENABLE_MONITORING_MODULE)
    endif()

    if(QTFORGE_PYTHON_ENABLE_THREADING_MODULE)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_PYTHON_ENABLE_THREADING_MODULE)
    endif()

    if(QTFORGE_PYTHON_ENABLE_TRANSACTIONS_MODULE)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_PYTHON_ENABLE_TRANSACTIONS_MODULE)
    endif()

    if(QTFORGE_PYTHON_ENABLE_COMPOSITION_MODULE)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_PYTHON_ENABLE_COMPOSITION_MODULE)
    endif()

    if(QTFORGE_PYTHON_ENABLE_MARKETPLACE_MODULE)
        target_compile_definitions(qtforge_python PRIVATE QTFORGE_PYTHON_ENABLE_MARKETPLACE_MODULE)
    endif()

    if(QTFORGE_PYTHON_BINDINGS_INSTALL)
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/src/python/__init__.py.in
            ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py
            @ONLY
        )

        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/python/__init__.py
            DESTINATION ${PYTHON_INSTALL_DIR}/qtforge
            COMPONENT PythonBindings
        )

        # Install Python type stubs if enabled
        if(QTFORGE_PYTHON_ENABLE_TYPE_STUBS)
            file(GLOB PYTHON_STUB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/qtforge/*.pyi")
            if(PYTHON_STUB_FILES)
                install(FILES ${PYTHON_STUB_FILES}
                    DESTINATION ${PYTHON_INSTALL_DIR}/qtforge
                    COMPONENT PythonBindings
                )
                message(STATUS "QtForge: Python type stubs will be installed")
            endif()
        endif()
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

# Lua Bindings
if(QTFORGE_BUILD_LUA_BINDINGS AND QTFORGE_LUA_FOUND AND QTFORGE_SOL2_FOUND)
    message(STATUS "QtForge: Building Lua bindings")

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

    if(Qt6Network_FOUND OR NOT QTFORGE_REQUIRE_NETWORK_FOR_COMMUNICATION)
        list(APPEND QTFORGE_LUA_SOURCES
            src/lua/communication/communication_bindings.cpp
        )
        message(STATUS "QtForge: Including Lua communication bindings")
    else()
        message(STATUS "QtForge: Skipping Lua communication bindings (Qt6::Network not found)")
    endif()

    add_library(qtforge_lua SHARED ${QTFORGE_LUA_SOURCES})

    set_target_properties(qtforge_lua PROPERTIES
        OUTPUT_NAME qtforge_lua
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN ON
    )

    target_include_directories(qtforge_lua PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lua
        ${QTFORGE_LUA_INCLUDE_DIR}
    )

    if(QTFORGE_SOL2_INCLUDE_DIR)
        target_include_directories(qtforge_lua PRIVATE ${QTFORGE_SOL2_INCLUDE_DIR})
    endif()

    target_link_libraries(qtforge_lua PRIVATE ${QTFORGE_LUA_LIBRARIES})
    target_compile_definitions(qtforge_lua PRIVATE
        QTPLUGIN_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
        QTPLUGIN_VERSION_MINOR=${PROJECT_VERSION_MINOR}
        QTPLUGIN_VERSION_PATCH=${PROJECT_VERSION_PATCH}
        QTFORGE_LUA_BINDINGS
    )

    if(QTFORGE_LUA_ENABLE_SANDBOX)
        target_compile_definitions(qtforge_lua PRIVATE QTFORGE_LUA_SANDBOX_ENABLED)
    endif()

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

# Examples
if(QTFORGE_BUILD_EXAMPLES)
    message(STATUS "QtForge: Building examples")
    add_subdirectory(examples)
else()
    message(STATUS "QtForge: Examples disabled")
endif()

# Tests
if(QTFORGE_BUILD_TESTS)
    message(STATUS "QtForge: Building tests")
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "QtForge: Tests disabled")
endif()

# Documentation
if(QTFORGE_BUILD_DOCS AND DOXYGEN_FOUND)
    message(STATUS "QtForge: Building documentation with Doxygen ${DOXYGEN_VERSION}")
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
    message(STATUS "QtForge: Use 'cmake --build . --target docs' to generate documentation")
elseif(QTFORGE_BUILD_DOCS)
    message(WARNING "QtForge: Documentation requested but Doxygen not found")
    message(STATUS "QtForge: Install Doxygen to enable documentation generation:")
    message(STATUS "  - Ubuntu/Debian: sudo apt-get install doxygen")
    message(STATUS "  - macOS: brew install doxygen")
    message(STATUS "  - Windows: Download from https://www.doxygen.nl/download.html")
else()
    message(STATUS "QtForge: Documentation disabled (QTFORGE_BUILD_DOCS=OFF)")
endif()

qtforge_configure_packaging()
qtforge_add_package_target()

install(DIRECTORY include/qtplugin
    DESTINATION ${QTFORGE_INSTALL_INCLUDE_DIR}
    COMPONENT Development
    FILES_MATCHING PATTERN "*.hpp"
)

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

# Clangd support
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

    add_custom_command(TARGET QtForgeCore POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/build/compile_commands.json"
        COMMENT "Copying compile_commands.json to build/ directory for clangd support"
        VERBATIM
    )

    add_custom_target(update_clangd_db
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/compile_commands.json"
            "${CMAKE_SOURCE_DIR}/build/compile_commands.json"
        COMMENT "Manually updating compile_commands.json for clangd"
        VERBATIM
    )

    message(STATUS "QtForge: compile_commands.json will be automatically copied to build/ directory after building QtForgeCore")
    message(STATUS "QtForge: Use 'cmake --build . --target update_clangd_db' to manually update clangd database")
else()
    message(STATUS "QtForge: CMAKE_EXPORT_COMPILE_COMMANDS is OFF, skipping compile_commands.json copy setup")
endif()

option(QTFORGE_BUILD_EXAMPLES "Build QtForge examples" ON)

if(QTFORGE_BUILD_EXAMPLES)
    message(STATUS "QtForge: Building examples")

    if(QTFORGE_BUILD_PYTHON_BINDINGS AND QTFORGE_PYTHON_FOUND)
        add_subdirectory(examples/python_plugins)
    endif()
else()
    message(STATUS "QtForge: Examples disabled (QTFORGE_BUILD_EXAMPLES=OFF)")
endif()

message(STATUS "")
message(STATUS "QtForge Build System Summary:")
message(STATUS "=============================")
message(STATUS "Modular build system successfully configured!")
message(STATUS "All feature toggles are available via CMake options.")
message(STATUS "Use -DQTFORGE_BUILD_<FEATURE>=OFF to disable specific features.")
message(STATUS "")
