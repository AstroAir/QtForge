/**
 * @file remote_plugin_configuration.cpp
 * @brief Implementation of remote plugin configuration system
 */

#include <qtplugin/remote/remote_plugin_configuration.hpp>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QRegularExpression>

namespace qtplugin {

// === ProxyConfiguration Implementation ===

QJsonObject ProxyConfiguration::to_json() const {
    QJsonObject json;
    json["type"] = static_cast<int>(type);
    json["host"] = host;
    json["port"] = port;
    json["username"] = username;
    // Note: password should be stored securely, not in JSON
    return json;
}

ProxyConfiguration ProxyConfiguration::from_json(const QJsonObject& json) {
    ProxyConfiguration config;
    config.type = static_cast<Type>(json["type"].toInt());
    config.host = json["host"].toString();
    config.port = json["port"].toInt();
    config.username = json["username"].toString();
    return config;
}

// === SecurityPolicyConfiguration Implementation ===

QJsonObject SecurityPolicyConfiguration::to_json() const {
    QJsonObject json;
    json["default_security_level"] = static_cast<int>(default_security_level);
    json["require_signature_verification"] = require_signature_verification;
    json["allow_self_signed_certificates"] = allow_self_signed_certificates;
    json["enable_certificate_pinning"] = enable_certificate_pinning;
    json["require_https"] = require_https;
    json["enable_sandbox"] = enable_sandbox;
    json["allow_network_access"] = allow_network_access;
    json["allow_file_system_access"] = allow_file_system_access;
    
    QJsonArray trusted_domains_array;
    for (const QString& domain : trusted_domains) {
        trusted_domains_array.append(domain);
    }
    json["trusted_domains"] = trusted_domains_array;
    
    QJsonArray blocked_domains_array;
    for (const QString& domain : blocked_domains) {
        blocked_domains_array.append(domain);
    }
    json["blocked_domains"] = blocked_domains_array;
    
    QJsonArray fingerprints_array;
    for (const QString& fingerprint : trusted_certificate_fingerprints) {
        fingerprints_array.append(fingerprint);
    }
    json["trusted_certificate_fingerprints"] = fingerprints_array;
    
    json["signature_cache_ttl"] = static_cast<qint64>(signature_cache_ttl.count());
    
    return json;
}

SecurityPolicyConfiguration SecurityPolicyConfiguration::from_json(const QJsonObject& json) {
    SecurityPolicyConfiguration config;
    config.default_security_level = static_cast<RemoteSecurityLevel>(json["default_security_level"].toInt());
    config.require_signature_verification = json["require_signature_verification"].toBool();
    config.allow_self_signed_certificates = json["allow_self_signed_certificates"].toBool();
    config.enable_certificate_pinning = json["enable_certificate_pinning"].toBool();
    config.require_https = json["require_https"].toBool();
    config.enable_sandbox = json["enable_sandbox"].toBool();
    config.allow_network_access = json["allow_network_access"].toBool();
    config.allow_file_system_access = json["allow_file_system_access"].toBool();
    
    QJsonArray trusted_domains_array = json["trusted_domains"].toArray();
    for (const auto& value : trusted_domains_array) {
        config.trusted_domains.append(value.toString());
    }
    
    QJsonArray blocked_domains_array = json["blocked_domains"].toArray();
    for (const auto& value : blocked_domains_array) {
        config.blocked_domains.append(value.toString());
    }
    
    QJsonArray fingerprints_array = json["trusted_certificate_fingerprints"].toArray();
    for (const auto& value : fingerprints_array) {
        config.trusted_certificate_fingerprints.append(value.toString());
    }
    
    config.signature_cache_ttl = std::chrono::seconds(json["signature_cache_ttl"].toInt());
    
    return config;
}

// === CacheConfiguration Implementation ===

QJsonObject CacheConfiguration::to_json() const {
    QJsonObject json;
    json["cache_directory"] = QString::fromStdString(cache_directory.string());
    json["max_cache_size"] = max_cache_size;
    json["default_ttl"] = static_cast<qint64>(default_ttl.count());
    json["cleanup_interval"] = static_cast<qint64>(cleanup_interval.count());
    json["enable_compression"] = enable_compression;
    json["enable_encryption"] = enable_encryption;
    json["max_concurrent_downloads"] = max_concurrent_downloads;
    json["max_file_size"] = max_file_size;
    json["default_cache_policy"] = static_cast<int>(default_cache_policy);
    return json;
}

CacheConfiguration CacheConfiguration::from_json(const QJsonObject& json) {
    CacheConfiguration config;
    config.cache_directory = json["cache_directory"].toString().toStdString();
    config.max_cache_size = json["max_cache_size"].toInt();
    config.default_ttl = std::chrono::seconds(json["default_ttl"].toInt());
    config.cleanup_interval = std::chrono::seconds(json["cleanup_interval"].toInt());
    config.enable_compression = json["enable_compression"].toBool();
    config.enable_encryption = json["enable_encryption"].toBool();
    config.max_concurrent_downloads = json["max_concurrent_downloads"].toInt();
    config.max_file_size = json["max_file_size"].toInt();
    config.default_cache_policy = static_cast<CachePolicy>(json["default_cache_policy"].toInt());
    return config;
}

// === NetworkConfiguration Implementation ===

QJsonObject NetworkConfiguration::to_json() const {
    QJsonObject json;
    json["connection_timeout"] = static_cast<qint64>(connection_timeout.count());
    json["read_timeout"] = static_cast<qint64>(read_timeout.count());
    json["max_retries"] = max_retries;
    json["retry_delay"] = static_cast<qint64>(retry_delay.count());
    json["enable_http2"] = enable_http2;
    json["verify_ssl_certificates"] = verify_ssl_certificates;
    json["user_agent"] = user_agent;
    json["proxy"] = proxy.to_json();
    json["custom_headers"] = custom_headers;
    return json;
}

NetworkConfiguration NetworkConfiguration::from_json(const QJsonObject& json) {
    NetworkConfiguration config;
    config.connection_timeout = std::chrono::seconds(json["connection_timeout"].toInt());
    config.read_timeout = std::chrono::seconds(json["read_timeout"].toInt());
    config.max_retries = json["max_retries"].toInt();
    config.retry_delay = std::chrono::seconds(json["retry_delay"].toInt());
    config.enable_http2 = json["enable_http2"].toBool();
    config.verify_ssl_certificates = json["verify_ssl_certificates"].toBool();
    config.user_agent = json["user_agent"].toString();
    config.proxy = ProxyConfiguration::from_json(json["proxy"].toObject());
    config.custom_headers = json["custom_headers"].toObject();
    return config;
}

// === UpdateConfiguration Implementation ===

QJsonObject UpdateConfiguration::to_json() const {
    QJsonObject json;
    json["policy"] = static_cast<int>(policy);
    json["check_interval"] = static_cast<qint64>(check_interval.count());
    json["notify_updates"] = notify_updates;
    json["backup_before_update"] = backup_before_update;
    json["rollback_on_failure"] = rollback_on_failure;
    
    QJsonArray channels_array;
    for (const QString& channel : update_channels) {
        channels_array.append(channel);
    }
    json["update_channels"] = channels_array;
    
    return json;
}

UpdateConfiguration UpdateConfiguration::from_json(const QJsonObject& json) {
    UpdateConfiguration config;
    config.policy = static_cast<AutoUpdatePolicy>(json["policy"].toInt());
    config.check_interval = std::chrono::seconds(json["check_interval"].toInt());
    config.notify_updates = json["notify_updates"].toBool();
    config.backup_before_update = json["backup_before_update"].toBool();
    config.rollback_on_failure = json["rollback_on_failure"].toBool();
    
    QJsonArray channels_array = json["update_channels"].toArray();
    config.update_channels.clear();
    for (const auto& value : channels_array) {
        config.update_channels.append(value.toString());
    }
    
    return config;
}

// === RemotePluginConfiguration Implementation ===

RemotePluginConfiguration::RemotePluginConfiguration() {
    initialize_defaults();
}

RemotePluginConfiguration::RemotePluginConfiguration(const QJsonObject& json) {
    *this = from_json(json);
}

RemotePluginConfiguration::RemotePluginConfiguration(const RemotePluginConfiguration& other)
    : m_remote_plugins_enabled(other.m_remote_plugins_enabled),
      m_version(other.m_version),
      m_security_policy(other.m_security_policy),
      m_cache_config(other.m_cache_config),
      m_network_config(other.m_network_config),
      m_update_config(other.m_update_config),
      m_source_manager(other.m_source_manager) {
}

RemotePluginConfiguration& RemotePluginConfiguration::operator=(const RemotePluginConfiguration& other) {
    if (this != &other) {
        m_remote_plugins_enabled = other.m_remote_plugins_enabled;
        m_version = other.m_version;
        m_security_policy = other.m_security_policy;
        m_cache_config = other.m_cache_config;
        m_network_config = other.m_network_config;
        m_update_config = other.m_update_config;
        m_source_manager = other.m_source_manager;
    }
    return *this;
}

RemotePluginConfiguration::~RemotePluginConfiguration() = default;

qtplugin::expected<void, PluginError> RemotePluginConfiguration::add_trusted_source(const RemotePluginSource& source) {
    return m_source_manager.add_source(source);
}

qtplugin::expected<void, PluginError> RemotePluginConfiguration::remove_source(const QString& source_id) {
    return m_source_manager.remove_source(source_id);
}

std::vector<RemotePluginSource> RemotePluginConfiguration::get_all_sources() const {
    return m_source_manager.get_all_sources();
}

qtplugin::expected<void, PluginError> RemotePluginConfiguration::validate() const {
    // Validate cache configuration
    if (m_cache_config.max_cache_size <= 0) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid cache size configuration");
    }
    
