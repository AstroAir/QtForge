/**
 * @file remote_security_manager.hpp
 * @brief Security manager for remote plugins with signature verification and sandboxing
 * @version 3.2.0
 */

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUrl>
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include "../../utils/error_handling.hpp"
// Security manager dependency removed

namespace qtplugin::remote {

// Forward declarations
class RemotePluginCache;
class PluginSignatureVerifier;
class RemoteSandbox;

/**
 * @brief Remote plugin security levels
 */
enum class RemoteSecurityLevel {
    Disabled = 0,    ///< No security checks (development only)
    Basic = 1,       ///< Basic URL and certificate validation
    Standard = 2,    ///< Standard security with signature verification
    Strict = 3,      ///< Strict security with additional validation
    Paranoid = 4     ///< Maximum security with full verification chain
};

/**
 * @brief Trust level for remote plugin publishers
 */
enum class PublisherTrustLevel {
    Untrusted = 0,   ///< Publisher not trusted
    Basic = 1,       ///< Basic trust level
    Verified = 2,    ///< Verified publisher
    Trusted = 3,     ///< Fully trusted publisher
    System = 4       ///< System-level trust (internal plugins)
};

/**
 * @brief Remote plugin signature information
 */
struct RemotePluginSignature {
    QString algorithm;           ///< Signature algorithm (RSA, ECDSA, etc.)
    QByteArray signature;        ///< Digital signature
    QSslCertificate certificate; ///< Publisher certificate
    QDateTime timestamp;         ///< Signature timestamp
    QString publisher_id;        ///< Publisher identifier
    PublisherTrustLevel trust_level; ///< Trust level
    bool is_valid = false;       ///< Signature validation result
    QString validation_error;    ///< Validation error message

    QJsonObject to_json() const;
    static RemotePluginSignature from_json(const QJsonObject& json);
};

/**
 * @brief Remote plugin validation result
 */
struct RemoteValidationResult {
    bool is_valid = false;
    RemoteSecurityLevel validated_level = RemoteSecurityLevel::Disabled;
    RemotePluginSignature signature;
    std::vector<QString> errors;
    std::vector<QString> warnings;
    QDateTime validation_time;
    QString validator_version;

    QJsonObject to_json() const;
    static RemoteValidationResult from_json(const QJsonObject& json);
};

/**
 * @brief Remote plugin trust store for managing trusted publishers
 */
class RemotePluginTrustStore : public QObject {
    Q_OBJECT

public:
    explicit RemotePluginTrustStore(QObject* parent = nullptr);
    ~RemotePluginTrustStore() override;

    /**
     * @brief Add trusted publisher certificate
     * @param publisher_id Publisher identifier
     * @param certificate Publisher certificate
     * @param trust_level Trust level to assign
     * @return Success or error
     */
    expected<void, PluginError> add_trusted_publisher(
        const QString& publisher_id,
        const QSslCertificate& certificate,
        PublisherTrustLevel trust_level = PublisherTrustLevel::Verified
    );

    /**
     * @brief Remove trusted publisher
     * @param publisher_id Publisher identifier
     */
    void remove_trusted_publisher(const QString& publisher_id);

    /**
     * @brief Check if publisher is trusted
     * @param publisher_id Publisher identifier
     * @return true if publisher is trusted
     */
    bool is_trusted_publisher(const QString& publisher_id) const;

    /**
     * @brief Get publisher trust level
     * @param publisher_id Publisher identifier
     * @return Trust level
     */
    PublisherTrustLevel get_trust_level(const QString& publisher_id) const;

    /**
     * @brief Get publisher certificate
     * @param publisher_id Publisher identifier
     * @return Certificate if found
     */
    std::optional<QSslCertificate> get_publisher_certificate(const QString& publisher_id) const;

    /**
     * @brief Verify certificate chain for publisher
     * @param publisher_id Publisher identifier
     * @param certificate Certificate to verify
     * @return true if certificate chain is valid
     */
    bool verify_certificate_chain(const QString& publisher_id, const QSslCertificate& certificate) const;

    /**
     * @brief Load trust store from file
     * @param file_path Trust store file path
     * @return Success or error
     */
    expected<void, PluginError> load_from_file(const QString& file_path);

    /**
     * @brief Save trust store to file
     * @param file_path Trust store file path
     * @return Success or error
     */
    expected<void, PluginError> save_to_file(const QString& file_path) const;

