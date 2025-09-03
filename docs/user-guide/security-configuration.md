# Security Configuration

QtForge provides comprehensive security features to protect your application from malicious plugins and ensure safe plugin execution.

## Overview

QtForge security system includes:
- Plugin signature verification
- Sandboxed execution environments
- Permission-based access control
- Trust management and publisher verification
- Resource usage limits
- Secure communication channels

## Security Levels

QtForge supports multiple security levels to balance security and functionality:

### None
- No security checks performed
- All plugins are trusted
- Maximum performance, minimum security
- **Use only in trusted development environments**

```cpp
qtforge::SecurityConfig config;
config.level = qtforge::SecurityLevel::None;
```

### Low
- Basic plugin validation
- File format verification
- Simple integrity checks
- Suitable for internal applications

```cpp
qtforge::SecurityConfig config;
config.level = qtforge::SecurityLevel::Low;
config.validateFileFormat = true;
config.checkBasicIntegrity = true;
```

### Medium (Default)
- Plugin signature verification
- Publisher trust validation
- Resource usage monitoring
- Sandboxed execution
- Recommended for most applications

```cpp
qtforge::SecurityConfig config;
config.level = qtforge::SecurityLevel::Medium;
config.requireSignatures = true;
config.enableSandbox = true;
config.monitorResources = true;
```

### High
- Strict signature verification
- Mandatory trusted publishers
- Enhanced sandboxing
- Detailed audit logging
- For security-critical applications

```cpp
qtforge::SecurityConfig config;
config.level = qtforge::SecurityLevel::High;
config.requireTrustedPublishers = true;
config.enableAuditLogging = true;
config.strictSandbox = true;
```

### Maximum
- Maximum security measures
- Whitelist-only plugin loading
- Comprehensive monitoring
- Real-time threat detection
- For high-security environments

```cpp
qtforge::SecurityConfig config;
config.level = qtforge::SecurityLevel::Maximum;
config.whitelistOnly = true;
config.enableThreatDetection = true;
config.realTimeMonitoring = true;
```

## Plugin Signatures

### Generating Signatures

Create digital signatures for your plugins:

```bash
# Generate key pair
qtforge-keygen --generate-keypair --output-dir ./keys

# Sign plugin
qtforge-sign --plugin MyPlugin.qtplugin --private-key ./keys/private.pem --output MyPlugin.signed.qtplugin
```

### Signature Verification

Configure signature verification:

```cpp
qtforge::SecurityManager securityManager;

// Add trusted public keys
securityManager.addTrustedKey("./keys/public.pem");
securityManager.addTrustedKey("./keys/company_public.pem");

// Verify plugin signature
auto result = securityManager.verifySignature("MyPlugin.signed.qtplugin");
if (result && result.value()) {
    std::cout << "Plugin signature is valid" << std::endl;
} else {
    std::cerr << "Invalid plugin signature" << std::endl;
}
```

### Code Signing Certificate

Use code signing certificates for production:

```cpp
qtforge::SecurityConfig config;
config.certificateStore = "./certificates/";
config.requireCodeSigningCertificate = true;
config.validateCertificateChain = true;
config.checkCertificateRevocation = true;
```

## Trust Management

### Publisher Trust

Manage trusted plugin publishers:

```cpp
qtforge::TrustManager trustManager;

// Add trusted publishers
trustManager.addTrustedPublisher("Acme Corporation", "./certs/acme_cert.pem");
trustManager.addTrustedPublisher("Trusted Software Inc.", "./certs/trusted_cert.pem");

// Check publisher trust
bool isTrusted = trustManager.isPublisherTrusted("Acme Corporation");
if (isTrusted) {
    std::cout << "Publisher is trusted" << std::endl;
}

// Revoke trust
trustManager.revokeTrust("Untrusted Publisher");
```

### Plugin Whitelist

Maintain a whitelist of approved plugins:

```cpp
qtforge::SecurityConfig config;
config.enableWhitelist = true;
config.whitelistFile = "./config/plugin_whitelist.json";

// Whitelist format (plugin_whitelist.json)
{
    "approved_plugins": [
        {
            "name": "CorePlugin",
            "version": ">=1.0.0",
            "publisher": "QtForge Team",
            "hash": "sha256:abc123..."
        },
        {
            "name": "DataProcessor",
            "version": ">=2.1.0",
            "publisher": "Acme Corporation",
            "hash": "sha256:def456..."
        }
    ]
}
```

