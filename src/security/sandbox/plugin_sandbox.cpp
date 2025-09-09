/**
 * @file plugin_sandbox.cpp
 * @brief Implementation of plugin sandboxing and security system
 * @version 3.2.0
 */

#include <qtplugin/security/sandbox/plugin_sandbox.hpp>
#include "resource_monitor.hpp"
#include "security_enforcer.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QMutexLocker>
#include <QStandardPaths>
#include <QtGlobal>

Q_LOGGING_CATEGORY(sandboxLog, "qtplugin.sandbox");

namespace qtplugin {

// === ResourceLimits Implementation ===

QJsonObject ResourceLimits::to_json() const {
    QJsonObject json;
    json["cpu_time_limit"] = static_cast<qint64>(cpu_time_limit.count());
    json["memory_limit_mb"] = static_cast<qint64>(memory_limit_mb);
    json["disk_space_limit_mb"] = static_cast<qint64>(disk_space_limit_mb);
    json["max_file_handles"] = max_file_handles;
    json["max_network_connections"] = max_network_connections;
    json["execution_timeout"] = static_cast<qint64>(execution_timeout.count());
    return json;
}

qtplugin::expected<ResourceLimits, PluginError> ResourceLimits::from_json(
    const QJsonObject& json) {
    ResourceLimits limits;

    if (json.contains("cpu_time_limit")) {
        limits.cpu_time_limit =
            std::chrono::milliseconds(json["cpu_time_limit"].toInt());
    }
    if (json.contains("memory_limit_mb")) {
        limits.memory_limit_mb = json["memory_limit_mb"].toInt();
    }
    if (json.contains("disk_space_limit_mb")) {
        limits.disk_space_limit_mb = json["disk_space_limit_mb"].toInt();
    }
    if (json.contains("max_file_handles")) {
        limits.max_file_handles = json["max_file_handles"].toInt();
    }
    if (json.contains("max_network_connections")) {
        limits.max_network_connections =
            json["max_network_connections"].toInt();
    }
    if (json.contains("execution_timeout")) {
        limits.execution_timeout =
            std::chrono::milliseconds(json["execution_timeout"].toInt());
    }

    return limits;
}

// === SecurityPermissions Implementation ===

QJsonObject SecurityPermissions::to_json() const {
    QJsonObject json;
    json["allow_file_system_read"] = allow_file_system_read;
    json["allow_file_system_write"] = allow_file_system_write;
    json["allow_network_access"] = allow_network_access;
    json["allow_process_creation"] = allow_process_creation;
    json["allow_system_calls"] = allow_system_calls;
    json["allow_registry_access"] = allow_registry_access;
    json["allow_environment_access"] = allow_environment_access;

    QJsonArray dirs_array;
    for (const QString& dir : allowed_directories) {
        dirs_array.append(dir);
    }
    json["allowed_directories"] = dirs_array;

    QJsonArray hosts_array;
    for (const QString& host : allowed_hosts) {
        hosts_array.append(host);
    }
    json["allowed_hosts"] = hosts_array;

    QJsonArray apis_array;
    for (const QString& api : blocked_apis) {
        apis_array.append(api);
    }
    json["blocked_apis"] = apis_array;

    return json;
}

qtplugin::expected<SecurityPermissions, PluginError>
SecurityPermissions::from_json(const QJsonObject& json) {
    SecurityPermissions permissions;

    permissions.allow_file_system_read =
        json["allow_file_system_read"].toBool();
    permissions.allow_file_system_write =
        json["allow_file_system_write"].toBool();
    permissions.allow_network_access = json["allow_network_access"].toBool();
    permissions.allow_process_creation =
        json["allow_process_creation"].toBool();
    permissions.allow_system_calls = json["allow_system_calls"].toBool();
    permissions.allow_registry_access = json["allow_registry_access"].toBool();
    permissions.allow_environment_access =
        json["allow_environment_access"].toBool();

    if (json.contains("allowed_directories") &&
        json["allowed_directories"].isArray()) {
        QJsonArray dirs_array = json["allowed_directories"].toArray();
        for (const QJsonValue& value : dirs_array) {
            permissions.allowed_directories.append(value.toString());
        }
    }

    if (json.contains("allowed_hosts") && json["allowed_hosts"].isArray()) {
        QJsonArray hosts_array = json["allowed_hosts"].toArray();
        for (const QJsonValue& value : hosts_array) {
            permissions.allowed_hosts.append(value.toString());
        }
    }

    if (json.contains("blocked_apis") && json["blocked_apis"].isArray()) {
        QJsonArray apis_array = json["blocked_apis"].toArray();
        for (const QJsonValue& value : apis_array) {
            permissions.blocked_apis.append(value.toString());
        }
    }

    return permissions;
}

// === SecurityPolicy Implementation ===

QJsonObject SecurityPolicy::to_json() const {
    QJsonObject json;
    json["level"] = static_cast<int>(level);
    json["limits"] = limits.to_json();
    json["permissions"] = permissions.to_json();
    json["policy_name"] = policy_name;
    json["description"] = description;
    return json;
}

qtplugin::expected<SecurityPolicy, PluginError> SecurityPolicy::from_json(
    const QJsonObject& json) {
    SecurityPolicy policy;

    if (json.contains("level")) {
        policy.level = static_cast<SandboxSecurityLevel>(json["level"].toInt());
    }

    if (json.contains("limits")) {
        auto limits_result =
            ResourceLimits::from_json(json["limits"].toObject());
        if (limits_result) {
            policy.limits = limits_result.value();
        }
    }

    if (json.contains("permissions")) {
        auto permissions_result =
            SecurityPermissions::from_json(json["permissions"].toObject());
        if (permissions_result) {
            policy.permissions = permissions_result.value();
        }
    }

    if (json.contains("policy_name")) {
        policy.policy_name = json["policy_name"].toString();
    }

    if (json.contains("description")) {
        policy.description = json["description"].toString();
    }

    return policy;
}

SecurityPolicy SecurityPolicy::create_unrestricted_policy() {
    SecurityPolicy policy;
    policy.level = SandboxSecurityLevel::Unrestricted;
    policy.policy_name = "unrestricted";
    policy.description = "Unrestricted access for trusted native plugins";

    // No limits for unrestricted policy
    policy.limits.cpu_time_limit = std::chrono::hours(24);
    policy.limits.memory_limit_mb = 8192;
    policy.limits.execution_timeout = std::chrono::hours(1);

    // All permissions allowed
    policy.permissions.allow_file_system_read = true;
    policy.permissions.allow_file_system_write = true;
    policy.permissions.allow_network_access = true;
    policy.permissions.allow_process_creation = true;
    policy.permissions.allow_system_calls = true;
    policy.permissions.allow_registry_access = true;
    policy.permissions.allow_environment_access = true;

    return policy;
}

SecurityPolicy SecurityPolicy::create_limited_policy() {
    SecurityPolicy policy;
    policy.level = SandboxSecurityLevel::Limited;
    policy.policy_name = "limited";
    policy.description = "Limited access with basic restrictions";

    // Moderate limits
    policy.limits.cpu_time_limit = std::chrono::minutes(10);
    policy.limits.memory_limit_mb = 512;
    policy.limits.disk_space_limit_mb = 200;
    policy.limits.max_file_handles = 100;
    policy.limits.max_network_connections = 20;
    policy.limits.execution_timeout = std::chrono::minutes(5);

    // Limited permissions
    policy.permissions.allow_file_system_read = true;
    policy.permissions.allow_file_system_write = false;  // Read-only by default
    policy.permissions.allow_network_access = true;
    policy.permissions.allow_process_creation = false;
    policy.permissions.allow_system_calls = false;
    policy.permissions.allow_registry_access = false;
    policy.permissions.allow_environment_access = false;

    // Allow access to common directories
    policy.permissions.allowed_directories = {
        QStandardPaths::writableLocation(QStandardPaths::TempLocation),
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation)
    };

