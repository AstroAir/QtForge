# SecurityManager

The `SecurityManager` class provides comprehensive security features for QtForge applications, including plugin validation, permission management, and threat detection.

## Overview

The SecurityManager is responsible for:
- Plugin signature verification
- Permission-based access control
- Security policy enforcement
- Threat detection and prevention
- Audit logging and compliance
- Secure communication channels

## Class Declaration

```cpp
namespace qtforge::security {

class QTFORGE_EXPORT SecurityManager {
public:
    // Construction and destruction
    SecurityManager();
    explicit SecurityManager(const SecurityConfig& config);
    ~SecurityManager();
    
    // Singleton access
    static SecurityManager& instance();
    
    // Configuration
    void setConfig(const SecurityConfig& config);
    const SecurityConfig& getConfig() const;
    
    // Plugin validation
    expected<bool, Error> validatePlugin(const std::string& pluginPath);
    expected<PluginSecurityInfo, Error> getPluginSecurityInfo(const std::string& pluginPath);
    expected<void, Error> verifyPluginSignature(const std::string& pluginPath);
    
    // Permission management
    expected<void, Error> grantPermission(const std::string& pluginName, 
                                        Permission permission, 
                                        const std::string& resource = "");
    expected<void, Error> revokePermission(const std::string& pluginName, 
                                         Permission permission, 
                                         const std::string& resource = "");
    bool hasPermission(const std::string& pluginName, 
                      Permission permission, 
                      const std::string& resource = "") const;
    
    // Trust management
    expected<void, Error> addTrustedPublisher(const std::string& publisher, 
                                            const std::string& certificatePath);
    expected<void, Error> removeTrustedPublisher(const std::string& publisher);
    bool isTrustedPublisher(const std::string& publisher) const;
    
    // Security policies
    void setSecurityPolicy(const std::string& policyName, const SecurityPolicy& policy);
    SecurityPolicy getSecurityPolicy(const std::string& policyName) const;
    expected<void, Error> enforcePolicy(const std::string& pluginName, 
                                      const std::string& policyName);
    
    // Threat detection
    void enableThreatDetection(bool enabled);
    bool isThreatDetectionEnabled() const;
    std::vector<SecurityThreat> getDetectedThreats() const;
    expected<void, Error> reportThreat(const SecurityThreat& threat);
    
    // Audit logging
    void enableAuditLogging(bool enabled);
    bool isAuditLoggingEnabled() const;
    void logSecurityEvent(const SecurityEvent& event);
    std::vector<SecurityEvent> getAuditLog(const TimeRange& timeRange) const;
    
    // Sandboxing
    expected<void, Error> createSandbox(const std::string& pluginName, 
                                      const SandboxConfig& config);
    expected<void, Error> destroySandbox(const std::string& pluginName);
    bool isPluginSandboxed(const std::string& pluginName) const;
    
private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

} // namespace qtforge::security
```

## Core Methods

### Plugin Validation

#### validatePlugin()

Validates a plugin against security policies and requirements.

```cpp
auto& securityManager = qtforge::SecurityManager::instance();

auto result = securityManager.validatePlugin("MyPlugin.qtplugin");
if (result && result.value()) {
    std::cout << "Plugin is valid and secure" << std::endl;
} else if (result) {
    std::cout << "Plugin validation failed" << std::endl;
} else {
    std::cerr << "Validation error: " << result.error().message() << std::endl;
}
```

#### verifyPluginSignature()

Verifies the digital signature of a plugin.

```cpp
auto result = securityManager.verifyPluginSignature("MyPlugin.qtplugin");
if (result) {
    std::cout << "Plugin signature is valid" << std::endl;
} else {
    std::cerr << "Signature verification failed: " << result.error().message() << std::endl;
}
```

### Permission Management

#### grantPermission()

Grants a specific permission to a plugin.

