cmake_minimum_required(VERSION 3.21)
project(QtForgeComprehensiveExample VERSION 3.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
find_package(QtForge REQUIRED COMPONENTS Core Security Communication Monitoring)

# Enable Qt MOC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Compiler-specific options
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Main application
set(MAIN_SOURCES
    main.cpp
)

set(MAIN_HEADERS
    # Headers are included in main.cpp
)

add_executable(comprehensive_demo ${MAIN_SOURCES} ${MAIN_HEADERS})

target_link_libraries(comprehensive_demo
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    QtForge::Core
    QtForge::Security
    QtForge::Communication
    QtForge::Monitoring
)

# Set target properties
set_target_properties(comprehensive_demo PROPERTIES
    OUTPUT_NAME "comprehensive_demo"
    DEBUG_POSTFIX "_d"
)

# Comprehensive Plugin
add_subdirectory(plugins/comprehensive_plugin)

# Configuration files
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config/application.json.in
    ${CMAKE_CURRENT_BINARY_DIR}/config/application.json
    @ONLY
)

# Copy configuration files to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/config/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/config/
     FILES_MATCHING PATTERN "*.json")

# Copy Python scripts if Python support is enabled
if(QTFORGE_PYTHON_SUPPORT)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/python/
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/python/
         FILES_MATCHING PATTERN "*.py")
endif()

# Create plugins directory in build
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plugins)

# Install targets
install(TARGETS comprehensive_demo
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install configuration files
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/config/
    DESTINATION share/qtforge/comprehensive_example/config
    FILES_MATCHING PATTERN "*.json"
)

# Install Python scripts
if(QTFORGE_PYTHON_SUPPORT)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python/
        DESTINATION share/qtforge/comprehensive_example/python
        FILES_MATCHING PATTERN "*.py"
    )
endif()

# Testing
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Documentation
option(BUILD_DOCUMENTATION "Build documentation" OFF)
if(BUILD_DOCUMENTATION)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        add_custom_target(doc_comprehensive_example ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "QtForge-Comprehensive-Example")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Comprehensive example demonstrating all QtForge features")
set(CPACK_PACKAGE_VENDOR "QtForge Development Team")
set(CPACK_PACKAGE_CONTACT "support@qtforge.org")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
else()
    set(CPACK_GENERATOR "DEB;RPM;TGZ")
endif()

include(CPack)

# Print configuration summary
message(STATUS "")
message(STATUS "QtForge Comprehensive Example Configuration Summary:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Qt version: ${Qt6_VERSION}")
message(STATUS "  QtForge version: ${QtForge_VERSION}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Python support: ${QTFORGE_PYTHON_SUPPORT}")
message(STATUS "  Build tests: ${BUILD_TESTING}")
message(STATUS "  Build documentation: ${BUILD_DOCUMENTATION}")
message(STATUS "")
