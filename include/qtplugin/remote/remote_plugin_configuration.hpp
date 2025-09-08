/**
 * @file remote_plugin_configuration.hpp
 * @brief Configuration system for remote plugin functionality
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "../utils/error_handling.hpp"
#include "remote_plugin_source.hpp"

namespace qtplugin {

/**
 * @brief Auto-update policy for remote plugins
 */
enum class AutoUpdatePolicy {
    Disabled,       ///< No automatic updates
    CheckOnly,      ///< Check for updates but don't download
    Minor,          ///< Auto-update minor versions only
    Patch,          ///< Auto-update patch versions only
    All             ///< Auto-update all versions
};

/**
 * @brief Network proxy configuration
 */
struct ProxyConfiguration {
    enum Type {
        NoProxy,
        HttpProxy,
        Socks5Proxy,
        SystemProxy
    };
    
    Type type = NoProxy;
    QString host;
    int port = 0;
    QString username;
    QString password;
    
    /**
     * @brief Check if proxy is configured
     */
    bool is_configured() const { return type != NoProxy && !host.isEmpty(); }
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static ProxyConfiguration from_json(const QJsonObject& json);
};

/**
 * @brief Security policy configuration for remote plugins
 */
struct SecurityPolicyConfiguration {
    RemoteSecurityLevel default_security_level = RemoteSecurityLevel::Standard;
    bool require_signature_verification = true;
    bool allow_self_signed_certificates = false;
    bool enable_certificate_pinning = true;
    bool require_https = true;
    bool enable_sandbox = true;
    bool allow_network_access = false;
    bool allow_file_system_access = false;
    QStringList trusted_domains;
    QStringList blocked_domains;
    QStringList trusted_certificate_fingerprints;
    std::chrono::seconds signature_cache_ttl{86400};  // 24 hours
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static SecurityPolicyConfiguration from_json(const QJsonObject& json);
};

/**
 * @brief Cache configuration for remote plugins
 */
struct CacheConfiguration {
    std::filesystem::path cache_directory;
    qint64 max_cache_size = 1024 * 1024 * 1024;  // 1GB default
    std::chrono::seconds default_ttl{3600};      // 1 hour default
    std::chrono::seconds cleanup_interval{3600}; // 1 hour default
    bool enable_compression = true;
    bool enable_encryption = false;
    int max_concurrent_downloads = 4;
    qint64 max_file_size = 100 * 1024 * 1024;    // 100MB default
    CachePolicy default_cache_policy = CachePolicy::PreferCache;
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static CacheConfiguration from_json(const QJsonObject& json);
};

/**
 * @brief Network configuration for remote plugin operations
 */
struct NetworkConfiguration {
    std::chrono::seconds connection_timeout{30};
    std::chrono::seconds read_timeout{60};
    int max_retries = 3;
    std::chrono::seconds retry_delay{1};
    bool enable_http2 = true;
    bool verify_ssl_certificates = true;
    QString user_agent = "QtForge-RemotePlugin/3.0.0";
    ProxyConfiguration proxy;
    QJsonObject custom_headers;
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static NetworkConfiguration from_json(const QJsonObject& json);
};

/**
 * @brief Update configuration for remote plugins
 */
struct UpdateConfiguration {
    AutoUpdatePolicy policy = AutoUpdatePolicy::CheckOnly;
    std::chrono::seconds check_interval{86400};  // 24 hours
    bool notify_updates = true;
    bool backup_before_update = true;
    bool rollback_on_failure = true;
    QStringList update_channels{"stable"};
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static UpdateConfiguration from_json(const QJsonObject& json);
};

/**
 * @brief Comprehensive configuration for remote plugin functionality
 */
class RemotePluginConfiguration {
public:
    /**
     * @brief Constructor with default configuration
     */
    RemotePluginConfiguration();
    
    /**
     * @brief Constructor from JSON configuration
     * @param json JSON configuration object
     */
    explicit RemotePluginConfiguration(const QJsonObject& json);
    
    /**
     * @brief Copy constructor
     */
    RemotePluginConfiguration(const RemotePluginConfiguration& other);
    
    /**
     * @brief Assignment operator
     */
    RemotePluginConfiguration& operator=(const RemotePluginConfiguration& other);
    
    /**
     * @brief Destructor
     */
    ~RemotePluginConfiguration();

    // === General Settings ===
    
    /**
     * @brief Check if remote plugin support is enabled
     */
    bool is_remote_plugins_enabled() const { return m_remote_plugins_enabled; }
    
    /**
     * @brief Enable/disable remote plugin support
     */
    void set_remote_plugins_enabled(bool enabled) { m_remote_plugins_enabled = enabled; }
    
    /**
     * @brief Get configuration version
     */
    QString version() const { return m_version; }
    