```cpp
// Grant file read permission
auto result = securityManager.grantPermission("MyPlugin", 
    qtforge::Permission::FileRead, "./data/");
if (!result) {
    std::cerr << "Failed to grant permission: " << result.error().message() << std::endl;
}

// Grant network access permission
securityManager.grantPermission("NetworkPlugin", 
    qtforge::Permission::NetworkAccess, "api.example.com:443");
```

#### hasPermission()

Checks if a plugin has a specific permission.

```cpp
bool canReadFiles = securityManager.hasPermission("MyPlugin", 
    qtforge::Permission::FileRead, "./data/input.txt");

if (canReadFiles) {
    // Proceed with file operation
    std::ifstream file("./data/input.txt");
    // ...
} else {
    std::cerr << "Plugin does not have file read permission" << std::endl;
}
```

## Security Configuration

### SecurityConfig

Configuration options for the SecurityManager:

```cpp
struct SecurityConfig {
    // Security level
    SecurityLevel level = SecurityLevel::Medium;
    
    // Plugin validation
    bool requireSignatures = true;
    bool validateCertificateChain = true;
    bool checkCertificateRevocation = false;
    
    // Trust management
    std::vector<std::string> trustedPublishers;
    std::string certificateStore = "./certificates/";
    
    // Sandboxing
    bool enableSandbox = true;
    SandboxConfig defaultSandboxConfig;
    
    // Threat detection
    bool enableThreatDetection = true;
    ThreatDetectionConfig threatDetectionConfig;
    
    // Audit logging
    bool enableAuditLogging = true;
    std::string auditLogPath = "./logs/security_audit.log";
    AuditLevel auditLevel = AuditLevel::Standard;
    
    // Permission defaults
    std::map<std::string, std::vector<Permission>> defaultPermissions;
};
```

### Usage Example

```cpp
qtforge::SecurityConfig config;
config.level = qtforge::SecurityLevel::High;
config.requireSignatures = true;
config.enableSandbox = true;
config.enableThreatDetection = true;
config.enableAuditLogging = true;

qtforge::SecurityManager securityManager(config);
```

## Permission System

### Permission Types

```cpp
enum class Permission {
    // File system
    FileRead,
    FileWrite,
    FileExecute,
    DirectoryCreate,
    DirectoryDelete,
    
    // Network
    NetworkAccess,
    NetworkListen,
    NetworkConnect,
    
    // System
    SystemCall,
    ProcessCreate,
    ProcessKill,
    
    // Plugin communication
    PluginCommunication,
    MessageBusAccess,
    ServiceAccess,
    
    // Configuration
    ConfigurationRead,
    ConfigurationWrite,
    
    // Hardware
    CameraAccess,
    MicrophoneAccess,
    LocationAccess,
    
    // Database
    DatabaseRead,
    DatabaseWrite,
    DatabaseAdmin
};
```

### Permission Management Example

```cpp
class SecurePlugin : public qtforge::IPlugin {
public:
    qtforge::expected<void, qtforge::Error> initialize() override {
        auto& securityManager = qtforge::SecurityManager::instance();
        
        // Request required permissions
        auto filePermission = securityManager.grantPermission(name(), 
            qtforge::Permission::FileRead, "./data/");
        if (!filePermission) {
            return qtforge::Error("File access permission denied");
        }
        
        auto networkPermission = securityManager.grantPermission(name(),
            qtforge::Permission::NetworkAccess, "api.example.com:443");
        if (!networkPermission) {
            return qtforge::Error("Network access permission denied");
        }
        
        return {};
    }
    
    qtforge::expected<std::string, qtforge::Error> readFile(const std::string& filename) {
        auto& securityManager = qtforge::SecurityManager::instance();
        
        // Check permission before file access
        if (!securityManager.hasPermission(name(), qtforge::Permission::FileRead, filename)) {
            return qtforge::Error("Permission denied: " + filename);
        }
        
        // Perform file operation
        std::ifstream file(filename);
        if (!file.is_open()) {
            return qtforge::Error("Failed to open file: " + filename);
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return content;
    }
};
```

## Trust Management

