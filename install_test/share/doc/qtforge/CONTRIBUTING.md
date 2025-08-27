# Contributing to QtPlugin

Thank you for your interest in contributing to QtPlugin! This document provides guidelines and information for contributors.

## 🎯 How to Contribute

### Types of Contributions

We welcome several types of contributions:

- **🐛 Bug Reports**: Report issues and bugs
- **✨ Feature Requests**: Suggest new features and improvements
- **📝 Documentation**: Improve documentation and examples
- **🔧 Code Contributions**: Fix bugs and implement features
- **🧪 Testing**: Add tests and improve test coverage
- **📦 Examples**: Create new examples and tutorials

## 🚀 Getting Started

### Prerequisites

- **Qt 6.0+** with Core, Network, Widgets, Test modules
- **C++20 compatible compiler** (GCC 10+, Clang 12+, MSVC 2019+)
- **CMake 3.21+**
- **Git** for version control

### Development Setup

1. **Fork and Clone**
   ```bash
   git clone https://github.com/YourOrg/QtForge.git
   cd QtForge
   ```

2. **Create Development Branch**
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b bugfix/issue-number
   ```

3. **Build and Test**
   ```bash
   mkdir build && cd build
   cmake .. -DQTPLUGIN_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
   cmake --build .
   ctest --output-on-failure
   ```

4. **Verify Everything Works**
   ```bash
   # Run all tests
   ctest -V
   
   # Run specific test suite
   ./tests/test_plugin_manager_comprehensive
   
   # Check examples
   cd ../examples/basic_plugin
   mkdir build && cd build
   cmake .. && cmake --build .
   ./test_basic_plugin
   ```

## 📋 Development Guidelines

### Code Style

#### C++ Style Guidelines

```cpp
// Use modern C++20 features
class PluginManager {
public:
    // Use expected<T,E> for error handling
    expected<std::string, PluginError> load_plugin(const std::string& path);
    
    // Use smart pointers for memory management
    std::shared_ptr<IPlugin> get_plugin(const std::string& id);
    
    // Use concepts for type validation
    template<PluginType T>
    expected<void, PluginError> register_plugin();
    
private:
    // Use RAII for resource management
    std::unique_ptr<PluginLoader> m_loader;
    
    // Use proper synchronization
    mutable std::shared_mutex m_plugins_mutex;
    std::map<std::string, std::shared_ptr<IPlugin>> m_plugins;
};
```

#### Naming Conventions

- **Classes**: `PascalCase` (e.g., `PluginManager`, `MessageBus`)
- **Functions/Methods**: `snake_case` (e.g., `load_plugin`, `get_metadata`)
- **Variables**: `snake_case` (e.g., `plugin_id`, `error_message`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_PLUGINS`, `DEFAULT_TIMEOUT`)
- **Member Variables**: `m_` prefix (e.g., `m_plugins`, `m_state`)
- **Private Members**: `m_` prefix (e.g., `m_loader`, `m_configuration`)

#### File Organization

```
include/qtplugin/
├── core/                   # Core plugin system
│   ├── plugin_interface.hpp
│   ├── plugin_manager.hpp
│   └── plugin_loader.hpp
├── managers/               # Resource managers
│   ├── configuration_manager.hpp
│   ├── logging_manager.hpp
│   └── resource_manager.hpp
├── communication/          # Inter-plugin communication
│   ├── message_bus.hpp
│   └── message_types.hpp
├── utils/                  # Utilities
│   ├── error_handling.hpp
│   ├── version.hpp
│   └── concepts.hpp
└── qtplugin.hpp           # Main header
```

### Error Handling

Always use the `expected<T,E>` pattern:

```cpp
// Good: Use expected<T,E> for fallible operations
expected<std::shared_ptr<IPlugin>, PluginError> load_plugin(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return make_unexpected(PluginError{
            PluginErrorCode::FileNotFound,
            "Plugin file not found: " + path
        });
    }
    
    try {
        auto plugin = create_plugin(path);
        return plugin;
    }
    catch (const std::exception& e) {
        return make_unexpected(PluginError{
            PluginErrorCode::LoadFailed,
            "Failed to load plugin: " + std::string(e.what())
        });
    }
}

// Good: Chain operations with monadic interface
auto result = load_plugin(path)
    .and_then([](auto plugin) { return plugin->initialize(); })
    .and_then([](auto) { return configure_plugin(); });

// Bad: Don't use exceptions for control flow
void bad_load_plugin(const std::string& path) {
    if (!exists(path)) {
        throw std::runtime_error("File not found");  // Don't do this
    }
}
```

