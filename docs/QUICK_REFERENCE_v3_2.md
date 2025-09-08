# QtForge v3.2.0 Quick Reference

Quick reference cards for new features and APIs in QtForge v3.2.0.

## 🚀 Python Plugin Quick Reference

### Basic Python Plugin

```python
import qtforge
from qtforge.core import IPlugin, PluginState

class MyPlugin(IPlugin):
    def __init__(self):
        super().__init__()
        self._name = "My Plugin"
        self._version = "1.0.0"

    def initialize(self):
        return {"success": True, "message": "Ready"}

    def execute_command(self, command, params):
        if command == "process":
            return {"success": True, "result": "Done"}
        return {"success": False, "error": "Unknown command"}

def create_plugin():
    return MyPlugin()
```

### Python Plugin Manager

```python
import qtforge

# Create manager
manager = qtforge.core.create_plugin_manager()

# Load plugin
result = manager.load_plugin("my_plugin.py")
if result.success:
    plugin = manager.get_plugin(result.plugin_id)
    plugin.initialize()
```

## 🌙 Lua Plugin Quick Reference

### Basic Lua Plugin

```lua
local plugin = {}

plugin.name = "My Lua Plugin"
plugin.version = "1.0.0"
plugin.description = "Example Lua plugin"

function plugin.initialize()
    qtforge.utils.log_info("Initializing " .. plugin.name)
    return {success = true, message = "Ready"}
end

function plugin.execute_command(command, params)
    if command == "process" then
        return {success = true, result = "Done"}
    end
    return {success = false, error = "Unknown command"}
end

return plugin
```

### Lua Plugin Manager

```lua
local qtforge = require('qtforge')

-- Create manager
local manager = qtforge.core.create_plugin_manager()

-- Load plugin
local result = manager:load_plugin("my_plugin.lua")
if result.success then
    local plugin = manager:get_plugin(result.plugin_id)
    plugin:initialize()
end
```

## 🔧 Advanced Plugin Interfaces

### IAdvancedPlugin (C++)

```cpp
class MyAdvancedPlugin : public qtforge::IAdvancedPlugin {
public:
    std::vector<qtforge::ServiceContract> getServiceContracts() override {
        qtforge::ServiceContract contract;
        contract.name = "MyService";
        contract.version = qtforge::ServiceVersion(1, 0, 0);
        contract.capabilities = {
            qtforge::ServiceCapability::DataTransformation
        };
        return {contract};
    }

    qtforge::InterfaceCapability getInterfaceCapability() override {
        qtforge::InterfaceCapability capability;
        capability.name = "MyInterface";
        capability.version = "1.0.0";
        capability.methods = {"process", "validate"};
        return capability;
    }
};
```

### IDynamicPlugin (C++)

```cpp
class MyDynamicPlugin : public qtforge::IDynamicPlugin {
public:
    qtforge::InterfaceDescriptor getInterfaceDescriptor() override {
        qtforge::InterfaceDescriptor desc;
        desc.name = "DynamicInterface";
        desc.version = "1.0.0";
        desc.supportedTypes = {
            qtforge::PluginType::Native,
            qtforge::PluginType::Python
        };
        return desc;
    }

    bool negotiateCapabilities(
        const std::vector<std::string>& requested) override {
        // Negotiate capabilities dynamically
        return true;
    }
};
```

## 🛡️ Enhanced Security Quick Reference

### Security Policy

```cpp
qtforge::SecurityPolicy policy;
policy.name = "MyPolicy";
policy.minimumTrustLevel = qtforge::TrustLevel::Medium;
policy.allowedPermissions = {
    qtforge::PluginPermission::FileSystemRead,
    qtforge::PluginPermission::NetworkAccess
};

// Validate policy
qtforge::SecurityPolicyValidator validator;
auto result = validator.validatePolicy(policy);
```

### Plugin Sandbox

```cpp
qtforge::PluginSandbox sandbox;
sandbox.setPolicy(policy);
sandbox.enableResourceMonitoring(true);
sandbox.setMaxMemoryUsage(100 * 1024 * 1024); // 100MB
sandbox.setMaxCpuTime(30.0); // 30 seconds

auto initResult = sandbox.initialize();
```

### Resource Monitoring

```cpp
qtforge::ResourceMonitorUtils monitor;
monitor.setMemoryThreshold(50 * 1024 * 1024); // 50MB
monitor.setCpuThreshold(80.0); // 80%

auto usage = monitor.getResourceUsage(pluginId);
if (monitor.checkMemoryThreshold(usage.memoryUsage)) {
    // Handle threshold exceeded
}
```

## ⚙️ Configuration Management

### Scoped Configuration (C++)

