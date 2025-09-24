# QtForge Modular XMake Build System

## Overview

The QtForge project uses a modular XMake build system that provides consistent, maintainable, and scalable build configurations across all examples and components. This system eliminates code duplication, standardizes build patterns, and makes it easy to add new examples.

## Architecture

### Directory Structure

```
cmake/xmake/
├── xmake.lua              # Module directory configuration
├── common.lua             # Shared build settings and compiler configurations
├── dependencies.lua       # Centralized Qt and dependency management
├── plugin_template.lua    # Standardized plugin target creation functions
└── example_template.lua   # Standardized example application creation functions

examples/
├── xmake.lua             # Main examples configuration with conditional building
├── 01-fundamentals/
│   ├── hello-world/
│   │   └── xmake.lua     # Individual example configuration
│   └── basic-plugin/
│       └── xmake.lua     # Plugin with test target
├── 02-communication/
│   └── message-bus/
│       └── xmake.lua     # Communication library example
└── 04-specialized/
    └── network/
        └── xmake.lua     # Network plugin example
```

### Core Modules

#### 1. `common.lua` - Shared Build Settings

- Project setup and versioning
- Compiler configuration (C++20, warnings, optimizations)
- Platform-specific settings
- Version defines and build constants

#### 2. `dependencies.lua` - Dependency Management

- Qt package detection and configuration
- Feature set definitions (basic, plugin, ui, network, comprehensive)
- Conditional dependency loading
- QtForge library integration

#### 3. `plugin_template.lua` - Plugin Templates

- Standardized plugin target creation
- Test target generation
- Metadata handling and installation
- Qt MOC integration

#### 4. `example_template.lua` - Example Templates

- Application target creation (console, widget, comprehensive)
- Documentation installation
- Test configuration
- Build artifact management

## Usage Guide

### Adding a New Example

1. **Create the example directory:**

   ```bash
   mkdir examples/category/example-name
   cd examples/category/example-name
   ```

2. **Create a basic xmake.lua file:**

   ```lua
   -- Example Name XMake Configuration
   -- Brief description of what this example demonstrates

   -- Set minimum xmake version
   set_xmakever("3.0.1")

   -- Set project info
   set_project("ExampleName")
   set_version("1.0.0")

   -- Set C++ standard
   set_languages("c++20")

   -- Add Qt dependencies
   add_requires("qt6core", {optional = true, configs = {shared = true}})

   -- Example target
   target("ExampleTarget")
       set_kind("shared")  -- or "binary" for applications
       set_basename("example_name")

       -- Add Qt rules
       add_rules("qt.shared")  -- or "qt.console" for applications

       -- Add source files
       add_files("*.cpp")
       add_headerfiles("*.hpp")

       -- Add Qt packages
       add_packages("qt6core")
       add_frameworks("QtCore")

       -- Link with QtForge
       add_deps("QtForgeCore")
       add_includedirs("../../../include", {public = false})

       -- Set output directory
       set_targetdir("$(buildir)/plugins")  -- or "$(buildir)/bin" for apps

       -- Set version
       set_version("1.0.0")

       -- Install target
       add_installfiles("$(targetdir)/$(targetname)$(extension)", {prefixdir = "plugins"})
   target_end()
   ```

3. **Add to examples configuration:**
   Edit `examples/xmake.lua` and add your example to the `example_configs` table:
   ```lua
   local example_configs = {
       -- ... existing examples ...
       ["category/example-name"] = {qt_features = "basic"},  -- or "plugin", "ui", "network", "comprehensive"
   }
   ```

### Qt Feature Sets

The system supports different Qt feature sets for conditional building:

- **basic**: Qt6Core only
- **plugin**: Qt6Core for plugin development
- **ui**: Qt6Core + Qt6Widgets + Qt6Gui for UI applications
- **network**: Qt6Core + Qt6Network + Qt6WebSockets + Qt6HttpServer
- **comprehensive**: All Qt features

### Build Configuration

#### Configure with Examples

```bash
xmake f --examples=y --mode=debug
```

#### Build All Examples

```bash
xmake build
```

#### Build Specific Example

```bash
xmake build ExampleTargetName
```

## Benefits

### 1. **Consistency**

- All examples follow the same build patterns
- Standardized output directories and naming conventions
- Consistent Qt integration and dependency management

### 2. **Maintainability**

- Centralized configuration reduces duplication
- Easy to update build settings across all examples
- Clear separation of concerns

### 3. **Scalability**

- Easy to add new examples without duplicating configuration
- Modular system supports different types of examples
- Conditional building based on available dependencies

### 4. **Developer Experience**

- Clear documentation and examples
- Consistent development patterns
- Easy debugging and troubleshooting

## Troubleshooting

### Common Issues

1. **Import Errors**: If you encounter import-related errors, ensure you're using direct configuration instead of template imports for now.

2. **Qt Package Not Found**: Make sure Qt6 is properly installed and configured in your environment.

3. **Build Failures**: Check that all required source files exist and QtForge dependencies are available.

### Getting Help

- Check the individual example xmake.lua files for reference implementations
- Review the modular templates in `cmake/xmake/` for advanced usage
- Consult the main project documentation for QtForge-specific requirements

## Migration from Template System

The original design included a sophisticated template system using xmake imports. Due to technical challenges with xmake's module system, the current implementation uses direct configuration. Here's how to migrate:

### From Template-Based Configuration

```lua
-- Old template approach (currently disabled)
local plugin_template = import("plugin_template")
plugin_template.create_plugin("MyPlugin", {...})
```

### To Direct Configuration

```lua
-- Current direct approach
target("MyPlugin")
    set_kind("shared")
    -- ... direct configuration ...
target_end()
```

## Best Practices

### 1. **Naming Conventions**

- Use descriptive target names that match the example purpose
- Follow the pattern: `CategoryExampleName` (e.g., `NetworkPlugin`, `HelloWorldPlugin`)
- Use snake_case for file basenames

### 2. **Directory Organization**

- Group related examples in numbered categories (01-fundamentals, 02-communication, etc.)
- Use descriptive directory names that clearly indicate the example's purpose
- Keep example directories focused on a single concept

### 3. **Configuration Management**

- Always specify Qt dependencies explicitly
- Use conditional building for optional features
- Include proper installation rules for all targets

### 4. **Documentation**

- Include clear comments explaining the example's purpose
- Document any special build requirements
- Provide usage instructions in README files

## Future Enhancements

- Enhanced template system with better import support
- Automatic example discovery and configuration
- Advanced dependency management features
- Integration with CI/CD pipelines
- Cross-platform build optimization
