# QtForgeCoreComponent.cmake Core component source and header configuration for
# QtForge Defines source files, headers, and MOC headers for the core plugin
# system

include_guard(GLOBAL)

#[=======================================================================[.rst:
QtForge Core Component
----------------------

This module defines the source files and headers for the QtForge core
plugin system component. It includes:

- Plugin interface and base classes
- Plugin manager and registry
- Plugin loader and dependency resolver
- Plugin lifecycle management

The core component provides the fundamental plugin system functionality
that all other components depend on.

Variables defined:
- QTFORGE_CORE_SOURCES: List of core component source files
- QTFORGE_CORE_HEADERS: List of core component headers requiring MOC
- QTFORGE_CORE_PUBLIC_HEADERS: List of public API headers
#]=======================================================================]

# Core plugin system sources
set(QTFORGE_CORE_SOURCES
    # Main entry point
    src/qtplugin.cpp
    # Core plugin system
    src/core/plugin_interface.cpp
    src/core/plugin_manager.cpp
    src/core/plugin_registry.cpp
    src/core/plugin_loader.cpp
    src/core/plugin_dependency_resolver.cpp
    src/core/plugin_lifecycle_manager.cpp)

# Conditionally add plugin bridges based on build options
if(QTFORGE_BUILD_PYTHON_BINDINGS AND QTFORGE_PYTHON_FOUND)
  list(APPEND QTFORGE_CORE_SOURCES src/bridges/python_plugin_bridge.cpp)
  message(STATUS "QtForge: Python plugin bridge enabled")
endif()

if(QTFORGE_LUA_BINDINGS AND QTFORGE_LUA_FOUND)
  list(APPEND QTFORGE_CORE_SOURCES src/bridges/lua_plugin_bridge.cpp)
  message(STATUS "QtForge: Lua plugin bridge enabled")
endif()

# Core headers requiring MOC processing (contain Q_OBJECT)
set(QTFORGE_CORE_MOC_HEADERS
    include/qtplugin/core/plugin_manager.hpp
    include/qtplugin/core/plugin_registry.hpp
    include/qtplugin/core/plugin_dependency_resolver.hpp
    include/qtplugin/core/plugin_lifecycle_manager.hpp)

# Conditionally add bridge headers that require MOC processing
if(QTFORGE_BUILD_PYTHON_BINDINGS AND QTFORGE_PYTHON_FOUND)
  list(APPEND QTFORGE_CORE_MOC_HEADERS
       include/qtplugin/bridges/python_plugin_bridge.hpp)
endif()

if(QTFORGE_LUA_BINDINGS AND QTFORGE_LUA_FOUND)
  list(APPEND QTFORGE_CORE_MOC_HEADERS
       include/qtplugin/bridges/lua_plugin_bridge.hpp)
endif()

# Public API headers for core component
set(QTFORGE_CORE_PUBLIC_HEADERS
    include/qtplugin/qtplugin.hpp
    include/qtplugin/components.hpp
    include/qtplugin/interfaces/core/plugin_interface.hpp
    include/qtplugin/core/plugin_manager.hpp
    include/qtplugin/core/plugin_registry.hpp
    include/qtplugin/core/plugin_loader.hpp
    include/qtplugin/core/plugin_dependency_resolver.hpp)

# Qt components required by core
set(QTFORGE_CORE_QT_COMPONENTS Core Concurrent)

message(STATUS "QtForge Core Component: ${CMAKE_CURRENT_LIST_FILE}")
