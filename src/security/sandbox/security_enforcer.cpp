/**
 * @file security_enforcer.cpp
 * @brief Implementation of security enforcement and process isolation
 * @version 3.2.0
 */

#include "security_enforcer.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QUrl>
#include <algorithm>

#ifdef Q_OS_WIN
#include <windows.h>
#include <processthreadsapi.h>
#elif defined(Q_OS_LINUX)
#include <sys/prctl.h>
#include <sys/resource.h>
#include <unistd.h>
#elif defined(Q_OS_MACOS)
#include <sys/resource.h>
#include <unistd.h>
#endif

Q_LOGGING_CATEGORY(securityEnforcerLog, "qtplugin.sandbox.security_enforcer");

namespace qtplugin {

// === SecurityEvent Implementation ===

QJsonObject SecurityEvent::to_json() const {
    QJsonObject json;
    json["type"] = static_cast<int>(type);
    json["description"] = description;
    json["resource_path"] = resource_path;
    json["details"] = details;
    
    // Convert timestamp to milliseconds since epoch
    auto duration = timestamp.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    json["timestamp"] = static_cast<qint64>(millis.count());
    
    return json;
}

// === SecurityEnforcer Implementation ===

SecurityEnforcer::SecurityEnforcer(const SecurityPolicy& policy, QObject* parent)
    : QObject(parent), m_policy(policy) {
    
    m_file_watcher = std::make_unique<QFileSystemWatcher>(this);
    m_activity_monitor = std::make_unique<QTimer>(this);
    
    // Connect signals
    connect(m_file_watcher.get(), &QFileSystemWatcher::fileChanged,
            this, &SecurityEnforcer::on_file_changed);
    connect(m_file_watcher.get(), &QFileSystemWatcher::directoryChanged,
            this, &SecurityEnforcer::on_directory_changed);
    connect(m_activity_monitor.get(), &QTimer::timeout,
            this, &SecurityEnforcer::check_process_activity);
}

SecurityEnforcer::~SecurityEnforcer() {
    shutdown();
}

bool SecurityEnforcer::initialize() {
    QMutexLocker locker(&m_mutex);
    
    qCDebug(securityEnforcerLog) << "Initializing security enforcer with policy:" << m_policy.policy_name;
    
    // Setup monitoring based on security level
    if (m_policy.level != SandboxSecurityLevel::Unrestricted) {
        setup_file_monitoring();
        setup_process_monitoring();
        
        // Start activity monitoring
        m_activity_monitor->start(5000); // Check every 5 seconds
    }
    
    qCDebug(securityEnforcerLog) << "Security enforcer initialized successfully";
    return true;
}

void SecurityEnforcer::shutdown() {
    QMutexLocker locker(&m_mutex);
    
    if (m_activity_monitor) {
        m_activity_monitor->stop();
    }
    
    if (m_file_watcher) {
        m_file_watcher->removePaths(m_file_watcher->files());
        m_file_watcher->removePaths(m_file_watcher->directories());
    }
    
    m_monitored_files.clear();
    m_monitored_directories.clear();
    
    qCDebug(securityEnforcerLog) << "Security enforcer shutdown completed";
}

void SecurityEnforcer::set_monitored_process(QProcess* process) {
    QMutexLocker locker(&m_mutex);
    m_monitored_process = process;
    
    if (process) {
        qCDebug(securityEnforcerLog) << "Monitoring process PID:" << process->processId();
    }
}

bool SecurityEnforcer::validate_file_access(const QString& path, bool write_access) {
    QMutexLocker locker(&m_mutex);
    
    // Check if file system access is allowed
    if (write_access && !m_policy.permissions.allow_file_system_write) {
        record_security_event(SecurityViolationType::UnauthorizedFileAccess,
                             "Unauthorized file write access attempted",
                             path);
        return false;
    }
    
    if (!write_access && !m_policy.permissions.allow_file_system_read) {
        record_security_event(SecurityViolationType::UnauthorizedFileAccess,
                             "Unauthorized file read access attempted",
                             path);
        return false;
    }
    
    // Check if path is in allowed directories
    if (!is_directory_allowed(path)) {
        record_security_event(SecurityViolationType::UnauthorizedFileAccess,
                             "File access outside allowed directories",
                             path);
        return false;
    }
    
    return true;
}

bool SecurityEnforcer::validate_network_access(const QString& host, int port) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_policy.permissions.allow_network_access) {
        record_security_event(SecurityViolationType::UnauthorizedNetworkAccess,
                             "Network access denied by policy",
                             host);
        return false;
    }
    
    if (!is_host_allowed(host)) {
        record_security_event(SecurityViolationType::UnauthorizedNetworkAccess,
                             "Access to unauthorized host",
                             host);
        return false;
    }
    
    Q_UNUSED(port) // Port-specific validation could be added here
    
    return true;
}

bool SecurityEnforcer::validate_process_creation(const QString& executable) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_policy.permissions.allow_process_creation) {
        record_security_event(SecurityViolationType::UnauthorizedProcessCreation,
                             "Process creation denied by policy",
                             executable);
        return false;
    }
    
    return true;
}

bool SecurityEnforcer::validate_system_call(const QString& call_name) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_policy.permissions.allow_system_calls) {
        record_security_event(SecurityViolationType::UnauthorizedSystemCall,
                             "System call denied by policy",
                             call_name);
        return false;
    }
    
    return true;
}

