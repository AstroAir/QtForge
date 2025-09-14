# Troubleshooting Guide

This comprehensive guide helps you diagnose and resolve common issues with QtForge v3.2.0, including new multilingual plugin support and enhanced security features. For quick answers to common questions, see the [FAQ](../appendix/faq.md).

## 🚨 Emergency Quick Fixes

### Plugin Won't Load
```bash
# 1. Check if plugin file exists and has correct permissions
ls -la /path/to/plugin.dll
chmod 755 /path/to/plugin.dll  # Linux/macOS

# 2. Verify plugin dependencies
ldd /path/to/plugin.dll  # Linux
otool -L /path/to/plugin.dll  # macOS
dumpbin /dependents plugin.dll  # Windows

# 3. Test with minimal example
./qtforge_test_plugin /path/to/plugin.dll
```

### Application Crashes on Startup
```cpp
// Add this to your main() function for debugging
#include <qtforge/debug.hpp>

int main() {
    qtforge::enable_crash_handler();
    qtforge::Logger::set_level(qtforge::LogLevel::Debug);

    // Your application code here
}
```

### Performance Issues
```cpp
// Enable performance monitoring
auto& monitor = manager->performance_monitor();
monitor.enable_profiling(true);
monitor.set_sampling_interval(std::chrono::milliseconds(100));

// Check plugin load times
auto stats = monitor.get_plugin_stats();
for (const auto& [name, stat] : stats) {
    if (stat.load_time > std::chrono::seconds(1)) {
        std::cout << "Slow plugin: " << name << " ("
                  << stat.load_time.count() << "ms)" << std::endl;
    }
}
```

## Quick Diagnostics

### System Check

Run this diagnostic code to check your QtForge installation:

```cpp
#include <qtforge/qtforge.hpp>
#include <iostream>

void run_diagnostics() {
    std::cout << "=== QtForge Diagnostics ===" << std::endl;

    // 1. Check library initialization
    std::cout << "1. Library initialization..." << std::endl;
    qtplugin::LibraryInitializer init;
    if (!init.is_initialized()) {
        std::cout << "❌ FAILED: Library initialization failed" << std::endl;
        return;
    }
    std::cout << "✅ SUCCESS: Library initialized" << std::endl;
    std::cout << "   Version: " << qtplugin::version_string() << std::endl;

    // 2. Check Qt version
    std::cout << "\n2. Qt version check..." << std::endl;
    std::cout << "   Qt version: " << QT_VERSION_STR << std::endl;
    std::cout << "   Runtime Qt: " << qVersion() << std::endl;

    // 3. Check plugin manager
    std::cout << "\n3. Plugin manager creation..." << std::endl;
    auto manager = qtplugin::PluginManager::create();
    if (!manager) {
        std::cout << "❌ FAILED: Plugin manager creation failed" << std::endl;
        return;
    }
    std::cout << "✅ SUCCESS: Plugin manager created" << std::endl;

    // 4. Check security manager
    std::cout << "\n4. Security manager check..." << std::endl;
    auto& security = manager->security_manager();
    std::cout << "✅ SUCCESS: Security manager available" << std::endl;
    std::cout << "   Security level: " << static_cast<int>(security.current_level()) << std::endl;

    // 5. Check message bus
    std::cout << "\n5. Message bus check..." << std::endl;
    auto& bus = manager->message_bus();
    std::cout << "✅ SUCCESS: Message bus available" << std::endl;

    std::cout << "\n=== Diagnostics Complete ===" << std::endl;
}
```

### Environment Check

Verify your development environment:

```bash
# Check Qt installation
qmake --version  # or qmake6 --version

# Check CMake version
cmake --version

# Check compiler version
g++ --version    # Linux
clang++ --version  # macOS
# Check Visual Studio version in IDE (Windows)

# Check QtPlugin installation
pkg-config --exists qtplugin && echo "QtPlugin found" || echo "QtPlugin not found"
pkg-config --modversion qtplugin
```

