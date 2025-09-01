# QtPlugin API Reference

Complete API documentation for the QtPlugin library.

## Core Components

### Plugin Interface

#### `qtplugin::IPlugin`

Base interface that all plugins must implement.

```cpp
class IPlugin {
public:
    virtual ~IPlugin() = default;

    // Core lifecycle methods
    virtual expected<void, PluginError> initialize() = 0;
    virtual void shutdown() = 0;
    virtual PluginState state() const = 0;
    virtual bool is_initialized() const = 0;

    // Metadata and identification
    virtual PluginMetadata metadata() const = 0;
    virtual std::string id() const = 0;
    virtual std::string name() const = 0;
    virtual std::string version() const = 0;
    virtual std::string description() const = 0;

    // Configuration management
    virtual expected<void, PluginError> configure(const QJsonObject& config) = 0;
    virtual QJsonObject current_configuration() const = 0;

    // Command execution
    virtual expected<QJsonObject, PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}
    ) = 0;

    // Capability queries
    virtual std::vector<std::string> supported_commands() const = 0;
    virtual bool supports_command(std::string_view command) const = 0;
    virtual PluginCapabilities capabilities() const = 0;
};
```

#### Plugin States

```cpp
enum class PluginState {
    Unloaded,    // Plugin not loaded
    Loaded,      // Plugin loaded but not initialized
    Running,     // Plugin initialized and running
    Error        // Plugin in error state
};
```

#### Plugin Metadata

```cpp
struct PluginMetadata {
    std::string id;
    std::string name;
    std::string version;
    std::string description;
    std::string author;
    std::string license;
    std::vector<std::string> dependencies;
    std::vector<std::string> tags;
    QJsonObject custom_data;
};
```

### Plugin Manager

#### `qtplugin::PluginManager`

Central component for managing plugins.

```cpp
class PluginManager : public QObject {
    Q_OBJECT

public:
    // Factory method
    static std::unique_ptr<PluginManager> create();

    // Plugin loading/unloading
    expected<std::string, PluginError> load_plugin(
        const std::string& path,
        const PluginLoadOptions& options = {}
    );

    expected<void, PluginError> unload_plugin(const std::string& plugin_id);

    expected<void, PluginError> reload_plugin(
        const std::string& plugin_id,
        bool preserve_state = false
    );

    // Plugin access
    std::shared_ptr<IPlugin> get_plugin(const std::string& plugin_id);
    std::vector<std::string> loaded_plugins() const;
    std::vector<PluginMetadata> plugin_metadata() const;

    // Plugin discovery
    std::vector<std::string> discover_plugins(const std::string& directory);

    // Dependency management
    expected<void, PluginError> resolve_dependencies(const std::string& plugin_id);
    std::vector<std::string> get_dependencies(const std::string& plugin_id);

signals:
    void plugin_loaded(const QString& plugin_id);
    void plugin_unloaded(const QString& plugin_id);
    void plugin_error(const QString& plugin_id, const QString& error);
};
```

#### Plugin Load Options

```cpp
struct PluginLoadOptions {
    bool initialize_immediately = true;
    bool check_dependencies = true;
    bool enable_hot_reload = false;
    SecurityLevel security_level = SecurityLevel::Medium;
    QJsonObject initial_configuration;
};
```

### Error Handling

#### `qtplugin::expected<T, E>`

Modern error handling without exceptions.

```cpp
template<typename T, typename E>
class expected {
public:
    // Check if value is present
    bool has_value() const noexcept;
    explicit operator bool() const noexcept;

    // Access value (throws if error)
    const T& value() const &;
    T& value() &;
    T&& value() &&;

    // Access value with default
    template<typename U>
    T value_or(U&& default_value) const &;

    // Access error
    const E& error() const &;
    E& error() &;
    E&& error() &&;

    // Monadic operations
    template<typename F>
    auto and_then(F&& f) const &;

    template<typename F>
    auto or_else(F&& f) const &;

    template<typename F>
    auto transform(F&& f) const &;
};
```

#### Error Codes

```cpp
enum class PluginErrorCode {
    // Success
    Success = 0,

    // General errors
    UnknownError = 1,
    InvalidArgument = 2,
    OutOfMemory = 3,

    // File system errors
    FileNotFound = 100,
    InvalidFormat = 101,
    LoadFailed = 102,
    UnloadFailed = 103,
    SymbolNotFound = 104,
    AlreadyLoaded = 105,
    NotLoaded = 106,
    PluginNotFound = 107,

    // Dependency errors
    DependencyMissing = 200,
    CircularDependency = 201,
    DependencyFailed = 202,

    // Security errors
    SecurityViolation = 300,
    UntrustedPlugin = 301,
    InvalidSignature = 302,

    // Runtime errors
    InitializationFailed = 400,
    ConfigurationError = 401,
    CommandNotSupported = 402,
    ExecutionFailed = 403
};
```

