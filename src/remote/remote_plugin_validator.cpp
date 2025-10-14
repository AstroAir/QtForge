/**
 * @file remote_plugin_validator.cpp
 * @brief Implementation of enhanced security validation for remote plugins
 */

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSslSocket>
#include <QStandardPaths>
#include <atomic>
#include <fstream>
#include <qtplugin/remote/remote_plugin_validator.hpp>

namespace qtplugin {

// === RemoteValidationResult Implementation ===

QJsonObject RemoteValidationResult::to_json() const {
    QJsonObject json;
    json["level"] = static_cast<int>(level);
    json["message"] = QString::fromStdString(message);
    json["details"] = QString::fromStdString(details);
    json["metadata"] = metadata;
    json["timestamp"] = QDateTime::fromSecsSinceEpoch(
                            std::chrono::duration_cast<std::chrono::seconds>(
                                timestamp.time_since_epoch())
                                .count())
                            .toString(Qt::ISODate);
    return json;
}

RemoteValidationResult RemoteValidationResult::from_json(
    const QJsonObject& json) {
    RemoteValidationResult result;
    result.level = static_cast<ValidationLevel>(json["level"].toInt());
    result.message = json["message"].toString().toStdString();
    result.details = json["details"].toString().toStdString();
    result.metadata = json["metadata"].toObject();

    QDateTime dt =
        QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    result.timestamp =
        std::chrono::system_clock::from_time_t(dt.toSecsSinceEpoch());

    return result;
}

// === CertificateValidation Implementation ===

QJsonObject CertificateValidation::to_json() const {
    QJsonObject json;
    json["is_valid"] = is_valid;
    json["is_self_signed"] = is_self_signed;
    json["is_expired"] = is_expired;
    json["is_trusted"] = is_trusted;
    json["fingerprint"] = fingerprint;
    json["issuer"] = issuer;
    json["subject"] = subject;
    json["expiry_date"] = QDateTime::fromSecsSinceEpoch(
                              std::chrono::duration_cast<std::chrono::seconds>(
                                  expiry_date.time_since_epoch())
                                  .count())
                              .toString(Qt::ISODate);

    QJsonArray errors_array;
    for (const QString& error : errors) {
        errors_array.append(error);
    }
    json["errors"] = errors_array;

    return json;
}

// === SignatureValidation Implementation ===

QJsonObject SignatureValidation::to_json() const {
    QJsonObject json;
    json["is_valid"] = is_valid;
    json["is_trusted"] = is_trusted;
    json["algorithm"] = algorithm;
    json["signer"] = signer;
    json["fingerprint"] = fingerprint;
    json["signature_date"] =
        QDateTime::fromSecsSinceEpoch(
            std::chrono::duration_cast<std::chrono::seconds>(
                signature_date.time_since_epoch())
                .count())
            .toString(Qt::ISODate);

    QJsonArray errors_array;
    for (const QString& error : errors) {
        errors_array.append(error);
    }
    json["errors"] = errors_array;

    return json;
}

// === SourceReputation Implementation ===

QJsonObject SourceReputation::to_json() const {
    QJsonObject json;
    json["level"] = static_cast<int>(level);
    json["download_count"] = download_count;
    json["success_rate"] = success_rate;
    json["last_verified"] =
        QDateTime::fromSecsSinceEpoch(
            std::chrono::duration_cast<std::chrono::seconds>(
                last_verified.time_since_epoch())
                .count())
            .toString(Qt::ISODate);

    QJsonArray sources_array;
    for (const QString& source : reputation_sources) {
        sources_array.append(source);
    }
    json["reputation_sources"] = sources_array;
    json["metadata"] = metadata;

    return json;
}

// === RemotePluginValidator Implementation ===

RemotePluginValidator::RemotePluginValidator(
    std::shared_ptr<ISecurityManager> security_manager,
    std::shared_ptr<RemotePluginConfiguration> configuration)
    : m_security_manager(security_manager), m_configuration(configuration) {
    // Load reputation cache
    load_reputation_cache();
}

RemotePluginValidator::~RemotePluginValidator() {
    // Save reputation cache
    save_reputation_cache();
}

qtplugin::expected<RemoteValidationResult, PluginError>
RemotePluginValidator::validate_source(const RemotePluginSource& source) {
    QString cache_key = generate_cache_key(source);

    // Check cache first
    if (is_validation_cached(cache_key)) {
        auto cached_result = get_cached_validation(cache_key);
        if (cached_result) {
            return *cached_result;
        }
    }

    m_validations_performed++;

    // Validate source configuration
    auto source_validation = source.validate();
    if (!source_validation) {
        RemoteValidationResult result = create_validation_result(
            ValidationLevel::Failed, "Source configuration validation failed",
            source_validation.error().message);

        cache_validation_result(cache_key, result);
        m_validations_failed++;
        return result;
    }

    // Check if domain is blocked
    QString domain = source.url().host();
    if (is_domain_blocked(domain)) {
        RemoteValidationResult result = create_validation_result(
            ValidationLevel::Blocked, "Domain is blocked",
            "Domain " + domain.toStdString() +
                " is in the blocked domains list");

        cache_validation_result(cache_key, result);
        m_validations_blocked++;
        return result;
    }

    // Validate URL
    RemoteSecurityLevel security_level = RemoteSecurityLevel::Standard;
    if (m_configuration) {
        security_level =
            m_configuration->security_policy().default_security_level;
    }

    auto url_validation = validate_url(source.url(), security_level);
    if (!url_validation) {
        return qtplugin::unexpected(url_validation.error());
    }

    if (url_validation.value().is_failed()) {
        cache_validation_result(cache_key, url_validation.value());
        m_validations_failed++;
        return url_validation.value();
    }

    // Check source reputation
    SourceReputation reputation = get_source_reputation(source);
    if (!reputation.is_acceptable()) {
        RemoteValidationResult result = create_validation_result(
            ValidationLevel::Warning, "Source has low reputation",
            "Source reputation level: " + std::to_string(reputation.level));

        cache_validation_result(cache_key, result);
        m_validations_passed++;  // Warning still counts as passed
        return result;
    }

    // All validations passed
    RemoteValidationResult result = create_validation_result(
        ValidationLevel::Passed, "Source validation passed",
        "All security checks passed");

    cache_validation_result(cache_key, result);
    m_validations_passed++;
    return result;
}

qtplugin::expected<RemoteValidationResult, PluginError>
RemotePluginValidator::validate_url(const QUrl& url,
                                    RemoteSecurityLevel security_level) {
    QString cache_key = generate_cache_key(url);

    // Check cache first
    if (is_validation_cached(cache_key)) {
        auto cached_result = get_cached_validation(cache_key);
        if (cached_result) {
            return *cached_result;
        }
    }

    // Validate URL scheme
    auto scheme_validation = validate_url_scheme(url);
    if (!scheme_validation) {
        return qtplugin::unexpected(scheme_validation.error());
    }

    if (scheme_validation.value().is_failed()) {
        cache_validation_result(cache_key, scheme_validation.value());
        return scheme_validation.value();
    }

    // Validate URL security based on security level
    auto security_validation = validate_url_security(url, security_level);
    if (!security_validation) {
        return qtplugin::unexpected(security_validation.error());
    }

    cache_validation_result(cache_key, security_validation.value());
    return security_validation.value();
}

bool RemotePluginValidator::is_domain_trusted(const QString& domain) const {
    if (!m_configuration) {
        return false;
    }

    return m_configuration->is_domain_trusted(domain);
}

bool RemotePluginValidator::is_domain_blocked(const QString& domain) const {
    if (!m_configuration) {
        return false;
    }

    return m_configuration->is_domain_blocked(domain);
}

CertificateValidation RemotePluginValidator::validate_certificate(
    const QSslCertificate& certificate, const QString& hostname) {
    CertificateValidation validation;

    // Check if certificate is valid
    validation.is_valid = !certificate.isNull();
    if (!validation.is_valid) {
        validation.errors.push_back("Certificate is null or invalid");
        return validation;
    }

    // Check expiry
    QDateTime now = QDateTime::currentDateTime();
    validation.is_expired = certificate.expiryDate() < now;
    if (validation.is_expired) {
        validation.errors.push_back("Certificate has expired");
    }

    // Extract certificate information
    validation.fingerprint =
        certificate.digest(QCryptographicHash::Sha256).toHex();
    validation.issuer = certificate.issuerDisplayName();
    validation.subject = certificate.subjectDisplayName();
    validation.expiry_date = std::chrono::system_clock::from_time_t(
        certificate.expiryDate().toSecsSinceEpoch());

    // Check if self-signed
    validation.is_self_signed =
        (certificate.issuerDisplayName() == certificate.subjectDisplayName());

    // Check hostname matching if provided
    if (!hostname.isEmpty()) {
        // Simple hostname validation - in production, this should be more
        // comprehensive
        QString subject_cn =
            certificate.subjectInfo(QSslCertificate::CommonName).join(", ");
        if (!subject_cn.contains(hostname, Qt::CaseInsensitive)) {
            validation.errors.push_back("Certificate hostname does not match");
        }
    }

    // Determine if certificate is trusted
    validation.is_trusted = !validation.is_expired &&
                            !validation.is_self_signed &&
                            validation.errors.empty();

    return validation;
}

CertificateValidation RemotePluginValidator::validate_certificate_chain(
    const std::vector<QSslCertificate>& certificates, const QString& hostname) {
    CertificateValidation validation;

    if (certificates.empty()) {
        validation.errors.push_back("Certificate chain is empty");
        return validation;
    }

    // Validate the leaf certificate (first in chain)
    validation = validate_certificate(certificates[0], hostname);

    // Additional chain validation would go here
    // For now, we just validate the leaf certificate

    return validation;
}

// === Helper Methods ===

QString RemotePluginValidator::generate_cache_key(
    const RemotePluginSource& source) const {
    return source.id();
}

QString RemotePluginValidator::generate_cache_key(const QUrl& url) const {
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(url.toString().toUtf8());
    return hash.result().toHex().left(16);
}

bool RemotePluginValidator::is_validation_cached(
    const QString& cache_key) const {
    std::lock_guard<std::mutex> lock(m_validation_cache_mutex);
    return m_validation_cache.find(cache_key) != m_validation_cache.end();
}

std::optional<RemoteValidationResult>
RemotePluginValidator::get_cached_validation(const QString& cache_key) const {
    std::lock_guard<std::mutex> lock(m_validation_cache_mutex);

    auto it = m_validation_cache.find(cache_key);
    if (it != m_validation_cache.end()) {
        // Check if cache entry is still valid (1 hour TTL)
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::hours>(
            now - it->second.timestamp);
        if (age.count() < 1) {
            return it->second;
        }
        // Note: We can't erase from const method, expired entries will be
        // cleaned up elsewhere
    }

    return std::nullopt;
}

void RemotePluginValidator::cache_validation_result(
    const QString& cache_key, const RemoteValidationResult& result) {
    std::lock_guard<std::mutex> lock(m_validation_cache_mutex);
    m_validation_cache[cache_key] = result;
}

RemoteValidationResult RemotePluginValidator::create_validation_result(
    ValidationLevel level, const std::string& message,
    const std::string& details) const {
    RemoteValidationResult result;
    result.level = level;
    result.message = message;
    result.details = details;
    result.timestamp = std::chrono::system_clock::now();
    return result;
}

qtplugin::expected<RemoteValidationResult, PluginError>
RemotePluginValidator::validate_url_scheme(const QUrl& url) const {
    QString scheme = url.scheme().toLower();

    // Check if scheme is supported
    auto supported_schemes = RemotePluginSource::supported_schemes();
    bool is_supported =
        std::find(supported_schemes.begin(), supported_schemes.end(), scheme) !=
        supported_schemes.end();

    if (!is_supported) {
        RemoteValidationResult result = create_validation_result(
            ValidationLevel::Failed, "Unsupported URL scheme",
            "Scheme '" + scheme.toStdString() + "' is not supported");
        return result;
    }

    RemoteValidationResult result = create_validation_result(
        ValidationLevel::Passed, "URL scheme validation passed",
        "Scheme '" + scheme.toStdString() + "' is supported");

    return result;
}

qtplugin::expected<RemoteValidationResult, PluginError>
RemotePluginValidator::validate_url_security(const QUrl& url,
                                             RemoteSecurityLevel level) const {
    QString scheme = url.scheme().toLower();

    // Check HTTPS requirement based on security level
    if (level >= RemoteSecurityLevel::Standard && scheme != "https") {
        if (m_configuration &&
            m_configuration->security_policy().require_https) {
            RemoteValidationResult result = create_validation_result(
                ValidationLevel::Failed, "HTTPS required",
                "Security policy requires HTTPS for remote plugin sources");
            return result;
        } else {
            RemoteValidationResult result = create_validation_result(
                ValidationLevel::Warning, "Non-HTTPS connection",
                "Using non-secure connection for plugin download");
            return result;
        }
    }

    RemoteValidationResult result = create_validation_result(
        ValidationLevel::Passed, "URL security validation passed",
        "URL meets security requirements");

    return result;
}

QString RemotePluginValidator::calculate_file_checksum(
    const std::filesystem::path& file_path) const {
    QFile file(QString::fromStdString(file_path.string()));
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(&file);
    return hash.result().toHex();
}

bool RemotePluginValidator::verify_file_checksum(
    const std::filesystem::path& file_path,
    const QString& expected_checksum) const {
    QString actual_checksum = calculate_file_checksum(file_path);
    return actual_checksum.compare(expected_checksum, Qt::CaseInsensitive) == 0;
}

void RemotePluginValidator::set_security_manager(
    std::shared_ptr<ISecurityManager> security_manager) {
    m_security_manager = security_manager;
}

void RemotePluginValidator::set_configuration(
    std::shared_ptr<RemotePluginConfiguration> configuration) {
    m_configuration = configuration;
}

void RemotePluginValidator::clear_validation_cache() {
    std::lock_guard<std::mutex> lock(m_validation_cache_mutex);
    m_validation_cache.clear();
}

void RemotePluginValidator::clear_reputation_cache() {
    std::lock_guard<std::mutex> lock(m_reputation_cache_mutex);
    m_reputation_cache.clear();
}

QJsonObject RemotePluginValidator::get_validation_statistics() const {
    QJsonObject stats;
    stats["validations_performed"] = m_validations_performed.load();
    stats["validations_passed"] = m_validations_passed.load();
    stats["validations_failed"] = m_validations_failed.load();
    stats["validations_blocked"] = m_validations_blocked.load();

    int total = m_validations_performed.load();
    if (total > 0) {
        stats["success_rate"] =
            static_cast<double>(m_validations_passed.load()) / total;
        stats["failure_rate"] =
            static_cast<double>(m_validations_failed.load()) / total;
        stats["block_rate"] =
            static_cast<double>(m_validations_blocked.load()) / total;
    }

    {
        std::lock_guard<std::mutex> lock(m_validation_cache_mutex);
        stats["cached_validations"] =
            static_cast<int>(m_validation_cache.size());
    }

    {
        std::lock_guard<std::mutex> lock(m_reputation_cache_mutex);
        stats["cached_reputations"] =
            static_cast<int>(m_reputation_cache.size());
    }

    return stats;
}

bool RemotePluginValidator::load_reputation_cache() {
    std::filesystem::path cache_path = get_reputation_cache_path();

    if (!std::filesystem::exists(cache_path)) {
        return true;  // No cache file yet
    }

    QFile file(QString::fromStdString(cache_path.string()));
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError) {
        return false;
    }

