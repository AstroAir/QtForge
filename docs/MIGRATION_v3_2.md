# Migration Guide: QtForge v3.0.0 to v3.2.0

This guide provides step-by-step instructions for migrating from QtForge v3.0.0 to v3.2.0, including breaking changes, new features, and code examples.

## Overview

QtForge v3.2.0 introduces significant enhancements while maintaining backward compatibility for most use cases. The major changes include:

- **Enhanced Multilingual Support**: Complete Python and Lua plugin support
- **Advanced Plugin Interfaces**: New IAdvancedPlugin and IDynamicPlugin interfaces
- **Enhanced Security**: Advanced sandboxing and policy validation
- **Service Contract System**: Complete communication system with service discovery
- **Configuration Management**: Scoped configuration access

## Breaking Changes

### 1. Configuration API Changes

**Old API (v3.0.0):**

```cpp
qtforge::ConfigurationManager config;
auto value = config.getValue("key");
config.setValue("key", "value");
```

**New API (v3.2.0):**

```cpp
auto config = qtforge::managers::create_configuration_manager();
auto value = config->get_value("key", qtforge::managers::ConfigurationScope::Global);
config->set_value("key", "value", qtforge::managers::ConfigurationScope::Global);
```

**Migration Steps:**

1. Replace direct ConfigurationManager instantiation with factory function
2. Add scope parameter to all get_value and set_value calls
3. Update method names from camelCase to snake_case

### 2. Plugin Interface Enhancements

**Old API (v3.0.0):**

```cpp
class MyPlugin : public qtforge::IPlugin {
    // Basic plugin implementation
};
```

**New API (v3.2.0) - Optional Enhancement:**

```cpp
class MyPlugin : public qtforge::IAdvancedPlugin {
    // Enhanced plugin with service contracts
    std::vector<qtforge::ServiceContract> getServiceContracts() override {
        // Return service contracts
    }
};
```

**Migration Steps:**

1. Existing IPlugin implementations continue to work unchanged
2. Optionally upgrade to IAdvancedPlugin for new features
3. Consider IDynamicPlugin for runtime adaptation needs

### 3. Security Policy Changes

**Old API (v3.0.0):**

```cpp
qtforge::SecurityManager security;
security.setSecurityLevel(qtforge::SecurityLevel::Medium);
```

**New API (v3.2.0):**

```cpp
qtforge::SecurityPolicy policy;
policy.minimumTrustLevel = qtforge::TrustLevel::Medium;
policy.allowedPermissions = {
    qtforge::PluginPermission::FileSystemRead,
    qtforge::PluginPermission::NetworkAccess
};

qtforge::SecurityManager security;
security.setSecurityPolicy(policy);
```

## New Features

### 1. Python Plugin Support

Create Python plugins using the enhanced bindings:

```python
import qtforge
from qtforge.core import IPlugin, PluginState

class MyPythonPlugin(IPlugin):
    def __init__(self):
        super().__init__()
        self._name = "My Python Plugin"
        self._version = "1.0.0"

    def initialize(self):
        return {"success": True, "message": "Initialized"}

    def execute_command(self, command, params):
        if command == "process":
            return {"success": True, "result": "Processed"}
        return {"success": False, "error": "Unknown command"}

def create_plugin():
    return MyPythonPlugin()
```

### 2. Lua Plugin Support

Create Lua plugins using the new bridge:

```lua
local plugin = {}

plugin.name = "My Lua Plugin"
plugin.version = "1.0.0"
plugin.description = "A Lua plugin example"

function plugin.initialize()
    qtforge.utils.log_info("Initializing Lua plugin")
    return {success = true, message = "Initialized"}
end

function plugin.execute_command(command, params)
    if command == "process" then
        return {success = true, result = "Processed"}
    end
    return {success = false, error = "Unknown command"}
end

return plugin
```

### 3. Service Contract System

Implement service contracts for advanced communication:

```cpp
class DataProcessingService : public qtforge::IAdvancedPlugin {
public:
    std::vector<qtforge::ServiceContract> getServiceContracts() override {
        qtforge::ServiceContract contract;
        contract.name = "DataProcessingService";
        contract.version = qtforge::ServiceVersion(2, 1, 0);
        contract.capabilities = {
            qtforge::ServiceCapability::DataTransformation,
            qtforge::ServiceCapability::AsyncProcessing
        };
        return {contract};
    }
};
```

### 4. Enhanced Security Features

Use the new security components:

```cpp
// Security Policy Validator
qtforge::SecurityPolicyValidator validator;
auto result = validator.validatePolicy(policy);

// Plugin Sandbox
qtforge::PluginSandbox sandbox;
sandbox.setPolicy(policy);
sandbox.enableResourceMonitoring(true);
sandbox.initialize();

// Resource Monitor
qtforge::ResourceMonitorUtils monitor;
auto usage = monitor.getResourceUsage(pluginId);
```

