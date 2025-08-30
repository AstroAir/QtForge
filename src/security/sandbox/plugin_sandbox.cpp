/**
 * @file plugin_sandbox.cpp
 * @brief Implementation of plugin sandboxing and security system
 * @version 3.2.0
 */

#include "../../../include/qtplugin/security/sandbox/plugin_sandbox.hpp"
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

// === PluginSandbox Implementation ===

PluginSandbox::PluginSandbox(const SecurityPolicy& policy, QObject* parent)
    : QObject(parent), m_policy(policy) {}

PluginSandbox::~PluginSandbox() = default;

void PluginSandbox::monitor_resources() {
    Q_UNIMPLEMENTED();
    qCWarning(sandboxLog) << "Resource monitoring is not yet implemented!";
}

void PluginSandbox::handle_process_finished(int exit_code,
                                            QProcess::ExitStatus exit_status) {
    Q_UNUSED(exit_code)
    Q_UNUSED(exit_status)
    // TODO: Implement process finished handling
}

void PluginSandbox::handle_process_error(QProcess::ProcessError error) {
    Q_UNUSED(error)
    // TODO: Implement process error handling
}

}  // namespace qtplugin
