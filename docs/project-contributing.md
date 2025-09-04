# Contributing to QtForge

Thank you for your interest in contributing to QtForge! This document provides guidelines and information for contributors.

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
   git clone https://github.com/AstroAir/QtForge.git
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
   cmake .. -DQTFORGE_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
   cmake --build .
   ctest --output-on-failure
   ```

4. **Verify Everything Works**
   ```bash
   # Run all tests
   ctest --verbose
   
   # Run examples
   ./examples/basic_plugin_example
   ./examples/service_plugin_example
   ```

## 📋 Development Guidelines

### Code Style

We follow modern C++ best practices:

- **C++20 Standards**: Use modern C++ features appropriately
- **Qt Conventions**: Follow Qt naming and coding conventions
- **RAII**: Use Resource Acquisition Is Initialization pattern
- **Smart Pointers**: Prefer smart pointers over raw pointers
- **Const Correctness**: Use const wherever possible

### Code Formatting

```cpp
// Use Qt-style naming
class PluginManager : public QObject {
    Q_OBJECT
    
public:
    explicit PluginManager(QObject* parent = nullptr);
    
    // Use camelCase for methods
    bool loadPlugin(const QString& pluginPath);
    QList<PluginInfo> getAvailablePlugins() const;
    
private slots:
    void onPluginLoaded(const QString& pluginId);
    
private:
    // Use m_ prefix for member variables
    QHash<QString, PluginInterface*> m_loadedPlugins;
    QStringList m_searchPaths;
};
```

### Documentation

- **Header Comments**: Document all public APIs
- **Doxygen Style**: Use Doxygen-compatible comments
- **Examples**: Include usage examples in documentation
- **Markdown**: Use Markdown for documentation files

```cpp
/**
 * @brief Loads a plugin from the specified path
 * @param pluginPath Path to the plugin file
 * @return true if plugin loaded successfully, false otherwise
 * 
 * @code
 * PluginManager manager;
 * if (manager.loadPlugin("/path/to/plugin.dll")) {
 *     qDebug() << "Plugin loaded successfully";
 * }
 * @endcode
 */
bool loadPlugin(const QString& pluginPath);
```

## 🧪 Testing

### Test Requirements

- **Unit Tests**: All new code must have unit tests
- **Integration Tests**: Test plugin interactions
- **Performance Tests**: For performance-critical code
- **Platform Tests**: Test on multiple platforms when possible

### Writing Tests

```cpp
class PluginManagerTest : public QObject {
    Q_OBJECT
    
private slots:
    void initTestCase();
    void cleanupTestCase();
    
    void testLoadPlugin();
    void testUnloadPlugin();
    void testPluginDependencies();
    
private:
    PluginManager* m_manager;
};

void PluginManagerTest::testLoadPlugin() {
    // Arrange
    QString pluginPath = "test_plugin.dll";
    
    // Act
    bool result = m_manager->loadPlugin(pluginPath);
    
    // Assert
    QVERIFY(result);
    QVERIFY(m_manager->isPluginLoaded("test_plugin"));
}
```

## 📝 Pull Request Process

### Before Submitting

1. **Update Documentation**: Update relevant documentation
2. **Add Tests**: Ensure new code has appropriate tests
3. **Run Tests**: All tests must pass
4. **Check Style**: Follow coding standards
5. **Update Changelog**: Add entry to CHANGELOG.md

### Pull Request Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Documentation update
- [ ] Performance improvement
- [ ] Code refactoring

## Testing
- [ ] Unit tests added/updated
- [ ] Integration tests added/updated
- [ ] All tests pass
- [ ] Manual testing completed

## Checklist
- [ ] Code follows style guidelines
- [ ] Self-review completed
- [ ] Documentation updated
- [ ] Changelog updated
```

### Review Process

1. **Automated Checks**: CI/CD pipeline runs automatically
2. **Code Review**: Maintainers review code and provide feedback
3. **Address Feedback**: Make requested changes
4. **Final Approval**: Maintainer approves and merges

## 🐛 Bug Reports

### Bug Report Template

```markdown
**Bug Description**
Clear description of the bug

**Steps to Reproduce**
1. Step one
2. Step two
3. Step three

**Expected Behavior**
What should happen

**Actual Behavior**
What actually happens

**Environment**
- OS: [e.g., Windows 10, Ubuntu 20.04]
- Qt Version: [e.g., 6.5.0]
- Compiler: [e.g., MSVC 2019, GCC 11]
- QtForge Version: [e.g., 1.0.0]

**Additional Context**
Any other relevant information
```

## ✨ Feature Requests

### Feature Request Template

```markdown
**Feature Description**
Clear description of the proposed feature

**Use Case**
Why is this feature needed?

**Proposed Solution**
How should this feature work?

**Alternatives Considered**
Other approaches considered

**Additional Context**
Any other relevant information
```

## 📚 Documentation

### Documentation Standards

- **Clear and Concise**: Write clear, easy-to-understand documentation
- **Examples**: Include practical examples
- **Cross-References**: Link to related documentation
- **Up-to-Date**: Keep documentation current with code changes

### Building Documentation

```bash
# Install MkDocs
pip install mkdocs mkdocs-material

# Serve documentation locally
mkdocs serve

# Build documentation
mkdocs build
```

## 🏆 Recognition

Contributors are recognized in:

- **CONTRIBUTORS.md**: List of all contributors
- **Release Notes**: Major contributions mentioned in releases
- **GitHub**: Contributor statistics and graphs

## 📞 Getting Help

- **GitHub Issues**: For bugs and feature requests
- **GitHub Discussions**: For questions and general discussion
- **Documentation**: Comprehensive guides and API reference

## 📄 License

By contributing to QtForge, you agree that your contributions will be licensed under the MIT License.

---

Thank you for contributing to QtForge! 🚀