    if (m_cache_config.max_file_size <= 0) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid max file size configuration");
    }
    
    // Validate network configuration
    if (m_network_config.connection_timeout.count() <= 0) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid connection timeout configuration");
    }
    
    if (m_network_config.max_retries < 0) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid max retries configuration");
    }
    
    // Validate update configuration
    if (m_update_config.check_interval.count() <= 0) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid update check interval configuration");
    }
    
    return qtplugin::make_success();
}

bool RemotePluginConfiguration::is_domain_trusted(const QString& domain) const {
    // Check if domain is in trusted list
    for (const QString& trusted : m_security_policy.trusted_domains) {
        QRegularExpression regex(trusted, QRegularExpression::CaseInsensitiveOption);
        if (regex.match(domain).hasMatch()) {
            return true;
        }
    }
    
    return false;
}

bool RemotePluginConfiguration::is_domain_blocked(const QString& domain) const {
    // Check if domain is in blocked list
    for (const QString& blocked : m_security_policy.blocked_domains) {
        QRegularExpression regex(blocked, QRegularExpression::CaseInsensitiveOption);
        if (regex.match(domain).hasMatch()) {
            return true;
        }
    }
    
    return false;
}

QJsonObject RemotePluginConfiguration::to_json() const {
    QJsonObject json;
    json["version"] = m_version;
    json["remote_plugins_enabled"] = m_remote_plugins_enabled;
    json["security_policy"] = m_security_policy.to_json();
    json["cache_config"] = m_cache_config.to_json();
    json["network_config"] = m_network_config.to_json();
    json["update_config"] = m_update_config.to_json();
    json["sources"] = m_source_manager.save_to_config();
    return json;
}