## Installation Issues

### Qt Not Found

**Problem**: CMake cannot find Qt6

**Symptoms**:

```
CMake Error: Could not find a package configuration file provided by "Qt6"
```

**Solutions**:

=== "Set Qt6_DIR"
```bash # Find Qt installation
find /usr -name "Qt6Config.cmake" 2>/dev/null # Windows: dir /s Qt6Config.cmake

    # Set Qt6_DIR in CMake
    cmake .. -DQt6_DIR=/path/to/qt6/lib/cmake/Qt6
    ```

=== "Environment Variables"
```bash # Linux/macOS
export CMAKE_PREFIX_PATH="/path/to/qt6:$CMAKE_PREFIX_PATH"
    export PATH="/path/to/qt6/bin:$PATH"

    # Windows
    set CMAKE_PREFIX_PATH=C:\Qt\6.5.0\msvc2019_64;%CMAKE_PREFIX_PATH%
    set PATH=C:\Qt\6.5.0\msvc2019_64\bin;%PATH%
    ```

=== "Qt Online Installer"
```bash # Install Qt using the official installer # Download from: https://www.qt.io/download-qt-installer

    # Or use package managers:
    # Ubuntu: sudo apt install qt6-base-dev
    # macOS: brew install qt6
    # Windows: winget install Qt.Qt
    ```

### Compiler Issues

**Problem**: C++20 features not available

**Symptoms**:

```
error: 'expected' is not a member of 'std'
error: 'concept' does not name a type
```

**Solutions**:

1. **Update Compiler**:

   - **GCC**: Use version 10 or later
   - **Clang**: Use version 12 or later
   - **MSVC**: Use Visual Studio 2019 16.8 or later

2. **Check Compiler Flags**:

   ```cmake
   # Ensure C++20 is enabled
   set(CMAKE_CXX_STANDARD 20)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)

   # Compiler-specific flags
   if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
       add_compile_options(-fcoroutines)
   elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
       add_compile_options(-fcoroutines-ts)
   elseif(MSVC)
       add_compile_options(/await)
   endif()
   ```

### CMake Version Issues

**Problem**: CMake version too old

**Symptoms**:

```
CMake Error: CMake 3.21 or higher is required. You are running version 3.16.3
```

**Solutions**:

=== "Linux (Ubuntu/Debian)"
```bash # Remove old version
sudo apt remove cmake

    # Add Kitware repository
    wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg
    sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
    sudo apt update && sudo apt install cmake
    ```

=== "macOS"
```bash # Using Homebrew
brew install cmake

    # Or download from cmake.org
    ```

=== "Windows"
```powershell # Using winget
winget install Kitware.CMake

    # Or download installer from cmake.org
    ```

## Build Issues

### Missing Dependencies

**Problem**: Build fails due to missing libraries

**Symptoms**:

```
fatal error: 'QCore' file not found
undefined reference to 'Qt6::Core'
```

**Solutions**:

1. **Install Qt Development Packages**:

   ```bash
   # Ubuntu/Debian
   sudo apt install qt6-base-dev qt6-tools-dev

   # CentOS/RHEL/Fedora
   sudo dnf install qt6-qtbase-devel qt6-qttools-devel

   # Arch Linux
   sudo pacman -S qt6-base qt6-tools
   ```

2. **Check CMake Configuration**:

   ```cmake
   # Verify required components
   find_package(Qt6 REQUIRED COMPONENTS Core)
   find_package(QtPlugin REQUIRED COMPONENTS Core Security)

   # Link libraries
   target_link_libraries(your_target
       Qt6::Core
       QtPlugin::Core
       QtPlugin::Security
   )
   ```

### Linker Errors

**Problem**: Undefined symbols during linking

**Symptoms**:

```
undefined reference to `qtplugin::PluginManager::create()'
undefined symbol: _ZN8qtplugin13PluginManager6createEv
```

**Solutions**:

1. **Check Library Linking**:

   ```cmake
   # Ensure proper linking order
   target_link_libraries(your_target
       QtPlugin::Core      # Link QtPlugin first
       QtPlugin::Security  # Then additional components
       Qt6::Core          # Then Qt
   )
   ```

2. **Verify Installation**:

   ```bash
   # Check if libraries are installed
   find /usr/local -name "libqtplugin*" 2>/dev/null

   # Check library symbols
   nm -D /usr/local/lib/libqtplugin-core.a | grep PluginManager
   ```

3. **Static vs Dynamic Linking**:

   ```cmake
   # For static linking (default)
   set(BUILD_SHARED_LIBS OFF)

   # For dynamic linking
   set(BUILD_SHARED_LIBS ON)
   ```

## Runtime Issues

### Plugin Loading Failures

**Problem**: Plugins fail to load at runtime

**Symptoms**:

```
Failed to load plugin: Library loading failed
Plugin validation failed
Symbol not found: plugin_create_instance
```

**Debugging Steps**:

1. **Enable Debug Logging**:

   ```cpp
   // Enable detailed logging
   qtplugin::set_log_level(qtplugin::LogLevel::Debug);
   qtplugin::enable_component_logging(true);

   // Load plugin with detailed error reporting
   auto result = manager->load_plugin("plugin.so");
   if (!result) {
       auto error = result.error();
       qDebug() << "Error code:" << static_cast<int>(error.code);
       qDebug() << "Message:" << error.message.c_str();
       qDebug() << "Details:" << error.details.c_str();
       qDebug() << "File:" << error.file.c_str();
       qDebug() << "Line:" << error.line;
   }
   ```

2. **Check Plugin File**:

   ```bash
   # Verify file exists and is readable
   ls -la plugin.so

   # Check file type
   file plugin.so

   # Check dependencies (Linux)
   ldd plugin.so

   # Check dependencies (macOS)
   otool -L plugin.so

   # Check exported symbols
   nm -D plugin.so | grep qtplugin
   ```

3. **Verify Plugin Interface**:

   ```cpp
   // Check if plugin exports required symbols
   QPluginLoader loader("plugin.so");
   if (!loader.load()) {
       qDebug() << "Qt plugin loading failed:" << loader.errorString();
   }

   // Check interface
   auto instance = loader.instance();
   auto plugin = qobject_cast<qtplugin::IPlugin*>(instance);
   if (!plugin) {
       qDebug() << "Plugin doesn't implement IPlugin interface";
   }
   ```

### ABI Compatibility Issues

**Problem**: Plugin loads but crashes or behaves incorrectly

**Symptoms**:

```
Segmentation fault
Pure virtual function called
Unexpected behavior
```

**Solutions**:

1. **Check Compiler Compatibility**:

   ```bash
   # Ensure same compiler for app and plugin
   # Check compiler version used for each
   strings your_app | grep GCC
   strings plugin.so | grep GCC
   ```

2. **Verify Qt Versions**:

   ```cpp
   // Check Qt version compatibility
   qDebug() << "App Qt version:" << QT_VERSION_STR;
   qDebug() << "Runtime Qt version:" << qVersion();

   // In plugin, check Qt version
   qDebug() << "Plugin compiled with Qt:" << QT_VERSION_STR;
   ```

3. **Check C++ Standard Library**:
   ```bash
   # Ensure same standard library
   ldd your_app | grep libstdc++
   ldd plugin.so | grep libstdc++
   ```

### Memory Issues

**Problem**: Memory leaks or crashes related to memory

**Symptoms**:

```
Heap corruption detected
Double free or corruption
Memory leak detected
```

**Debugging Tools**:

1. **Valgrind (Linux)**:

   ```bash
   # Check for memory leaks
   valgrind --leak-check=full --show-leak-kinds=all ./your_app

   # Check for memory errors
   valgrind --tool=memcheck ./your_app
   ```

2. **AddressSanitizer**:

   ```cmake
   # Enable AddressSanitizer
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
   set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
   ```

3. **Qt Creator Debugger**:
   - Use built-in memory analyzer
   - Set breakpoints in destructors
   - Monitor object lifetimes

## Performance Issues

### Slow Plugin Loading

**Problem**: Plugins take too long to load

**Symptoms**:

- Application freezes during plugin loading
- Long startup times
- Timeout errors

**Solutions**:

1. **Profile Plugin Loading**:

   ```cpp
   #include <QElapsedTimer>

   QElapsedTimer timer;
   timer.start();

   auto result = manager->load_plugin("plugin.so");

   qDebug() << "Plugin loading took:" << timer.elapsed() << "ms";
   ```

2. **Use Parallel Loading**:

   ```cpp
   #include <future>
   #include <vector>

   std::vector<std::future<expected<std::string, PluginError>>> futures;

   for (const auto& path : plugin_paths) {
       futures.push_back(std::async(std::launch::async, [&]() {
           return manager->load_plugin(path);
       }));
   }

   // Wait for all plugins to load
   for (auto& future : futures) {
       auto result = future.get();
       // Handle result...
   }
   ```

3. **Optimize Plugin Initialization**:

   ```cpp
   // Load plugin without immediate initialization
   auto result = manager->load_plugin(path, LoadOptions{.initialize = false});

   // Initialize later when needed
   auto plugin = manager->get_plugin(result.value());
   plugin->initialize();
   ```

### High Memory Usage

**Problem**: Application uses too much memory with plugins

**Solutions**:

1. **Monitor Memory Usage**:

   ```cpp
   // Get memory statistics
   auto stats = manager->memory_statistics();
   qDebug() << "Total plugin memory:" << stats.total_memory_usage;
   qDebug() << "Plugin count:" << stats.loaded_plugin_count;
   qDebug() << "Average per plugin:" << stats.average_memory_per_plugin;
   ```

2. **Implement Plugin Unloading**:

   ```cpp
   // Unload unused plugins
   auto unused_plugins = manager->find_unused_plugins(std::chrono::minutes(10));
   for (const auto& plugin_id : unused_plugins) {
       manager->unload_plugin(plugin_id);
   }
   ```

3. **Use Memory Limits**:

   ```cpp
   // Set memory limits for plugins
   PluginLoadOptions options;
   options.max_memory_usage = 50 * 1024 * 1024; // 50MB
   options.enable_memory_monitoring = true;

   auto result = manager->load_plugin("plugin.so", options);
   ```

## Security Issues

### Plugin Validation Failures

**Problem**: Plugins fail security validation

**Symptoms**:

```
Plugin validation failed: Untrusted publisher
Security check failed: Invalid signature
Plugin blocked by security policy
```

**Solutions**:

1. **Check Security Configuration**:

   ```cpp
   auto& security = manager->security_manager();
   auto config = security.current_configuration();

   qDebug() << "Security level:" << static_cast<int>(config.level);
   qDebug() << "Signature validation:" << config.require_signatures;
   qDebug() << "Trust validation:" << config.require_trusted_publishers;
   ```

2. **Adjust Security Settings**:

   ```cpp
   SecurityConfiguration config;
   config.level = SecurityLevel::Medium;  // Reduce from High
   config.require_signatures = false;     // Disable signature requirement
   config.allow_unsigned_plugins = true;  // Allow unsigned plugins

   security.configure(config);
   ```

3. **Add Trusted Publishers**:

   ```cpp
   // Add publisher to trust list
   security.add_trusted_publisher("com.example.publisher");

   // Or trust specific plugin
   security.add_trusted_plugin("com.example.specific-plugin");
   ```

### Permission Denied Errors

**Problem**: Plugins can't access required resources

**Solutions**:

1. **Check File Permissions**:

   ```bash
   # Verify plugin file permissions
   ls -la plugin.so

   # Check directory permissions
   ls -ld /path/to/plugins/

   # Fix permissions if needed
   chmod 755 plugin.so
   chmod 755 /path/to/plugins/
   ```

2. **Configure Plugin Permissions**:

   ```cpp
   PluginPermissions permissions;
   permissions.allow_file_system_access = true;
   permissions.allow_network_access = true;
   permissions.allowed_directories = {"/tmp", "/var/data"};

   manager->set_plugin_permissions("plugin.id", permissions);
   ```

## Communication Issues

### Message Bus Problems

**Problem**: Inter-plugin communication not working

**Symptoms**:

- Messages not delivered
- Subscribers not receiving messages
- Request-response timeouts

**Debugging**:

1. **Enable Message Bus Logging**:

   ```cpp
   auto& bus = manager->message_bus();
   bus.enable_debug_logging(true);

   // Check message statistics
   auto stats = bus.statistics();
   qDebug() << "Messages sent:" << stats.messages_sent;
   qDebug() << "Messages delivered:" << stats.messages_delivered;
   qDebug() << "Active subscriptions:" << stats.active_subscriptions;
   ```

2. **Verify Message Types**:

   ```cpp
   // Ensure message types are properly registered
   bus.register_message_type<MyMessageType>();

   // Check if type is registered
   if (!bus.is_message_type_registered<MyMessageType>()) {
       qDebug() << "Message type not registered!";
   }
   ```

3. **Test Message Delivery**:

   ```cpp
   // Simple test message
   struct TestMessage {
       QString content;
   };

   // Subscribe
   bus.subscribe<TestMessage>([](const TestMessage& msg) {
       qDebug() << "Received test message:" << msg.content;
   });

   // Publish
   bus.publish(TestMessage{"Hello, World!"});
   ```

## Development Issues

### Plugin Development Problems

**Problem**: Issues during plugin development

**Common Issues and Solutions**:

1. **Plugin Not Recognized**:

   ```cpp
   // Ensure proper Qt plugin macros
   Q_OBJECT
   Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "metadata.json")
   Q_INTERFACES(qtplugin::IPlugin)

   // Include moc file
   #include "myplugin.moc"
   ```

2. **Metadata File Issues**:

   ```json
   {
     "id": "com.example.myplugin",
     "name": "My Plugin",
     "version": "1.0.0",
     "description": "Plugin description",
     "author": "Author Name",
     "qtplugin_version": "3.0.0"
   }
   ```

3. **CMake Configuration**:
   ```cmake
   # Use QtPlugin helper function
   qtplugin_add_plugin(my_plugin
       TYPE service
       SOURCES my_plugin.cpp
       HEADERS my_plugin.hpp
       METADATA metadata.json
       DEPENDENCIES Qt6::Network
   )
   ```

### Testing Issues

**Problem**: Difficulty testing plugins

**Solutions**:

1. **Unit Testing Framework**:

   ```cpp
   #include <QtTest>
   #include <qtplugin/testing/plugin_test_framework.hpp>

   class PluginTest : public QObject {
       Q_OBJECT

   private slots:
       void testPluginLoading();
       void testPluginCommands();
   };

   void PluginTest::testPluginLoading() {
       qtplugin::testing::PluginTestFramework framework;

       auto result = framework.load_test_plugin("my_plugin.so");
       QVERIFY(result.has_value());

       auto plugin = framework.get_plugin(result.value());
       QVERIFY(plugin != nullptr);
       QVERIFY(plugin->is_initialized());
   }
   ```

2. **Mock Plugin Manager**:

   ```cpp
   #include <qtplugin/testing/mock_plugin_manager.hpp>

   qtplugin::testing::MockPluginManager mock_manager;
   mock_manager.expect_load_plugin("test_plugin.so")
               .will_return("test.plugin.id");

   // Test your code with mock manager
   ```

## Getting Help

### Diagnostic Information

When seeking help, provide this diagnostic information:

```cpp
void collect_diagnostic_info() {
    qDebug() << "=== System Information ===";
    qDebug() << "OS:" << QSysInfo::prettyProductName();
    qDebug() << "Architecture:" << QSysInfo::currentCpuArchitecture();
    qDebug() << "Qt version:" << qVersion();
    qDebug() << "QtPlugin version:" << qtplugin::version_string();

    qDebug() << "\n=== Build Information ===";
    qDebug() << "Compiler:" <<
    #ifdef __GNUC__
        "GCC" << __GNUC__ << "." << __GNUC_MINOR__;
    #elif defined(__clang__)
        "Clang" << __clang_major__ << "." << __clang_minor__;
    #elif defined(_MSC_VER)
        "MSVC" << _MSC_VER;
    #else
        "Unknown";
    #endif

    qDebug() << "C++ standard:" << __cplusplus;

    qDebug() << "\n=== Plugin Manager State ===";
    if (auto manager = qtplugin::PluginManager::instance()) {
        auto stats = manager->statistics();
        qDebug() << "Loaded plugins:" << stats.loaded_plugin_count;
        qDebug() << "Failed loads:" << stats.failed_load_count;
        qDebug() << "Memory usage:" << stats.total_memory_usage;
    }
}
```

### Support Channels

- **📖 Documentation**: Browse this comprehensive guide
- **💡 Examples**: Check [examples](../examples/index.md) for working code
- **🐛 Bug Reports**: [GitHub Issues](https://github.com/QtForge/QtPlugin/issues)
- **💬 Questions**: [GitHub Discussions](https://github.com/QtForge/QtPlugin/discussions)
- **📧 Enterprise**: Contact maintainers for commercial support

### Creating Bug Reports

Include this information in bug reports:

1. **System Information** (use diagnostic code above)
2. **Reproduction Steps** (minimal example)
3. **Expected vs Actual Behavior**
4. **Error Messages** (complete output)
5. **Code Samples** (minimal reproducible case)

### Performance Profiling

For performance issues, use profiling tools:

```bash
# Linux - perf
perf record -g ./your_app
perf report