## Sandboxing

### Basic Sandbox

Enable basic sandboxing for plugin isolation:

```cpp
qtforge::SandboxConfig sandboxConfig;
sandboxConfig.enabled = true;
sandboxConfig.isolateFileSystem = true;
sandboxConfig.isolateNetwork = true;
sandboxConfig.isolateProcesses = true;

qtforge::SecurityManager securityManager;
securityManager.setSandboxConfig(sandboxConfig);
```

### Advanced Sandbox

Configure advanced sandboxing options:

```cpp
qtforge::SandboxConfig sandboxConfig;
sandboxConfig.enabled = true;

// File system restrictions
sandboxConfig.allowedDirectories = {
    "./plugins/data/",
    "./temp/",
    "/tmp/"
};
sandboxConfig.readOnlyDirectories = {
    "./config/",
    "./resources/"
};
sandboxConfig.blockedDirectories = {
    "/etc/",
    "/usr/bin/",
    "C:\\Windows\\System32\\"
};

// Network restrictions
sandboxConfig.allowedHosts = {
    "api.example.com",
    "cdn.example.com"
};
sandboxConfig.allowedPorts = {80, 443, 8080};
sandboxConfig.blockOutboundConnections = false;

// System call restrictions
sandboxConfig.allowedSystemCalls = {
    "read", "write", "open", "close",
    "socket", "connect", "send", "recv"
};
sandboxConfig.blockedSystemCalls = {
    "exec", "fork", "kill", "ptrace"
};
```

## Permission System

### Permission Types

Define granular permissions for plugins:

```cpp
enum class Permission {
    FileRead,
    FileWrite,
    FileExecute,
    NetworkAccess,
    SystemCall,
    PluginCommunication,
    ConfigurationAccess,
    DatabaseAccess,
    CameraAccess,
    MicrophoneAccess
};
```

### Permission Management

Manage plugin permissions:

```cpp
qtforge::PermissionManager permissionManager;

// Grant permissions to plugin
permissionManager.grantPermission("MyPlugin", Permission::FileRead, "./data/");
permissionManager.grantPermission("MyPlugin", Permission::NetworkAccess, "api.example.com:443");

// Check permissions
bool hasPermission = permissionManager.hasPermission("MyPlugin", Permission::FileWrite, "./output/");
if (!hasPermission) {
    std::cerr << "Plugin does not have write permission" << std::endl;
}

// Revoke permissions
permissionManager.revokePermission("MyPlugin", Permission::SystemCall);
```

### Permission Requests

Plugins can request permissions at runtime:

```cpp
class SecurePlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        auto& permissionManager = qtforge::PermissionManager::instance();
        
        // Request file access permission
        auto filePermission = permissionManager.requestPermission(
            name(), Permission::FileRead, "./data/input.txt");
        if (!filePermission) {
            return qtforge::Error("File access permission denied");
        }
        
        // Request network access permission
        auto networkPermission = permissionManager.requestPermission(
            name(), Permission::NetworkAccess, "api.example.com:443");
        if (!networkPermission) {
            return qtforge::Error("Network access permission denied");
        }
        
        return {};
    }
};
```

## Resource Limits

### Memory Limits

Set memory usage limits for plugins:

```cpp
qtforge::ResourceLimits limits;
limits.maxMemoryUsage = 256 * 1024 * 1024; // 256MB
limits.maxHeapSize = 128 * 1024 * 1024;    // 128MB
limits.enableMemoryMonitoring = true;

qtforge::SecurityManager securityManager;
securityManager.setResourceLimits("MyPlugin", limits);
```

### CPU Limits

Control CPU usage:

```cpp
qtforge::ResourceLimits limits;
limits.maxCpuUsage = 25.0; // 25% CPU usage
limits.maxExecutionTime = std::chrono::minutes(5);
limits.enableCpuThrottling = true;

securityManager.setResourceLimits("MyPlugin", limits);
```

### Network Limits

Limit network usage:

```cpp
qtforge::ResourceLimits limits;
limits.maxNetworkBandwidth = 1024 * 1024; // 1MB/s
limits.maxConnections = 10;
limits.connectionTimeout = std::chrono::seconds(30);

securityManager.setResourceLimits("MyPlugin", limits);
```

