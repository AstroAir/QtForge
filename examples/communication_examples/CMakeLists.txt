cmake_minimum_required(VERSION 3.21)
project(QtForgeCommunicationExamples)

# Find QtForge
find_package(QtForge REQUIRED COMPONENTS Core Communication)
find_package(Qt6 REQUIRED COMPONENTS Core Network)

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
    QtForge::Core
    QtForge::Communication
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
    QtForge::Core
    QtForge::Communication
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
    QtForge::Core
    QtForge::Communication
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
    QtForge::Core
    QtForge::Communication
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
    QtForge::Core
    QtForge::Communication
    Qt6::Core
    Qt6::Network
    communication_messages
    communication_filters
    communication_statistics
    communication_utils
)

# Request-Response Example
add_executable(request_response_example
    request_response_example.cpp
    request_response_example.hpp
)

target_link_libraries(request_response_example
    QtForge::Core
    QtForge::Communication
    Qt6::Core
    Qt6::Network
)

# Event Broadcasting Example
add_executable(event_broadcasting_example
    event_broadcasting_example.cpp
    event_broadcasting_example.hpp
)

target_link_libraries(event_broadcasting_example
    QtForge::Core
    QtForge::Communication
    Qt6::Core
    Qt6::Network
)

# Message Filtering Example
add_executable(message_filtering_example
    message_filtering_example.cpp
    message_filtering_example.hpp
)

target_link_libraries(message_filtering_example
    QtForge::Core
    QtForge::Communication
    Qt6::Core
    Qt6::Network
)

# Test targets
enable_testing()

add_test(NAME AdvancedMessageBusTest COMMAND advanced_message_bus)
add_test(NAME RequestResponseTest COMMAND request_response_example)
add_test(NAME EventBroadcastingTest COMMAND event_broadcasting_example)
add_test(NAME MessageFilteringTest COMMAND message_filtering_example)

# Install examples
install(TARGETS
    advanced_message_bus
    request_response_example
    event_broadcasting_example
    message_filtering_example
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
