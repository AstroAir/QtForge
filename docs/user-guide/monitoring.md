# Monitoring Guide

This guide covers comprehensive monitoring strategies for QtForge applications, including metrics collection, health monitoring, performance tracking, and observability best practices.

## Overview

Effective monitoring in QtForge provides:
- **Real-time Visibility**: Live insights into system performance and health
- **Proactive Issue Detection**: Early warning of potential problems
- **Performance Optimization**: Data-driven optimization opportunities
- **Troubleshooting Support**: Detailed information for debugging issues
- **Capacity Planning**: Historical data for scaling decisions

## Monitoring Architecture

### Core Components

```
┌─────────────────────────────────────────────────────────────┐
│                    Monitoring System                        │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │   Metrics   │  │   Health    │  │     Performance     │  │
│  │ Collector   │  │  Monitor    │  │      Tracker        │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │    Log      │  │   Alert     │  │      Dashboard      │  │
│  │ Aggregator  │  │  Manager    │  │      Service        │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                    Data Storage                             │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │ Time Series │  │    Logs     │  │      Events         │  │
│  │ Database    │  │  Database   │  │     Database        │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

## Metrics Collection

### System Metrics

Monitor core system performance:

```cpp
class SystemMetricsCollector {
public:
    struct SystemMetrics {
        double cpuUsagePercent;
        size_t memoryUsedBytes;
        size_t memoryTotalBytes;
        double memoryUsagePercent;
        size_t diskUsedBytes;
        size_t diskTotalBytes;
        double diskUsagePercent;
        size_t networkBytesIn;
        size_t networkBytesOut;
        std::chrono::system_clock::time_point timestamp;
    };
    
    SystemMetrics collectSystemMetrics() {
        SystemMetrics metrics;
        metrics.timestamp = std::chrono::system_clock::now();
        
        // CPU usage
        metrics.cpuUsagePercent = getCpuUsage();
        
        // Memory usage
        auto memInfo = getMemoryInfo();
        metrics.memoryUsedBytes = memInfo.used;
        metrics.memoryTotalBytes = memInfo.total;
        metrics.memoryUsagePercent = (static_cast<double>(memInfo.used) / memInfo.total) * 100.0;
        
        // Disk usage
        auto diskInfo = getDiskInfo();
        metrics.diskUsedBytes = diskInfo.used;
        metrics.diskTotalBytes = diskInfo.total;
        metrics.diskUsagePercent = (static_cast<double>(diskInfo.used) / diskInfo.total) * 100.0;
        
        // Network usage
        auto networkInfo = getNetworkInfo();
        metrics.networkBytesIn = networkInfo.bytesIn;
        metrics.networkBytesOut = networkInfo.bytesOut;
        
        return metrics;
    }
    
    void startCollection(std::chrono::seconds interval = std::chrono::seconds(30)) {
        collectionTimer_ = std::make_unique<QTimer>();
        collectionTimer_->setInterval(interval);
        
        QObject::connect(collectionTimer_.get(), &QTimer::timeout, [this]() {
            auto metrics = collectSystemMetrics();
            publishMetrics(metrics);
        });
        
        collectionTimer_->start();
    }

private:
    std::unique_ptr<QTimer> collectionTimer_;
    
    double getCpuUsage() {
        // Platform-specific CPU usage calculation
        return 0.0; // Placeholder
    }
    
    struct MemoryInfo { size_t used; size_t total; };
    MemoryInfo getMemoryInfo() {
        // Platform-specific memory info
        return {0, 0}; // Placeholder
    }
    
    struct DiskInfo { size_t used; size_t total; };
    DiskInfo getDiskInfo() {
        // Platform-specific disk info
        return {0, 0}; // Placeholder
    }
    
    struct NetworkInfo { size_t bytesIn; size_t bytesOut; };
    NetworkInfo getNetworkInfo() {
        // Platform-specific network info
        return {0, 0}; // Placeholder
    }
    