    return policy;
}

SecurityPolicy SecurityPolicy::create_sandboxed_policy() {
    SecurityPolicy policy;
    policy.level = SandboxSecurityLevel::Sandboxed;
    policy.policy_name = "sandboxed";
    policy.description = "Full sandboxing with process isolation";

    // Restrictive limits
    policy.limits.cpu_time_limit = std::chrono::minutes(5);
    policy.limits.memory_limit_mb = 256;
    policy.limits.disk_space_limit_mb = 100;
    policy.limits.max_file_handles = 50;
    policy.limits.max_network_connections = 10;
    policy.limits.execution_timeout = std::chrono::minutes(2);

    // Restrictive permissions
    policy.permissions.allow_file_system_read = false;
    policy.permissions.allow_file_system_write = false;
    policy.permissions.allow_network_access = false;
    policy.permissions.allow_process_creation = false;
    policy.permissions.allow_system_calls = false;
    policy.permissions.allow_registry_access = false;
    policy.permissions.allow_environment_access = false;

    // Only allow access to temp directory
    policy.permissions.allowed_directories = {
        QStandardPaths::writableLocation(QStandardPaths::TempLocation)
    };

    return policy;
}

SecurityPolicy SecurityPolicy::create_strict_policy() {
    SecurityPolicy policy;
    policy.level = SandboxSecurityLevel::Strict;
    policy.policy_name = "strict";
    policy.description = "Maximum security with minimal permissions";

    // Very restrictive limits
    policy.limits.cpu_time_limit = std::chrono::minutes(2);
    policy.limits.memory_limit_mb = 128;
    policy.limits.disk_space_limit_mb = 50;
    policy.limits.max_file_handles = 25;
    policy.limits.max_network_connections = 5;
    policy.limits.execution_timeout = std::chrono::minutes(1);

    // Minimal permissions - everything denied by default
    policy.permissions.allow_file_system_read = false;
    policy.permissions.allow_file_system_write = false;
    policy.permissions.allow_network_access = false;
    policy.permissions.allow_process_creation = false;
    policy.permissions.allow_system_calls = false;
    policy.permissions.allow_registry_access = false;
    policy.permissions.allow_environment_access = false;

    // No directory access allowed
    policy.permissions.allowed_directories.clear();

    // Block common dangerous APIs
    policy.permissions.blocked_apis = {
        "system", "exec", "fork", "CreateProcess", "ShellExecute",
        "LoadLibrary", "dlopen", "mmap", "VirtualAlloc"
    };

    return policy;
}

