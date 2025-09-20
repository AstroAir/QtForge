/**
 * @file remote_plugin_validator.hpp
 * @brief Enhanced security validation for remote plugins
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QUrl>
#include <QtNetwork/QSslCertificate>

#include <chrono>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../security/security_manager.hpp"
#include "../utils/error_handling.hpp"
#include "remote_plugin_configuration.hpp"
#include "remote_plugin_source.hpp"

namespace qtplugin {

/**
 * @brief Validation result levels
 */
enum class ValidationLevel {
    Passed,   ///< Validation passed completely
    Warning,  ///< Validation passed with warnings
    Failed,   ///< Validation failed
    Blocked   ///< Source/plugin is blocked
};

/**
 * @brief Remote validation result information
 */
struct RemoteValidationResult {
    ValidationLevel level = ValidationLevel::Failed;  ///< Validation level
    std::string message;                              ///< Validation message
    std::string details;                              ///< Validation details
    QJsonObject metadata;                             ///< Additional metadata
    std::chrono::system_clock::time_point
        timestamp;  ///< Timestamp of validation

    /**
     * @brief Check if validation passed (including warnings)
     * @return True if valid, false otherwise
     */
    bool is_valid() const {
        return level == ValidationLevel::Passed ||
               level == ValidationLevel::Warning;
    }

    /**
     * @brief Check if validation failed
     * @return True if failed, false otherwise
     */
    bool is_failed() const {
        return level == ValidationLevel::Failed ||
               level == ValidationLevel::Blocked;
    }

    /**
     * @brief Convert to JSON representation
     * @return JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     * @param json JSON object to parse
     * @return RemoteValidationResult instance
     */
    static RemoteValidationResult from_json(const QJsonObject& json);
};

/**
 * @brief Certificate validation information
 */
struct CertificateValidation {
    bool is_valid = false;        ///< Is certificate valid
    bool is_self_signed = false;  ///< Is certificate self-signed
    bool is_expired = false;      ///< Is certificate expired
    bool is_trusted = false;      ///< Is certificate trusted
    QString fingerprint;          ///< Certificate fingerprint
    QString issuer;               ///< Certificate issuer
    QString subject;              ///< Certificate subject
    std::chrono::system_clock::time_point
        expiry_date;              ///< Certificate expiry date
    std::vector<QString> errors;  ///< Validation errors

    /**
     * @brief Convert to JSON representation
     * @return JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Signature validation information
 */
struct SignatureValidation {
    bool is_valid = false;    ///< Is signature valid
    bool is_trusted = false;  ///< Is signature trusted
    QString algorithm;        ///< Signature algorithm
    QString signer;           ///< Signer name
    QString fingerprint;      ///< Signature fingerprint
    std::chrono::system_clock::time_point signature_date;  ///< Signature date
    std::vector<QString> errors;  ///< Validation errors

    /**
     * @brief Convert to JSON representation
     * @return JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Source reputation information
 */
struct SourceReputation {
    enum Level {
        Unknown = 0,    ///< Unknown reputation
        Untrusted = 1,  ///< Untrusted source
        Low = 2,        ///< Low reputation
        Medium = 3,     ///< Medium reputation
        High = 4,       ///< High reputation
        Trusted = 5     ///< Trusted source
    };

    Level level = Unknown;   ///< Reputation level
    int download_count = 0;  ///< Download count
    int success_rate = 0;    ///< Success rate percentage
    std::chrono::system_clock::time_point
        last_verified;                        ///< Last verified time
    std::vector<QString> reputation_sources;  ///< Reputation sources
    QJsonObject metadata;                     ///< Additional metadata

    /**
     * @brief Check if reputation is acceptable
     * @param minimum_level Minimum acceptable level
     * @return True if acceptable, false otherwise
     */
    bool is_acceptable(Level minimum_level = Medium) const {
        return level >= minimum_level;
    }

    /**
     * @brief Convert to JSON representation
     * @return JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Enhanced security validator for remote plugins
 */
class RemotePluginValidator {
public:
    /**
     * @brief Constructor
     * @param security_manager Security manager instance
     * @param configuration Remote plugin configuration
     */
    explicit RemotePluginValidator(
        std::shared_ptr<SecurityManager> security_manager = nullptr,
        std::shared_ptr<RemotePluginConfiguration> configuration = nullptr);

    /**
     * @brief Destructor
     */
    ~RemotePluginValidator();

    // === Source Validation ===

    /**
     * @brief Validate remote plugin source
     * @param source Remote plugin source to validate
     * @return Validation result
     */
    qtplugin::expected<RemoteValidationResult, PluginError> validate_source(
        const RemotePluginSource& source);

    /**
     * @brief Validate source URL
     * @param url Source URL to validate
     * @param security_level Security level to apply
     * @return Validation result
     */
    qtplugin::expected<RemoteValidationResult, PluginError> validate_url(
        const QUrl& url,
        RemoteSecurityLevel security_level = RemoteSecurityLevel::Standard);

    /**
     * @brief Check if domain is trusted
     * @param domain Domain to check
     * @return True if domain is trusted
     */
    bool is_domain_trusted(const QString& domain) const;

    /**
     * @brief Check if domain is blocked
     * @param domain Domain to check
     * @return True if domain is blocked
     */
    bool is_domain_blocked(const QString& domain) const;

    // === Certificate Validation ===

    /**
     * @brief Validate SSL certificate
     * @param certificate Certificate to validate
     * @param hostname Expected hostname
     * @return Certificate validation result
     */
    CertificateValidation validate_certificate(
        const QSslCertificate& certificate,
        const QString& hostname = QString());