    QJsonObject root = doc.object();
    QJsonObject reputations = root["reputations"].toObject();

    std::lock_guard<std::mutex> lock(m_reputation_cache_mutex);
    m_reputation_cache.clear();

    for (auto it = reputations.begin(); it != reputations.end(); ++it) {
        try {
            SourceReputation reputation;
            QJsonObject rep_obj = it.value().toObject();

            reputation.level =
                static_cast<SourceReputation::Level>(rep_obj["level"].toInt());
            reputation.download_count = rep_obj["download_count"].toInt();
            reputation.success_rate = rep_obj["success_rate"].toInt();

            QDateTime dt = QDateTime::fromString(
                rep_obj["last_verified"].toString(), Qt::ISODate);
            reputation.last_verified =
                std::chrono::system_clock::from_time_t(dt.toSecsSinceEpoch());

            QJsonArray sources_array = rep_obj["reputation_sources"].toArray();
            for (const auto& source_value : sources_array) {
                reputation.reputation_sources.push_back(
                    source_value.toString());
            }

            reputation.metadata = rep_obj["metadata"].toObject();

            m_reputation_cache[it.key()] = reputation;
        } catch (const std::exception& e) {
            // Skip invalid entries
            continue;
        }
    }

    return true;
}

bool RemotePluginValidator::save_reputation_cache() const {
    std::filesystem::path cache_path = get_reputation_cache_path();

    // Create directory if it doesn't exist
    std::filesystem::path dir = cache_path.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        if (ec) {
            return false;
        }
    }

    QJsonObject reputations;
    {
        std::lock_guard<std::mutex> lock(m_reputation_cache_mutex);
        for (const auto& pair : m_reputation_cache) {
            reputations[pair.first] = pair.second.to_json();
        }
    }

    QJsonObject root;
    root["version"] = "1.0";
    root["reputations"] = reputations;

    QFile file(QString::fromStdString(cache_path.string()));
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonDocument doc(root);
    file.write(doc.toJson());

    return true;
}

