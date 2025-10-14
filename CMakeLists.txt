# QtForge - Modern C++ Plugin System for Qt Applications Version 3.2.0
#
# This CMakeLists.txt uses a modular architecture: - Component modules
# (cmake/modules/components/) define source lists for each component - Platform
# modules (cmake/modules/platform/) provide platform-specific configurations -
# Core modules (cmake/modules/) provide build options, dependencies, targets,
# compiler, and packaging
#
# Build System Structure: 1. Platform detection and configuration
# (QtForgePlatform.cmake + platform/*.cmake) 2. Build options definition
# (QtForgeOptions.cmake) 3. Dependency management (QtForgeDependencies.cmake) 4.
# Component source aggregation (components/*.cmake) 5. Target creation
# (QtForgeTargets.cmake) 6. Compiler configuration (QtForgeCompiler.cmake) 7.
# Packaging setup (QtForgePackaging.cmake)

cmake_minimum_required(VERSION 3.21)

if(POLICY CMP0156)
  cmake_policy(SET CMP0156 NEW)
endif()

if(POLICY CMP0181)
  cmake_policy(SET CMP0181 NEW)
endif()

project(
  QtForge
  VERSION 3.2.0
  DESCRIPTION
    "Modern C++ Plugin System for Qt Applications with Modular Architecture"
  LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add cmake modules path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Include QtForge core modules
include(QtForgeOptions)
include(QtForgeDependencies)
include(QtForgeTargets)
include(QtForgeCompiler)
include(QtForgePackaging)

# Include platform-specific modules (automatically applied based on platform
# detection)
include(platform/Windows)
include(platform/Unix)
include(platform/MSYS2)

# Define build options
qtforge_define_options()

# Setup dependencies
qtforge_setup_dependencies()

# Configure compiler settings
qtforge_configure_compiler()

# Include component configuration modules
include(components/QtForgeUtilsComponent)
include(components/QtForgeCoreComponent)
include(components/QtForgeManagersComponent)
include(components/QtForgeCommunicationComponent)
include(components/QtForgeWorkflowComponent)
include(components/QtForgeMonitoringComponent)

# Security component sources (not in a separate component module)
set(QTFORGE_SECURITY_SOURCES
    src/security/components/security_validator.cpp
    src/security/components/signature_verifier.cpp
    src/security/components/security_policy_engine.cpp
    src/security/components/permission_manager.cpp)

set(QTFORGE_SECURITY_MOC_HEADERS
    include/qtplugin/security/components/security_validator.hpp
    include/qtplugin/security/components/signature_verifier.hpp
    include/qtplugin/security/components/security_policy_engine.hpp
    include/qtplugin/security/components/permission_manager.hpp)

# Aggregate all component sources into the core library Component modules
# define: QTFORGE_<COMPONENT>_SOURCES variables
set(QTFORGE_ALL_SOURCES
    # Utils component
    ${QTFORGE_UTILS_SOURCES}
    # Core component
    ${QTFORGE_CORE_SOURCES}
    # Managers component
    ${QTFORGE_MANAGERS_SOURCES}
    # Communication component
    ${QTFORGE_COMMUNICATION_SOURCES}
    # Workflow component
    ${QTFORGE_WORKFLOW_SOURCES}
    # Monitoring component
    ${QTFORGE_MONITORING_SOURCES}
    # Security component
    ${QTFORGE_SECURITY_SOURCES})

# Aggregate all component MOC headers Component modules define:
# QTFORGE_<COMPONENT>_MOC_HEADERS variables
set(QTFORGE_ALL_MOC_HEADERS
    # Core component MOC headers
    ${QTFORGE_CORE_MOC_HEADERS}
    # Managers component MOC headers
    ${QTFORGE_MANAGERS_MOC_HEADERS}
    # Communication component MOC headers
    ${QTFORGE_COMMUNICATION_MOC_HEADERS}
    # Workflow component MOC headers
    ${QTFORGE_WORKFLOW_MOC_HEADERS}
    # Monitoring component MOC headers
    ${QTFORGE_MONITORING_MOC_HEADERS}
    # Security component MOC headers
    ${QTFORGE_SECURITY_MOC_HEADERS})

# Remote plugin headers are handled by their own library in src/remote They
# should NOT be added to QtForgeCore's MOC processing

# Create the core library with all component sources This library includes all
# components: core, utils, managers, communication, workflow, monitoring
qtforge_add_library(
  NAME
  QtForgeCore
  SOURCES
  ${QTFORGE_ALL_SOURCES}
  HEADERS
  ${QTFORGE_ALL_MOC_HEADERS}
  PUBLIC_HEADERS
  # Core component public headers
  ${QTFORGE_CORE_PUBLIC_HEADERS}
  # Utils component public headers
  ${QTFORGE_UTILS_PUBLIC_HEADERS}
  # Communication component public headers
  ${QTFORGE_COMMUNICATION_PUBLIC_HEADERS}
  QT_COMPONENTS
  ${QTFORGE_CORE_QT_COMPONENTS}
  DEPENDENCIES
  Qt6::Core
  Qt6::Concurrent)

# Add subdirectories for components
if(QTFORGE_BUILD_REMOTE_PLUGINS)
  add_subdirectory(src/remote)
endif()

if(QTFORGE_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if(QTFORGE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Configure packaging
qtforge_configure_packaging()

# Print configuration summary
qtforge_print_configuration_summary()

message(STATUS "QtForge build configured successfully!")
message(STATUS "Building QtForge v${PROJECT_VERSION} with Qt6 ${Qt6_VERSION}")