    void publishMetrics(const SystemMetrics& metrics) {
        auto& messageBus = qtforge::MessageBus::instance();
        
        qtforge::MetricsMessage metricsMsg;
        metricsMsg.source = "system";
        metricsMsg.timestamp = metrics.timestamp;
        metricsMsg.metrics["cpu_usage_percent"] = metrics.cpuUsagePercent;
        metricsMsg.metrics["memory_usage_percent"] = metrics.memoryUsagePercent;
        metricsMsg.metrics["disk_usage_percent"] = metrics.diskUsagePercent;
        metricsMsg.metrics["network_bytes_in"] = static_cast<double>(metrics.networkBytesIn);
        metricsMsg.metrics["network_bytes_out"] = static_cast<double>(metrics.networkBytesOut);
        
        messageBus.publish("monitoring.metrics", metricsMsg);
    }
};
```

### Plugin Metrics

Monitor individual plugin performance:

```cpp
class PluginMetricsCollector {
public:
    struct PluginMetrics {
        std::string pluginName;
        qtforge::PluginState state;
        std::chrono::milliseconds initializationTime;
        std::chrono::milliseconds activationTime;
        size_t memoryUsage;
        size_t messagesSent;
        size_t messagesReceived;
        size_t errorsCount;
        double cpuUsage;
        std::chrono::system_clock::time_point lastActivity;
        std::map<std::string, double> customMetrics;
    };
    
    void registerPlugin(const std::string& pluginName) {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        PluginMetrics metrics;
        metrics.pluginName = pluginName;
        metrics.state = qtforge::PluginState::Unloaded;
        metrics.initializationTime = std::chrono::milliseconds(0);
        metrics.activationTime = std::chrono::milliseconds(0);
        metrics.memoryUsage = 0;
        metrics.messagesSent = 0;
        metrics.messagesReceived = 0;
        metrics.errorsCount = 0;
        metrics.cpuUsage = 0.0;
        metrics.lastActivity = std::chrono::system_clock::now();
        
        pluginMetrics_[pluginName] = metrics;
    }
    
    void recordPluginStateChange(const std::string& pluginName, 
                                qtforge::PluginState newState,
                                std::chrono::milliseconds transitionTime) {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        auto it = pluginMetrics_.find(pluginName);
        if (it != pluginMetrics_.end()) {
            it->second.state = newState;
            it->second.lastActivity = std::chrono::system_clock::now();
            
            if (newState == qtforge::PluginState::Initialized) {
                it->second.initializationTime = transitionTime;
            } else if (newState == qtforge::PluginState::Active) {
                it->second.activationTime = transitionTime;
            }
        }
    }
    
    void recordMessageActivity(const std::string& pluginName, bool sent) {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        auto it = pluginMetrics_.find(pluginName);
        if (it != pluginMetrics_.end()) {
            if (sent) {
                it->second.messagesSent++;
            } else {
                it->second.messagesReceived++;
            }
            it->second.lastActivity = std::chrono::system_clock::now();
        }
    }
    
    void recordError(const std::string& pluginName) {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        auto it = pluginMetrics_.find(pluginName);
        if (it != pluginMetrics_.end()) {
            it->second.errorsCount++;
            it->second.lastActivity = std::chrono::system_clock::now();
        }
    }
    
    std::vector<PluginMetrics> getAllMetrics() const {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        std::vector<PluginMetrics> metrics;
        for (const auto& [name, pluginMetrics] : pluginMetrics_) {
            metrics.push_back(pluginMetrics);
        }
        
        return metrics;
    }

private:
    mutable std::mutex metricsMutex_;
    std::unordered_map<std::string, PluginMetrics> pluginMetrics_;
};
```

## Health Monitoring

### Health Check System

Implement comprehensive health checks:

```cpp
class HealthMonitor {
public:
    enum class HealthStatus { Healthy, Degraded, Unhealthy, Unknown };
    
    struct HealthCheck {
        std::string name;
        std::string description;
        HealthStatus status;
        std::string message;
        std::chrono::system_clock::time_point lastCheck;
        std::chrono::milliseconds checkDuration;
        std::map<std::string, std::string> details;
    };
    
    void registerHealthCheck(const std::string& name,
                           const std::string& description,
                           std::function<HealthCheck()> checkFunction,
                           std::chrono::seconds interval = std::chrono::seconds(60)) {
        
        HealthCheckInfo info;
        info.name = name;
        info.description = description;
        info.checkFunction = checkFunction;
        info.interval = interval;
        info.lastRun = std::chrono::system_clock::time_point::min();
        
        std::lock_guard<std::mutex> lock(healthChecksMutex_);
        healthChecks_[name] = info;
    }
    
