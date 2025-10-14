# QtPlugin Examples CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Force C++20 for all examples (required by QtForge core library)
set(CMAKE_CXX_STANDARD
    20
    CACHE STRING "C++ standard for examples" FORCE)
set(CMAKE_CXX_STANDARD_REQUIRED
    ON
    CACHE BOOL "Require C++ standard" FORCE)

# Enhanced Basic Plugin Example
add_subdirectory(01-fundamentals/basic-plugin)

# Configuration Plugin Example - TEMPORARILY DISABLED (needs API update to
# current IPlugin interface) add_subdirectory(01-fundamentals/configuration)

# Request-Response Communication Example - TEMPORARILY DISABLED (needs API
# update to current IPlugin interface)
# add_subdirectory(02-communication/request-response)

# Service Plugin Example (background processing and MessageBus integration) -
# TEMPORARILY DISABLED (needs API update)
# add_subdirectory(03-services/background-tasks)

# UI Plugin Example (Qt Widgets integration and theme support) - TEMPORARILY
# DISABLED (needs API update) if(QTFORGE_BUILD_UI)
# add_subdirectory(04-specialized/ui-integration) else() message(STATUS "UI
# Plugin example skipped (QTFORGE_BUILD_UI=OFF)") endif()

# Threading and Transactions Examples - TEMPORARILY DISABLED (needs API update)
# add_subdirectory(04-specialized/threading)
# add_subdirectory(04-specialized/transactions)

# Composition Basics Example - TEMPORARILY DISABLED (needs API update)
# add_subdirectory(01-fundamentals/composition-basics)

# Security Example (stubbed) - TEMPORARILY DISABLED (needs API update)
# if(QTFORGE_BUILD_SECURITY) add_subdirectory(04-specialized/security) else()
# message(STATUS "Security example skipped (QTFORGE_BUILD_SECURITY=OFF)")
# endif()

# Communication Examples (advanced MessageBus patterns) - temporarily disabled
# due to interface issues add_subdirectory(02-communication/event-driven)
# add_subdirectory(02-communication/message-bus)

# Python Integration Examples (commented out - directory doesn't exist)
# if(QTPLUGIN_BUILD_PYTHON) add_subdirectory(python) else() message(STATUS
# "Python examples skipped (QTPLUGIN_BUILD_PYTHON=OFF)") endif()

# Version Management Example - temporarily disabled due to complex dependencies
# add_subdirectory(05-integration/version-management)

# === NEW COMPREHENSIVE EXAMPLES ===

# Security Plugin Example removed

# Monitoring Plugin Example (hot reload and metrics collection) - TEMPORARILY
# DISABLED (needs API update) if(QTPLUGIN_BUILD_MONITORING)
# add_subdirectory(04-specialized/monitoring) message(STATUS "Monitoring Plugin
# example included") else() message(STATUS "Monitoring Plugin example skipped
# (QTPLUGIN_BUILD_MONITORING=OFF)") endif()

# Network Plugin Example (REST API and WebSocket integration) - TEMPORARILY
# DISABLED (needs API update) if(QTPLUGIN_BUILD_NETWORK)
# add_subdirectory(04-specialized/network) message(STATUS "Network Plugin
# example included") else() message(STATUS "Network Plugin example skipped
# (QTPLUGIN_BUILD_NETWORK=OFF)") endif()

# === PLANNED EXAMPLES (commented out until implemented) ===
# Database Plugin Example (database integration patterns)
# if(QTPLUGIN_BUILD_DATABASE) add_subdirectory(database_plugin) message(STATUS
# "Database Plugin example included") else() message(STATUS "Database Plugin
# example skipped (QTPLUGIN_BUILD_DATABASE=OFF)") endif()

# Composition Plugin Example (plugin pipelines and orchestration)
# if(QTPLUGIN_BUILD_COMPOSITION) add_subdirectory(composition_plugin)
# message(STATUS "Composition Plugin example included") else() message(STATUS
# "Composition Plugin example skipped (QTPLUGIN_BUILD_COMPOSITION=OFF)") endif()

# Bridge Plugin Example (cross-platform and language bridging)
# if(QTPLUGIN_BUILD_BRIDGES) add_subdirectory(bridge_plugin) message(STATUS
# "Bridge Plugin example included") else() message(STATUS "Bridge Plugin example
# skipped (QTPLUGIN_BUILD_BRIDGES=OFF)") endif()

# Advanced Configuration Example (complex configuration management)
# add_subdirectory(advanced_configuration)

# File System Plugin Example (file operations and monitoring)
# add_subdirectory(filesystem_plugin)

# Scripting Plugin Example (script engine integration)
# if(QTPLUGIN_BUILD_SCRIPTING) add_subdirectory(scripting_plugin) message(STATUS
# "Scripting Plugin example included") else() message(STATUS "Scripting Plugin
# example skipped (QTPLUGIN_BUILD_SCRIPTING=OFF)") endif()

# === EXAMPLE SUMMARY ===
message(STATUS "")
message(STATUS "=== QtForge Plugin Examples Summary ===")
message(STATUS "�?Basic Plugin - Core plugin interface and lifecycle")
message(STATUS "�?Service Plugin - Background processing and MessageBus")
message(STATUS "�?UI Plugin - Qt Widgets integration and themes")
message(STATUS "�?Communication Examples - Advanced MessageBus patterns")
message(STATUS "📋 Python Integration - Python bindings and scripting (planned)")
message(
  STATUS "📋 Version Management - Plugin versioning and compatibility (planned)")
message(STATUS "�?Security Example - Validator and basic checks (stub)")
message(STATUS "�?Monitoring Plugin - Hot reload and metrics collection")
message(STATUS "�?Network Plugin - REST API and WebSocket integration")
message(STATUS "📋 Database Plugin - Database integration patterns (planned)")
message(
  STATUS "📋 Composition Plugin - Plugin pipelines and orchestration (planned)")
message(
  STATUS "📋 Marketplace Plugin - Plugin management and distribution (planned)")
message(
  STATUS "📋 Bridge Plugin - Cross-platform and language bridging (planned)")
message(
  STATUS "📋 Advanced Configuration - Complex configuration management (planned)"
)
message(
  STATUS "📋 File System Plugin - File operations and monitoring (planned)")
message(STATUS "📋 Scripting Plugin - Script engine integration (planned)")
message(STATUS "")
message(STATUS "Legend: �?= Implemented, 📋 = Planned")
message(STATUS "==========================================")