// === PluginSandbox Implementation ===

PluginSandbox::PluginSandbox(const SecurityPolicy& policy, QObject* parent)
    : QObject(parent), m_policy(policy) {
    // Initialize timers
    m_resource_monitor_timer = std::make_unique<QTimer>(this);
    m_execution_timer = std::make_unique<QTimer>(this);

    // Initialize resource monitor and security enforcer
    m_resource_monitor = std::make_unique<SandboxResourceMonitor>(this);
    m_security_enforcer = std::make_unique<SecurityEnforcer>(policy, this);

    // Connect signals
    connect(m_resource_monitor_timer.get(), &QTimer::timeout, this, &PluginSandbox::monitor_resources);
    connect(m_execution_timer.get(), &QTimer::timeout, this, [this]() {
        qCWarning(sandboxLog) << "Plugin execution timeout reached, terminating";
        terminate_plugin();
    });

    // Set single shot for execution timer
    m_execution_timer->setSingleShot(true);
}

PluginSandbox::~PluginSandbox() {
    if (m_active) {
        shutdown();
    }
}

qtplugin::expected<void, PluginError> PluginSandbox::initialize() {
    qCWarning(sandboxLog) << "INITIALIZE: PluginSandbox::initialize() called";
    QMutexLocker locker(&m_mutex);

    if (m_active) {
        qCWarning(sandboxLog) << "INITIALIZE: Already active, returning error";
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidState,
            "Sandbox is already initialized"
        });
    }

    // Validate security policy
    qCWarning(sandboxLog) << "VALIDATION: Checking policy with name:" << m_policy.policy_name << "isEmpty:" << m_policy.policy_name.isEmpty();
    if (m_policy.policy_name.isEmpty()) {
        qCWarning(sandboxLog) << "VALIDATION: Policy validation failed: empty name";
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidConfiguration,
            "Security policy name cannot be empty"
        });
    }

    if (m_policy.limits.memory_limit_mb == 0) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidConfiguration,
            "Memory limit must be greater than 0"
        });
    }

    if (m_policy.limits.cpu_time_limit.count() <= 0) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidConfiguration,
            "CPU time limit must be positive"
        });
    }

    if (m_policy.limits.execution_timeout.count() <= 0) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidConfiguration,
            "Execution timeout must be positive"
        });
    }

    // Initialize resource usage tracking
    m_resource_usage.start_time = std::chrono::steady_clock::now();
    m_resource_usage.cpu_time_used = std::chrono::milliseconds(0);
    m_resource_usage.memory_used_mb = 0;
    m_resource_usage.disk_space_used_mb = 0;
    m_resource_usage.file_handles_used = 0;
    m_resource_usage.network_connections_used = 0;

    // Initialize security enforcer
    if (m_security_enforcer && !m_security_enforcer->initialize()) {
        qCWarning(sandboxLog) << "Failed to initialize security enforcer";
    }

    // Initialize and start resource monitoring if enabled
    if (m_policy.level != SandboxSecurityLevel::Unrestricted) {
        if (m_resource_monitor->initialize()) {
            m_resource_monitor_timer->start(100); // Monitor every 100ms for better responsiveness
            qCDebug(sandboxLog) << "Resource monitoring started";
        } else {
            qCWarning(sandboxLog) << "Failed to initialize resource monitoring";
        }
    }

    m_active = true;
    qCDebug(sandboxLog) << "Sandbox initialized with policy:" << m_policy.policy_name;

    return {};
}