```cpp
auto config = qtforge::managers::create_configuration_manager();

// Set values with different scopes
config->set_value("app.theme", "dark",
    qtforge::managers::ConfigurationScope::User);
config->set_value("plugin.timeout", 30,
    qtforge::managers::ConfigurationScope::Plugin);

// Get values
auto theme = config->get_value("app.theme",
    qtforge::managers::ConfigurationScope::User);
```

### Python Configuration

```python
import qtforge

config = qtforge.managers.create_configuration_manager()

# Set scoped values
config.set_value("user.theme", "dark",
    qtforge.managers.ConfigurationScope.User)

# Get values
theme = config.get_value("user.theme",
    qtforge.managers.ConfigurationScope.User)
```

### Lua Configuration

```lua
local config = qtforge.managers.create_configuration_manager()

-- Set scoped values
config:set_value("user.theme", "dark",
    qtforge.managers.ConfigurationScope.User)

-- Get values
local theme = config:get_value("user.theme",
    qtforge.managers.ConfigurationScope.User)
```

## 📡 Service Contract System

### Service Contract (C++)

```cpp
qtforge::ServiceContract contract;
contract.name = "DataProcessingService";
contract.version = qtforge::ServiceVersion(2, 1, 0);
contract.description = "Data processing service";
contract.capabilities = {
    qtforge::ServiceCapability::DataTransformation,
    qtforge::ServiceCapability::AsyncProcessing
};

// Register service
auto registry = qtforge::communication::create_service_registry();
registry->registerService(contract);
```

### Message Bus

```cpp
auto messageBus = qtforge::communication::create_message_bus();

// Create and publish message
auto message = qtforge::communication::create_message(
    "data.process", {{"input", "test_data"}});
messageBus->publish(message);

// Subscribe to messages
messageBus->subscribe("data.process",
    [](const qtforge::Message& msg) {
        // Handle message
    });
```

## 🔄 Plugin Types

### Plugin Type Enumeration

```cpp
enum class PluginType {
    Native,      // C++ plugins
    Python,      // Python plugins
    Lua,         // Lua plugins
    JavaScript,  // JavaScript plugins (planned)
    Remote,      // Remote process plugins
    Composite    // Multi-language plugins
};
```

### Plugin Capabilities

```cpp
enum class PluginCapability {
    Service,              // Background service
    DataProcessing,       // Data transformation
    UserInterface,        // UI components
    NetworkAccess,        // Network operations
    FileSystemAccess,     // File operations
    DatabaseAccess,       // Database operations
    AsyncProcessing,      // Asynchronous operations
    EventHandling,        // Event processing
    Monitoring,           // System monitoring
    Security              // Security operations
};
```

## 🎯 Common Patterns

### Factory Pattern

```cpp
// C++ Factory
auto manager = qtforge::core::create_plugin_manager();
auto config = qtforge::managers::create_configuration_manager();
auto security = qtforge::security::create_security_manager();
```

```python
# Python Factory
manager = qtforge.core.create_plugin_manager()
config = qtforge.managers.create_configuration_manager()
security = qtforge.security.create_security_manager()
```

### Error Handling

```cpp
// C++ Error Handling
auto result = manager->load_plugin("plugin.so");
if (!result.success) {
    std::cerr << "Error: " << result.errorMessage << std::endl;
    return false;
}
```

```python
# Python Error Handling
result = manager.load_plugin("plugin.py")
if not result.success:
    print(f"Error: {result.error_message}")
    return False
```

## 📋 Migration Checklist

### From v3.0.0 to v3.2.0

- [ ] Update version references to 3.2.0
- [ ] Replace `ConfigurationManager` with factory function
- [ ] Update configuration API calls to use scoped access
- [ ] Update security configuration to use policies
- [ ] Consider upgrading to advanced plugin interfaces
- [ ] Test multilingual plugin support if needed

### Quick Migration Commands

```cpp
// Old v3.0.0
qtforge::ConfigurationManager config;
auto value = config.getValue("key");

// New v3.2.0
auto config = qtforge::managers::create_configuration_manager();
auto value = config->get_value("key",
    qtforge::managers::ConfigurationScope::Global);
```

## 🔗 Quick Links

- **[Complete Documentation](DOCUMENTATION_INDEX.md)** - Full documentation index
- **[Migration Guide](MIGRATION_v3_2.md)** - Detailed migration instructions
- **[Feature Comparison](FEATURE_COMPARISON_v3_0_to_v3_2.md)** - v3.0.0 vs v3.2.0
- **[API Reference](api/overview.md)** - Complete API documentation
- **[Examples](examples/index.md)** - Code examples and tutorials

---

_Quick Reference for QtForge v3.2.0 | September 2024_