## Secure Communication

### Encrypted Messaging

Enable encrypted communication between plugins:

```cpp
qtforge::SecureMessageBus messageBus;

// Configure encryption
qtforge::EncryptionConfig encConfig;
encConfig.algorithm = qtforge::EncryptionAlgorithm::AES256;
encConfig.keyExchange = qtforge::KeyExchange::ECDH;
encConfig.enablePerfectForwardSecrecy = true;

messageBus.setEncryptionConfig(encConfig);

// Send encrypted message
qtforge::SecureMessage message;
message.recipient = "TrustedPlugin";
message.content = "Sensitive data";
message.requireEncryption = true;

messageBus.publish("secure.channel", message);
```

### Message Authentication

Ensure message integrity and authenticity:

```cpp
qtforge::MessageAuthConfig authConfig;
authConfig.enableAuthentication = true;
authConfig.algorithm = qtforge::AuthAlgorithm::HMAC_SHA256;
authConfig.requireSignatures = true;

messageBus.setAuthenticationConfig(authConfig);
```

## Audit Logging

### Security Events

Log security-related events:

```cpp
qtforge::AuditLogger auditLogger;

// Configure audit logging
qtforge::AuditConfig auditConfig;
auditConfig.logFile = "./logs/security_audit.log";
auditConfig.logLevel = qtforge::AuditLevel::Detailed;
auditConfig.enableRealTimeAlerts = true;

auditLogger.setConfig(auditConfig);

// Log security events automatically
auditLogger.logPluginLoad("MyPlugin", "user123", true);
auditLogger.logPermissionRequest("MyPlugin", Permission::FileWrite, "./sensitive/", false);
auditLogger.logSecurityViolation("UntrustedPlugin", "Attempted unauthorized file access");
```

### Compliance Reporting

Generate compliance reports:

```cpp
qtforge::ComplianceReporter reporter;

// Generate security report
auto report = reporter.generateSecurityReport(
    std::chrono::system_clock::now() - std::chrono::hours(24),
    std::chrono::system_clock::now()
);

// Export to various formats
reporter.exportToJson(report, "./reports/security_report.json");
reporter.exportToPdf(report, "./reports/security_report.pdf");
reporter.exportToCsv(report, "./reports/security_report.csv");
```

## Best Practices

### Development

1. **Start with Medium security level** for most applications
2. **Use code signing** for production plugins
3. **Implement least privilege principle** - grant minimal required permissions
4. **Regular security audits** of plugin code and configurations
5. **Keep security configurations up to date**

### Deployment

1. **Validate all plugins** before deployment
2. **Use trusted publishers only** in production
3. **Monitor resource usage** continuously
4. **Enable audit logging** for compliance
5. **Regular security updates** and patches

### Monitoring

1. **Real-time threat detection** for suspicious activities
2. **Resource usage monitoring** to prevent abuse
3. **Network traffic analysis** for unauthorized communications
4. **File system monitoring** for unauthorized access
5. **Regular security assessments** and penetration testing

## Troubleshooting

### Common Security Issues

**Plugin signature verification failed**
```cpp
// Check certificate validity
auto certResult = securityManager.validateCertificate("./certs/plugin_cert.pem");
if (!certResult) {
    std::cerr << "Certificate validation failed: " << certResult.error().message() << std::endl;
}
```

**Permission denied errors**
```cpp
// Debug permission issues
auto permissions = permissionManager.getPluginPermissions("MyPlugin");
for (const auto& permission : permissions) {
    std::cout << "Permission: " << permission.type << " -> " << permission.resource << std::endl;
}
```

**Sandbox violations**
```cpp
// Check sandbox configuration
auto sandboxStatus = securityManager.getSandboxStatus("MyPlugin");
if (sandboxStatus.hasViolations()) {
    for (const auto& violation : sandboxStatus.violations()) {
        std::cerr << "Sandbox violation: " << violation.description << std::endl;
    }
}
```

## See Also

- **[Security Manager API](../api/security/security-manager.md)**: Security API reference
- **[Plugin Development Guide](plugin-development.md)**: Secure plugin development
- **[Advanced Security](advanced-security.md)**: Advanced security topics
- **[Compliance Guide](../compliance/security-compliance.md)**: Security compliance requirements
