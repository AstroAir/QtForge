# Coding Standards

This document outlines the coding standards and style guidelines for contributing to QtForge.

## General Principles

1. **Consistency**: Follow existing code patterns
2. **Readability**: Write self-documenting code
3. **Maintainability**: Keep code modular and testable
4. **Modern C++**: Use contemporary C++ features appropriately
5. **Qt Integration**: Follow Qt conventions for Qt-related code

## C++ Style Guide

### Naming Conventions

#### Classes and Types

```cpp
// Use PascalCase for classes, structs, enums
class PluginManager {
public:
    enum class LoadingState {
        Unloaded,
        Loading,
        Loaded,
        Failed
    };
};

// Use PascalCase for type aliases
using PluginPtr = std::shared_ptr<Plugin>;
using PluginList = QList<PluginPtr>;
```

#### Functions and Methods

```cpp
// Use camelCase for functions and methods
class PluginLoader {
public:
    bool loadPlugin(const QString& path);
    void unloadPlugin(const QString& id);

private:
    bool validatePluginSignature(const QString& path);
    void updateRegistryEntry(const PluginInfo& info);
};
```

#### Variables

```cpp
// Use camelCase for variables
class PluginManager {
private:
    QString pluginDirectory;
    QList<Plugin*> loadedPlugins;
    bool isInitialized;

    // Member variables can optionally use m_ prefix
    QString m_pluginDirectory;
    QList<Plugin*> m_loadedPlugins;
    bool m_isInitialized;
};
```

#### Constants

```cpp
// Use UPPER_SNAKE_CASE for constants
namespace Constants {
    constexpr int MAX_PLUGINS = 100;
    constexpr const char* DEFAULT_PLUGIN_EXT = ".qtplugin";
    constexpr std::chrono::seconds PLUGIN_TIMEOUT{30};
}

// Or use inline namespace for scoped constants
namespace PluginConstants {
    inline constexpr int MaxPlugins = 100;
    inline constexpr const char* DefaultExtension = ".qtplugin";
}
```

### File Organization

#### Header Files (.hpp)

```cpp
#pragma once

// System includes
#include <memory>
#include <string>
#include <vector>

// Qt includes
#include <QString>
#include <QObject>
#include <QList>

// Third-party includes
#include <fmt/format.h>

// Project includes
#include "qtforge/core/plugin_interface.hpp"
#include "qtforge/core/plugin_info.hpp"

namespace QtForge {

class PluginManager : public QObject {
    Q_OBJECT

public:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager() override;

    // Public interface
    bool loadPlugin(const QString& path);
    void unloadPlugin(const QString& id);

signals:
    void pluginLoaded(const QString& id);
    void pluginUnloaded(const QString& id);

private:
    // Private implementation
    struct Impl;
    std::unique_ptr<Impl> d_ptr;
};

} // namespace QtForge
```

#### Source Files (.cpp)

```cpp
#include "plugin_manager.hpp"

// System includes
#include <algorithm>
#include <stdexcept>

// Qt includes
#include <QDebug>
#include <QFileInfo>
#include <QDir>

// Project includes
#include "qtforge/core/plugin_loader.hpp"
#include "qtforge/utils/logger.hpp"

namespace QtForge {

struct PluginManager::Impl {
    QList<std::shared_ptr<Plugin>> plugins;
    QString pluginDirectory;
    bool initialized = false;
};

PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
    , d_ptr(std::make_unique<Impl>())
{
    // Constructor implementation
}

PluginManager::~PluginManager() = default;

bool PluginManager::loadPlugin(const QString& path) {
    // Implementation
    return true;
}

} // namespace QtForge
```

### Code Formatting

#### Indentation and Spacing

```cpp
// Use 4 spaces for indentation (no tabs)
class Example {
public:
    void function() {
        if (condition) {
            doSomething();
        }
    }
};

// Space after control keywords
if (condition) {
    // code
}

while (condition) {
    // code
}

for (int i = 0; i < count; ++i) {
    // code
}

// Space around operators
int result = a + b * c;
bool valid = (x >= 0) && (x < max_value);
```

#### Braces and Line Breaks

```cpp
// Opening brace on same line for classes, functions
class PluginManager {
public:
    bool loadPlugin(const QString& path) {
        if (path.isEmpty()) {
            return false;
        }

        // Implementation
        return true;
    }
};

// Consistent brace style for control structures
if (condition) {
    doSomething();
} else {
    doSomethingElse();
}
```

### Modern C++ Features

#### Use Modern C++ Constructs