## Communication System

### Message Bus

#### `qtplugin::MessageBus`

Inter-plugin communication system.

```cpp
class MessageBus : public QObject {
    Q_OBJECT

public:
    // Message publishing
    void publish(const Message& message);
    void publish(const std::string& topic, const QJsonObject& data);

    // Subscription management
    void subscribe(const std::string& topic, QObject* receiver, const char* slot);
    void unsubscribe(const std::string& topic, QObject* receiver);

    // Request-response pattern
    expected<QJsonObject, MessageError> send_request(
        const std::string& target_plugin,
        const std::string& command,
        const QJsonObject& params,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(5000)
    );

signals:
    void message_received(const QString& topic, const QJsonObject& data);
};
```

#### Message Types

```cpp
struct Message {
    std::string id;
    std::string topic;
    std::string sender;
    std::string target;
    QJsonObject data;
    std::chrono::system_clock::time_point timestamp;
    MessagePriority priority = MessagePriority::Normal;
};

enum class MessagePriority {
    Low,
    Normal,
    High,
    Critical
};
```

## Resource Management

### Resource Manager

#### `qtplugin::ResourceManager`

Manages plugin resources and lifecycle.

```cpp
class ResourceManager {
public:
    // Resource registration
    template<typename T>
    void register_resource(const std::string& id, std::shared_ptr<T> resource);

    // Resource access
    template<typename T>
    std::shared_ptr<T> get_resource(const std::string& id);

    // Resource cleanup
    void cleanup_plugin_resources(const std::string& plugin_id);
    void cleanup_all_resources();

    // Resource monitoring
    ResourceUsage get_usage_statistics() const;
    std::vector<ResourceInfo> list_resources() const;
};
```

## Security System

### Security Manager

#### `qtplugin::SecurityManager`

Plugin security and validation.

```cpp
class SecurityManager {
public:
    // Plugin validation
    expected<void, SecurityError> validate_plugin(const std::string& path);

    // Trust management
    void add_trusted_publisher(const std::string& publisher);
    void remove_trusted_publisher(const std::string& publisher);
    bool is_trusted_publisher(const std::string& publisher) const;

    // Security levels
    void set_security_level(SecurityLevel level);
    SecurityLevel get_security_level() const;

    // Signature verification
    expected<bool, SecurityError> verify_signature(const std::string& path);
};

enum class SecurityLevel {
    None,     // No security checks
    Low,      // Basic validation only
    Medium,   // Standard security checks
    High,     // Strict validation and signatures
    Maximum   // Maximum security, trusted publishers only
};
```

## Utility Classes

### Version Information

```cpp
namespace qtplugin {
    constexpr int version_major = 3;
    constexpr int version_minor = 0;
    constexpr int version_patch = 0;
    constexpr const char* version_string = "3.0.0";

    std::string get_version_string();
    std::tuple<int, int, int> get_version_tuple();
}
```

### Concepts

```cpp
// Plugin concept validation
template<typename T>
concept PluginType = requires(T t) {
    { t.initialize() } -> std::same_as<expected<void, PluginError>>;
    { t.shutdown() } -> std::same_as<void>;
    { t.metadata() } -> std::same_as<PluginMetadata>;
};

// Message concept validation
template<typename T>
concept MessageType = requires(T t) {
    { t.topic } -> std::convertible_to<std::string>;
    { t.data } -> std::convertible_to<QJsonObject>;
};
```

## Examples

### Basic Plugin Implementation

```cpp
class ExamplePlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    expected<void, PluginError> initialize() override {
        m_state = PluginState::Running;
        return {};
    }

    void shutdown() override {
        m_state = PluginState::Unloaded;
    }

    PluginMetadata metadata() const override {
        return {
            .id = "com.example.plugin",
            .name = "Example Plugin",
            .version = "1.0.0",
            .description = "Example plugin implementation"
        };
    }

    // ... implement other interface methods
};
```

### Plugin Manager Usage

```cpp
#include <qtplugin/qtplugin.hpp>

int main() {
    auto manager = qtplugin::PluginManager::create();

    // Load plugin
    auto result = manager->load_plugin("plugins/example.qtplugin");
    if (result.has_value()) {
        std::string plugin_id = result.value();

        // Get plugin instance
        auto plugin = manager->get_plugin(plugin_id);
        if (plugin) {
            // Execute command
            auto cmd_result = plugin->execute_command("hello", {{"name", "World"}});
            if (cmd_result.has_value()) {
                qDebug() << "Command result:" << cmd_result.value();
            }
        }
    }

    return 0;
}
```