### Memory Management

Follow RAII principles strictly:

```cpp
// Good: Use smart pointers and RAII
class PluginManager {
    std::unique_ptr<PluginLoader> m_loader;
    std::map<std::string, std::shared_ptr<IPlugin>> m_plugins;
    
public:
    PluginManager() : m_loader(std::make_unique<PluginLoader>()) {}
    
    ~PluginManager() {
        // Automatic cleanup through RAII
        shutdown_all_plugins();
    }
};

// Good: Use RAII for resource management
class ResourceHandle {
    std::string m_resource_id;
    ResourceManager* m_manager;
    
public:
    ResourceHandle(std::string id, ResourceManager* mgr)
        : m_resource_id(std::move(id)), m_manager(mgr) {}
    
    ~ResourceHandle() {
        if (m_manager) {
            m_manager->release_resource(m_resource_id);
        }
    }
    
    // Move-only semantics
    ResourceHandle(const ResourceHandle&) = delete;
    ResourceHandle& operator=(const ResourceHandle&) = delete;
    ResourceHandle(ResourceHandle&&) = default;
    ResourceHandle& operator=(ResourceHandle&&) = default;
};
```

### Thread Safety

Design for concurrent access:

```cpp
// Good: Use proper synchronization
class ThreadSafePluginManager {
    mutable std::shared_mutex m_mutex;
    std::map<std::string, std::shared_ptr<IPlugin>> m_plugins;
    
public:
    // Read operations use shared lock
    std::shared_ptr<IPlugin> get_plugin(const std::string& id) const {
        std::shared_lock lock(m_mutex);
        auto it = m_plugins.find(id);
        return (it != m_plugins.end()) ? it->second : nullptr;
    }
    
    // Write operations use exclusive lock
    expected<void, PluginError> add_plugin(const std::string& id, std::shared_ptr<IPlugin> plugin) {
        std::unique_lock lock(m_mutex);
        m_plugins[id] = std::move(plugin);
        return {};
    }
};

// Good: Use atomic operations for simple counters
class Statistics {
    std::atomic<size_t> m_plugins_loaded{0};
    std::atomic<size_t> m_commands_executed{0};
    
public:
    void increment_plugins_loaded() { ++m_plugins_loaded; }
    size_t get_plugins_loaded() const { return m_plugins_loaded.load(); }
};
```

## 🧪 Testing Guidelines

### Test Requirements

All contributions must include appropriate tests:

- **Unit Tests**: Test individual components in isolation
- **Integration Tests**: Test component interactions
- **Error Path Tests**: Test error handling and recovery
- **Performance Tests**: Validate performance requirements

### Writing Tests

```cpp
// Good: Comprehensive test with setup and cleanup
class TestPluginManager : public QObject {
    Q_OBJECT
    
private slots:
    void initTestCase() {
        // Global test setup
        m_temp_dir = std::make_unique<QTemporaryDir>();
        QVERIFY(m_temp_dir->isValid());
    }
    
    void init() {
        // Per-test setup
        m_manager = PluginManager::create();
        QVERIFY(m_manager != nullptr);
    }
    
    void testPluginLoading() {
        // Create test plugin
        QString plugin_path = createTestPlugin();
        
        // Test loading
        auto result = m_manager->load_plugin(plugin_path.toStdString());
        QVERIFY(result.has_value());
        
        QString plugin_id = QString::fromStdString(result.value());
        QVERIFY(!plugin_id.isEmpty());
        
        // Verify plugin is loaded
        auto plugin = m_manager->get_plugin(plugin_id.toStdString());
        QVERIFY(plugin != nullptr);
        QCOMPARE(plugin->state(), qtplugin::PluginState::Running);
    }
    
    void testErrorHandling() {
        // Test loading non-existent plugin
        auto result = m_manager->load_plugin("nonexistent.qtplugin");
        QVERIFY(!result.has_value());
        QCOMPARE(result.error().code, qtplugin::PluginErrorCode::FileNotFound);
    }
    
    void cleanup() {
        // Per-test cleanup
        m_manager.reset();
    }
    
    void cleanupTestCase() {
        // Global test cleanup
        m_temp_dir.reset();
    }
    
private:
    std::unique_ptr<PluginManager> m_manager;
    std::unique_ptr<QTemporaryDir> m_temp_dir;
    
    QString createTestPlugin() {
        // Helper to create test plugin files
        // Implementation details...
    }
};

QTEST_MAIN(TestPluginManager)
#include "test_plugin_manager.moc"
```

### Running Tests