void PluginSandbox::shutdown() {
    QMutexLocker locker(&m_mutex);

    if (!m_active) {
        return;
    }

    // Stop timers
    if (m_resource_monitor_timer) {
        m_resource_monitor_timer->stop();
    }
    if (m_execution_timer) {
        m_execution_timer->stop();
    }

    // Shutdown resource monitor and security enforcer
    if (m_resource_monitor) {
        m_resource_monitor->shutdown();
    }
    if (m_security_enforcer) {
        m_security_enforcer->shutdown();
    }

    // Terminate any running process
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(5000)) {
            m_process->kill();
            m_process->waitForFinished(2000);
        }
    }

    m_active = false;
    qCDebug(sandboxLog) << "Sandbox shutdown completed";
}

qtplugin::expected<QJsonObject, PluginError> PluginSandbox::execute_plugin(
    const QString& plugin_path, PluginType plugin_type, const QJsonObject& arguments) {

    QMutexLocker locker(&m_mutex);

    if (!m_active) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidState,
            "Sandbox is not initialized"
        });
    }

    if (m_process && m_process->state() != QProcess::NotRunning) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidState,
            "Another plugin is already running in this sandbox"
        });
    }

    // Validate plugin path
    QFileInfo plugin_info(plugin_path);
    if (!plugin_info.exists()) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::FileNotFound,
            "Plugin file not found: " + plugin_path.toStdString()
        });
    }

    // Validate permissions
    auto permission_result = validate_permissions("execute_plugin");
    if (!permission_result) {
        return qtplugin::unexpected(permission_result.error());
    }

    // Create process
    m_process = std::make_unique<QProcess>(this);

    // Connect process signals
    connect(m_process.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PluginSandbox::handle_process_finished);
    connect(m_process.get(), &QProcess::errorOccurred,
            this, &PluginSandbox::handle_process_error);

    // Set the process for security monitoring
    if (m_security_enforcer) {
        m_security_enforcer->set_monitored_process(m_process.get());
    }

    // Setup process environment
    auto env_result = setup_process_environment();
    if (!env_result) {
        return qtplugin::unexpected(env_result.error());
    }

    // Apply resource limits
    auto limits_result = apply_resource_limits();
    if (!limits_result) {
        return qtplugin::unexpected(limits_result.error());
    }

    // Create sandbox script based on plugin type
    QString sandbox_script = create_sandbox_script(plugin_type, plugin_path);

    // Start execution timer
    m_execution_timer->start(static_cast<int>(m_policy.limits.execution_timeout.count()));

    // Execute the plugin
    QString program;
    QStringList args;

    switch (plugin_type) {
        case PluginType::Native:
            program = plugin_path;
            break;
        case PluginType::Python:
            program = "python";
            args << plugin_path;
            break;
        case PluginType::JavaScript:
            program = "node";
            args << plugin_path;
            break;
        default:
            return qtplugin::unexpected(PluginError{
                PluginErrorCode::NotSupported,
                "Unsupported plugin type"
            });
    }

    // Add arguments from JSON
    if (!arguments.isEmpty()) {
        QJsonDocument doc(arguments);
        args << "--args" << doc.toJson(QJsonDocument::Compact);
    }

    qCDebug(sandboxLog) << "Starting plugin execution:" << program << args;
    m_process->start(program, args);

    if (!m_process->waitForStarted(5000)) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::ExecutionFailed,
            "Failed to start plugin process: " + m_process->errorString().toStdString()
        });
    }

    // Emit initial resource usage update to ensure at least one signal is sent
    if (m_policy.level != SandboxSecurityLevel::Unrestricted) {
        QTimer::singleShot(50, this, [this]() {
            update_resource_usage();
        });
    }

    // Return success - actual result will be emitted via signals
    QJsonObject result;
    result["status"] = "started";
    result["pid"] = static_cast<qint64>(m_process->processId());
    result["plugin_path"] = plugin_path;

    return result;
}