RemotePluginConfiguration RemotePluginConfiguration::from_json(const QJsonObject& json) {
    RemotePluginConfiguration config;
    config.m_version = json["version"].toString();
    config.m_remote_plugins_enabled = json["remote_plugins_enabled"].toBool();
    config.m_security_policy = SecurityPolicyConfiguration::from_json(json["security_policy"].toObject());
    config.m_cache_config = CacheConfiguration::from_json(json["cache_config"].toObject());
    config.m_network_config = NetworkConfiguration::from_json(json["network_config"].toObject());
    config.m_update_config = UpdateConfiguration::from_json(json["update_config"].toObject());

    // Load sources
    config.m_source_manager.load_from_config(json["sources"].toObject());

    return config;
}

void RemotePluginConfiguration::initialize_defaults() {
    m_remote_plugins_enabled = true;
    m_version = "1.0";

    // Set default cache directory
    QString cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_cache_config.cache_directory = std::filesystem::path(cache_path.toStdString()) / "qtforge" / "remote_plugins";

    // Apply standard security level
    apply_security_level(RemoteSecurityLevel::Standard);
}

void RemotePluginConfiguration::apply_security_level(RemoteSecurityLevel level) {
    m_security_policy.default_security_level = level;

    switch (level) {
        case RemoteSecurityLevel::Minimal:
            m_security_policy.require_signature_verification = false;
            m_security_policy.allow_self_signed_certificates = true;
            m_security_policy.enable_certificate_pinning = false;
            m_security_policy.require_https = false;
            m_security_policy.enable_sandbox = false;
            m_security_policy.allow_network_access = true;
            m_security_policy.allow_file_system_access = true;
            break;

        case RemoteSecurityLevel::Standard:
            m_security_policy.require_signature_verification = true;
            m_security_policy.allow_self_signed_certificates = false;
            m_security_policy.enable_certificate_pinning = false;
            m_security_policy.require_https = true;
            m_security_policy.enable_sandbox = true;
            m_security_policy.allow_network_access = false;
            m_security_policy.allow_file_system_access = false;
            break;

        case RemoteSecurityLevel::High:
            m_security_policy.require_signature_verification = true;
            m_security_policy.allow_self_signed_certificates = false;
            m_security_policy.enable_certificate_pinning = true;
            m_security_policy.require_https = true;
            m_security_policy.enable_sandbox = true;
            m_security_policy.allow_network_access = false;
            m_security_policy.allow_file_system_access = false;
            break;

        case RemoteSecurityLevel::Paranoid:
            m_security_policy.require_signature_verification = true;
            m_security_policy.allow_self_signed_certificates = false;
            m_security_policy.enable_certificate_pinning = true;
            m_security_policy.require_https = true;
            m_security_policy.enable_sandbox = true;
            m_security_policy.allow_network_access = false;
            m_security_policy.allow_file_system_access = false;
            // Additional paranoid settings
            m_cache_config.enable_encryption = true;
            m_network_config.verify_ssl_certificates = true;
            m_update_config.policy = AutoUpdatePolicy::Disabled;
            break;
    }
}