# macOS - Instruments
instruments -t "Time Profiler" ./your_app

# Windows - Visual Studio Profiler
# Use built-in profiler in Visual Studio
```

## QtForge v3.2.0 Specific Issues

### Python Plugin Issues

#### Python Plugin Not Loading

**Symptoms**:
```
Error: Failed to load Python plugin: ModuleNotFoundError
```

**Solutions**:

1. **Check Python Installation**:
   ```bash
   python3 --version  # Should be 3.8+
   python3 -c "import qtforge"  # Should not error
   ```

2. **Install QtForge Python Bindings**:
   ```bash
   pip install qtforge
   # Or from source
   cd bindings/python && pip install .
   ```

3. **Check Python Path**:
   ```python
   import sys
   print(sys.path)  # Ensure qtforge is in path
   ```

#### Python Type Stub Issues

**Symptoms**:
```
IDE shows "Cannot find module 'qtforge'" despite working code
```

**Solutions**:

1. **Install Type Stubs**:
   ```bash
   pip install qtforge[stubs]
   ```

2. **IDE Configuration**:
   - **PyCharm**: Mark qtforge as source root
   - **VSCode**: Update python.analysis.extraPaths

### Lua Plugin Issues

#### Lua Plugin Bridge Not Available

**Symptoms**:
```
Error: Lua plugin bridge not initialized
```

**Solutions**:

1. **Check Lua Installation**:
   ```bash
   lua -v  # Should be 5.4+
   ```

2. **Enable Lua Bridge in Build**:
   ```cmake
   set(QTFORGE_ENABLE_LUA_BRIDGE ON)
   ```

3. **Check Sol2 Dependency**:
   ```cpp
   #include <sol/sol.hpp>  // Should compile
   ```

#### Lua Plugin Loading Errors

**Symptoms**:
```
Error: Lua plugin syntax error or runtime error
```

**Solutions**:

1. **Check Lua Syntax**:
   ```bash
   lua -c your_plugin.lua  # Check syntax
   ```

2. **Debug Lua Plugin**:
   ```lua
   -- Add debug prints
   print("Plugin loading...")
   qtforge.utils.log_debug("Debug message")
   ```

### Configuration API Migration Issues

#### Configuration Manager Compilation Errors

**Symptoms**:
```cpp
error: 'getValue' is not a member of 'qtforge::ConfigurationManager'
```

**Solutions**:

1. **Update to Factory Pattern**:
   ```cpp
   // Old v3.0.0
   qtforge::ConfigurationManager config;

   // New v3.2.0
   auto config = qtforge::managers::create_configuration_manager();
   ```

2. **Update Method Names**:
   ```cpp
   // Old
   config.getValue("key")

   // New
   config->get_value("key", qtforge::managers::ConfigurationScope::Global)
   ```

### Security Policy Issues

#### Security Policy Validation Failures

**Symptoms**:
```
Error: Security policy validation failed
```

**Solutions**:

1. **Use Policy Validator**:
   ```cpp
   qtforge::SecurityPolicyValidator validator;
   auto result = validator.validatePolicy(policy);
   if (!result.isValid) {
       std::cout << "Error: " << result.errorMessage << std::endl;
   }
   ```

2. **Check Policy Structure**:
   ```cpp
   qtforge::SecurityPolicy policy;
   policy.name = "ValidPolicy";  // Required
   policy.minimumTrustLevel = qtforge::TrustLevel::Medium;  // Required
   policy.allowedPermissions = {  // At least one required
       qtforge::PluginPermission::FileSystemRead
   };
   ```

### Advanced Plugin Interface Issues

#### IAdvancedPlugin Compilation Errors

**Symptoms**:
```cpp
error: cannot declare variable 'plugin' to be of abstract type 'MyAdvancedPlugin'
```

**Solutions**:

1. **Implement All Pure Virtual Methods**:
   ```cpp
   class MyAdvancedPlugin : public qtforge::IAdvancedPlugin {
   public:
       // Must implement all IPlugin methods
       std::string name() const override { return "MyPlugin"; }
       std::string version() const override { return "1.0.0"; }
       // ... other IPlugin methods

       // Must implement IAdvancedPlugin methods
       std::vector<qtforge::ServiceContract> getServiceContracts() override {
           return {};  // Return empty if no contracts
       }
   };
   ```

### Cross-Language Communication Issues

#### Message Bus Communication Failures

**Symptoms**:
```
Warning: Message not delivered to target plugin
```

**Solutions**:

1. **Check Message Format**:
   ```cpp
   // Ensure consistent message format across languages
   auto message = qtforge::communication::create_message(
       "service.command",
       {{"param1", "value1"}, {"param2", "value2"}}
   );
   ```

2. **Verify Subscription**:
   ```python
   # Python subscriber
   def handle_message(message):
       print(f"Received: {message}")

   message_bus.subscribe("service.command", handle_message)
   ```

### Build System Issues

#### CMake Configuration Errors with v3.2.0

**Symptoms**:
```
CMake Error: Could not find QtForge 3.2
```

**Solutions**:

1. **Update CMake Version Requirement**:
   ```cmake
   find_package(QtForge 3.2 REQUIRED)
   ```

2. **Enable New Features**:
   ```cmake
   set(QTFORGE_ENABLE_PYTHON_BINDINGS ON)
   set(QTFORGE_ENABLE_LUA_BRIDGE ON)
   ```

### Performance Issues with v3.2.0

#### Slow Plugin Loading

**Symptoms**:
Plugin loading takes significantly longer than expected.

**Solutions**:

1. **Check Resource Monitoring**:
   ```cpp
   // Disable resource monitoring if not needed
   sandbox.enableResourceMonitoring(false);
   ```

2. **Optimize Security Policies**:
   ```cpp
   // Use minimal required permissions
   policy.allowedPermissions = {
       qtforge::PluginPermission::FileSystemRead  // Only what's needed
   };
   ```

Remember: Most issues have simple solutions. Check the [FAQ](../appendix/faq.md) first, then use the debugging techniques in this guide to narrow down the problem.
