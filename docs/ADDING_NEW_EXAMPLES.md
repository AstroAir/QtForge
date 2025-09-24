# Adding New Examples to QtForge

This guide provides step-by-step instructions for adding new examples to the QtForge modular build system.

## Quick Start

### 1. Choose Example Category

First, determine which category your example belongs to:

- **fundamentals** - Basic QtForge concepts
- **communication** - Inter-plugin communication
- **ui** - User interface examples
- **specialized** - Advanced use cases
- **plugins** - Plugin development

### 2. Create Example Directory

Create your example directory following the naming convention:

```bash
# For fundamentals category
mkdir -p examples/01-fundamentals/my-new-example

# For communication category
mkdir -p examples/02-communication/my-new-example

# For UI category
mkdir -p examples/03-ui/my-new-example

# For specialized category
mkdir -p examples/04-specialized/my-new-example
```

### 3. Create Basic Structure

```bash
cd examples/01-fundamentals/my-new-example

# Create source directories
mkdir -p src include

# Create basic files
touch src/main.cpp
touch include/my_example.h
touch xmake.lua
touch README.md
```

### 4. Write xmake.lua Configuration

Create a simple `xmake.lua` using template inheritance:

```lua
-- examples/01-fundamentals/my-new-example/xmake.lua

-- Import factory for template inheritance
local factory = import("cmake.xmake.factory")

-- Create example with category-based template
target("my_new_example")
    factory.create_inherited_example("my_new_example", "fundamentals", {
        -- Custom configuration
        sources = {
            "src/main.cpp",
            "src/my_example.cpp"
        },
        headers = {
            "include/my_example.h"
        },
        defines = {
            "MY_EXAMPLE_VERSION=\"1.0.0\""
        }
    })
```

## Detailed Configuration Options

### Basic Configuration

```lua
{
    -- Source files
    sources = {"src/main.cpp", "src/example.cpp"},
    headers = {"include/example.h"},

    -- Qt features required
    qt_features = "basic",  -- basic|network|ui|comprehensive

    -- Application type
    app_type = "console",   -- console|gui

    -- Build type
    kind = "binary",        -- binary|shared|static

    -- Additional packages
    packages = {"qt6network", "qt6widgets"},

    -- Preprocessor defines
    defines = {"EXAMPLE_DEFINE", "VERSION=\"1.0\""},

    -- Compiler flags
    cxxflags = {"-Wall", "-Wextra"},

    -- Linker flags
    ldflags = {"-static"},

    -- Installation
    install = false,        -- true to install example

    -- Debug postfix
    debug_postfix = true    -- Adds 'd' suffix in debug builds
}
```

### Advanced Configuration

```lua
target("advanced_example")
    factory.create_inherited_example("advanced_example", "communication", {
        sources = {
            "src/main.cpp",
            "src/network_handler.cpp",
            "src/message_processor.cpp"
        },
        headers = {
            "include/network_handler.h",
            "include/message_processor.h"
        },

        -- Qt features
        qt_features = "network",

        -- Additional packages
        packages = {
            "qt6network",
            "qt6concurrent"
        },

        -- Custom defines
        defines = {
            "NETWORK_ENABLED",
            "MAX_CONNECTIONS=100"
        },

        -- Toolchain-specific settings
        toolchain_config = {
            msvc = {
                cxxflags = {"/W4"},
                defines = {"MSVC_BUILD"}
            },
            mingw64 = {
                cxxflags = {"-Wall", "-Wextra"},
                defines = {"MINGW_BUILD"}
            }
        },

        -- Custom validation
        validation_rules = {
            function(config)
                if config.qt_features ~= "network" then
                    return false, "This example requires network features"
                end
                return true
            end
        }
    })
```

## Category-Specific Templates

### Fundamentals Examples

```lua
-- Inherits from fundamentals base template
target("basic_example")
    factory.create_inherited_example("basic_example", "fundamentals", {
        sources = {"src/main.cpp"},
        -- Automatically gets:
        -- - qt_features = "basic"
        -- - app_type = "console"
        -- - defines = {"QTFORGE_EXAMPLE_FUNDAMENTALS"}
    })
```

### Communication Examples

```lua
-- Inherits from communication base template
target("ipc_example")
    factory.create_inherited_example("ipc_example", "communication", {
        sources = {"src/main.cpp", "src/ipc_handler.cpp"},
        -- Automatically gets:
        -- - qt_features = "network"
        -- - packages = {"qt6network"}
        -- - defines = {"QTFORGE_EXAMPLE_COMMUNICATION", "QTFORGE_NETWORK_ENABLED"}
    })
```

### UI Examples

```lua
-- Inherits from ui base template
target("widget_example")
    factory.create_inherited_example("widget_example", "ui", {
        sources = {"src/main.cpp", "src/main_window.cpp"},
        headers = {"include/main_window.h"},
        -- Automatically gets:
        -- - qt_features = "ui"
        -- - app_type = "gui"
        -- - packages = {"qt6widgets", "qt6gui"}
        -- - defines = {"QTFORGE_EXAMPLE_UI", "QTFORGE_UI_ENABLED"}
    })
```

### Plugin Examples

