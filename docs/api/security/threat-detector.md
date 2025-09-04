# Threat Detector

The Threat Detector provides real-time security monitoring and threat detection for plugins and system operations.

## Overview

The Threat Detector offers:
- **Real-time Monitoring**: Continuous security monitoring
- **Threat Detection**: Identify potential security threats
- **Behavioral Analysis**: Monitor plugin behavior patterns
- **Anomaly Detection**: Detect unusual activities
- **Response Actions**: Automatic threat response and mitigation

## Class Reference

### ThreatDetector

```cpp
class ThreatDetector {
public:
    // Detection control
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;
    
    // Threat detection
    void scanPlugin(const QString& pluginId);
    void scanAllPlugins();
    ThreatLevel assessThreat(const QString& pluginId) const;
    
    // Rule management
    bool addDetectionRule(std::unique_ptr<DetectionRule> rule);
    bool removeDetectionRule(const QString& ruleId);
    QList<DetectionRule*> getActiveRules() const;
    
    // Response configuration
    void setResponseAction(ThreatLevel level, ResponseAction action);
    ResponseAction getResponseAction(ThreatLevel level) const;
    
    // Monitoring configuration
    void setMonitoringInterval(int intervalMs);
    void setSensitivityLevel(SensitivityLevel level);
    
signals:
    void threatDetected(const QString& pluginId, ThreatLevel level, const QString& description);
    void anomalyDetected(const QString& pluginId, const QString& anomaly);
    void responseTriggered(const QString& pluginId, ResponseAction action);
};
```

### DetectionRule

```cpp
class DetectionRule {
public:
    virtual ~DetectionRule() = default;
    
    // Rule identification
    virtual QString getId() const = 0;
    virtual QString getName() const = 0;
    virtual QString getDescription() const = 0;
    
    // Rule evaluation
    virtual bool evaluate(const PluginContext& context) const = 0;
    virtual ThreatLevel getThreatLevel() const = 0;
    
    // Rule configuration
    virtual void configure(const QVariantMap& config) {}
    virtual QVariantMap getConfiguration() const { return {}; }
};
```

## Usage Examples

### Basic Threat Detection

```cpp
ThreatDetector* detector = ThreatDetector::instance();

// Start monitoring
detector->startMonitoring();

// Connect to threat detection signals
connect(detector, &ThreatDetector::threatDetected,
        [](const QString& pluginId, ThreatLevel level, const QString& description) {
    qWarning() << "Threat detected in plugin" << pluginId 
               << "Level:" << level << "Description:" << description;
});

// Scan specific plugin
detector->scanPlugin("com.suspicious.plugin");
```

### Custom Detection Rule

```cpp
class SuspiciousFileAccessRule : public DetectionRule {
public:
    QString getId() const override { return "suspicious_file_access"; }
    QString getName() const override { return "Suspicious File Access"; }
    QString getDescription() const override { 
        return "Detects plugins accessing sensitive system files"; 
    }
    
    bool evaluate(const PluginContext& context) const override {
        const auto& fileAccesses = context.getFileAccesses();
        
        for (const QString& file : fileAccesses) {
            if (file.startsWith("/etc/") || file.startsWith("/sys/")) {
                return true; // Threat detected
            }
        }
        return false;
    }
    
    ThreatLevel getThreatLevel() const override {
        return ThreatLevel::Medium;
    }
};

// Add custom rule
auto rule = std::make_unique<SuspiciousFileAccessRule>();
detector->addDetectionRule(std::move(rule));
```

### Response Configuration

```cpp
// Configure automatic responses
detector->setResponseAction(ThreatLevel::Low, ResponseAction::Log);
detector->setResponseAction(ThreatLevel::Medium, ResponseAction::Quarantine);
detector->setResponseAction(ThreatLevel::High, ResponseAction::Terminate);
detector->setResponseAction(ThreatLevel::Critical, ResponseAction::Block);
```

### Behavioral Monitoring

```cpp
// Set monitoring parameters
detector->setMonitoringInterval(1000); // Check every second
detector->setSensitivityLevel(SensitivityLevel::High);

// Monitor for anomalies
connect(detector, &ThreatDetector::anomalyDetected,
        [](const QString& pluginId, const QString& anomaly) {
    qInfo() << "Anomaly detected in plugin" << pluginId << ":" << anomaly;
});
```

## Threat Levels

The system recognizes the following threat levels:

- **None**: No threat detected
- **Low**: Minor security concern
- **Medium**: Moderate security risk
- **High**: Significant security threat
- **Critical**: Severe security threat requiring immediate action

## Response Actions

Available response actions include:

- **Log**: Log the threat for review
- **Warn**: Display warning to user
- **Quarantine**: Isolate the plugin
- **Terminate**: Stop the plugin immediately
- **Block**: Prevent plugin from loading

## Built-in Detection Rules

### File System Rules
- **Sensitive File Access**: Detects access to system configuration files
- **Excessive File Operations**: Monitors for unusual file activity
- **Unauthorized Directory Access**: Checks for access to restricted directories

### Network Rules
- **Suspicious Connections**: Monitors for connections to known malicious hosts
- **Data Exfiltration**: Detects large data transfers
- **Port Scanning**: Identifies port scanning activities

### System Rules
- **Process Injection**: Detects attempts to inject code into other processes
- **Registry Manipulation**: Monitors Windows registry modifications
- **Privilege Escalation**: Identifies attempts to gain elevated privileges

### Behavioral Rules
- **Resource Abuse**: Monitors excessive CPU, memory, or disk usage
- **API Abuse**: Detects unusual API call patterns
- **Timing Attacks**: Identifies suspicious timing patterns

## Configuration

### Sensitivity Levels
- **Low**: Minimal monitoring, fewer false positives
- **Medium**: Balanced monitoring and detection
- **High**: Aggressive monitoring, may have more false positives

### Monitoring Intervals
Configure how frequently the detector scans for threats:

```cpp
detector->setMonitoringInterval(500); // 500ms intervals
```

## Integration with Security Manager

The Threat Detector works closely with the Security Manager:

```cpp
SecurityManager* securityManager = SecurityManager::instance();
ThreatDetector* detector = securityManager->getThreatDetector();

// Threat detector automatically reports to security manager
```

## Performance Considerations

- Monitoring frequency affects system performance
- Rule complexity impacts detection speed
- Consider disabling detailed monitoring in production for performance-critical applications

## Thread Safety

The Threat Detector is thread-safe and performs monitoring in background threads to avoid blocking the main application.

## See Also

- [Security Manager](security-manager.md)
- [Permission Manager](permission-manager.md)
- [Plugin Validator](plugin-validator.md)
