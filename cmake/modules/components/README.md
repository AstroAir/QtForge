# QtForge Component Modules

This directory contains CMake modules that define the source files, headers, and configuration for each logical component of the QtForge library.

## Purpose

Component modules provide a centralized, maintainable way to manage source file lists and component-specific configurations. Each module defines:

- **Source files**: `.cpp` files for the component
- **MOC headers**: Headers containing `Q_OBJECT` that require Qt's Meta-Object Compiler
- **Public headers**: API headers exposed to library users
- **Qt components**: Required Qt modules for the component

## Available Components

### Core Component (`QtForgeCoreComponent.cmake`)

The fundamental plugin system providing:

- Plugin interface and base classes
- Plugin manager and registry
- Plugin loader and dependency resolver
- Plugin lifecycle management

**Variables:**

- `QTFORGE_CORE_SOURCES`
- `QTFORGE_CORE_MOC_HEADERS`
- `QTFORGE_CORE_PUBLIC_HEADERS`
- `QTFORGE_CORE_QT_COMPONENTS`

### Utils Component (`QtForgeUtilsComponent.cmake`)

Shared utility functionality:

- Version management
- Error handling
- Common utility functions

**Variables:**

- `QTFORGE_UTILS_SOURCES`
- `QTFORGE_UTILS_PUBLIC_HEADERS`

### Managers Component (`QtForgeManagersComponent.cmake`)

High-level management functionality:

- Configuration manager
- Logging manager
- Plugin version manager
- Resource manager and lifecycle
- Resource monitor

**Variables:**

- `QTFORGE_MANAGERS_SOURCES`
- `QTFORGE_MANAGERS_MOC_HEADERS`

### Communication Component (`QtForgeCommunicationComponent.cmake`)

Inter-plugin communication:

- Message bus
- Plugin service contracts
- Request-response system
- Typed event system
- Message routing and publishing

**Variables:**

- `QTFORGE_COMMUNICATION_SOURCES`
- `QTFORGE_COMMUNICATION_MOC_HEADERS`
- `QTFORGE_COMMUNICATION_PUBLIC_HEADERS`

### Workflow Component (`QtForgeWorkflowComponent.cmake`)

Advanced workflow orchestration:

- Workflow composition and orchestration
- Error recovery and transaction handling
- Progress tracking and monitoring
- State persistence
- Rollback management

**Variables:**

- `QTFORGE_WORKFLOW_SOURCES`
- `QTFORGE_WORKFLOW_MOC_HEADERS`

### Monitoring Component (`QtForgeMonitoringComponent.cmake`)

Runtime monitoring and hot reload:

- Plugin hot reload manager
- Plugin metrics collector

**Variables:**

- `QTFORGE_MONITORING_SOURCES`
- `QTFORGE_MONITORING_MOC_HEADERS`

## Usage

Component modules are included in the root `CMakeLists.txt` or component-specific build files:

```cmake
# Include component module
include(components/QtForgeCoreComponent)

# Use the defined variables
add_library(MyTarget ${QTFORGE_CORE_SOURCES})
```

## Adding New Components

To add a new component:

1. Create a new `.cmake` file in this directory
2. Follow the naming convention: `QtForge<ComponentName>Component.cmake`
3. Define appropriate variables with `PARENT_SCOPE`
4. Add documentation using RST-style comments
5. Include the module in the appropriate build file

## Best Practices

- **Use `include_guard(GLOBAL)`** to prevent multiple inclusions
- **Set variables with `PARENT_SCOPE`** to make them available to the including scope
- **Document all variables** using RST-style comments
- **Keep source lists organized** and commented
- **Use relative paths** from the project root