### Publisher Trust

```cpp
class TrustManager {
public:
    // Add trusted publisher
    expected<void, Error> addTrustedPublisher(const std::string& publisher, 
                                            const std::string& certificatePath) {
        try {
            // Load and validate certificate
            auto cert = loadCertificate(certificatePath);
            if (!cert) {
                return Error("Failed to load certificate: " + certificatePath);
            }
            
            // Verify certificate chain
            if (!verifyCertificateChain(cert.value())) {
                return Error("Certificate chain verification failed");
            }
            
            // Add to trusted publishers
            trustedPublishers_[publisher] = cert.value();
            
            // Log trust event
            SecurityEvent event;
            event.type = SecurityEventType::TrustAdded;
            event.publisher = publisher;
            event.timestamp = std::chrono::system_clock::now();
            
            securityManager_.logSecurityEvent(event);
            
            return {};
            
        } catch (const std::exception& e) {
            return Error("Failed to add trusted publisher: " + std::string(e.what()));
        }
    }
    
    bool isTrustedPublisher(const std::string& publisher) const {
        return trustedPublishers_.find(publisher) != trustedPublishers_.end();
    }

private:
    std::map<std::string, Certificate> trustedPublishers_;
    SecurityManager& securityManager_;
};
```

## Threat Detection

### Threat Detection System

```cpp
class ThreatDetector {
public:
    struct ThreatDetectionConfig {
        bool enableBehaviorAnalysis = true;
        bool enableAnomalyDetection = true;
        bool enableSignatureDetection = true;
        
        // Thresholds
        double anomalyThreshold = 0.8;
        int maxFileOperationsPerSecond = 100;
        int maxNetworkConnectionsPerMinute = 50;
        
        // Monitoring intervals
        std::chrono::seconds monitoringInterval = std::chrono::seconds(10);
        std::chrono::minutes reportingInterval = std::chrono::minutes(5);
    };
    
    void detectThreats(const std::string& pluginName) {
        // Behavior analysis
        if (config_.enableBehaviorAnalysis) {
            analyzeBehavior(pluginName);
        }
        
        // Anomaly detection
        if (config_.enableAnomalyDetection) {
            detectAnomalies(pluginName);
        }
        
        // Signature-based detection
        if (config_.enableSignatureDetection) {
            checkThreatSignatures(pluginName);
        }
    }
    
private:
    void analyzeBehavior(const std::string& pluginName) {
        auto metrics = getPluginMetrics(pluginName);
        
        // Check for suspicious file operations
        if (metrics.fileOperationsPerSecond > config_.maxFileOperationsPerSecond) {
            SecurityThreat threat;
            threat.type = ThreatType::SuspiciousFileActivity;
            threat.pluginName = pluginName;
            threat.severity = ThreatSeverity::Medium;
            threat.description = "Excessive file operations detected";
            threat.timestamp = std::chrono::system_clock::now();
            
            reportThreat(threat);
        }
        
        // Check for suspicious network activity
        if (metrics.networkConnectionsPerMinute > config_.maxNetworkConnectionsPerMinute) {
            SecurityThreat threat;
            threat.type = ThreatType::SuspiciousNetworkActivity;
            threat.pluginName = pluginName;
            threat.severity = ThreatSeverity::High;
            threat.description = "Excessive network connections detected";
            threat.timestamp = std::chrono::system_clock::now();
            
            reportThreat(threat);
        }
    }
    
    ThreatDetectionConfig config_;
};
```

## Sandboxing

### Sandbox Configuration

```cpp
struct SandboxConfig {
    // File system restrictions
    std::vector<std::string> allowedDirectories;
    std::vector<std::string> readOnlyDirectories;
    std::vector<std::string> blockedDirectories;
    
    // Network restrictions
    std::vector<std::string> allowedHosts;
    std::vector<int> allowedPorts;
    bool blockOutboundConnections = false;
    
    // System call restrictions
    std::vector<std::string> allowedSystemCalls;
    std::vector<std::string> blockedSystemCalls;
    
    // Resource limits
    size_t maxMemoryUsage = 256 * 1024 * 1024; // 256MB
    double maxCpuUsage = 25.0; // 25%
    int maxFileDescriptors = 100;
    
    // Process restrictions
    bool allowProcessCreation = false;
    bool allowThreadCreation = true;
    int maxThreads = 10;
};
```