    HealthCheck runHealthCheck(const std::string& name) {
        std::lock_guard<std::mutex> lock(healthChecksMutex_);
        
        auto it = healthChecks_.find(name);
        if (it == healthChecks_.end()) {
            HealthCheck result;
            result.name = name;
            result.status = HealthStatus::Unknown;
            result.message = "Health check not found";
            return result;
        }
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        try {
            HealthCheck result = it->second.checkFunction();
            
            auto endTime = std::chrono::high_resolution_clock::now();
            result.checkDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime);
            result.lastCheck = std::chrono::system_clock::now();
            
            it->second.lastRun = result.lastCheck;
            it->second.lastResult = result;
            
            return result;
            
        } catch (const std::exception& e) {
            HealthCheck result;
            result.name = name;
            result.description = it->second.description;
            result.status = HealthStatus::Unhealthy;
            result.message = "Health check failed: " + std::string(e.what());
            result.lastCheck = std::chrono::system_clock::now();
            
            auto endTime = std::chrono::high_resolution_clock::now();
            result.checkDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - startTime);
            
            it->second.lastRun = result.lastCheck;
            it->second.lastResult = result;
            
            return result;
        }
    }
    
    std::vector<HealthCheck> runAllHealthChecks() {
        std::vector<HealthCheck> results;
        
        std::lock_guard<std::mutex> lock(healthChecksMutex_);
        for (const auto& [name, info] : healthChecks_) {
            // Check if it's time to run this health check
            auto now = std::chrono::system_clock::now();
            auto timeSinceLastRun = now - info.lastRun;
            
            if (timeSinceLastRun >= info.interval) {
                lock.~lock_guard(); // Unlock before running check
                auto result = runHealthCheck(name);
                results.push_back(result);
                lock.lock_guard(healthChecksMutex_); // Re-lock
            } else if (info.lastResult.has_value()) {
                results.push_back(info.lastResult.value());
            }
        }
        
        return results;
    }
    
    HealthStatus getOverallHealth() const {
        auto checks = const_cast<HealthMonitor*>(this)->runAllHealthChecks();
        
        bool hasUnhealthy = false;
        bool hasDegraded = false;
        
        for (const auto& check : checks) {
            switch (check.status) {
                case HealthStatus::Unhealthy:
                    hasUnhealthy = true;
                    break;
                case HealthStatus::Degraded:
                    hasDegraded = true;
                    break;
                case HealthStatus::Healthy:
                    break;
                case HealthStatus::Unknown:
                    hasDegraded = true; // Treat unknown as degraded
                    break;
            }
        }
        
        if (hasUnhealthy) {
            return HealthStatus::Unhealthy;
        } else if (hasDegraded) {
            return HealthStatus::Degraded;
        } else {
            return HealthStatus::Healthy;
        }
    }

private:
    struct HealthCheckInfo {
        std::string name;
        std::string description;
        std::function<HealthCheck()> checkFunction;
        std::chrono::seconds interval;
        std::chrono::system_clock::time_point lastRun;
        std::optional<HealthCheck> lastResult;
    };
    
    mutable std::mutex healthChecksMutex_;
    std::unordered_map<std::string, HealthCheckInfo> healthChecks_;
};
```

### Built-in Health Checks

Common health checks for QtForge applications:

```cpp
class StandardHealthChecks {
public:
    static HealthMonitor::HealthCheck checkDatabaseConnection() {
        HealthMonitor::HealthCheck check;
        check.name = "database_connection";
        check.description = "Database connectivity check";
        
        try {
            // Attempt database connection
            auto& serviceRegistry = qtforge::ServiceRegistry::instance();
            auto dbService = serviceRegistry.getService<IDatabaseService>("database.service");
            
            if (!dbService) {
                check.status = HealthMonitor::HealthStatus::Unhealthy;
                check.message = "Database service not available";
                return check;
            }
            
            // Test connection with simple query
            auto result = dbService.value()->query("SELECT 1", {});
            if (result) {
                check.status = HealthMonitor::HealthStatus::Healthy;
                check.message = "Database connection OK";
            } else {
                check.status = HealthMonitor::HealthStatus::Unhealthy;
                check.message = "Database query failed: " + result.error().message();
            }
            
        } catch (const std::exception& e) {
            check.status = HealthMonitor::HealthStatus::Unhealthy;
            check.message = "Database check exception: " + std::string(e.what());
        }
        
        return check;
    }
    