bool SecurityEnforcer::validate_api_call(const QString& api_name) {
    QMutexLocker locker(&m_mutex);
    
    if (is_api_blocked(api_name)) {
        record_security_event(SecurityViolationType::BlockedAPICall,
                             "Blocked API call attempted",
                             api_name);
        return false;
    }
    
    return true;
}

bool SecurityEnforcer::is_directory_allowed(const QString& path) {
    QString normalized_path = normalize_path(path);
    
    // If no allowed directories specified, allow all (for unrestricted mode)
    if (m_policy.permissions.allowed_directories.isEmpty()) {
        return m_policy.level == SandboxSecurityLevel::Unrestricted;
    }
    
    return is_path_allowed(normalized_path, m_policy.permissions.allowed_directories);
}

bool SecurityEnforcer::is_host_allowed(const QString& host) {
    // If no allowed hosts specified, allow all (for unrestricted mode)
    if (m_policy.permissions.allowed_hosts.isEmpty()) {
        return m_policy.level == SandboxSecurityLevel::Unrestricted;
    }
    
    // Check exact match
    if (m_policy.permissions.allowed_hosts.contains(host)) {
        return true;
    }
    
    // Check wildcard patterns
    for (const QString& allowed_host : m_policy.permissions.allowed_hosts) {
        if (allowed_host.contains('*')) {
            QRegExp regex(allowed_host);
            regex.setPatternSyntax(QRegExp::Wildcard);
            if (regex.exactMatch(host)) {
                return true;
            }
        }
    }
    
    return false;
}

bool SecurityEnforcer::is_api_blocked(const QString& api_name) {
    return m_policy.permissions.blocked_apis.contains(api_name);
}

void SecurityEnforcer::update_policy(const SecurityPolicy& policy) {
    QMutexLocker locker(&m_mutex);
    
    m_policy = policy;
    qCDebug(securityEnforcerLog) << "Security policy updated to:" << policy.policy_name;
    
    // Reinitialize monitoring with new policy
    shutdown();
    initialize();
}

std::vector<SecurityEvent> SecurityEnforcer::get_security_events() const {
    QMutexLocker locker(&m_mutex);
    return m_security_events;
}

void SecurityEnforcer::clear_security_events() {
    QMutexLocker locker(&m_mutex);
    m_security_events.clear();
}

void SecurityEnforcer::on_file_changed(const QString& path) {
    qCDebug(securityEnforcerLog) << "File changed:" << path;
    
    // Check if this change is authorized
    if (!validate_file_access(path, true)) {
        emit suspicious_activity_detected("Unauthorized file modification", 
                                        QJsonObject{{"path", path}});
    }
}

void SecurityEnforcer::on_directory_changed(const QString& path) {
    qCDebug(securityEnforcerLog) << "Directory changed:" << path;
    
    // Monitor new files in the directory
    QDir dir(path);
    QStringList files = dir.entryList(QDir::Files);
    for (const QString& file : files) {
        QString file_path = dir.absoluteFilePath(file);
        if (m_monitored_files.find(file_path) == m_monitored_files.end()) {
            m_file_watcher->addPath(file_path);
            m_monitored_files.insert(file_path);
        }
    }
}

void SecurityEnforcer::check_process_activity() {
    if (!m_monitored_process || m_monitored_process->state() != QProcess::Running) {
        return;
    }
    
    analyze_process_behavior();
}

void SecurityEnforcer::setup_file_monitoring() {
    // Monitor allowed directories
    for (const QString& dir_path : m_policy.permissions.allowed_directories) {
        QDir dir(dir_path);
        if (dir.exists()) {
            m_file_watcher->addPath(dir.absolutePath());
            m_monitored_directories.insert(dir.absolutePath());
            
            // Also monitor files in the directory
            QStringList files = dir.entryList(QDir::Files);
            for (const QString& file : files) {
                QString file_path = dir.absoluteFilePath(file);
                m_file_watcher->addPath(file_path);
                m_monitored_files.insert(file_path);
            }
        }
    }
}

void SecurityEnforcer::setup_process_monitoring() {
    // Process monitoring setup - platform specific implementation would go here
    qCDebug(securityEnforcerLog) << "Process monitoring setup completed";
}

void SecurityEnforcer::record_security_event(SecurityViolationType type, const QString& description,
                                           const QString& resource, const QJsonObject& details) {
    SecurityEvent event;
    event.type = type;
    event.description = description;
    event.resource_path = resource;
    event.details = details;
    event.timestamp = std::chrono::steady_clock::now();
    
    m_security_events.push_back(event);
    
    // Limit the number of stored events
    if (m_security_events.size() > 1000) {
        m_security_events.erase(m_security_events.begin());
    }
    
    qCWarning(securityEnforcerLog) << "Security violation:" << description << "Resource:" << resource;
    emit security_violation_detected(event);
}

bool SecurityEnforcer::is_path_allowed(const QString& path, const QStringList& allowed_paths) {
    for (const QString& allowed_path : allowed_paths) {
        QString normalized_allowed = normalize_path(allowed_path);
        if (path.startsWith(normalized_allowed)) {
            return true;
        }
    }
    return false;
}

QString SecurityEnforcer::normalize_path(const QString& path) {
    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

void SecurityEnforcer::analyze_process_behavior() {
    // Placeholder for behavioral analysis
    // This could include checking for:
    // - Unusual CPU usage patterns
    // - Excessive memory allocation
    // - Suspicious network activity
    // - File system scanning behavior
    
    qCDebug(securityEnforcerLog) << "Process behavior analysis completed";
}

} // namespace qtplugin