## Migration Steps

### Step 1: Update Build Configuration

Update your CMakeLists.txt:

```cmake
# Old
find_package(QtForge 3.0 REQUIRED)

# New
find_package(QtForge 3.2 REQUIRED)

# Optional: Enable new features
set(QTFORGE_ENABLE_PYTHON_BINDINGS ON)
set(QTFORGE_ENABLE_LUA_BRIDGE ON)
```

### Step 2: Update Configuration Code

Replace configuration manager usage:

```cpp
// Old code
qtforge::ConfigurationManager config;
auto theme = config.getValue("ui.theme");
config.setValue("ui.theme", "dark");

// New code
auto config = qtforge::managers::create_configuration_manager();
auto theme = config->get_value("ui.theme", qtforge::managers::ConfigurationScope::User);
config->set_value("ui.theme", "dark", qtforge::managers::ConfigurationScope::User);
```

### Step 3: Update Security Configuration

Migrate security settings:

```cpp
// Old code
qtforge::SecurityManager security;
security.setSecurityLevel(qtforge::SecurityLevel::High);

// New code
qtforge::SecurityPolicy policy;
policy.name = "HighSecurityPolicy";
policy.minimumTrustLevel = qtforge::TrustLevel::High;
policy.allowedPermissions = {
    qtforge::PluginPermission::FileSystemRead,
    qtforge::PluginPermission::SystemInfo
};

qtforge::SecurityManager security;
security.setSecurityPolicy(policy);
```

### Step 4: Optional Enhancements

Consider upgrading to advanced interfaces:

```cpp
// Upgrade from IPlugin to IAdvancedPlugin
class MyPlugin : public qtforge::IAdvancedPlugin {
    // Implement new methods for enhanced functionality
    std::vector<qtforge::ServiceContract> getServiceContracts() override;
    qtforge::InterfaceCapability getInterfaceCapability() override;
};
```

## Testing Migration

### 1. Compile-Time Verification

Ensure your code compiles with v3.2.0:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make
```

### 2. Runtime Testing

Test plugin loading and functionality:

```cpp
// Test plugin loading
auto manager = qtforge::core::create_plugin_manager();
auto result = manager->load_plugin("path/to/plugin");

// Test configuration
auto config = qtforge::managers::create_configuration_manager();
config->set_value("test.key", "test.value", qtforge::managers::ConfigurationScope::Plugin);
auto value = config->get_value("test.key", qtforge::managers::ConfigurationScope::Plugin);
```

### 3. Integration Testing

Test multilingual plugin support:

```cpp
// Test Python plugin
auto pythonResult = manager->load_plugin("python_plugin.py");

// Test Lua plugin
auto luaResult = manager->load_plugin("lua_plugin.lua");

// Test cross-language communication
auto message = qtforge::communication::create_message("test", {{"data", "value"}});
messageBus->publish(message);
```

## Common Issues and Solutions

### Issue 1: Configuration API Compilation Errors

**Error:** `'getValue' is not a member of 'qtforge::ConfigurationManager'`

**Solution:** Update to new API with scoped access:

```cpp
// Replace
config.getValue("key")
// With
config->get_value("key", qtforge::managers::ConfigurationScope::Global)
```

### Issue 2: Security Policy Validation Failures

**Error:** Security policy validation fails at runtime

**Solution:** Use SecurityPolicyValidator to check policy:

```cpp
qtforge::SecurityPolicyValidator validator;
auto result = validator.validatePolicy(policy);
if (!result.isValid) {
    std::cerr << "Policy error: " << result.errorMessage << std::endl;
}
```

### Issue 3: Plugin Loading Failures

**Error:** Plugins fail to load with new security requirements

**Solution:** Update plugin metadata and permissions:

```json
{
  "name": "MyPlugin",
  "version": "1.0.0",
  "type": "native",
  "permissions": ["filesystem_read", "network_access"],
  "trust_level": "medium"
}
```

## Rollback Plan

If migration issues occur, you can rollback to v3.0.0:

1. **Revert CMakeLists.txt:** Change version requirement back to 3.0
2. **Revert API calls:** Use old configuration and security APIs
3. **Remove new features:** Comment out v3.2.0-specific code
4. **Rebuild:** Clean build with v3.0.0

## Support and Resources

- **Documentation:** [QtForge v3.2.0 Documentation](../index.md)
- **Examples:** [Migration Examples](../examples/migration-examples.md)
- **API Reference:** [v3.2.0 API Reference](../api/index.md)
- **Community:** [GitHub Discussions](https://github.com/AstroAir/QtForge/discussions)

---

_Migration Guide for QtForge v3.2.0 | Last Updated: September 2024_
