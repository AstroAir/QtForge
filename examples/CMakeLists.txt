# QtPlugin Examples CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

# Enhanced Basic Plugin Example
add_subdirectory(basic_plugin)

# Service Plugin Example (background processing and MessageBus integration)
add_subdirectory(service_plugin)

# UI Plugin Example (Qt Widgets integration and theme support)
if(QTPLUGIN_BUILD_UI)
    add_subdirectory(ui_plugin)
else()
    message(STATUS "UI Plugin example skipped (QTPLUGIN_BUILD_UI=OFF)")
endif()

# Version Management Example (existing)
# Note: version_management example exists but may need CMakeLists.txt