ResourceUsage PluginSandbox::get_resource_usage() const {
    QMutexLocker locker(&m_mutex);
    return m_resource_usage;
}

qtplugin::expected<void, PluginError> PluginSandbox::update_policy(const SecurityPolicy& policy) {
    QMutexLocker locker(&m_mutex);

    if (m_process && m_process->state() != QProcess::NotRunning) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidState,
            "Cannot update policy while plugin is running"
        });
    }

    m_policy = policy;
    qCDebug(sandboxLog) << "Security policy updated to:" << policy.policy_name;

    return {};
}

void PluginSandbox::terminate_plugin() {
    QMutexLocker locker(&m_mutex);

    if (m_process && m_process->state() != QProcess::NotRunning) {
        qCWarning(sandboxLog) << "Terminating plugin execution";
        m_process->terminate();

        if (!m_process->waitForFinished(3000)) {
            qCWarning(sandboxLog) << "Force killing plugin process";
            m_process->kill();
            m_process->waitForFinished(1000);
        }
    }

    // Stop execution timer
    if (m_execution_timer) {
        m_execution_timer->stop();
    }
}

void PluginSandbox::monitor_resources() {
    if (!m_active || !m_process || m_process->state() != QProcess::Running) {
        return;
    }

    // Update resource usage
    update_resource_usage();

    // Check if limits are exceeded
    if (m_resource_usage.exceeds_limits(m_policy.limits)) {
        QString resource = "unknown";

        // Determine which resource exceeded the limit
        if (m_resource_usage.cpu_time_used > m_policy.limits.cpu_time_limit) {
            resource = "cpu_time";
        } else if (m_resource_usage.memory_used_mb > m_policy.limits.memory_limit_mb) {
            resource = "memory";
        } else if (m_resource_usage.disk_space_used_mb > m_policy.limits.disk_space_limit_mb) {
            resource = "disk_space";
        } else if (m_resource_usage.file_handles_used > m_policy.limits.max_file_handles) {
            resource = "file_handles";
        } else if (m_resource_usage.network_connections_used > m_policy.limits.max_network_connections) {
            resource = "network_connections";
        }

        qCWarning(sandboxLog) << "Resource limit exceeded:" << resource;
        emit resource_limit_exceeded(resource, m_resource_usage.to_json());

        // Terminate the plugin
        terminate_plugin();
    }
}

