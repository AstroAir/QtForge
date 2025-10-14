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
#include <vector>

#include "../utils/error_handling.hpp"
#include "remote_plugin_source.hpp"

namespace qtplugin {

/**
 * @brief Auto-update policy for remote plugins
 */
enum class AutoUpdatePolicy {
    Disabled,   ///< No automatic updates
    CheckOnly,  ///< Check for updates but don't download
    Minor,      ///< Auto-update minor versions only
    Patch,      ///< Auto-update patch versions only
    All         ///< Auto-update all versions
};

/**
 * @brief Network proxy configuration
 */
struct ProxyConfiguration {
    enum Type { NoProxy, HttpProxy, Socks5Proxy, SystemProxy };

    Type type = NoProxy;  ///< Proxy type
    QString host;         ///< Proxy host
    int port = 0;         ///< Proxy port
    QString username;     ///< Proxy username
    QString password;     ///< Proxy password

    /**
     * @brief Check if proxy is configured
     * @return True if configured, false otherwise
     */
    bool is_configured() const { return type != NoProxy && !host.isEmpty(); }

    /**
     * @brief Convert to JSON representation
     * @return JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     * @param json JSON object to parse
     * @return ProxyConfiguration instance
     */
    static ProxyConfiguration from_json(const QJsonObject& json);
};

/**
 * @brief Security policy configuration for remote plugins
 */
struct SecurityPolicyConfiguration {
    RemoteSecurityLevel default_security_level =
        RemoteSecurityLevel::Standard;  ///< Default security level
    bool require_signature_verification =
        true;  ///< Require signature verification
    bool allow_self_signed_certificates =
        false;                               ///< Allow self-signed certificates
    bool enable_certificate_pinning = true;  ///< Enable certificate pinning
    bool require_https = true;               ///< Require HTTPS
    bool enable_sandbox = true;              ///< Enable sandbox
    bool allow_network_access = false;       ///< Allow network access
    bool allow_file_system_access = false;   ///< Allow file system access
    QStringList trusted_domains;             ///< Trusted domains
    QStringList blocked_domains;             ///< Blocked domains
    QStringList
        trusted_certificate_fingerprints;  ///< Trusted certificate fingerprints
    std::chrono::seconds signature_cache_ttl{
        86400};  ///< Signature cache TTL (24 hours)

    /**
     * @brief Convert to JSON representation
     * @return JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     * @param json JSON object to parse
     * @return SecurityPolicyConfiguration instance
     */
    static SecurityPolicyConfiguration from_json(const QJsonObject& json);
};

/**
 * @brief Cache configuration for remote plugins
 */
struct CacheConfiguration {
    std::filesystem::path cache_directory;  ///< Cache directory
    qint64 max_cache_size =
        1024 * 1024 * 1024;                  ///< Max cache size (1GB default)
    std::chrono::seconds default_ttl{3600};  ///< Default TTL (1 hour)
    std::chrono::seconds cleanup_interval{3600};  ///< Cleanup interval (1 hour)
    bool enable_compression = true;               ///< Enable compression
    bool enable_encryption = false;               ///< Enable encryption
    int max_concurrent_downloads = 4;             ///< Max concurrent downloads
    qint64 max_file_size =
        100 * 1024 * 1024;  ///< Max file size (100MB default)
    CachePolicy default_cache_policy =
        CachePolicy::PreferCache;  ///< Default cache policy

    /**
     * @brief Convert to JSON representation
     * @return JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     * @param json JSON object to parse
     * @return CacheConfiguration instance
     */
    static CacheConfiguration from_json(const QJsonObject& json);
};

/**
 * @brief Network configuration for remote plugin operations
 */
struct NetworkConfiguration {
    std::chrono::seconds connection_timeout{30};  ///< Connection timeout
    std::chrono::seconds read_timeout{60};        ///< Read timeout
    int max_retries = 3;                          ///< Max retries
    std::chrono::seconds retry_delay{1};          ///< Retry delay
    bool enable_http2 = true;                     ///< Enable HTTP/2
    bool verify_ssl_certificates = true;          ///< Verify SSL certificates
    QString user_agent = "QtForge-RemotePlugin/3.0.0";  ///< User agent
    ProxyConfiguration proxy;                           ///< Proxy configuration
    QJsonObject custom_headers;                         ///< Custom headers

    /**
     * @brief Convert to JSON representation
     * @return JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     * @param json JSON object to parse
     * @return NetworkConfiguration instance
     */
    static NetworkConfiguration from_json(const QJsonObject& json);
};

/**
 * @brief Update configuration for remote plugins
 */
struct UpdateConfiguration {
    AutoUpdatePolicy policy =
        AutoUpdatePolicy::CheckOnly;             ///< Auto-update policy
    std::chrono::seconds check_interval{86400};  ///< Check interval (24 hours)
    bool notify_updates = true;                  ///< Notify updates
    bool backup_before_update = true;            ///< Backup before update
    bool rollback_on_failure = true;             ///< Rollback on failure
    QStringList update_channels{"stable"};       ///< Update channels

    /**
     * @brief Convert to JSON representation
     * @return JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     * @param json JSON object to parse
     * @return UpdateConfiguration instance
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
     * @brief Destructor
     */
    ~RemotePluginConfiguration();

    // Delete copy and move operations (contains non-copyable
    // RemoteSourceManager)
    RemotePluginConfiguration(const RemotePluginConfiguration&) = delete;
    RemotePluginConfiguration& operator=(const RemotePluginConfiguration&) =
        delete;
    RemotePluginConfiguration(RemotePluginConfiguration&&) = delete;
    RemotePluginConfiguration& operator=(RemotePluginConfiguration&&) = delete;

    // === General Settings ===

