# Network Plugin Example CMakeLists.txt
cmake_minimum_required(VERSION 3.21)

project(NetworkPluginExample
    VERSION 3.0.0
    DESCRIPTION "Network plugin demonstrating QtForge network features"
    LANGUAGES CXX
)

# Qt6 is already found by parent CMakeLists.txt

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Network Plugin Library
set(NETWORK_PLUGIN_SOURCES
    network_plugin.cpp
    network_plugin.hpp
)

# Enable Qt MOC processing
set(CMAKE_AUTOMOC ON)

# Create the plugin library
add_library(network_plugin SHARED ${NETWORK_PLUGIN_SOURCES})

# Set target properties
set_target_properties(network_plugin PROPERTIES
    OUTPUT_NAME "network_plugin"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
)

# Link libraries
target_link_libraries(network_plugin
    PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::WebSockets
        Qt6::HttpServer
        QtForgeCore
)

# Include directories
target_include_directories(network_plugin
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Compiler definitions
target_compile_definitions(network_plugin
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
    target_compile_options(network_plugin PRIVATE /W4 /WX)
else()
    target_compile_options(network_plugin PRIVATE -Wall -Wextra -Werror -pedantic)
endif()

# Install plugin
install(TARGETS network_plugin
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/qtforge/plugins
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install metadata
install(FILES network_plugin.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/qtforge/plugins
)

# Test executable (commented out - test file doesn't exist)
# add_executable(test_network_plugin
#     test_network_plugin.cpp
# )
#
# target_link_libraries(test_network_plugin
#     PRIVATE
#         Qt6::Core
#         Qt6::Network
#         Qt6::WebSockets
#         QtForgeCore
#         QtForgeNetwork
#         network_plugin
# )
#
# # Test properties
# set_target_properties(test_network_plugin PROPERTIES
#     CXX_VISIBILITY_PRESET hidden
#     VISIBILITY_INLINES_HIDDEN YES
# )

# Enable testing
enable_testing()

# Add tests (commented out - test executable doesn't exist)
# add_test(NAME NetworkPluginBasicTest
#     COMMAND test_network_plugin basic
# )
#
# add_test(NAME NetworkPluginHttpTest
#     COMMAND test_network_plugin http
# )
#
# add_test(NAME NetworkPluginWebSocketTest
#     COMMAND test_network_plugin websocket
# )
#
# add_test(NAME NetworkPluginServerTest
#     COMMAND test_network_plugin server
# )
#
# # Set test properties
# set_tests_properties(NetworkPluginBasicTest NetworkPluginHttpTest
#                      NetworkPluginWebSocketTest NetworkPluginServerTest
#     PROPERTIES
#         TIMEOUT 60
#         ENVIRONMENT "QT_LOGGING_RULES=qtforge.network.debug=true"
# )

# Documentation target
if(QTFORGE_BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_PROJECT_NAME "NetworkPlugin Example")
        set(DOXYGEN_PROJECT_BRIEF "Network plugin demonstrating QtForge network features")
        set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/docs)
        set(DOXYGEN_INPUT ${CMAKE_CURRENT_SOURCE_DIR})
        set(DOXYGEN_RECURSIVE YES)
        set(DOXYGEN_EXTRACT_ALL YES)
        set(DOXYGEN_GENERATE_HTML YES)
        set(DOXYGEN_GENERATE_LATEX NO)

        doxygen_add_docs(network_plugin_docs
            ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Generating NetworkPlugin documentation"
        )
    endif()
endif()

# Package configuration
if(QTFORGE_BUILD_EXAMPLES_PACKAGE)
    set(CPACK_PACKAGE_NAME "QtForge-NetworkPlugin-Example")
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
    target_compile_definitions(network_plugin PRIVATE DEBUG_BUILD)
    if(NOT MSVC)
        target_compile_options(network_plugin PRIVATE -g -O0)
    endif()
endif()

# Static analysis support
if(QTFORGE_ENABLE_STATIC_ANALYSIS)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        set_target_properties(network_plugin PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,readability-*,performance-*,modernize-*"
        )
    endif()
endif()

# Code coverage
if(QTFORGE_ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(network_plugin PRIVATE --coverage)
        target_link_options(network_plugin PRIVATE --coverage)
    endif()
endif()