    /**
     * @brief Get all trusted publishers
     * @return List of publisher IDs
     */
    QStringList get_trusted_publishers() const;

    /**
     * @brief Clear all trusted publishers
     */
    void clear();

signals:
    void publisher_added(const QString& publisher_id, PublisherTrustLevel trust_level);
    void publisher_removed(const QString& publisher_id);
    void trust_level_changed(const QString& publisher_id, PublisherTrustLevel old_level, PublisherTrustLevel new_level);

private:
    struct PublisherInfo {
        QSslCertificate certificate;
        PublisherTrustLevel trust_level;
        QDateTime added_time;
        QString description;
    };

    mutable std::shared_mutex m_trust_mutex;
    std::unordered_map<QString, PublisherInfo> m_trusted_publishers;
    QString m_store_file_path;

    void log_trust_event(const QString& event, const QString& publisher_id) const;
};

/**
 * @brief Remote plugin signature verifier
 */
class RemotePluginSignatureVerifier : public QObject {
    Q_OBJECT

public:
    explicit RemotePluginSignatureVerifier(RemotePluginTrustStore* trust_store, QObject* parent = nullptr);
    ~RemotePluginSignatureVerifier() override;

    /**
     * @brief Verify plugin signature from remote source
     * @param plugin_data Plugin binary data
     * @param signature_info Signature information
     * @param security_level Required security level
     * @return Validation result
     */
    RemoteValidationResult verify_signature(
        const QByteArray& plugin_data,
        const RemotePluginSignature& signature_info,
        RemoteSecurityLevel security_level = RemoteSecurityLevel::Standard
    ) const;

    /**
     * @brief Verify plugin signature from URL metadata
     * @param plugin_url Plugin URL
     * @param signature_url Signature URL
     * @param security_level Required security level
     * @return Validation result future
     */
    QFuture<RemoteValidationResult> verify_signature_async(
        const QUrl& plugin_url,
        const QUrl& signature_url,
        RemoteSecurityLevel security_level = RemoteSecurityLevel::Standard
    );

    /**
     * @brief Extract signature information from plugin metadata
     * @param metadata Plugin metadata JSON
     * @return Signature information
     */
    RemotePluginSignature extract_signature_info(const QJsonObject& metadata) const;

    /**
     * @brief Validate certificate chain
     * @param certificate Certificate to validate
     * @param chain Certificate chain
     * @return true if chain is valid
     */
    bool validate_certificate_chain(const QSslCertificate& certificate,
                                   const QList<QSslCertificate>& chain = {}) const;

    /**
     * @brief Check certificate revocation status
     * @param certificate Certificate to check
     * @return true if certificate is not revoked
     */
    QFuture<bool> check_certificate_revocation(const QSslCertificate& certificate);

private:
    RemotePluginTrustStore* m_trust_store;
    QNetworkAccessManager* m_network_manager;
    std::unique_ptr<QTimer> m_revocation_cache_timer;
    std::unordered_map<QString, std::pair<bool, QDateTime>> m_revocation_cache;

    bool verify_rsa_signature(const QByteArray& data, const QByteArray& signature,
                             const QSslKey& public_key) const;
    bool verify_ecdsa_signature(const QByteArray& data, const QByteArray& signature,
                               const QSslKey& public_key) const;
    QByteArray hash_plugin_data(const QByteArray& data, const QString& algorithm) const;
    void cleanup_revocation_cache();
};

/**
 * @brief Remote plugin security configuration
 */
struct RemoteSecurityConfig {
    RemoteSecurityLevel security_level = RemoteSecurityLevel::Standard;
    bool require_signatures = true;
    bool allow_self_signed = false;
    bool check_certificate_revocation = true;
    bool enable_sandbox = true;
    bool verify_publisher_identity = true;
    bool allow_http_sources = false;  ///< Only HTTPS by default
    bool strict_tls_verification = true;

    // Network security
    std::chrono::seconds network_timeout{30};
    int max_redirects = 3;
    QStringList allowed_domains;  ///< Whitelist of allowed domains
    QStringList blocked_domains;  ///< Blacklist of blocked domains

    // Trust settings
    PublisherTrustLevel minimum_trust_level = PublisherTrustLevel::Basic;
    bool allow_untrusted_development = false;  ///< Allow untrusted for development