    /**
     * @brief Validate certificate chain
     * @param certificates Certificate chain to validate
     * @param hostname Expected hostname
     * @return Certificate validation result
     */
    CertificateValidation validate_certificate_chain(
        const std::vector<QSslCertificate>& certificates,
        const QString& hostname = QString());

    /**
     * @brief Check certificate pinning
     * @param certificate Certificate to check
     * @param hostname Hostname for pinning lookup
     * @return True if certificate is pinned or pinning is not required
     */
    bool check_certificate_pinning(const QSslCertificate& certificate,
                                   const QString& hostname);

    // === Plugin File Validation ===

    /**
     * @brief Validate downloaded plugin file
     * @param file_path Path to plugin file
     * @param source Source information
     * @param expected_checksum Expected file checksum (optional)
     * @return Validation result
     */
    qtplugin::expected<RemoteValidationResult, PluginError> validate_plugin_file(
        const std::filesystem::path& file_path,
        const RemotePluginSource& source,
        const QString& expected_checksum = QString());

    /**
     * @brief Validate plugin signature
     * @param file_path Path to plugin file
     * @return Signature validation result
     */
    SignatureValidation validate_signature(
        const std::filesystem::path& file_path);

    /**
     * @brief Validate plugin metadata
     * @param metadata Plugin metadata to validate
     * @param source Source information
     * @return Validation result
     */
    qtplugin::expected<RemoteValidationResult, PluginError> validate_metadata(
        const QJsonObject& metadata, const RemotePluginSource& source);

    // === Reputation System ===

    /**
     * @brief Get source reputation
     * @param source Remote plugin source
     * @return Source reputation information
     */
    SourceReputation get_source_reputation(const RemotePluginSource& source);

    /**
     * @brief Update source reputation
     * @param source Remote plugin source
     * @param success Whether the operation was successful
     * @param metadata Additional metadata
     */
    void update_source_reputation(const RemotePluginSource& source,
                                  bool success,
                                  const QJsonObject& metadata = {});

    /**
     * @brief Check if source reputation is acceptable
     * @param source Remote plugin source
     * @param minimum_level Minimum acceptable reputation level
     * @return True if reputation is acceptable
     */
    bool is_reputation_acceptable(
        const RemotePluginSource& source,
        SourceReputation::Level minimum_level = SourceReputation::Medium);

    // === Configuration ===

    /**
     * @brief Set security manager
     * @param security_manager Security manager instance
     */
    void set_security_manager(
        std::shared_ptr<SecurityManager> security_manager);

    /**
     * @brief Get security manager
     * @return Security manager instance
     */
    std::shared_ptr<SecurityManager> security_manager() const {
        return m_security_manager;
    }

    /**
     * @brief Set configuration
     * @param configuration Remote plugin configuration
     */
    void set_configuration(
        std::shared_ptr<RemotePluginConfiguration> configuration);

    /**
     * @brief Get configuration
     * @return Remote plugin configuration
     */
    std::shared_ptr<RemotePluginConfiguration> configuration() const {
        return m_configuration;
    }

    // === Cache Management ===

    /**
     * @brief Clear validation cache
     */
    void clear_validation_cache();

    /**
     * @brief Clear reputation cache
     */
    void clear_reputation_cache();

    /**
     * @brief Get validation statistics
     * @return JSON object with statistics
     */
    QJsonObject get_validation_statistics() const;

private:
    /// Core components
    std::shared_ptr<SecurityManager> m_security_manager;  ///< Security manager
    std::shared_ptr<RemotePluginConfiguration>
        m_configuration;  ///< Configuration

    /// Validation cache
    mutable std::mutex
        m_validation_cache_mutex;  ///< Mutex for validation cache
    std::unordered_map<QString, RemoteValidationResult>
        m_validation_cache;  ///< Validation cache

    /// Reputation cache
    mutable std::mutex
        m_reputation_cache_mutex;  ///< Mutex for reputation cache
    std::unordered_map<QString, SourceReputation>
        m_reputation_cache;  ///< Reputation cache

    /// Statistics
    mutable std::mutex m_stats_mutex;             ///< Mutex for statistics
    std::atomic<int> m_validations_performed{0};  ///< Validations performed
    std::atomic<int> m_validations_passed{0};     ///< Validations passed
    std::atomic<int> m_validations_failed{0};     ///< Validations failed
    std::atomic<int> m_validations_blocked{0};    ///< Validations blocked

    // Helper methods
    QString generate_cache_key(const RemotePluginSource& source) const;
    QString generate_cache_key(const QUrl& url) const;
    bool is_validation_cached(const QString& cache_key) const;
    std::optional<RemoteValidationResult> get_cached_validation(
        const QString& cache_key) const;
    void cache_validation_result(const QString& cache_key,
                                 const RemoteValidationResult& result);

    RemoteValidationResult create_validation_result(
        ValidationLevel level, const std::string& message,
        const std::string& details = "") const;

    qtplugin::expected<RemoteValidationResult, PluginError> validate_url_scheme(
        const QUrl& url) const;
    qtplugin::expected<RemoteValidationResult, PluginError> validate_url_security(
        const QUrl& url, RemoteSecurityLevel level) const;

    bool load_reputation_cache();
    bool save_reputation_cache() const;
    std::filesystem::path get_reputation_cache_path() const;

    QString calculate_file_checksum(
        const std::filesystem::path& file_path) const;
    bool verify_file_checksum(const std::filesystem::path& file_path,
                              const QString& expected_checksum) const;
};

}  // namespace qtplugin
