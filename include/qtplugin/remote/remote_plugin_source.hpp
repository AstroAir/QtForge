/**
 * @file remote_plugin_source.hpp
 * @brief Remote plugin source representation and management
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QString>
#include <QUrl>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "../utils/error_handling.hpp"

namespace qtplugin {

/**
 * @brief Types of remote plugin sources
 */
enum class RemoteSourceType {
    Http,           ///< HTTP/HTTPS direct download
    Git,            ///< Git repository
    Registry,       ///< Plugin registry (npm-like)
    Ftp,            ///< FTP server
    Custom          ///< Custom protocol handler
};

/**
 * @brief Authentication methods for remote sources
 */
enum class AuthenticationType {
    None,           ///< No authentication
    Basic,          ///< HTTP Basic authentication
    Bearer,         ///< Bearer token
    ApiKey,         ///< API key authentication
    Certificate,    ///< Client certificate
    OAuth2          ///< OAuth2 flow
};

/**
 * @brief Cache policy for remote plugins
 */
enum class CachePolicy {
    NoCache,        ///< Always download fresh
    PreferCache,    ///< Use cache if available
    CacheOnly,      ///< Only use cached versions
    CacheFirst      ///< Check cache first, fallback to download
};

/**
 * @brief Security level for remote plugin sources
 */
enum class RemoteSecurityLevel {
    Minimal,        ///< Basic validation only
    Standard,       ///< Standard security checks
    High,           ///< Enhanced security validation
    Paranoid        ///< Maximum security, strict validation
};

/**
 * @brief Authentication credentials for remote sources
 */
struct AuthenticationCredentials {
    AuthenticationType type = AuthenticationType::None;
    QString username;
    QString password;
    QString token;
    QString api_key;
    QString certificate_path;
    QString private_key_path;
    QJsonObject oauth2_config;
    
    /**
     * @brief Check if credentials are valid
     */
    bool is_valid() const;
    
    /**
     * @brief Convert to JSON for serialization
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON
     */
    static AuthenticationCredentials from_json(const QJsonObject& json);
};

/**
 * @brief Configuration for remote plugin source
 */
struct RemoteSourceConfig {
    CachePolicy cache_policy = CachePolicy::PreferCache;
    RemoteSecurityLevel security_level = RemoteSecurityLevel::Standard;
    std::chrono::seconds cache_ttl{3600};  // 1 hour default
    std::chrono::seconds timeout{30};      // 30 seconds default
    int max_retries = 3;
    bool verify_ssl = true;
    bool allow_redirects = true;
    size_t max_download_size = 100 * 1024 * 1024;  // 100MB default
    QJsonObject custom_headers;
    QJsonObject custom_options;
    
    /**
     * @brief Convert to JSON for serialization
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON
     */
    static RemoteSourceConfig from_json(const QJsonObject& json);
};

/**
 * @brief Represents a remote plugin source
 */
class RemotePluginSource {
public:
    /**
     * @brief Constructor
     * @param url Source URL
     * @param type Source type (auto-detected if not specified)
     * @param name Human-readable name for the source
     */
    explicit RemotePluginSource(const QUrl& url, 
                               RemoteSourceType type = RemoteSourceType::Http,
                               const QString& name = QString());
    
    /**
     * @brief Constructor from JSON
     * @param json JSON configuration
     */
    explicit RemotePluginSource(const QJsonObject& json);
    
    /**
     * @brief Copy constructor
     */
    RemotePluginSource(const RemotePluginSource& other);
    
    /**
     * @brief Assignment operator
     */
    RemotePluginSource& operator=(const RemotePluginSource& other);
    
    /**
     * @brief Destructor
     */
    ~RemotePluginSource();

    // === Basic Properties ===
    
    /**
     * @brief Get source URL
     */
    const QUrl& url() const { return m_url; }
    
    /**
     * @brief Set source URL
     */
    void set_url(const QUrl& url);
    
    /**
     * @brief Get source type
     */
    RemoteSourceType type() const { return m_type; }
    
    /**
     * @brief Set source type
     */
    void set_type(RemoteSourceType type) { m_type = type; }
    