```cpp
// Prefer auto for type deduction
auto plugin = std::make_shared<Plugin>();
auto it = plugins.find(id);

// Use range-based for loops
for (const auto& plugin : loadedPlugins) {
    plugin->initialize();
}

// Use uniform initialization
PluginInfo info{
    .id = "example.plugin",
    .name = "Example Plugin",
    .version = Version{1, 0, 0}
};

// Use constexpr and const where possible
constexpr int DefaultTimeout = 5000;
const QString& getPluginName() const;
```

#### Smart Pointers

```cpp
// Prefer smart pointers over raw pointers
class PluginManager {
private:
    std::vector<std::unique_ptr<Plugin>> m_plugins;
    std::shared_ptr<PluginRegistry> m_registry;

public:
    // Return smart pointers from factory functions
    std::unique_ptr<Plugin> createPlugin(const QString& type);
    std::shared_ptr<PluginRegistry> getRegistry() const;
};
```

#### Error Handling

```cpp
// Use std::expected for error handling (C++23)
#include <expected>

std::expected<PluginPtr, LoadError> loadPlugin(const QString& path) {
    if (!QFileInfo::exists(path)) {
        return std::unexpected(LoadError::FileNotFound);
    }

    // Load plugin logic
    auto plugin = std::make_shared<Plugin>();
    return plugin;
}

// Or use Qt-style result types for Qt integration
class Result {
public:
    template<typename T>
    static Result<T> success(T&& value);
    static Result<void> error(const QString& message);
};
```

### Qt Integration

#### Qt Object Model

```cpp
// Use Q_OBJECT macro for QObject-derived classes
class PluginManager : public QObject {
    Q_OBJECT

public:
    explicit PluginManager(QObject* parent = nullptr);

signals:
    void pluginLoaded(const QString& id);
    void pluginFailed(const QString& id, const QString& error);

public slots:
    void loadPlugin(const QString& path);
    void unloadPlugin(const QString& id);

private slots:
    void onPluginStateChanged();
};
```

#### Qt Containers vs. STL

```cpp
// Prefer Qt containers for Qt integration
class PluginRegistry {
private:
    QHash<QString, PluginInfo> m_plugins;
    QStringList m_searchPaths;
    QList<Plugin*> m_loadedPlugins;

public:
    // Use STL containers for algorithms and modern C++ features
    std::vector<std::unique_ptr<Plugin>> createPluginChain();
    std::unordered_map<std::string, PluginMetrics> getMetrics();
};
```

### Documentation Standards

#### Header Documentation

```cpp
/**
 * @brief Manages plugin loading, unloading, and lifecycle
 *
 * The PluginManager class provides a centralized interface for managing
 * plugins within the QtForge framework. It handles plugin discovery,
 * loading, dependency resolution, and lifecycle management.
 *
 * @since 1.0.0
 * @thread-safe This class is thread-safe for all public methods
 */
class PluginManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructs a new PluginManager
     * @param parent Parent QObject for memory management
     */
    explicit PluginManager(QObject* parent = nullptr);

    /**
     * @brief Loads a plugin from the specified path
     * @param path Absolute path to the plugin file
     * @return true if the plugin was loaded successfully, false otherwise
     * @throws PluginLoadException if the plugin file is corrupted
     */
    bool loadPlugin(const QString& path);
};
```

#### Inline Comments

```cpp
bool PluginManager::loadPlugin(const QString& path) {
    // Validate input parameters
    if (path.isEmpty()) {
        qWarning() << "Plugin path cannot be empty";
        return false;
    }

    // Check if plugin is already loaded
    if (m_loadedPlugins.contains(path)) {
        qDebug() << "Plugin already loaded:" << path;
        return true;
    }

    // Load and validate plugin
    auto loader = std::make_unique<PluginLoader>();
    if (!loader->load(path)) {
        // Log specific error from loader
        qWarning() << "Failed to load plugin:" << loader->errorString();
        return false;
    }

    return true;
}
```

## Testing Standards

### Unit Test Structure

```cpp
#include <QtTest>
#include "qtforge/core/plugin_manager.hpp"

class PluginManagerTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testLoadValidPlugin();
    void testLoadInvalidPlugin();
    void testUnloadPlugin();

    void testLoadPlugin_data();
    void testLoadPlugin();

private:
    std::unique_ptr<PluginManager> m_manager;
    QTemporaryDir m_tempDir;
};

void PluginManagerTest::testLoadValidPlugin() {
    // Arrange
    const QString pluginPath = createTestPlugin();

    // Act
    bool result = m_manager->loadPlugin(pluginPath);

    // Assert
    QVERIFY(result);
    QCOMPARE(m_manager->loadedPluginCount(), 1);
}
```

