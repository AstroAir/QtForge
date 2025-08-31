# Monitoring Plugin Example CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(MonitoringPluginExample
    VERSION 3.0.0
    DESCRIPTION "Monitoring plugin demonstrating QtForge monitoring features"
    LANGUAGES CXX
)

# Qt6 is already found by parent CMakeLists.txt

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Monitoring Plugin Library
set(MONITORING_PLUGIN_SOURCES
    monitoring_plugin.cpp
    monitoring_plugin.hpp
)

# Create the plugin library
add_library(monitoring_plugin STATIC ${MONITORING_PLUGIN_SOURCES})

# Set target properties
set_target_properties(monitoring_plugin PROPERTIES
    OUTPUT_NAME "monitoring_plugin"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    AUTOMOC ON
)

# Link libraries
target_link_libraries(monitoring_plugin
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        QtForgeCore
)

# Include directories
target_include_directories(monitoring_plugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Compiler definitions
target_compile_definitions(monitoring_plugin
    PRIVATE
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
        QT_NO_URL_CAST_FROM_STRING
        QT_NO_CAST_FROM_BYTEARRAY
        QT_USE_QSTRINGBUILDER
        QT_STRICT_ITERATORS
)

# Compiler options
if(MSVC)
    target_compile_options(monitoring_plugin PRIVATE /W4 /WX)
else()
    target_compile_options(monitoring_plugin PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

# Install plugin
install(TARGETS monitoring_plugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtforge/plugins
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install metadata
install(FILES monitoring_plugin.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/qtforge/plugins
)

# Test executable
add_executable(test_monitoring_plugin
    test_monitoring_plugin.cpp
)

target_link_libraries(test_monitoring_plugin
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        QtForgeCore
        monitoring_plugin
)

# Test properties
set_target_properties(test_monitoring_plugin PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    AUTOMOC ON
)

# Enable testing
enable_testing()

# Add tests
add_test(NAME MonitoringPluginBasicTest
    COMMAND test_monitoring_plugin basic
)

add_test(NAME MonitoringPluginHotReloadTest
    COMMAND test_monitoring_plugin hot_reload
)

add_test(NAME MonitoringPluginMetricsTest
    COMMAND test_monitoring_plugin metrics
)

add_test(NAME MonitoringPluginAlertsTest
    COMMAND test_monitoring_plugin alerts
)

# Set test properties
set_tests_properties(MonitoringPluginBasicTest MonitoringPluginHotReloadTest
                     MonitoringPluginMetricsTest MonitoringPluginAlertsTest
    PROPERTIES
        TIMEOUT 60
        ENVIRONMENT "QT_LOGGING_RULES=qtforge.monitoring.debug=true"
)

# Documentation target
if(QTFORGE_BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_PROJECT_NAME "MonitoringPlugin Example")
        set(DOXYGEN_PROJECT_BRIEF "Monitoring plugin demonstrating QtForge monitoring features")
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
        set(DOXYGEN_INPUT ${CMAKE_CURRENT_SOURCE_DIR})
        set(DOXYGEN_RECURSIVE YES)
        set(DOXYGEN_EXTRACT_ALL YES)
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_GENERATE_LATEX NO)

        doxygen_add_docs(monitoring_plugin_docs
            ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating MonitoringPlugin documentation"
        )
    endif()
endif()

# Package configuration
if(QTFORGE_BUILD_EXAMPLES_PACKAGE)
    set(CPACK_PACKAGE_NAME "QtForge-MonitoringPlugin-Example")
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
    set(CPACK_PACKAGE_VENDOR "QtForge Team")

    include(CPack)
endif()

# IDE support
if(CMAKE_EXPORT_COMPILE_COMMANDS)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Debug information
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(monitoring_plugin PRIVATE DEBUG_BUILD)
    if(NOT MSVC)
        target_compile_options(monitoring_plugin PRIVATE -g -O0)
    endif()
endif()

# Static analysis support
if(QTFORGE_ENABLE_STATIC_ANALYSIS)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set_target_properties(monitoring_plugin PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,readability-*,performance-*,modernize-*"
        )
    endif()
endif()

# Code coverage
if(QTFORGE_ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(monitoring_plugin PRIVATE --coverage)
        target_link_options(monitoring_plugin PRIVATE --coverage)
    endif()
endif()
