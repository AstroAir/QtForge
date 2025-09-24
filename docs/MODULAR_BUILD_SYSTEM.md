# QtForge Modular Build System Documentation

## Overview

The QtForge modular build system is a comprehensive, maintainable, and scalable xmake-based build solution that supports multiple toolchains (MSVC, MinGW64) and provides a rich set of features for building Qt-based applications and plugins.

## Architecture

### Core Modules

The build system is organized into several specialized modules:

#### 1. **common.lua** - Core Utilities

- Provides common utility functions used across all modules
- Handles toolchain detection and configuration
- Contains shared constants and helper functions

#### 2. **dependencies.lua** - Dependency Management

- Manages Qt6 and other external dependencies
- Provides toolchain-specific dependency resolution
- Handles automatic dependency validation and setup

#### 3. **example_template.lua** - Example Templates

- Standardized templates for creating examples
- Toolchain-aware configuration
- Validation and error handling

#### 4. **plugin_template.lua** - Plugin Templates

- Specialized templates for plugin development
- Plugin-specific configuration options
- Integration with QtForge plugin system

#### 5. **registry.lua** - Example Registry

- Centralized example discovery and management
- Automatic example registration
- Metadata and validation tracking

#### 6. **validator.lua** - Validation Framework

- Comprehensive validation for build configurations
- Dual toolchain testing support
- Detailed error reporting and suggestions

#### 7. **factory.lua** - Template Factory

- Advanced template creation and management
- Template inheritance system
- Category-based example organization

#### 8. **toolchain_selector.lua** - Toolchain Management

- Intelligent toolchain detection and selection
- Support for MSVC and MinGW64
- User preference handling

#### 9. **msys2_mingw64.lua** - MSYS2 Integration

- MSYS2 MinGW64 environment detection
- Package manager integration
- Environment validation

#### 10. **qt6_detector.lua** - Qt6 Detection

- Cross-toolchain Qt6 detection
- Tool configuration (MOC, UIC, RCC)
- Component validation

#### 11. **optimization.lua** - Build Optimization

- Compiler cache support (ccache/sccache)
- Link-time optimization (LTO)
- Parallel build configuration
- Precompiled headers

#### 12. **logger.lua** - Error Handling & Logging

- Comprehensive logging system
- Error handling utilities
- Performance monitoring
- Memory usage tracking

## Usage Guide

### Basic Example Creation

To create a new example using the modular system:

```lua
-- In your example's xmake.lua
local example_template = import("cmake.xmake.example_template")

target("my_example")
    example_template.create_validated_example("my_example", {
        qt_features = "basic",
        app_type = "console",
        sources = {"src/main.cpp"},
        headers = {"include/my_example.h"}
    })
```

### Template Inheritance

Use category-based templates for consistent configuration:

```lua
local factory = import("cmake.xmake.factory")

target("communication_example")
    factory.create_inherited_example("communication_example", "communication", {
        sources = {"src/main.cpp", "src/network_handler.cpp"}
    })
```

Available categories:

- `fundamentals` - Basic examples
- `communication` - Network/IPC examples
- `ui` - GUI examples
- `specialized` - Advanced examples
- `plugins` - Plugin examples

### Toolchain Selection

The system automatically detects available toolchains:

```bash
# Automatic detection
xmake f -c

# Explicit toolchain selection
xmake f -c --toolchain=msvc
xmake f -c --toolchain=mingw64

# Prefer MinGW64 when available
xmake f -c --prefer_mingw64=true

# Custom MSYS2 root
xmake f -c --msys2_root=D:/msys64
```

### Build Optimization

Enable various optimizations:

```bash
# Enable link-time optimization
xmake f -c --lto=true

# Enable compiler cache
xmake f -c --ccache=true

# Set parallel jobs
xmake f -c --jobs=8
```

### Validation and Testing

The system provides comprehensive validation:

```lua
local validator = import("cmake.xmake.validator")

-- Validate dual toolchain support
local report = validator.validate_dual_toolchain_support()
print("Validation result:", report.overall_status)
```

## Configuration Options

### Global Configuration

Set in main `xmake.lua`:

```lua
-- Toolchain preferences
set_config("prefer_mingw64", false)
set_config("msys2_root", "D:/msys64")

-- Build optimizations
set_config("lto", false)
set_config("ccache", true)
set_config("jobs", 4)

-- Feature toggles
set_config("python_bindings", true)
set_config("lua_bindings", true)
set_config("tests", true)
set_config("benchmarks", false)
```

### Example Configuration

Configure individual examples:

```lua
{
    -- Basic settings
    qt_features = "basic|network|ui|comprehensive",
    app_type = "console|gui",
    kind = "binary|shared|static",

    -- Sources and headers
    sources = {"src/main.cpp"},
    headers = {"include/example.h"},

    -- Dependencies
    packages = {"qt6core", "qt6network"},

    -- Build settings
    debug_postfix = true,
    install = false,

    -- Compiler settings
    defines = {"EXAMPLE_DEFINE"},
    cxxflags = {"-Wall"},
    ldflags = {"-static"}
}
```

## Advanced Features

### Custom Validation Rules

Add custom validation to your examples:

```lua
local validator = import("cmake.xmake.validator")

validator.add_custom_rule("my_rule", function(config)
    if config.custom_feature and not config.required_dependency then
        return false, "Custom feature requires specific dependency"
    end
    return true
end)
```

### Performance Monitoring

Monitor build performance:

```lua
local logger = import("cmake.xmake.logger")

logger.start_timer("build_phase")
-- ... build operations ...
logger.end_timer("build_phase")

logger.log_memory_usage("after_compilation")
```

### Error Handling

Robust error handling throughout:

```lua
local logger = import("cmake.xmake.logger")

local success, result = logger.try_catch(
    function()
        -- Potentially failing operation
        return risky_operation()
    end,
    function(error)
        -- Error handler
        logger.error("Operation failed: " .. error)
        return default_value
    end,
    function()
        -- Cleanup
        cleanup_resources()
    end
)
```

## Troubleshooting

### Common Issues

1. **Import Errors**
   - Ensure modules are in `cmake/xmake/` directory
   - Use direct configuration instead of global imports

2. **Toolchain Detection Failures**
   - Check toolchain installation
   - Verify PATH environment variables
   - Use explicit toolchain selection

3. **Qt6 Package Issues**
   - Ensure Qt6 is properly installed
   - Check package manager integration
   - Verify component availability

4. **Build Failures**
   - Check validation reports
   - Review error logs in `build/qtforge.log`
   - Verify dependency resolution

### Debug Mode

Enable detailed logging:

```bash
xmake f -c --verbose
xmake -v
```

Set log level:

```lua
local logger = import("cmake.xmake.logger")
logger.set_level("DEBUG")
```

## Best Practices

1. **Use Template Inheritance** - Leverage category-based templates for consistency
2. **Validate Configurations** - Always validate example configurations
3. **Handle Errors Gracefully** - Use the error handling utilities
4. **Monitor Performance** - Use timing and memory monitoring
5. **Document Custom Rules** - Document any custom validation rules
6. **Test Both Toolchains** - Ensure compatibility with MSVC and MinGW64

## Extension Guide

### Adding New Modules

1. Create module in `cmake/xmake/`
2. Follow naming convention: `module_name.lua`
3. Export module table with functions
4. Add validation and error handling
5. Update documentation

### Adding New Template Categories

1. Add to `factory.base_templates`
2. Define category-specific defaults
3. Add description and validation
4. Test with examples

### Custom Toolchain Support

1. Add detection logic to `toolchain_selector.lua`
2. Update `common.lua` with toolchain-specific utilities
3. Add Qt6 detection support
4. Update validation framework

This modular build system provides a robust, maintainable foundation for QtForge development while supporting multiple toolchains and advanced build features.