std::filesystem::path RemotePluginValidator::get_reputation_cache_path() const {
    QString cache_dir =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return std::filesystem::path(cache_dir.toStdString()) / "qtforge" /
           "remote_plugin_reputation.json";
}

SourceReputation RemotePluginValidator::get_source_reputation(
    const RemotePluginSource& source) {
    // TODO: Implement actual reputation checking
    // For now, return a default acceptable reputation
    (void)source;  // Suppress unused parameter warning
    SourceReputation reputation;
    reputation.level = SourceReputation::Medium;
    reputation.download_count = 0;
    reputation.success_rate = 100;
    reputation.last_verified = std::chrono::system_clock::now();
    return reputation;
}

qtplugin::expected<RemoteValidationResult, PluginError>
RemotePluginValidator::validate_plugin_file(
    const std::filesystem::path& file_path, const RemotePluginSource& source,
    const QString& expected_checksum) {
    // TODO: Implement actual file validation
    // For now, return a basic validation result
    (void)file_path;  // Suppress unused parameter warning
    (void)source;
    (void)expected_checksum;

    RemoteValidationResult result;
    result.level = ValidationLevel::Passed;
    result.message = "Plugin file validation not yet implemented";
    result.details = "Stub implementation - always passes";
    result.timestamp = std::chrono::system_clock::now();
    result.metadata = QJsonObject();

    return result;
}

}  // namespace qtplugin