### Test Data and Fixtures

```cpp
void PluginManagerTest::testLoadPlugin_data() {
    QTest::addColumn<QString>("pluginPath");
    QTest::addColumn<bool>("expectedResult");
    QTest::addColumn<QString>("expectedError");

    QTest::newRow("valid plugin")
        << ":/test/valid_plugin.qtplugin"
        << true
        << QString();

    QTest::newRow("invalid path")
        << "/nonexistent/plugin.qtplugin"
        << false
        << "File not found";
}
```

## Error Handling Guidelines

### Exception Safety

```cpp
// Provide strong exception guarantee
class PluginManager {
public:
    bool loadPlugin(const QString& path) noexcept {
        try {
            // Use RAII for resource management
            auto loader = std::make_unique<PluginLoader>();
            auto plugin = loader->load(path);

            // Atomic operation - commit changes only on success
            m_plugins.insert(plugin->id(), std::move(plugin));
            emit pluginLoaded(plugin->id());

            return true;
        } catch (const std::exception& e) {
            qWarning() << "Plugin load failed:" << e.what();
            return false;
        }
    }
};
```

### Error Reporting

```cpp
// Use structured error types
enum class PluginError {
    None,
    FileNotFound,
    InvalidFormat,
    MissingDependency,
    InitializationFailed,
    SecurityViolation
};

class PluginResult {
public:
    static PluginResult success(PluginPtr plugin);
    static PluginResult error(PluginError code, const QString& message);

    bool isSuccess() const { return m_error == PluginError::None; }
    PluginError error() const { return m_error; }
    QString errorString() const { return m_errorMessage; }
    PluginPtr plugin() const { return m_plugin; }

private:
    PluginError m_error;
    QString m_errorMessage;
    PluginPtr m_plugin;
};
```

## Performance Guidelines

### Memory Management

```cpp
// Use RAII for resource management
class PluginManager {
private:
    struct PluginEntry {
        std::unique_ptr<Plugin> plugin;
        std::unique_ptr<QLibrary> library;
        QDateTime loadTime;

        // Automatic cleanup on destruction
        ~PluginEntry() {
            if (plugin) {
                plugin->shutdown();
            }
            // library automatically unloaded
        }
    };

    QHash<QString, std::unique_ptr<PluginEntry>> m_plugins;
};
```

### Avoid Premature Optimization

```cpp
// Profile first, then optimize
class PluginLoader {
public:
    PluginPtr load(const QString& path) {
        // Simple, readable implementation first
        QFileInfo info(path);
        if (!info.exists()) {
            return nullptr;
        }

        // Optimize only if profiling shows bottleneck
        auto library = std::make_unique<QLibrary>(path);
        if (!library->load()) {
            return nullptr;
        }

        return createPlugin(library.get());
    }
};
```

## Code Review Guidelines

### Pre-submission Checklist

- [ ] Code follows naming conventions
- [ ] All public APIs are documented
- [ ] Unit tests cover new functionality
- [ ] No compiler warnings
- [ ] Code formatted with clang-format
- [ ] Follows RAII principles
- [ ] Error handling is appropriate
- [ ] Thread safety considered

### Review Focus Areas

1. **API Design**: Is the interface intuitive and consistent?
2. **Error Handling**: Are edge cases handled appropriately?
3. **Performance**: Are there obvious performance issues?
4. **Security**: Could this code introduce vulnerabilities?
5. **Testing**: Is the code adequately tested?
6. **Documentation**: Is the code self-documenting?

## Tools and Automation

### Static Analysis

```bash
# Use clang-tidy for static analysis
clang-tidy src/**/*.cpp -- -std=c++20

# Use cppcheck for additional checks
cppcheck --enable=all --std=c++20 src/
```

### Formatting

```bash
# Format code with clang-format
find src/ -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i

# Check formatting
clang-format --dry-run --Werror src/**/*.{cpp,hpp}
```

### Pre-commit Hooks

```yaml
# .pre-commit-config.yaml
repos:
  - repo: local
    hooks:
      - id: clang-format
        name: clang-format
        entry: clang-format
        language: system
        files: \.(cpp|hpp)$
        args: [-i]

      - id: clang-tidy
        name: clang-tidy
        entry: clang-tidy
        language: system
        files: \.(cpp)$
```

This coding standards document ensures consistent, maintainable, and high-quality code across the QtForge project.
