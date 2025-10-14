# QtForgeUtilsComponent.cmake Utility component source and header configuration
# for QtForge Defines source files and headers for utility classes

include_guard(GLOBAL)

#[=======================================================================[.rst:
QtForge Utils Component
-----------------------

This module defines the source files and headers for the QtForge utility
component. It includes:

- Version management
- Error handling
- Common utility functions

The utils component provides shared utility functionality used across
all other components.

Variables defined:
- QTFORGE_UTILS_SOURCES: List of utils component source files
- QTFORGE_UTILS_PUBLIC_HEADERS: List of public API headers
#]=======================================================================]

# Utility sources
set(QTFORGE_UTILS_SOURCES src/utils/version.cpp src/utils/error_handling.cpp)

# Public API headers for utils component
set(QTFORGE_UTILS_PUBLIC_HEADERS include/qtplugin/utils/version.hpp
                                 include/qtplugin/utils/error_handling.hpp)

message(STATUS "QtForge Utils Component: ${CMAKE_CURRENT_LIST_FILE}")