    /**
     * @brief Get source name
     */
    const QString& name() const { return m_name; }
    
    /**
     * @brief Set source name
     */
    void set_name(const QString& name) { m_name = name; }
    
    /**
     * @brief Get unique identifier for this source
     */
    QString id() const;
    
    /**
     * @brief Check if source is enabled
     */
    bool is_enabled() const { return m_enabled; }
    
    /**
     * @brief Enable/disable source
     */
    void set_enabled(bool enabled) { m_enabled = enabled; }

    // === Authentication ===
    
    /**
     * @brief Set authentication credentials
     */
    void set_authentication(const AuthenticationCredentials& credentials);
    
    /**
     * @brief Get authentication credentials
     */
    const AuthenticationCredentials& authentication() const { return m_auth; }
    
    /**
     * @brief Check if authentication is configured
     */
    bool has_authentication() const;

    // === Configuration ===
    
    /**
     * @brief Set source configuration
     */
    void set_configuration(const RemoteSourceConfig& config);
    
    /**
     * @brief Get source configuration
     */
    const RemoteSourceConfig& configuration() const { return m_config; }
    
    /**
     * @brief Update configuration option
     */
    void set_config_option(const QString& key, const QJsonValue& value);
    
    /**
     * @brief Get configuration option
     */
    QJsonValue get_config_option(const QString& key) const;

    // === Validation ===
    
    /**
     * @brief Validate source configuration
     * @return Success or error details
     */
    qtplugin::expected<void, PluginError> validate() const;
    
    /**
     * @brief Check if source is reachable
     * @return Success or error details
     */
    qtplugin::expected<void, PluginError> test_connection() const;

    // === Serialization ===
    
    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
    
    /**
     * @brief Create from JSON representation
     */
    static RemotePluginSource from_json(const QJsonObject& json);
    
    /**
     * @brief Convert to string representation
     */
    QString to_string() const;

    // === Utility ===
    
    /**
     * @brief Auto-detect source type from URL
     */
    static RemoteSourceType detect_source_type(const QUrl& url);
    
    /**
     * @brief Check if URL is supported
     */
    static bool is_supported_url(const QUrl& url);
    
    /**
     * @brief Get supported URL schemes
     */
    static std::vector<QString> supported_schemes();

    // === Operators ===
    
    bool operator==(const RemotePluginSource& other) const;
    bool operator!=(const RemotePluginSource& other) const;

private:
    QUrl m_url;
    RemoteSourceType m_type;
    QString m_name;
    bool m_enabled = true;
    AuthenticationCredentials m_auth;
    RemoteSourceConfig m_config;
    
    // Helper methods
    void initialize_defaults();
    QString generate_id() const;
};

/**
 * @brief Collection of remote plugin sources
 */
class RemoteSourceManager {
public:
    /**
     * @brief Constructor
     */
    RemoteSourceManager();
    
    /**
     * @brief Destructor
     */
    ~RemoteSourceManager();

    /**
     * @brief Add a remote source
     */
    qtplugin::expected<void, PluginError> add_source(const RemotePluginSource& source);
    
    /**
     * @brief Remove a remote source
     */
    qtplugin::expected<void, PluginError> remove_source(const QString& source_id);
    
    /**
     * @brief Get source by ID
     */
    std::optional<RemotePluginSource> get_source(const QString& source_id) const;
    
    /**
     * @brief Get all sources
     */
    std::vector<RemotePluginSource> get_all_sources() const;
    
    /**
     * @brief Get enabled sources
     */
    std::vector<RemotePluginSource> get_enabled_sources() const;
    
    /**
     * @brief Clear all sources
     */
    void clear();
    
    /**
     * @brief Load sources from configuration
     */
    qtplugin::expected<void, PluginError> load_from_config(const QJsonObject& config);
    
    /**
     * @brief Save sources to configuration
     */
    QJsonObject save_to_config() const;

private:
    std::unordered_map<QString, RemotePluginSource> m_sources;
    mutable std::shared_mutex m_sources_mutex;
};

}  // namespace qtplugin