    static HealthMonitor::HealthCheck checkMemoryUsage() {
        HealthMonitor::HealthCheck check;
        check.name = "memory_usage";
        check.description = "System memory usage check";
        
        try {
            auto memInfo = getMemoryInfo();
            double usagePercent = (static_cast<double>(memInfo.used) / memInfo.total) * 100.0;
            
            check.details["used_bytes"] = std::to_string(memInfo.used);
            check.details["total_bytes"] = std::to_string(memInfo.total);
            check.details["usage_percent"] = std::to_string(usagePercent);
            
            if (usagePercent < 80.0) {
                check.status = HealthMonitor::HealthStatus::Healthy;
                check.message = "Memory usage normal (" + std::to_string(usagePercent) + "%)";
            } else if (usagePercent < 90.0) {
                check.status = HealthMonitor::HealthStatus::Degraded;
                check.message = "Memory usage high (" + std::to_string(usagePercent) + "%)";
            } else {
                check.status = HealthMonitor::HealthStatus::Unhealthy;
                check.message = "Memory usage critical (" + std::to_string(usagePercent) + "%)";
            }
            
        } catch (const std::exception& e) {
            check.status = HealthMonitor::HealthStatus::Unknown;
            check.message = "Memory check failed: " + std::string(e.what());
        }
        
        return check;
    }
    
    static HealthMonitor::HealthCheck checkPluginHealth() {
        HealthMonitor::HealthCheck check;
        check.name = "plugin_health";
        check.description = "Plugin system health check";
        
        try {
            auto& pluginManager = qtforge::PluginManager::instance();
            auto loadedPlugins = pluginManager.getLoadedPlugins();
            
            int activeCount = 0;
            int errorCount = 0;
            
            for (const auto& pluginName : loadedPlugins) {
                auto plugin = pluginManager.getPlugin(pluginName);
                if (plugin) {
                    switch (plugin->state()) {
                        case qtforge::PluginState::Active:
                            activeCount++;
                            break;
                        case qtforge::PluginState::Error:
                            errorCount++;
                            break;
                        default:
                            break;
                    }
                }
            }
            
            check.details["total_plugins"] = std::to_string(loadedPlugins.size());
            check.details["active_plugins"] = std::to_string(activeCount);
            check.details["error_plugins"] = std::to_string(errorCount);
            
            if (errorCount == 0) {
                check.status = HealthMonitor::HealthStatus::Healthy;
                check.message = "All plugins healthy (" + std::to_string(activeCount) + " active)";
            } else if (errorCount < loadedPlugins.size() / 2) {
                check.status = HealthMonitor::HealthStatus::Degraded;
                check.message = std::to_string(errorCount) + " plugins in error state";
            } else {
                check.status = HealthMonitor::HealthStatus::Unhealthy;
                check.message = "Multiple plugins in error state (" + std::to_string(errorCount) + ")";
            }
            
        } catch (const std::exception& e) {
            check.status = HealthMonitor::HealthStatus::Unknown;
            check.message = "Plugin health check failed: " + std::string(e.what());
        }
        
        return check;
    }

private:
    struct MemoryInfo { size_t used; size_t total; };
    static MemoryInfo getMemoryInfo() {
        // Platform-specific implementation
        return {0, 0}; // Placeholder
    }
};
```

## Performance Tracking

### Performance Profiler

Track performance metrics across the application:

```cpp
class PerformanceTracker {
public:
    struct PerformanceMetrics {
        std::string operation;
        std::chrono::microseconds duration;
        std::chrono::system_clock::time_point timestamp;
        std::string component;
        std::map<std::string, std::string> tags;
        bool success;
    };
    
