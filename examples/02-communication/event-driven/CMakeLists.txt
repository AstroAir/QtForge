# Event-Driven Communication Example
cmake_minimum_required(VERSION 3.21)

project(EventDrivenExample VERSION 3.0.0)

# Qt6 is already found by parent project

# Create the example library
add_library(
  EventDrivenExample SHARED
  message_filters.cpp message_filters.hpp ../messages/system_event_message.cpp
  ../messages/system_event_message.hpp)

# Set target properties
set_target_properties(
  EventDrivenExample
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED ON
             VERSION ${PROJECT_VERSION}
             SOVERSION ${PROJECT_VERSION_MAJOR})

# Include directories
target_include_directories(
  EventDrivenExample
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..
  PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link libraries
target_link_libraries(EventDrivenExample PUBLIC Qt6::Core QtForgeCore)

# Compile definitions
target_compile_definitions(EventDrivenExample PRIVATE QT_NO_KEYWORDS)

# Install the library
install(
  TARGETS EventDrivenExample
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin)

# Install headers
install(FILES message_filters.hpp ../messages/system_event_message.hpp
        DESTINATION include/qtplugin/examples/communication)