void PluginSandbox::handle_process_finished(int exit_code, QProcess::ExitStatus exit_status) {
    qCDebug(sandboxLog) << "Plugin process finished with exit code:" << exit_code
                        << "status:" << (exit_status == QProcess::NormalExit ? "Normal" : "Crashed");

    // Stop timers
    if (m_execution_timer) {
        m_execution_timer->stop();
    }

    // Prepare result
    QJsonObject result;
    result["exit_code"] = exit_code;
    result["exit_status"] = (exit_status == QProcess::NormalExit) ? "normal" : "crashed";
    result["resource_usage"] = m_resource_usage.to_json();

    // Get process output if available
    if (m_process) {
        QString stdout_data = m_process->readAllStandardOutput();
        QString stderr_data = m_process->readAllStandardError();

        if (!stdout_data.isEmpty()) {
            result["stdout"] = stdout_data;
        }
        if (!stderr_data.isEmpty()) {
            result["stderr"] = stderr_data;
        }
    }

    emit execution_completed(exit_code, result);
}

void PluginSandbox::handle_process_error(QProcess::ProcessError error) {
    QString error_string;
    switch (error) {
        case QProcess::FailedToStart:
            error_string = "Failed to start";
            break;
        case QProcess::Crashed:
            error_string = "Process crashed";
            break;
        case QProcess::Timedout:
            error_string = "Process timed out";
            break;
        case QProcess::WriteError:
            error_string = "Write error";
            break;
        case QProcess::ReadError:
            error_string = "Read error";
            break;
        default:
            error_string = "Unknown error";
            break;
    }

    qCWarning(sandboxLog) << "Plugin process error:" << error_string;

    // Stop execution timer
    if (m_execution_timer) {
        m_execution_timer->stop();
    }

    // Emit security violation for process errors
    QJsonObject details;
    details["error_type"] = error_string;
    details["error_code"] = static_cast<int>(error);
    details["resource_usage"] = m_resource_usage.to_json();

    emit security_violation("process_error", details);
}

qtplugin::expected<void, PluginError> PluginSandbox::setup_process_environment() {
    if (!m_process) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidState,
            "Process not initialized"
        });
    }

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // Restrict environment variables based on security policy
    if (!m_policy.permissions.allow_environment_access) {
        // Clear all environment variables and set only essential ones
        env.clear();
        env.insert("PATH", QProcessEnvironment::systemEnvironment().value("PATH"));

        // Add minimal required variables
        #ifdef Q_OS_WIN
        env.insert("SYSTEMROOT", QProcessEnvironment::systemEnvironment().value("SYSTEMROOT"));
        env.insert("TEMP", QStandardPaths::writableLocation(QStandardPaths::TempLocation));
        #else
        env.insert("HOME", QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
        env.insert("TMPDIR", QStandardPaths::writableLocation(QStandardPaths::TempLocation));
        #endif
    }

    // Set sandbox-specific variables
    env.insert("QTPLUGIN_SANDBOX", "1");
    env.insert("QTPLUGIN_SECURITY_LEVEL", QString::number(static_cast<int>(m_policy.level)));
    env.insert("QTPLUGIN_POLICY_NAME", m_policy.policy_name);

    m_process->setProcessEnvironment(env);

    return {};
}