    class ScopedTimer {
    public:
        ScopedTimer(PerformanceTracker& tracker, const std::string& operation,
                   const std::string& component = "", 
                   const std::map<std::string, std::string>& tags = {})
            : tracker_(tracker), operation_(operation), component_(component), 
              tags_(tags), start_(std::chrono::high_resolution_clock::now()), success_(true) {}
        
        ~ScopedTimer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
            
            PerformanceMetrics metrics;
            metrics.operation = operation_;
            metrics.duration = duration;
            metrics.timestamp = std::chrono::system_clock::now();
            metrics.component = component_;
            metrics.tags = tags_;
            metrics.success = success_;
            
            tracker_.recordMetrics(metrics);
        }
        
        void markFailure() { success_ = false; }
        
    private:
        PerformanceTracker& tracker_;
        std::string operation_;
        std::string component_;
        std::map<std::string, std::string> tags_;
        std::chrono::high_resolution_clock::time_point start_;
        bool success_;
    };
    
    void recordMetrics(const PerformanceMetrics& metrics) {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        // Store recent metrics
        recentMetrics_.push_back(metrics);
        
        // Keep only recent metrics (last 1000)
        if (recentMetrics_.size() > 1000) {
            recentMetrics_.erase(recentMetrics_.begin());
        }
        
        // Update aggregated statistics
        updateAggregatedStats(metrics);
        
        // Publish metrics
        publishMetrics(metrics);
    }
    
    std::vector<PerformanceMetrics> getRecentMetrics(size_t count = 100) const {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        if (recentMetrics_.size() <= count) {
            return recentMetrics_;
        }
        
        return std::vector<PerformanceMetrics>(
            recentMetrics_.end() - count, recentMetrics_.end());
    }

private:
    mutable std::mutex metricsMutex_;
    std::vector<PerformanceMetrics> recentMetrics_;
    
    struct AggregatedStats {
        size_t count = 0;
        std::chrono::microseconds totalDuration{0};
        std::chrono::microseconds minDuration{std::chrono::microseconds::max()};
        std::chrono::microseconds maxDuration{0};
        size_t successCount = 0;
        size_t failureCount = 0;
    };
    
    std::unordered_map<std::string, AggregatedStats> aggregatedStats_;
    
    void updateAggregatedStats(const PerformanceMetrics& metrics) {
        auto& stats = aggregatedStats_[metrics.operation];
        
        stats.count++;
        stats.totalDuration += metrics.duration;
        stats.minDuration = std::min(stats.minDuration, metrics.duration);
        stats.maxDuration = std::max(stats.maxDuration, metrics.duration);
        
        if (metrics.success) {
            stats.successCount++;
        } else {
            stats.failureCount++;
        }
    }
    
    void publishMetrics(const PerformanceMetrics& metrics) {
        auto& messageBus = qtforge::MessageBus::instance();
        
        qtforge::PerformanceMetricsMessage msg;
        msg.operation = metrics.operation;
        msg.duration = metrics.duration;
        msg.timestamp = metrics.timestamp;
        msg.component = metrics.component;
        msg.tags = metrics.tags;
        msg.success = metrics.success;
        
        messageBus.publish("monitoring.performance", msg);
    }
};

// Macro for easy performance tracking
#define TRACK_PERFORMANCE(tracker, operation) \
    PerformanceTracker::ScopedTimer timer(tracker, operation, __FUNCTION__)

#define TRACK_PERFORMANCE_WITH_TAGS(tracker, operation, tags) \
    PerformanceTracker::ScopedTimer timer(tracker, operation, __FUNCTION__, tags)
```

## Alerting System

### Alert Manager

Implement intelligent alerting:

```cpp
class AlertManager {
public:
    enum class AlertSeverity { Info, Warning, Error, Critical };
    enum class AlertStatus { Active, Acknowledged, Resolved };
    
    struct Alert {
        std::string id;
        std::string title;
        std::string description;
        AlertSeverity severity;
        AlertStatus status;
        std::string source;
        std::chrono::system_clock::time_point createdAt;
        std::chrono::system_clock::time_point updatedAt;
        std::map<std::string, std::string> labels;
        std::vector<std::string> actions;
    };
    
