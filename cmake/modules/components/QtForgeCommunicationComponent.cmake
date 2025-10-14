# QtForgeCommunicationComponent.cmake Communication component source and header
# configuration for QtForge Defines source files, headers, and MOC headers for
# communication system

include_guard(GLOBAL)

#[=======================================================================[.rst:
QtForge Communication Component
--------------------------------

This module defines the source files and headers for the QtForge communication
component. It includes:

- Message bus
- Plugin service contracts
- Request-response system
- Typed event system
- Message routing and publishing
- Statistics collection

The communication component provides inter-plugin communication functionality.

Variables defined:
- QTFORGE_COMMUNICATION_SOURCES: List of communication component source files
- QTFORGE_COMMUNICATION_MOC_HEADERS: List of communication component headers requiring MOC
- QTFORGE_COMMUNICATION_PUBLIC_HEADERS: List of public API headers (includes deprecated headers for backward compatibility)
#]=======================================================================]

# Communication sources
set(QTFORGE_COMMUNICATION_SOURCES
    src/communication/message_bus.cpp
    src/communication/plugin_service_contracts.cpp
    src/communication/factory.cpp
    src/communication/message_publisher.cpp
    src/communication/message_router.cpp
    src/communication/request_response_service_impl.cpp
    src/communication/request_response_system.cpp
    src/communication/statistics_collector.cpp
    src/communication/subscription_manager.cpp
    src/communication/typed_event_system.cpp
    src/communication/event_system_impl.cpp)

# Communication headers requiring MOC processing (contain Q_OBJECT)
set(QTFORGE_COMMUNICATION_MOC_HEADERS
    include/qtplugin/communication/message_bus.hpp
    include/qtplugin/communication/request_response_system.hpp
    include/qtplugin/communication/typed_event_system.hpp)

# Public API headers for communication component
set(QTFORGE_COMMUNICATION_PUBLIC_HEADERS
    include/qtplugin/communication/factory.hpp
    include/qtplugin/communication/interfaces.hpp
    include/qtplugin/communication/message_bus.hpp
    include/qtplugin/communication/message_types.hpp
    include/qtplugin/communication/plugin_service_contracts.hpp
    include/qtplugin/communication/plugin_service_discovery.hpp
    include/qtplugin/communication/request_response_system.hpp
    include/qtplugin/communication/typed_event_system.hpp
    # Deprecated headers (backward compatibility)
    include/qtplugin/communication/request_response.hpp
    include/qtplugin/communication/plugin_communication.hpp)

message(STATUS "QtForge Communication Component: ${CMAKE_CURRENT_LIST_FILE}")
