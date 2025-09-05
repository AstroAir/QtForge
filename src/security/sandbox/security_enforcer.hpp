/**
 * @file security_enforcer.hpp
 * @brief Security enforcement and process isolation for sandbox system
 * @version 3.2.0
 */

#pragma once

#include "../../../include/qtplugin/security/sandbox/plugin_sandbox.hpp"
#include <QObject>
#include <QProcess>
#include <QFileSystemWatcher>
#include <QNetworkAccessManager>
#include <QTimer>
#include <memory>
#include <unordered_set>

namespace qtplugin {

/**
 * @brief Security violation types
 */
enum class SecurityViolationType {
    UnauthorizedFileAccess,
    UnauthorizedNetworkAccess,
    UnauthorizedProcessCreation,
    UnauthorizedSystemCall,
    UnauthorizedRegistryAccess,
    UnauthorizedEnvironmentAccess,
    ResourceLimitExceeded,
    BlockedAPICall,
    SuspiciousActivity
};

/**
 * @brief Security event information
 */
struct SecurityEvent {
    SecurityViolationType type;
    QString description;
    QString resource_path;
    QJsonObject details;
    std::chrono::steady_clock::time_point timestamp;

    QJsonObject to_json() const;
};

/**
 * @brief Process isolation and security enforcement
 */
class SecurityEnforcer : public QObject {
    Q_OBJECT

public:
    explicit SecurityEnforcer(const SecurityPolicy& policy, QObject* parent = nullptr);
    ~SecurityEnforcer() override;

    /**
     * @brief Initialize security enforcement
     */
    bool initialize();

    /**
     * @brief Shutdown security enforcement
     */
    void shutdown();

    /**
     * @brief Set the process to monitor
     */
    void set_monitored_process(QProcess* process);

    /**
     * @brief Validate file system access
     */
    bool validate_file_access(const QString& path, bool write_access = false);

    /**
     * @brief Validate network access
     */
    bool validate_network_access(const QString& host, int port = -1);

    /**
     * @brief Validate process creation
     */
    bool validate_process_creation(const QString& executable);

    /**
     * @brief Validate system call
     */
    bool validate_system_call(const QString& call_name);

    /**
     * @brief Validate API call
     */
    bool validate_api_call(const QString& api_name);

    /**
     * @brief Check if directory is allowed
     */
    bool is_directory_allowed(const QString& path);

    /**
     * @brief Check if host is allowed
     */
    bool is_host_allowed(const QString& host);

    /**
     * @brief Check if API is blocked
     */
    bool is_api_blocked(const QString& api_name);

    /**
     * @brief Get security policy
     */
    const SecurityPolicy& get_policy() const { return m_policy; }

    /**
     * @brief Update security policy
     */
    void update_policy(const SecurityPolicy& policy);

    /**
     * @brief Get security events
     */
    std::vector<SecurityEvent> get_security_events() const;

    /**
     * @brief Clear security events
     */
    void clear_security_events();

signals:
    /**
     * @brief Emitted when a security violation is detected
     */
    void security_violation_detected(const SecurityEvent& event);

    /**
     * @brief Emitted when suspicious activity is detected
     */
    void suspicious_activity_detected(const QString& description, const QJsonObject& details);

private slots:
    void on_file_changed(const QString& path);
    void on_directory_changed(const QString& path);
    void check_process_activity();

private:
    SecurityPolicy m_policy;
    QProcess* m_monitored_process = nullptr;
    std::unique_ptr<QFileSystemWatcher> m_file_watcher;
    std::unique_ptr<QTimer> m_activity_monitor;
    std::vector<SecurityEvent> m_security_events;
    std::unordered_set<QString> m_monitored_files;
    std::unordered_set<QString> m_monitored_directories;
    mutable QMutex m_mutex;

    void setup_file_monitoring();
    void setup_process_monitoring();
    void record_security_event(SecurityViolationType type, const QString& description,
                              const QString& resource = QString(), const QJsonObject& details = QJsonObject{});
    bool is_path_allowed(const QString& path, const QStringList& allowed_paths);
    QString normalize_path(const QString& path);
    void analyze_process_behavior();
};

/**
 * @brief Security policy validator
 */
class SecurityPolicyValidator {
public:
    /**
     * @brief Validate security policy configuration
     */
    static bool validate_policy(const SecurityPolicy& policy, QString& error_message);

    /**
     * @brief Check policy compatibility
     */
    static bool is_policy_compatible(const SecurityPolicy& policy1, const SecurityPolicy& policy2);

    /**
     * @brief Get recommended policy for plugin type
     */
    static SecurityPolicy get_recommended_policy(PluginType plugin_type);

    /**
     * @brief Merge security policies
     */
    static SecurityPolicy merge_policies(const SecurityPolicy& base, const SecurityPolicy& override);

private:
    static bool validate_resource_limits(const ResourceLimits& limits, QString& error);
    static bool validate_permissions(const SecurityPermissions& permissions, QString& error);
};

/**
 * @brief Process isolation utilities
 */
class ProcessIsolationUtils {
public:
    /**
     * @brief Create isolated process environment
     */
    static QProcessEnvironment create_isolated_environment(const SecurityPolicy& policy);

    /**
     * @brief Setup process working directory
     */
    static QString setup_isolated_directory(const QString& base_path);

    /**
     * @brief Apply process restrictions (platform-specific)
     */
    static bool apply_process_restrictions(QProcess* process, const SecurityPolicy& policy);

    /**
     * @brief Cleanup isolated resources
     */
    static void cleanup_isolated_resources(const QString& isolated_path);

private:
#ifdef Q_OS_WIN
    static bool apply_windows_restrictions(QProcess* process, const SecurityPolicy& policy);
#elif defined(Q_OS_LINUX)
    static bool apply_linux_restrictions(QProcess* process, const SecurityPolicy& policy);
#elif defined(Q_OS_MACOS)
    static bool apply_macos_restrictions(QProcess* process, const SecurityPolicy& policy);
#endif
};

} // namespace qtplugin

// Register SecurityEvent as a Qt metatype for use in signals/slots
Q_DECLARE_METATYPE(qtplugin::SecurityEvent)