    void createAlert(const std::string& title, const std::string& description,
                    AlertSeverity severity, const std::string& source,
                    const std::map<std::string, std::string>& labels = {}) {
        
        Alert alert;
        alert.id = generateAlertId();
        alert.title = title;
        alert.description = description;
        alert.severity = severity;
        alert.status = AlertStatus::Active;
        alert.source = source;
        alert.createdAt = std::chrono::system_clock::now();
        alert.updatedAt = alert.createdAt;
        alert.labels = labels;
        
        // Add suggested actions based on alert type
        alert.actions = generateSuggestedActions(alert);
        
        std::lock_guard<std::mutex> lock(alertsMutex_);
        activeAlerts_[alert.id] = alert;
        
        // Publish alert
        publishAlert(alert);
        
        // Send notifications
        sendNotifications(alert);
    }
    
    void acknowledgeAlert(const std::string& alertId, const std::string& acknowledgedBy) {
        std::lock_guard<std::mutex> lock(alertsMutex_);
        
        auto it = activeAlerts_.find(alertId);
        if (it != activeAlerts_.end()) {
            it->second.status = AlertStatus::Acknowledged;
            it->second.updatedAt = std::chrono::system_clock::now();
            it->second.labels["acknowledged_by"] = acknowledgedBy;
            
            publishAlert(it->second);
        }
    }
    
    void resolveAlert(const std::string& alertId, const std::string& resolvedBy) {
        std::lock_guard<std::mutex> lock(alertsMutex_);
        
        auto it = activeAlerts_.find(alertId);
        if (it != activeAlerts_.end()) {
            it->second.status = AlertStatus::Resolved;
            it->second.updatedAt = std::chrono::system_clock::now();
            it->second.labels["resolved_by"] = resolvedBy;
            
            publishAlert(it->second);
            
            // Move to resolved alerts
            resolvedAlerts_[alertId] = it->second;
            activeAlerts_.erase(it);
        }
    }
    
    std::vector<Alert> getActiveAlerts() const {
        std::lock_guard<std::mutex> lock(alertsMutex_);
        
        std::vector<Alert> alerts;
        for (const auto& [id, alert] : activeAlerts_) {
            alerts.push_back(alert);
        }
        
        // Sort by severity and creation time
        std::sort(alerts.begin(), alerts.end(), [](const Alert& a, const Alert& b) {
            if (a.severity != b.severity) {
                return static_cast<int>(a.severity) > static_cast<int>(b.severity);
            }
            return a.createdAt > b.createdAt;
        });
        
        return alerts;
    }

private:
    mutable std::mutex alertsMutex_;
    std::unordered_map<std::string, Alert> activeAlerts_;
    std::unordered_map<std::string, Alert> resolvedAlerts_;
    
    std::string generateAlertId() {
        static std::atomic<size_t> counter{0};
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();
        
        return "alert_" + std::to_string(timestamp) + "_" + std::to_string(counter++);
    }
    
    std::vector<std::string> generateSuggestedActions(const Alert& alert) {
        std::vector<std::string> actions;
        
        if (alert.source == "memory_usage" && alert.severity >= AlertSeverity::Warning) {
            actions.push_back("Check for memory leaks");
            actions.push_back("Restart high-memory plugins");
            actions.push_back("Increase memory limits");
        }
        
        if (alert.source == "database_connection" && alert.severity >= AlertSeverity::Error) {
            actions.push_back("Check database server status");
            actions.push_back("Verify network connectivity");
            actions.push_back("Check database credentials");
        }
        
        if (alert.source == "plugin_health" && alert.severity >= AlertSeverity::Warning) {
            actions.push_back("Check plugin logs");
            actions.push_back("Restart failed plugins");
            actions.push_back("Check plugin dependencies");
        }
        
        return actions;
    }
    
    void publishAlert(const Alert& alert) {
        auto& messageBus = qtforge::MessageBus::instance();
        
        qtforge::AlertMessage alertMsg;
        alertMsg.alert = alert;
        alertMsg.timestamp = std::chrono::system_clock::now();
        
        messageBus.publish("monitoring.alerts", alertMsg);
    }
    
    void sendNotifications(const Alert& alert) {
        // Send notifications based on severity
        if (alert.severity >= AlertSeverity::Error) {
            sendEmailNotification(alert);
        }
        
        if (alert.severity == AlertSeverity::Critical) {
            sendSmsNotification(alert);
            sendSlackNotification(alert);
        }
    }
    
