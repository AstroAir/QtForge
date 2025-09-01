# QtForge Sandbox System

## Overview

The QtForge Sandbox System provides comprehensive security and isolation for plugin execution. It includes resource monitoring, security enforcement, and process isolation to ensure safe execution of untrusted code.

## Features

### Core Components

1. **PluginSandbox** - Main sandbox class for individual plugin execution
2. **SandboxManager** - Singleton manager for multiple sandbox instances
3. **ResourceMonitor** - Cross-platform resource monitoring
4. **SecurityEnforcer** - Security policy enforcement and violation detection

### Security Levels

- **Unrestricted** - No restrictions (trusted native plugins)
- **Limited** - Basic restrictions (file system, network)
- **Sandboxed** - Full sandboxing with process isolation
- **Strict** - Maximum security with minimal permissions

### Resource Monitoring

- CPU time usage tracking
- Memory consumption monitoring
- Disk space usage tracking
- File handle counting
- Network connection monitoring
- Cross-platform implementation (Windows, Linux, macOS)

### Security Enforcement

- File system access validation
- Network access control
- Process creation restrictions
- System call monitoring
- API call blocking
- Directory and host whitelisting

## Usage Examples

### Basic Sandbox Creation

```cpp
#include "qtplugin/security/sandbox/plugin_sandbox.hpp"

// Create a sandbox with strict security policy
SecurityPolicy policy = SecurityPolicy::create_strict_policy();
auto sandbox = std::make_unique<PluginSandbox>(policy);

// Initialize the sandbox
auto result = sandbox->initialize();
if (result) {
    // Execute a plugin
    auto exec_result = sandbox->execute_plugin("/path/to/plugin", PluginType::Native);
    if (exec_result) {
        qDebug() << "Plugin started successfully";
    }
}

// Cleanup
sandbox->shutdown();
```

### Using SandboxManager

```cpp
// Get the singleton instance
SandboxManager& manager = SandboxManager::instance();

// Create a sandbox
auto policy = manager.get_policy("sandboxed").value();
auto sandbox_result = manager.create_sandbox("my_sandbox", policy);

if (sandbox_result) {
    auto sandbox = sandbox_result.value();
    // Use the sandbox...
}

// Cleanup
manager.remove_sandbox("my_sandbox");
```

### Custom Security Policy

```cpp
SecurityPolicy custom_policy;
custom_policy.level = SandboxSecurityLevel::Limited;
custom_policy.policy_name = "custom_policy";

// Set resource limits
custom_policy.limits.memory_limit_mb = 512;
custom_policy.limits.cpu_time_limit = std::chrono::minutes(5);

// Set permissions
custom_policy.permissions.allow_file_system_read = true;
custom_policy.permissions.allow_network_access = true;
custom_policy.permissions.allowed_directories = {"/tmp", "/var/cache"};
custom_policy.permissions.allowed_hosts = {"api.example.com", "*.trusted.com"};

// Register the policy
SandboxManager::instance().register_policy("custom", custom_policy);
```

### Resource Monitoring

```cpp
auto sandbox = std::make_unique<PluginSandbox>(policy);

// Connect to monitoring signals
connect(sandbox.get(), &PluginSandbox::resource_usage_updated,
        [](const ResourceUsage& usage) {
            qDebug() << "Memory usage:" << usage.memory_used_mb << "MB";
            qDebug() << "CPU time:" << usage.cpu_time_used.count() << "ms";
        });

connect(sandbox.get(), &PluginSandbox::resource_limit_exceeded,
        [](const QString& resource, const QJsonObject& usage) {
            qWarning() << "Resource limit exceeded:" << resource;
        });
```

## Architecture

### Class Hierarchy

```
PluginSandbox (QObject)
├── ResourceMonitor
├── SecurityEnforcer
└── QProcess (managed)

SandboxManager (QObject, Singleton)
├── std::unordered_map<QString, std::shared_ptr<PluginSandbox>>
└── std::unordered_map<QString, SecurityPolicy>
```

### Security Policy Structure

```cpp
struct SecurityPolicy {
    SandboxSecurityLevel level;
    ResourceLimits limits;
    SecurityPermissions permissions;
    QString policy_name;
    QString description;
};

struct ResourceLimits {
    std::chrono::milliseconds cpu_time_limit;
    size_t memory_limit_mb;
    size_t disk_space_limit_mb;
    int max_file_handles;
    int max_network_connections;
    std::chrono::milliseconds execution_timeout;
};

struct SecurityPermissions {
    bool allow_file_system_read;
    bool allow_file_system_write;
    bool allow_network_access;
    bool allow_process_creation;
    bool allow_system_calls;
    bool allow_registry_access;
    bool allow_environment_access;
    QStringList allowed_directories;
    QStringList allowed_hosts;
    QStringList blocked_apis;
};
```

## Platform Support

### Windows
- Performance Counters for resource monitoring
- Job Objects for process isolation
- Registry access control
- Handle counting

### Linux
- /proc filesystem for resource monitoring
- cgroups for resource limits
- File descriptor counting
- Process isolation using namespaces

### macOS
- Mach APIs for resource monitoring
- BSD resource limits
- Process isolation

## Thread Safety

All sandbox components are thread-safe:
- QMutex protection for shared data
- Signal-slot communication for async operations
- Atomic operations for counters

## Error Handling

The system uses the `qtplugin::expected<T, PluginError>` pattern for robust error handling:

```cpp
auto result = sandbox->execute_plugin(path, type);
if (!result) {
    qWarning() << "Execution failed:" << result.error().message.c_str();
    return;
}
```

## Configuration

Security policies can be loaded from JSON:

```json
{
    "level": 2,
    "policy_name": "web_plugin",
    "description": "Policy for web-based plugins",
    "limits": {
        "cpu_time_limit": 300000,
        "memory_limit_mb": 256,
        "execution_timeout": 120000
    },
    "permissions": {
        "allow_network_access": true,
        "allowed_hosts": ["api.example.com", "*.cdn.com"],
        "blocked_apis": ["system", "exec"]
    }
}
```

## Performance Considerations

- Resource monitoring runs at 1-second intervals by default
- Security checks are cached for frequently accessed resources
- Process isolation adds minimal overhead (~5-10ms startup time)
- Memory overhead: ~2-5MB per sandbox instance

## Best Practices

1. **Choose appropriate security levels** based on plugin trust level
2. **Set reasonable resource limits** to prevent resource exhaustion
3. **Use whitelisting** for directories and network hosts
4. **Monitor security events** for suspicious activity
5. **Clean up sandboxes** when no longer needed
6. **Test policies** with representative workloads

## Troubleshooting

### Common Issues

1. **Sandbox initialization fails**
   - Check platform support
   - Verify permissions
   - Review system resources

2. **Resource monitoring not working**
   - Ensure platform-specific APIs are available
   - Check process permissions
   - Verify /proc filesystem access (Linux)

3. **Security violations**
   - Review security policy settings
   - Check allowed directories/hosts
   - Verify API blocking configuration

### Debug Logging

Enable debug logging for detailed information:

```cpp
QLoggingCategory::setFilterRules("qtplugin.sandbox.debug=true");
```

## Future Enhancements

- Container-based isolation (Docker/Podman)
- GPU resource monitoring
- Network traffic analysis
- Machine learning-based anomaly detection
- Policy templates for common use cases