```bash
# Run all tests
ctest --output-on-failure

# Run specific test with verbose output
ctest -R "PluginManager" -V

# Run tests with memory checking (if available)
ctest -T memcheck

# Run performance tests
ctest -R "Performance" --timeout 300
```

## 📝 Documentation Guidelines

### Code Documentation

Use clear, comprehensive comments:

```cpp
/**
 * @brief Loads a plugin from the specified file path
 * 
 * This method loads a plugin from a .qtplugin file, validates its metadata,
 * resolves dependencies, and initializes the plugin if requested.
 * 
 * @param path Absolute or relative path to the plugin file
 * @param options Loading options including initialization and security settings
 * @return Expected containing plugin ID on success, or PluginError on failure
 * 
 * @note This method is thread-safe and can be called concurrently
 * @warning The plugin file must be readable and have valid metadata
 * 
 * @see unload_plugin(), get_plugin(), PluginLoadOptions
 * 
 * Example usage:
 * @code
 * auto manager = PluginManager::create();
 * auto result = manager->load_plugin("plugins/example.qtplugin");
 * if (result.has_value()) {
 *     std::string plugin_id = result.value();
 *     auto plugin = manager->get_plugin(plugin_id);
 *     // Use plugin...
 * }
 * @endcode
 */
expected<std::string, PluginError> load_plugin(
    const std::string& path,
    const PluginLoadOptions& options = {}
);
```

### API Documentation

- Document all public APIs with Doxygen comments
- Include usage examples for complex APIs
- Document error conditions and return values
- Provide cross-references to related functions

### User Documentation

- Update README.md for user-facing changes
- Add examples for new features
- Update migration guides for breaking changes
- Include troubleshooting information

## 🔄 Pull Request Process

### Before Submitting

1. **Ensure Tests Pass**
   ```bash
   ctest --output-on-failure
   ```

2. **Check Code Style**
   ```bash
   # Use clang-format if available
   find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i
   ```

3. **Update Documentation**
   - Update API documentation
   - Add examples if needed
   - Update CHANGELOG.md

4. **Verify Examples Work**
   ```bash
   cd examples/basic_plugin
   mkdir build && cd build
   cmake .. && cmake --build .
   ./test_basic_plugin
   ```

### Pull Request Template

```markdown
## Description
Brief description of changes and motivation.

## Type of Change
- [ ] Bug fix (non-breaking change that fixes an issue)
- [ ] New feature (non-breaking change that adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update

## Testing
- [ ] All existing tests pass
- [ ] New tests added for new functionality
- [ ] Manual testing completed

## Checklist
- [ ] Code follows project style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] Examples updated if needed
- [ ] CHANGELOG.md updated
```

### Review Process

1. **Automated Checks**: CI/CD pipeline runs tests and checks
2. **Code Review**: Maintainers review code quality and design
3. **Testing**: Verify functionality and performance
4. **Documentation**: Check documentation completeness
5. **Approval**: Maintainer approval required for merge

## 🐛 Bug Reports

### Bug Report Template

```markdown
## Bug Description
Clear and concise description of the bug.

## Steps to Reproduce
1. Step one
2. Step two
3. Step three

## Expected Behavior
What you expected to happen.

## Actual Behavior
What actually happened.

## Environment
- OS: [e.g., Windows 11, Ubuntu 22.04, macOS 13]
- Qt Version: [e.g., 6.5.0]
- Compiler: [e.g., GCC 11.2, MSVC 2022]
- QtPlugin Version: [e.g., 3.0.0]

## Additional Context
Any other context about the problem.

## Possible Solution
If you have ideas for fixing the issue.
```

## ✨ Feature Requests

### Feature Request Template

```markdown
## Feature Description
Clear and concise description of the feature.

## Motivation
Why is this feature needed? What problem does it solve?

## Proposed Solution
Detailed description of the proposed implementation.

## Alternatives Considered
Other approaches you've considered.

## Additional Context
Any other context or screenshots about the feature request.
```

## 📞 Getting Help

### Communication Channels

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: General questions and discussions
- **Documentation**: Comprehensive guides and API reference

### Response Times

- **Bug Reports**: 1-3 business days
- **Feature Requests**: 1-7 business days
- **Pull Requests**: 1-5 business days

## 📄 License

By contributing to QtPlugin, you agree that your contributions will be licensed under the MIT License.

## 🙏 Recognition

Contributors will be recognized in:
- CONTRIBUTORS.md file
- Release notes for significant contributions
- GitHub contributor statistics

Thank you for contributing to QtPlugin! 🚀