    void sendEmailNotification(const Alert& alert) {
        // Implementation for email notifications
    }
    
    void sendSmsNotification(const Alert& alert) {
        // Implementation for SMS notifications
    }
    
    void sendSlackNotification(const Alert& alert) {
        // Implementation for Slack notifications
    }
};
```

## Dashboard and Visualization

### Monitoring Dashboard

Create real-time monitoring dashboards:

```cpp
class MonitoringDashboard : public QWidget {
    Q_OBJECT

public:
    MonitoringDashboard(QWidget* parent = nullptr) : QWidget(parent) {
        setupUI();
        setupDataConnections();
        startDataRefresh();
    }

private slots:
    void updateSystemMetrics(const SystemMetrics& metrics);
    void updatePluginMetrics(const std::vector<PluginMetrics>& metrics);
    void updateHealthStatus(const std::vector<HealthCheck>& checks);
    void updateAlerts(const std::vector<Alert>& alerts);

private:
    void setupUI() {
        auto* layout = new QVBoxLayout(this);
        
        // System overview
        systemOverview_ = new SystemOverviewWidget();
        layout->addWidget(systemOverview_);
        
        // Plugin status
        pluginStatus_ = new PluginStatusWidget();
        layout->addWidget(pluginStatus_);
        
        // Health checks
        healthChecks_ = new HealthChecksWidget();
        layout->addWidget(healthChecks_);
        
        // Active alerts
        alertsWidget_ = new AlertsWidget();
        layout->addWidget(alertsWidget_);
        
        // Performance charts
        performanceCharts_ = new PerformanceChartsWidget();
        layout->addWidget(performanceCharts_);
    }
    
    void setupDataConnections() {
        auto& messageBus = qtforge::MessageBus::instance();
        
        // Subscribe to monitoring data
        messageBus.subscribe<qtforge::MetricsMessage>("monitoring.metrics",
            [this](const qtforge::MetricsMessage& msg) {
                if (msg.source == "system") {
                    // Convert to SystemMetrics and update
                }
            });
        
        messageBus.subscribe<qtforge::AlertMessage>("monitoring.alerts",
            [this](const qtforge::AlertMessage& msg) {
                // Update alerts display
            });
    }
    
    void startDataRefresh() {
        refreshTimer_ = new QTimer(this);
        refreshTimer_->setInterval(5000); // 5 second refresh
        connect(refreshTimer_, &QTimer::timeout, this, &MonitoringDashboard::refreshData);
        refreshTimer_->start();
    }
    
    void refreshData() {
        // Trigger data collection and updates
        emit dataRefreshRequested();
    }
    
    SystemOverviewWidget* systemOverview_;
    PluginStatusWidget* pluginStatus_;
    HealthChecksWidget* healthChecks_;
    AlertsWidget* alertsWidget_;
    PerformanceChartsWidget* performanceCharts_;
    QTimer* refreshTimer_;

signals:
    void dataRefreshRequested();
};
```

## Best Practices

### 1. Monitoring Strategy

- **Start Simple**: Begin with basic metrics and expand gradually
- **Monitor What Matters**: Focus on business-critical metrics
- **Set Meaningful Thresholds**: Avoid alert fatigue with appropriate thresholds
- **Document Everything**: Maintain clear documentation for all monitoring

### 2. Performance Considerations

- **Efficient Collection**: Minimize overhead of monitoring itself
- **Batch Operations**: Batch metric collection and transmission
- **Sampling**: Use sampling for high-frequency events
- **Storage Optimization**: Implement appropriate data retention policies

### 3. Alert Management

- **Severity Levels**: Use appropriate severity levels
- **Actionable Alerts**: Ensure alerts provide actionable information
- **Alert Correlation**: Group related alerts to reduce noise
- **Escalation Policies**: Implement proper escalation procedures

## See Also

- **[Performance Optimization](performance-optimization.md)**: Performance tuning strategies
- **[Troubleshooting](troubleshooting.md)**: Debugging and troubleshooting guide
- **[Plugin Architecture](plugin-architecture.md)**: Understanding plugin architecture
- **[Security Configuration](security-configuration.md)**: Security monitoring practices
