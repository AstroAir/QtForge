# QtForgeManagersComponent.cmake Managers component source and header
# configuration for QtForge Defines source files, headers, and MOC headers for
# manager classes

include_guard(GLOBAL)

#[=======================================================================[.rst:
QtForge Managers Component
---------------------------

This module defines the source files and headers for the QtForge managers
component. It includes:

- Configuration manager
- Logging manager
- Plugin version manager
- Resource manager and lifecycle
- Resource monitor

The managers component provides high-level management functionality for
plugins and resources.

Variables defined:
- QTFORGE_MANAGERS_SOURCES: List of managers component source files
- QTFORGE_MANAGERS_HEADERS: List of managers component headers requiring MOC
- QTFORGE_MANAGERS_PUBLIC_HEADERS: List of public API headers
#]=======================================================================]

# Managers sources
set(QTFORGE_MANAGERS_SOURCES
    src/managers/configuration_manager.cpp
    src/managers/logging_manager.cpp
    src/managers/plugin_version_manager.cpp
    src/managers/resource_manager.cpp
    src/managers/resource_lifecycle.cpp
    src/managers/resource_monitor.cpp
    # Configuration components
    src/managers/components/configuration_storage.cpp
    src/managers/components/configuration_validator.cpp
    src/managers/components/configuration_merger.cpp
    src/managers/components/configuration_watcher.cpp
    # Resource components
    src/managers/components/resource_allocator.cpp
    src/managers/components/resource_pool.cpp)

# Managers headers requiring MOC processing (contain Q_OBJECT)
set(QTFORGE_MANAGERS_MOC_HEADERS
    include/qtplugin/managers/configuration_manager.hpp
    include/qtplugin/managers/configuration_manager_impl.hpp
    include/qtplugin/managers/logging_manager.hpp
    include/qtplugin/managers/logging_manager_impl.hpp
    include/qtplugin/managers/resource_manager.hpp
    include/qtplugin/managers/resource_manager_impl.hpp
    include/qtplugin/managers/resource_lifecycle.hpp
    include/qtplugin/managers/resource_lifecycle_impl.hpp
    include/qtplugin/managers/resource_monitor.hpp
    include/qtplugin/managers/resource_monitor_impl.hpp
    # Configuration component headers
    include/qtplugin/managers/components/configuration_storage.hpp
    include/qtplugin/managers/components/configuration_validator.hpp
    include/qtplugin/managers/components/configuration_merger.hpp
    include/qtplugin/managers/components/configuration_watcher.hpp
    # Resource component headers
    include/qtplugin/managers/components/resource_allocator.hpp
    include/qtplugin/managers/components/resource_pool.hpp)

message(STATUS "QtForge Managers Component: ${CMAKE_CURRENT_LIST_FILE}")
