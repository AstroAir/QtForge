cmake_minimum_required(VERSION 3.21)
project(QtForgeCommunicationExamples)

# Qt6 is already found by parent CMakeLists.txt

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Message Components Library
add_library(communication_messages STATIC
    messages/system_event_message.cpp
    messages/system_event_message.hpp
    messages/performance_metrics_message.cpp
    messages/performance_metrics_message.hpp
)

target_link_libraries(communication_messages
    QtForgeCore
    Qt6::Core
)

target_include_directories(communication_messages PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Filter Components Library
add_library(communication_filters STATIC
    filters/message_filters.cpp
    filters/message_filters.hpp
)

target_link_libraries(communication_filters
    QtForgeCore
    Qt6::Core
    communication_messages
)

target_include_directories(communication_filters PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Statistics Components Library
add_library(communication_statistics STATIC
    statistics/message_statistics.cpp
    statistics/message_statistics.hpp
)

target_link_libraries(communication_statistics
    QtForgeCore
    Qt6::Core
)

target_include_directories(communication_statistics PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Utility Components Library
add_library(communication_utils STATIC
    utils/message_utils.cpp
    utils/message_utils.hpp
)

target_link_libraries(communication_utils
    QtForgeCore
    Qt6::Core
    communication_messages
)

target_include_directories(communication_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Advanced MessageBus Example (Reorganized)
add_executable(advanced_message_bus
    main.cpp
    core/message_bus_example.cpp
    core/message_bus_example.hpp
)

target_link_libraries(advanced_message_bus
    QtForgeCore
    Qt6::Core
    Qt6::Network
    communication_messages
    communication_filters
    communication_statistics
    communication_utils
)

# Main communication examples executable
add_executable(communication_examples_main
    main.cpp
)

target_link_libraries(communication_examples_main
    QtForgeCore
    Qt6::Core
    Qt6::Network
    communication_messages
    communication_filters
    communication_statistics
    communication_utils
)

# Test targets
enable_testing()

add_test(NAME AdvancedMessageBusTest COMMAND advanced_message_bus)
add_test(NAME CommunicationExamplesTest COMMAND communication_examples_main)

# Install examples
install(TARGETS
    advanced_message_bus
    communication_examples_main
    DESTINATION bin/examples/communication
)

# Install reorganized source files for reference
install(FILES
    # Core files
    main.cpp
    README.md
    DESTINATION share/qtforge/examples/communication
)

# Install modular component files
install(DIRECTORY
    core/
    messages/
    filters/
    statistics/
    utils/
    DESTINATION share/qtforge/examples/communication
    FILES_MATCHING PATTERN "*.cpp" PATTERN "*.hpp"
)

# Install other example files (if they exist)
install(FILES
    request_response_example.cpp
    request_response_example.hpp
    event_broadcasting_example.cpp
    event_broadcasting_example.hpp
    message_filtering_example.cpp
    message_filtering_example.hpp
    DESTINATION share/qtforge/examples/communication
    OPTIONAL
)
