# QtForgeMonitoringComponent.cmake Monitoring component source and header
# configuration for QtForge Defines source files, headers, and MOC headers for
# monitoring system

include_guard(GLOBAL)

#[=======================================================================[.rst:
QtForge Monitoring Component
-----------------------------

This module defines the source files and headers for the QtForge monitoring
component. It includes:

- Plugin hot reload manager
- Plugin metrics collector

The monitoring component provides runtime monitoring and hot reload functionality.

Variables defined:
- QTFORGE_MONITORING_SOURCES: List of monitoring component source files
- QTFORGE_MONITORING_HEADERS: List of monitoring component headers requiring MOC
#]=======================================================================]

# Monitoring sources
set(QTFORGE_MONITORING_SOURCES src/monitoring/plugin_hot_reload_manager.cpp
                               src/monitoring/plugin_metrics_collector.cpp)

# Monitoring headers requiring MOC processing (contain Q_OBJECT)
set(QTFORGE_MONITORING_MOC_HEADERS
    include/qtplugin/monitoring/plugin_hot_reload_manager.hpp
    include/qtplugin/monitoring/plugin_metrics_collector.hpp)

message(STATUS "QtForge Monitoring Component: ${CMAKE_CURRENT_LIST_FILE}")
