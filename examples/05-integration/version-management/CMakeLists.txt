# Version Management Example
cmake_minimum_required(VERSION 3.21)

project(VersionManagementExample VERSION 3.0.0)

# Qt6 is already found by parent project

# Create the example executable
add_executable(VersionManagementExample
    version_management_example.cpp
)

# Set target properties
set_target_properties(VersionManagementExample PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Include directories
target_include_directories(VersionManagementExample
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(VersionManagementExample
    PRIVATE
        Qt6::Core
        QtForgeCore
)

# Compile definitions
target_compile_definitions(VersionManagementExample
    PRIVATE
        QT_NO_KEYWORDS
)

# Install the executable
install(TARGETS VersionManagementExample
    RUNTIME DESTINATION bin
)
