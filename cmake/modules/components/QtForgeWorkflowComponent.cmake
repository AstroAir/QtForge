# QtForgeWorkflowComponent.cmake Workflow component source and header
# configuration for QtForge Defines source files, headers, and MOC headers for
# workflow system

include_guard(GLOBAL)

#[=======================================================================[.rst:
QtForge Workflow Component
---------------------------

This module defines the source files and headers for the QtForge workflow
component. It includes:

- Workflow composition and orchestration
- Error recovery and transaction handling
- Progress tracking and monitoring
- State persistence
- Rollback management

The workflow component provides advanced workflow orchestration functionality.

Variables defined:
- QTFORGE_WORKFLOW_SOURCES: List of workflow component source files
- QTFORGE_WORKFLOW_HEADERS: List of workflow component headers requiring MOC
#]=======================================================================]

# Workflow sources
set(QTFORGE_WORKFLOW_SOURCES
    src/workflow/composition.cpp
    src/workflow/error_recovery.cpp
    src/workflow/integration.cpp
    src/workflow/orchestration.cpp
    src/workflow/progress_message_bus.cpp
    src/workflow/progress_monitoring.cpp
    src/workflow/progress_tracking.cpp
    src/workflow/rollback_manager.cpp
    src/workflow/state_persistence.cpp
    src/workflow/transaction_error_handler.cpp
    src/workflow/transactions.cpp
    src/workflow/workflow_manager.cpp
    src/workflow/workflow_validator.cpp)

# Workflow headers requiring MOC processing (contain Q_OBJECT)
set(QTFORGE_WORKFLOW_MOC_HEADERS
    include/qtplugin/workflow/composition.hpp
    include/qtplugin/workflow/error_recovery.hpp
    include/qtplugin/workflow/integration.hpp
    include/qtplugin/workflow/orchestration.hpp
    include/qtplugin/workflow/progress_message_bus.hpp
    include/qtplugin/workflow/progress_monitoring.hpp
    include/qtplugin/workflow/progress_tracking.hpp
    include/qtplugin/workflow/rollback_manager.hpp
    include/qtplugin/workflow/state_persistence.hpp
    include/qtplugin/workflow/transaction_error_handler.hpp
    include/qtplugin/workflow/transactions.hpp
    include/qtplugin/workflow/workflow_validator.hpp)

message(STATUS "QtForge Workflow Component: ${CMAKE_CURRENT_LIST_FILE}")