    /**
     * @brief Check if remote plugin support is enabled
     * @return True if enabled, false otherwise
     */
    bool is_remote_plugins_enabled() const { return m_remote_plugins_enabled; }

    /**
     * @brief Enable/disable remote plugin support
     * @param enabled True to enable, false to disable
     */
    void set_remote_plugins_enabled(bool enabled) {
        m_remote_plugins_enabled = enabled;
    }

    /**
     * @brief Get configuration version
     * @return Configuration version
     */
    QString version() const { return m_version; }

    /**
     * @brief Set configuration version
     * @param version New version
     */
    void set_version(const QString& version) { m_version = version; }

    // === Source Management ===

    /**
     * @brief Get remote source manager
     * @return Reference to source manager
     */
    RemoteSourceManager& source_manager() { return m_source_manager; }

    /**
     * @brief Get remote source manager (const)
     * @return Const reference to source manager
     */
    const RemoteSourceManager& source_manager() const {
        return m_source_manager;
    }

    /**
     * @brief Add a trusted remote source
     * @param source Source to add
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> add_trusted_source(
        const RemotePluginSource& source);

    /**
     * @brief Remove a remote source
     * @param source_id ID of source to remove
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> remove_source(
        const QString& source_id);

    /**
     * @brief Get all configured sources
     * @return Vector of sources
     */
    std::vector<RemotePluginSource> get_all_sources() const;

    // === Configuration Access ===

    /**
     * @brief Get security policy configuration
     * @return Security policy configuration
     */
    const SecurityPolicyConfiguration& security_policy() const {
        return m_security_policy;
    }

    /**
     * @brief Set security policy configuration
     * @param policy New security policy
     */
    void set_security_policy(const SecurityPolicyConfiguration& policy) {
        m_security_policy = policy;
    }

    /**
     * @brief Get cache configuration
     * @return Cache configuration
     */
    const CacheConfiguration& cache_config() const { return m_cache_config; }

    /**
     * @brief Set cache configuration
     * @param config New cache configuration
     */
    void set_cache_config(const CacheConfiguration& config) {
        m_cache_config = config;
    }

    /**
     * @brief Get network configuration
     * @return Network configuration
     */
    const NetworkConfiguration& network_config() const {
        return m_network_config;
    }

    /**
     * @brief Set network configuration
     * @param config New network configuration
     */
    void set_network_config(const NetworkConfiguration& config) {
        m_network_config = config;
    }

    /**
     * @brief Get update configuration
     * @return Update configuration
     */
    const UpdateConfiguration& update_config() const { return m_update_config; }

    /**
     * @brief Set update configuration
     * @param config New update configuration
     */
    void set_update_config(const UpdateConfiguration& config) {
        m_update_config = config;
    }

    // === Validation ===

    /**
     * @brief Validate configuration
     * @return Success or error details
     */
    qtplugin::expected<void, PluginError> validate() const;

    /**
     * @brief Check if a domain is trusted
     * @param domain Domain to check
     * @return True if domain is trusted
     */
    bool is_domain_trusted(const QString& domain) const;

    /**
     * @brief Check if a domain is blocked
     * @param domain Domain to check
     * @return True if domain is blocked
     */
    bool is_domain_blocked(const QString& domain) const;

    // === Serialization ===

    /**
     * @brief Convert to JSON representation
     * @return JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     * @param json JSON object to parse
     * @return RemotePluginConfiguration instance
     */
    static std::shared_ptr<RemotePluginConfiguration> from_json(
        const QJsonObject& json);

    /**
     * @brief Load configuration from file
     * @param file_path Configuration file path
     * @return Configuration or error
     */
    static qtplugin::expected<std::shared_ptr<RemotePluginConfiguration>,
                              PluginError>
    load_from_file(const std::filesystem::path& file_path);

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
     * @return Default configuration
     */
    static std::shared_ptr<RemotePluginConfiguration> create_default();

    /**
     * @brief Create secure configuration (high security)
     * @return Secure configuration
     */
    static std::shared_ptr<RemotePluginConfiguration> create_secure();

    /**
     * @brief Create permissive configuration (low security)
     * @return Permissive configuration
     */
    static std::shared_ptr<RemotePluginConfiguration> create_permissive();

    /**
     * @brief Create enterprise configuration
     * @return Enterprise configuration
     */
    static std::shared_ptr<RemotePluginConfiguration> create_enterprise();

private:
    // General settings
    bool m_remote_plugins_enabled = true;  ///< Remote plugins enabled flag
    QString m_version = "1.0";             ///< Configuration version

    // Component configurations
    SecurityPolicyConfiguration m_security_policy;  ///< Security policy
    CacheConfiguration m_cache_config;              ///< Cache configuration
    NetworkConfiguration m_network_config;          ///< Network configuration
    UpdateConfiguration m_update_config;            ///< Update configuration

    // Source management
    RemoteSourceManager m_source_manager;  ///< Source manager

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
     * @return Reference to singleton instance
     */
    static RemotePluginConfigurationManager& instance();

    /**
     * @brief Get current configuration
     * @return Current configuration
     */
    std::shared_ptr<const RemotePluginConfiguration> configuration() const {
        return m_configuration;
    }

    /**
     * @brief Set configuration
     * @param config New configuration
     */
    void set_configuration(std::shared_ptr<RemotePluginConfiguration> config);

    /**
     * @brief Load configuration from default location
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> load_default_configuration();

    /**
     * @brief Save configuration to default location
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> save_default_configuration() const;

    /**
     * @brief Get default configuration file path
     * @return Default config path
     */
    std::filesystem::path default_config_path() const;

private:
    RemotePluginConfigurationManager() =
        default;  ///< Private constructor for singleton
    std::shared_ptr<RemotePluginConfiguration>
        m_configuration;  ///< Current configuration
};

}  // namespace qtplugin
