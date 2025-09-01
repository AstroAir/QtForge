# Message Bus Communication Example
cmake_minimum_required(VERSION 3.21)

project(MessageBusExample VERSION 3.0.0)

# Qt6 is already found by parent project

# Create the example library
add_library(MessageBusExample SHARED
    message_bus_example.cpp
    message_bus_example.hpp
)

# Set target properties
set_target_properties(MessageBusExample PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Include directories
target_include_directories(MessageBusExample
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(MessageBusExample
    PUBLIC
        Qt6::Core
        QtForgeCore
)

# Compile definitions
target_compile_definitions(MessageBusExample
    PRIVATE
        QT_NO_KEYWORDS
)

# Install the library
install(TARGETS MessageBusExample
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(FILES
    message_bus_example.hpp
    DESTINATION include/qtplugin/examples/communication
)