    /**
     * @brief Set configuration version
     */
    void set_version(const QString& version) { m_version = version; }

    // === Source Management ===
    
    /**
     * @brief Get remote source manager
     */
    RemoteSourceManager& source_manager() { return m_source_manager; }
    
    /**
     * @brief Get remote source manager (const)
     */
    const RemoteSourceManager& source_manager() const { return m_source_manager; }
    
    /**
     * @brief Add a trusted remote source
     */
    qtplugin::expected<void, PluginError> add_trusted_source(const RemotePluginSource& source);
    
    /**
     * @brief Remove a remote source
     */
    qtplugin::expected<void, PluginError> remove_source(const QString& source_id);
    
    /**
     * @brief Get all configured sources
     */
    std::vector<RemotePluginSource> get_all_sources() const;

    // === Configuration Access ===
    
    /**
     * @brief Get security policy configuration
     */
    const SecurityPolicyConfiguration& security_policy() const { return m_security_policy; }
    
    /**
     * @brief Set security policy configuration
     */
    void set_security_policy(const SecurityPolicyConfiguration& policy) { m_security_policy = policy; }
    
    /**
     * @brief Get cache configuration
     */
    const CacheConfiguration& cache_config() const { return m_cache_config; }
    
    /**
     * @brief Set cache configuration
     */
    void set_cache_config(const CacheConfiguration& config) { m_cache_config = config; }
    
    /**
     * @brief Get network configuration
     */
    const NetworkConfiguration& network_config() const { return m_network_config; }
    
    /**
     * @brief Set network configuration
     */
    void set_network_config(const NetworkConfiguration& config) { m_network_config = config; }
    
    /**
     * @brief Get update configuration
     */
    const UpdateConfiguration& update_config() const { return m_update_config; }
    
    /**
     * @brief Set update configuration
     */
    void set_update_config(const UpdateConfiguration& config) { m_update_config = config; }

    // === Validation ===
    
    /**
     * @brief Validate configuration
     * @return Success or error details
     */
    qtplugin::expected<void, PluginError> validate() const;
    
    /**
     * @brief Check if a domain is trusted
     * @param domain Domain to check
     * @return true if domain is trusted
     */
    bool is_domain_trusted(const QString& domain) const;
    
    /**
     * @brief Check if a domain is blocked
     * @param domain Domain to check
     * @return true if domain is blocked
     */
    bool is_domain_blocked(const QString& domain) const;

    // === Serialization ===
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static RemotePluginConfiguration from_json(const QJsonObject& json);
    
    /**
     * @brief Load configuration from file
     * @param file_path Configuration file path
     * @return Configuration or error
     */
    static qtplugin::expected<RemotePluginConfiguration, PluginError> load_from_file(
        const std::filesystem::path& file_path);
    
    /**
     * @brief Save configuration to file
     * @param file_path Configuration file path
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> save_to_file(
        const std::filesystem::path& file_path) const;

    // === Default Configurations ===
    
    /**
     * @brief Create default configuration
     */
    static RemotePluginConfiguration create_default();
    
    /**
     * @brief Create secure configuration (high security)
     */
    static RemotePluginConfiguration create_secure();
    
    /**
     * @brief Create permissive configuration (low security)
     */
    static RemotePluginConfiguration create_permissive();
    
    /**
     * @brief Create enterprise configuration
     */
    static RemotePluginConfiguration create_enterprise();

private:
    // General settings
    bool m_remote_plugins_enabled = true;
    QString m_version = "1.0";
    
    // Component configurations
    SecurityPolicyConfiguration m_security_policy;
    CacheConfiguration m_cache_config;
    NetworkConfiguration m_network_config;
    UpdateConfiguration m_update_config;
    
    // Source management
    RemoteSourceManager m_source_manager;
    
    // Helper methods
    void initialize_defaults();
    void apply_security_level(RemoteSecurityLevel level);
};

/**
 * @brief Global configuration manager for remote plugins
 */
class RemotePluginConfigurationManager {
public:
    /**
     * @brief Get singleton instance
     */
    static RemotePluginConfigurationManager& instance();
    
    /**
     * @brief Get current configuration
     */
    const RemotePluginConfiguration& configuration() const { return m_configuration; }
    
    /**
     * @brief Set configuration
     */
    void set_configuration(const RemotePluginConfiguration& config);
    
    /**
     * @brief Load configuration from default location
     */
    qtplugin::expected<void, PluginError> load_default_configuration();
    
    /**
     * @brief Save configuration to default location
     */
    qtplugin::expected<void, PluginError> save_default_configuration() const;
    
    /**
     * @brief Get default configuration file path
     */
    std::filesystem::path default_config_path() const;

private:
    RemotePluginConfigurationManager() = default;
    RemotePluginConfiguration m_configuration;
};

}  // namespace qtplugin