qtplugin::expected<void, PluginError> PluginSandbox::apply_resource_limits() {
    if (!m_process) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidState,
            "Process not initialized"
        });
    }

    // Set working directory to a restricted location
    QString temp_dir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString sandbox_dir = temp_dir + "/qtplugin_sandbox_" + QString::number(QCoreApplication::applicationPid());

    QDir().mkpath(sandbox_dir);
    m_process->setWorkingDirectory(sandbox_dir);

    // Note: Actual resource limits (CPU, memory) would require platform-specific implementation
    // This is a placeholder for the interface
    qCDebug(sandboxLog) << "Applied resource limits - working directory:" << sandbox_dir;

    return {};
}

qtplugin::expected<void, PluginError> PluginSandbox::validate_permissions(const QString& operation) {
    // Check if the operation is allowed based on current security policy
    if (operation == "execute_plugin") {
        // Always allow plugin execution - specific permissions are checked during execution
        return {};
    }

    if (operation == "file_read" && !m_policy.permissions.allow_file_system_read) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::PermissionDenied,
            "File system read access denied"
        });
    }

    if (operation == "file_write" && !m_policy.permissions.allow_file_system_write) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::PermissionDenied,
            "File system write access denied"
        });
    }

    if (operation == "network_access" && !m_policy.permissions.allow_network_access) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::PermissionDenied,
            "Network access denied"
        });
    }

    return {};
}

void PluginSandbox::update_resource_usage() {
    if (!m_process || m_process->state() != QProcess::Running || !m_resource_monitor) {
        return;
    }

    // Get process ID
    qint64 pid = m_process->processId();
    if (pid <= 0) {
        return;
    }

    // Use the resource monitor to get accurate usage
    m_resource_usage = m_resource_monitor->get_process_usage(pid);

    // Emit signal for external monitoring
    emit resource_usage_updated(m_resource_usage);
}

QString PluginSandbox::create_sandbox_script(PluginType plugin_type, const QString& plugin_path) {
    Q_UNUSED(plugin_type)
    Q_UNUSED(plugin_path)

    // This would create a wrapper script for additional sandboxing
    // For now, return empty string to indicate direct execution
    return QString();
}

// === ResourceUsage Implementation ===

QJsonObject ResourceUsage::to_json() const {
    QJsonObject json;
    json["cpu_time_used"] = static_cast<qint64>(cpu_time_used.count());
    json["memory_used_mb"] = static_cast<qint64>(memory_used_mb);
    json["disk_space_used_mb"] = static_cast<qint64>(disk_space_used_mb);
    json["file_handles_used"] = file_handles_used;
    json["network_connections_used"] = network_connections_used;

    // Convert time_point to milliseconds since epoch
    auto duration = start_time.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    json["start_time"] = static_cast<qint64>(millis.count());

    return json;
}

bool ResourceUsage::exceeds_limits(const ResourceLimits& limits) const {
    // Check CPU time limit
    if (cpu_time_used > limits.cpu_time_limit) {
        return true;
    }

    // Check memory limit
    if (memory_used_mb > limits.memory_limit_mb) {
        return true;
    }

    // Check disk space limit
    if (disk_space_used_mb > limits.disk_space_limit_mb) {
        return true;
    }

    // Check file handles limit
    if (file_handles_used > limits.max_file_handles) {
        return true;
    }

    // Check network connections limit
    if (network_connections_used > limits.max_network_connections) {
        return true;
    }

    // Check execution timeout
    auto current_time = std::chrono::steady_clock::now();
    auto elapsed = current_time - start_time;
    if (elapsed > limits.execution_timeout) {
        return true;
    }

    return false;
}

// === SandboxManager Implementation ===

SandboxManager& SandboxManager::instance() {
    static SandboxManager instance;
    static bool initialized = false;

    if (!initialized) {
        instance.setup_default_policies();
        initialized = true;
    }

    return instance;
}