```lua
-- Inherits from plugins base template
target("service_plugin")
    factory.create_inherited_plugin("service_plugin", "plugins", {
        sources = {"src/service_plugin.cpp"},
        headers = {"include/service_plugin.h"},
        plugin_type = "service",
        -- Automatically gets:
        -- - kind = "shared"
        -- - defines = {"QTFORGE_PLUGIN_EXAMPLE"}
        -- - install = true
    })
```

## Step-by-Step Example Creation

### Example: Creating a Network Monitor

1. **Create Directory Structure**

```bash
mkdir -p examples/04-specialized/network-monitor/{src,include}
cd examples/04-specialized/network-monitor
```

2. **Create Source Files**

`src/main.cpp`:

```cpp
#include <QtCore/QCoreApplication>
#include <QtNetwork/QNetworkAccessManager>
#include <QtCore/QDebug>
#include "include/network_monitor.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    NetworkMonitor monitor;
    monitor.start();

    return app.exec();
}
```

`include/network_monitor.h`:

```cpp
#pragma once
#include <QtCore/QObject>
#include <QtNetwork/QNetworkAccessManager>

class NetworkMonitor : public QObject
{
    Q_OBJECT

public:
    explicit NetworkMonitor(QObject *parent = nullptr);
    void start();

private slots:
    void onRequestFinished();

private:
    QNetworkAccessManager *m_manager;
};
```

`src/network_monitor.cpp`:

```cpp
#include "include/network_monitor.h"
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QUrl>
#include <QtCore/QDebug>

NetworkMonitor::NetworkMonitor(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &NetworkMonitor::onRequestFinished);
}

void NetworkMonitor::start()
{
    qDebug() << "Starting network monitor...";
    QNetworkRequest request(QUrl("https://httpbin.org/get"));
    m_manager->get(request);
}

void NetworkMonitor::onRequestFinished()
{
    qDebug() << "Network request completed";
}
```

3. **Create xmake.lua**

```lua
-- examples/04-specialized/network-monitor/xmake.lua

local factory = import("cmake.xmake.factory")

target("network_monitor")
    factory.create_inherited_example("network_monitor", "specialized", {
        sources = {
            "src/main.cpp",
            "src/network_monitor.cpp"
        },
        headers = {
            "include/network_monitor.h"
        },

        -- Override specialized template for network features
        qt_features = "network",
        packages = {"qt6network"},

        defines = {
            "NETWORK_MONITOR_VERSION=\"1.0.0\"",
            "QTFORGE_NETWORK_ENABLED"
        },

        -- Custom validation
        validation_rules = {
            function(config)
                if not config.packages or not table.contains(config.packages, "qt6network") then
                    return false, "Network monitor requires qt6network package"
                end
                return true
            end
        }
    })
```

4. **Create Documentation**

`README.md`:

````markdown
# Network Monitor Example

This example demonstrates network monitoring capabilities using Qt's networking features.

## Features

- HTTP request monitoring
- Network status checking
- Asynchronous network operations

## Building

```bash
cd examples/04-specialized/network-monitor
xmake f -c
xmake
```
````

## Running

```bash
xmake run network_monitor
```

## Requirements

- Qt6 Network module
- Internet connection for testing

````

5. **Test the Example**
```bash
cd examples/04-specialized/network-monitor
xmake f -c
xmake
xmake run network_monitor
````

## Validation and Testing

### Automatic Validation

The modular system automatically validates your example:

```bash
# Validate specific example
cd examples/04-specialized/network-monitor
xmake f -c --validate

# Run system-wide validation
lua scripts/validate_build_system.lua
```

### Custom Validation Rules

Add custom validation to your example:

```lua
target("my_example")
    factory.create_inherited_example("my_example", "fundamentals", {
        -- ... configuration ...

        validation_rules = {
            function(config)
                -- Custom validation logic
                if config.custom_feature and not config.required_dependency then
                    return false, "Custom feature requires specific dependency"
                end
                return true
            end,

            function(config)
                -- Another validation rule
                if #config.sources == 0 then
                    return false, "Example must have at least one source file"
                end
                return true
            end
        }
    })
```

## Best Practices

1. **Use Template Inheritance** - Always use category-based templates
2. **Follow Naming Conventions** - Use kebab-case for directories and snake_case for targets
3. **Add Documentation** - Include README.md with build and usage instructions
4. **Validate Configuration** - Add custom validation rules when needed
5. **Test Both Toolchains** - Ensure compatibility with MSVC and MinGW64
6. **Keep It Simple** - Start with basic configuration and add complexity as needed

## Troubleshooting

### Common Issues

1. **Import Errors**
   - Ensure you're using `local factory = import("cmake.xmake.factory")`
   - Check that cmake/xmake modules are available

2. **Build Failures**
   - Verify Qt6 packages are correctly specified
   - Check source file paths are correct
   - Ensure headers are properly included

3. **Validation Errors**
   - Review validation output for specific issues
   - Check that required dependencies are specified
   - Verify category template requirements are met

### Getting Help

- Check the validation output: `xmake f -c --validate`
- Review the build logs: `xmake -v`
- Run system validation: `lua scripts/validate_build_system.lua`
- Check the modular system documentation: `docs/MODULAR_BUILD_SYSTEM.md`

This guide should help you create new examples efficiently using the QtForge modular build system!