RemotePluginConfiguration RemotePluginConfiguration::create_default() {
    RemotePluginConfiguration config;
    config.initialize_defaults();
    return config;
}

RemotePluginConfiguration RemotePluginConfiguration::create_secure() {
    RemotePluginConfiguration config;
    config.initialize_defaults();
    config.apply_security_level(RemoteSecurityLevel::Paranoid);
    return config;
}

RemotePluginConfiguration RemotePluginConfiguration::create_permissive() {
    RemotePluginConfiguration config;
    config.initialize_defaults();
    config.apply_security_level(RemoteSecurityLevel::Minimal);
    return config;
}

RemotePluginConfiguration RemotePluginConfiguration::create_enterprise() {
    RemotePluginConfiguration config;
    config.initialize_defaults();
    config.apply_security_level(RemoteSecurityLevel::High);

    // Enterprise-specific settings
    config.m_security_policy.require_signature_verification = true;
    config.m_security_policy.enable_certificate_pinning = true;
    config.m_security_policy.require_https = true;
    config.m_update_config.policy = AutoUpdatePolicy::CheckOnly;
    config.m_update_config.backup_before_update = true;
    config.m_update_config.rollback_on_failure = true;

    return config;
}

// === RemotePluginConfigurationManager Implementation ===

RemotePluginConfigurationManager& RemotePluginConfigurationManager::instance() {
    static RemotePluginConfigurationManager instance;
    return instance;
}

void RemotePluginConfigurationManager::set_configuration(const RemotePluginConfiguration& config) {
    m_configuration = config;
}

std::filesystem::path RemotePluginConfigurationManager::default_config_path() const {
    QString config_dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    return std::filesystem::path(config_dir.toStdString()) / "qtforge" / "remote_plugins.json";
}

}  // namespace qtplugin