    // Validation settings
    std::chrono::hours signature_max_age{24 * 7};  ///< 1 week
    std::chrono::minutes certificate_cache_time{60};  ///< 1 hour

    QJsonObject to_json() const;
    static RemoteSecurityConfig from_json(const QJsonObject& json);
};

/**
 * @brief Main remote plugin security manager
 */
class RemoteSecurityManager : public QObject {
    Q_OBJECT

public:
    explicit RemoteSecurityManager(QObject* parent = nullptr);
    ~RemoteSecurityManager() override;

    /**
     * @brief Get singleton instance
     * @return Security manager instance
     */
    static RemoteSecurityManager& instance();

    /**
     * @brief Initialize security manager with configuration
     * @param config Security configuration
     * @return Success or error
     */
    expected<void, PluginError> initialize(const RemoteSecurityConfig& config);

    /**
     * @brief Validate remote plugin security
     * @param plugin_url Plugin URL
     * @param metadata Plugin metadata
     * @return Validation result future
     */
    QFuture<RemoteValidationResult> validate_remote_plugin(
        const QUrl& plugin_url,
        const QJsonObject& metadata
    );

    /**
     * @brief Validate plugin binary data
     * @param plugin_data Plugin binary data
     * @param signature_info Signature information
     * @return Validation result
     */
    RemoteValidationResult validate_plugin_data(
        const QByteArray& plugin_data,
        const RemotePluginSignature& signature_info
    );

    /**
     * @brief Check if URL is allowed for plugin downloads
     * @param url URL to check
     * @return true if URL is allowed
     */
    bool is_url_allowed(const QUrl& url) const;

    /**
     * @brief Create secure network request
     * @param url Target URL
     * @return Configured network request
     */
    QNetworkRequest create_secure_request(const QUrl& url) const;

    /**
     * @brief Get trust store
     * @return Trust store instance
     */
    RemotePluginTrustStore* get_trust_store() const { return m_trust_store.get(); }

    /**
     * @brief Get signature verifier
     * @return Signature verifier instance
     */
    RemotePluginSignatureVerifier* get_signature_verifier() const { return m_signature_verifier.get(); }

    /**
     * @brief Get current security configuration
     * @return Security configuration
     */
    const RemoteSecurityConfig& get_config() const { return m_config; }

    /**
     * @brief Update security configuration
     * @param config New configuration
     * @return Success or error
     */
    expected<void, PluginError> update_config(const RemoteSecurityConfig& config);

    /**
     * @brief Enable/disable security features for development
     * @param enable_dev_mode true to enable development mode
     */
    void set_development_mode(bool enable_dev_mode);

    /**
     * @brief Check if running in development mode
     * @return true if in development mode
     */
    bool is_development_mode() const { return m_development_mode; }

signals:
    void security_violation_detected(const QString& plugin_url, const QString& violation);
    void validation_completed(const QString& plugin_url, bool success);
    void trust_level_required(const QString& publisher_id, PublisherTrustLevel required_level);

private slots:
    void handle_ssl_errors(QNetworkReply* reply, const QList<QSslError>& errors);
    void cleanup_validation_cache();

private:
    RemoteSecurityConfig m_config;
    std::unique_ptr<RemotePluginTrustStore> m_trust_store;
    std::unique_ptr<RemotePluginSignatureVerifier> m_signature_verifier;
    std::unique_ptr<QNetworkAccessManager> m_network_manager;
    std::unique_ptr<QTimer> m_cache_cleanup_timer;

    // Validation cache
    mutable std::shared_mutex m_cache_mutex;
    std::unordered_map<QString, std::pair<RemoteValidationResult, QDateTime>> m_validation_cache;

    bool m_development_mode = false;
    bool m_initialized = false;

    void setup_network_security();
    void setup_ssl_configuration();
    bool is_domain_allowed(const QString& domain) const;
    bool is_domain_blocked(const QString& domain) const;
    QString generate_cache_key(const QUrl& url, const QJsonObject& metadata) const;
    void log_security_event(const QString& event, const QString& details) const;
};

} // namespace qtplugin::remote

Q_DECLARE_METATYPE(qtplugin::remote::RemoteSecurityLevel)
Q_DECLARE_METATYPE(qtplugin::remote::PublisherTrustLevel)
Q_DECLARE_METATYPE(qtplugin::remote::RemotePluginSignature)
Q_DECLARE_METATYPE(qtplugin::remote::RemoteValidationResult)