### Sandbox Usage

```cpp
// Create sandbox for plugin
qtforge::SandboxConfig sandboxConfig;
sandboxConfig.allowedDirectories = {"./plugins/data/", "./temp/"};
sandboxConfig.allowedHosts = {"api.example.com"};
sandboxConfig.allowedPorts = {80, 443};
sandboxConfig.maxMemoryUsage = 128 * 1024 * 1024; // 128MB

auto result = securityManager.createSandbox("MyPlugin", sandboxConfig);
if (!result) {
    std::cerr << "Failed to create sandbox: " << result.error().message() << std::endl;
}
```

## Audit Logging

### Security Events

```cpp
enum class SecurityEventType {
    PluginLoaded,
    PluginUnloaded,
    PermissionGranted,
    PermissionDenied,
    ThreatDetected,
    PolicyViolation,
    TrustAdded,
    TrustRevoked,
    SandboxCreated,
    SandboxViolation
};

struct SecurityEvent {
    SecurityEventType type;
    std::string pluginName;
    std::string publisher;
    std::string description;
    std::chrono::system_clock::time_point timestamp;
    std::map<std::string, std::string> metadata;
};
```

### Audit Logging Example

```cpp
// Log security event
SecurityEvent event;
event.type = SecurityEventType::PermissionGranted;
event.pluginName = "MyPlugin";
event.description = "File read permission granted for ./data/";
event.timestamp = std::chrono::system_clock::now();
event.metadata["permission"] = "FileRead";
event.metadata["resource"] = "./data/";

securityManager.logSecurityEvent(event);

// Retrieve audit log
auto timeRange = TimeRange{
    std::chrono::system_clock::now() - std::chrono::hours(24),
    std::chrono::system_clock::now()
};

auto auditLog = securityManager.getAuditLog(timeRange);
for (const auto& event : auditLog) {
    std::cout << "Event: " << static_cast<int>(event.type) 
              << " Plugin: " << event.pluginName 
              << " Time: " << formatTime(event.timestamp) << std::endl;
}
```

## Best Practices

1. **Principle of Least Privilege**: Grant minimal required permissions
2. **Defense in Depth**: Use multiple security layers
3. **Regular Audits**: Monitor and review security events regularly
4. **Certificate Management**: Keep certificates up to date
5. **Threat Intelligence**: Stay informed about security threats

## Error Handling

### Common Security Errors

```cpp
enum class SecurityError {
    InvalidSignature,
    UntrustedPublisher,
    PermissionDenied,
    PolicyViolation,
    CertificateExpired,
    SandboxViolation,
    ThreatDetected
};
```

### Error Handling Example

```cpp
auto result = securityManager.validatePlugin("suspicious_plugin.qtplugin");
if (!result) {
    auto error = result.error();
    switch (error.code()) {
        case SecurityError::InvalidSignature:
            std::cerr << "Plugin signature is invalid" << std::endl;
            break;
        case SecurityError::UntrustedPublisher:
            std::cerr << "Plugin publisher is not trusted" << std::endl;
            break;
        case SecurityError::ThreatDetected:
            std::cerr << "Security threat detected in plugin" << std::endl;
            break;
        default:
            std::cerr << "Security validation failed: " << error.message() << std::endl;
    }
}
```

## See Also

- **[Security Configuration](../../user-guide/security-configuration.md)**: Security configuration guide
- **[Plugin Validation](plugin-validator.md)**: Plugin validation utilities
- **[Permission Manager](permission-manager.md)**: Permission management system
- **[Threat Detection](threat-detector.md)**: Threat detection system