qtplugin::expected<std::shared_ptr<PluginSandbox>, PluginError>
SandboxManager::create_sandbox(const QString& sandbox_id, const SecurityPolicy& policy) {
    QMutexLocker locker(&m_mutex);

    // Check if sandbox already exists
    if (m_sandboxes.find(sandbox_id) != m_sandboxes.end()) {
        return qtplugin::unexpected(PluginError{
            PluginErrorCode::InvalidArgument,
            "Sandbox with ID '" + sandbox_id.toStdString() + "' already exists"
        });
    }

    // Create new sandbox
    auto sandbox = std::make_shared<PluginSandbox>(policy);

    // Initialize the sandbox
    auto init_result = sandbox->initialize();
    if (!init_result) {
        return qtplugin::unexpected(init_result.error());
    }

    // Store the sandbox
    m_sandboxes[sandbox_id] = sandbox;

    qCDebug(sandboxLog) << "Created sandbox:" << sandbox_id << "with policy:" << policy.policy_name;
    emit sandbox_created(sandbox_id);

    return sandbox;
}

std::shared_ptr<PluginSandbox> SandboxManager::get_sandbox(const QString& sandbox_id) {
    QMutexLocker locker(&m_mutex);

    auto it = m_sandboxes.find(sandbox_id);
    if (it != m_sandboxes.end()) {
        return it->second;
    }

    return nullptr;
}

void SandboxManager::remove_sandbox(const QString& sandbox_id) {
    QMutexLocker locker(&m_mutex);

    auto it = m_sandboxes.find(sandbox_id);
    if (it != m_sandboxes.end()) {
        // Shutdown the sandbox before removing
        it->second->shutdown();
        m_sandboxes.erase(it);

        qCDebug(sandboxLog) << "Removed sandbox:" << sandbox_id;
        emit sandbox_removed(sandbox_id);
    }
}

std::vector<QString> SandboxManager::get_active_sandboxes() const {
    QMutexLocker locker(&m_mutex);

    std::vector<QString> active_sandboxes;
    for (const auto& pair : m_sandboxes) {
        if (pair.second->is_active()) {
            active_sandboxes.push_back(pair.first);
        }
    }

    return active_sandboxes;
}

void SandboxManager::register_policy(const QString& policy_name, const SecurityPolicy& policy) {
    QMutexLocker locker(&m_mutex);

    m_policies[policy_name] = policy;
    qCDebug(sandboxLog) << "Registered security policy:" << policy_name;
}

qtplugin::expected<SecurityPolicy, PluginError>
SandboxManager::get_policy(const QString& policy_name) {
    QMutexLocker locker(&m_mutex);

    auto it = m_policies.find(policy_name);
    if (it != m_policies.end()) {
        return it->second;
    }

    return qtplugin::unexpected(PluginError{
        PluginErrorCode::NotFound,
        "Security policy '" + policy_name.toStdString() + "' not found"
    });
}

std::vector<QString> SandboxManager::get_registered_policies() const {
    QMutexLocker locker(&m_mutex);

    std::vector<QString> policy_names;
    for (const auto& pair : m_policies) {
        policy_names.push_back(pair.first);
    }

    return policy_names;
}

void SandboxManager::shutdown_all() {
    QMutexLocker locker(&m_mutex);

    qCDebug(sandboxLog) << "Shutting down all sandboxes";

    for (auto& pair : m_sandboxes) {
        pair.second->shutdown();
        emit sandbox_removed(pair.first);
    }

    m_sandboxes.clear();
    qCDebug(sandboxLog) << "All sandboxes shutdown completed";
}

void SandboxManager::setup_default_policies() {
    // Register default security policies
    register_policy("unrestricted", SecurityPolicy::create_unrestricted_policy());
    register_policy("limited", SecurityPolicy::create_limited_policy());
    register_policy("sandboxed", SecurityPolicy::create_sandboxed_policy());
    register_policy("strict", SecurityPolicy::create_strict_policy());

    qCDebug(sandboxLog) << "Default security policies registered";
}

}  // namespace qtplugin
