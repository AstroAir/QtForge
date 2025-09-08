/**
 * @file remote_plugin_source.cpp
 * @brief Implementation of remote plugin source management
 */

#include <qtplugin/remote/remote_plugin_source.hpp>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QTimer>
#include <QUuid>
#include <algorithm>
#include <mutex>
#include <shared_mutex>

namespace qtplugin {

// === AuthenticationCredentials Implementation ===

bool AuthenticationCredentials::is_valid() const {
    switch (type) {
        case AuthenticationType::None:
            return true;
        case AuthenticationType::Basic:
            return !username.isEmpty() && !password.isEmpty();
        case AuthenticationType::Bearer:
        case AuthenticationType::ApiKey:
            return !token.isEmpty() || !api_key.isEmpty();
        case AuthenticationType::Certificate:
            return !certificate_path.isEmpty();
        case AuthenticationType::OAuth2:
            return !oauth2_config.isEmpty();
        default:
            return false;
    }
}

QJsonObject AuthenticationCredentials::to_json() const {
    QJsonObject json;
    json["type"] = static_cast<int>(type);
    
    // Only store non-sensitive data in JSON
    if (type == AuthenticationType::Basic) {
        json["username"] = username;
        // Note: password should be stored securely, not in JSON
    } else if (type == AuthenticationType::Certificate) {
        json["certificate_path"] = certificate_path;
        json["private_key_path"] = private_key_path;
    } else if (type == AuthenticationType::OAuth2) {
        json["oauth2_config"] = oauth2_config;
    }
    
    return json;
}

AuthenticationCredentials AuthenticationCredentials::from_json(const QJsonObject& json) {
    AuthenticationCredentials creds;
    creds.type = static_cast<AuthenticationType>(json["type"].toInt());
    creds.username = json["username"].toString();
    creds.certificate_path = json["certificate_path"].toString();
    creds.private_key_path = json["private_key_path"].toString();
    creds.oauth2_config = json["oauth2_config"].toObject();
    return creds;
}

// === RemoteSourceConfig Implementation ===

QJsonObject RemoteSourceConfig::to_json() const {
    QJsonObject json;
    json["cache_policy"] = static_cast<int>(cache_policy);
    json["security_level"] = static_cast<int>(security_level);
    json["cache_ttl"] = static_cast<qint64>(cache_ttl.count());
    json["timeout"] = static_cast<qint64>(timeout.count());
    json["max_retries"] = max_retries;
    json["verify_ssl"] = verify_ssl;
    json["allow_redirects"] = allow_redirects;
    json["max_download_size"] = static_cast<qint64>(max_download_size);
    json["custom_headers"] = custom_headers;
    json["custom_options"] = custom_options;
    return json;
}

RemoteSourceConfig RemoteSourceConfig::from_json(const QJsonObject& json) {
    RemoteSourceConfig config;
    config.cache_policy = static_cast<CachePolicy>(json["cache_policy"].toInt());
    config.security_level = static_cast<RemoteSecurityLevel>(json["security_level"].toInt());
    config.cache_ttl = std::chrono::seconds(json["cache_ttl"].toInt());
    config.timeout = std::chrono::seconds(json["timeout"].toInt());
    config.max_retries = json["max_retries"].toInt();
    config.verify_ssl = json["verify_ssl"].toBool();
    config.allow_redirects = json["allow_redirects"].toBool();
    config.max_download_size = json["max_download_size"].toInt();
    config.custom_headers = json["custom_headers"].toObject();
    config.custom_options = json["custom_options"].toObject();
    return config;
}

// === RemotePluginSource Implementation ===

RemotePluginSource::RemotePluginSource(const QUrl& url, RemoteSourceType type, const QString& name)
    : m_url(url), m_type(type), m_name(name) {
    
    if (m_type == RemoteSourceType::Http && type == RemoteSourceType::Http) {
        // Auto-detect type if default was used
        m_type = detect_source_type(url);
    }
    
    if (m_name.isEmpty()) {
        m_name = url.host();
    }
    
    initialize_defaults();
}

RemotePluginSource::RemotePluginSource(const QJsonObject& json) {
    *this = from_json(json);
}

RemotePluginSource::RemotePluginSource(const RemotePluginSource& other)
    : m_url(other.m_url),
      m_type(other.m_type),
      m_name(other.m_name),
      m_enabled(other.m_enabled),
      m_auth(other.m_auth),
      m_config(other.m_config) {
}

RemotePluginSource& RemotePluginSource::operator=(const RemotePluginSource& other) {
    if (this != &other) {
        m_url = other.m_url;
        m_type = other.m_type;
        m_name = other.m_name;
        m_enabled = other.m_enabled;
        m_auth = other.m_auth;
        m_config = other.m_config;
    }
    return *this;
}

RemotePluginSource::~RemotePluginSource() = default;

void RemotePluginSource::set_url(const QUrl& url) {
    m_url = url;
    // Auto-update type if it was auto-detected
    if (m_type == detect_source_type(m_url)) {
        m_type = detect_source_type(url);
    }
}

QString RemotePluginSource::id() const {
    return generate_id();
}

void RemotePluginSource::set_authentication(const AuthenticationCredentials& credentials) {
    m_auth = credentials;
}

bool RemotePluginSource::has_authentication() const {
    return m_auth.type != AuthenticationType::None && m_auth.is_valid();
}

void RemotePluginSource::set_configuration(const RemoteSourceConfig& config) {
    m_config = config;
}

void RemotePluginSource::set_config_option(const QString& key, const QJsonValue& value) {
    m_config.custom_options[key] = value;
}

QJsonValue RemotePluginSource::get_config_option(const QString& key) const {
    return m_config.custom_options[key];
}

qtplugin::expected<void, PluginError> RemotePluginSource::validate() const {
    // Validate URL
    if (!m_url.isValid()) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid URL: " + m_url.toString().toStdString());
    }
    
    // Validate scheme
    if (!is_supported_url(m_url)) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Unsupported URL scheme: " + m_url.scheme().toStdString());
    }
    
    // Validate authentication
    if (has_authentication() && !m_auth.is_valid()) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid authentication configuration");
    }
    
    // Validate configuration
    if (m_config.timeout.count() <= 0) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid timeout configuration");
    }
    
    if (m_config.max_download_size == 0) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid max download size configuration");
    }
    
    return qtplugin::make_success();
}

qtplugin::expected<void, PluginError> RemotePluginSource::test_connection() const {
    // Basic validation first
    auto validation_result = validate();
    if (!validation_result) {
        return validation_result;
    }
    
    // For HTTP sources, try a HEAD request
    if (m_type == RemoteSourceType::Http) {
        QNetworkAccessManager manager;
        QNetworkRequest request(m_url);
        
        // Set timeout
        request.setTransferTimeout(static_cast<int>(m_config.timeout.count() * 1000));
        
        // Add authentication if configured
        if (has_authentication()) {
            if (m_auth.type == AuthenticationType::Basic) {
                QString credentials = m_auth.username + ":" + m_auth.password;
                QString encoded = credentials.toUtf8().toBase64();
                request.setRawHeader("Authorization", "Basic " + encoded.toUtf8());
            } else if (m_auth.type == AuthenticationType::Bearer) {
                request.setRawHeader("Authorization", "Bearer " + m_auth.token.toUtf8());
            } else if (m_auth.type == AuthenticationType::ApiKey) {
                request.setRawHeader("X-API-Key", m_auth.api_key.toUtf8());
            }
        }
        
        // Add custom headers
        for (auto it = m_config.custom_headers.begin(); it != m_config.custom_headers.end(); ++it) {
            request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
        }
        
        QNetworkReply* reply = manager.head(request);
        
        // Wait for response with timeout
        QTimer timer;
        timer.setSingleShot(true);
        timer.start(static_cast<int>(m_config.timeout.count() * 1000));
        
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        loop.exec();
        
        if (timer.isActive()) {
            timer.stop();
            if (reply->error() == QNetworkReply::NoError) {
                reply->deleteLater();
                return qtplugin::make_success();
            } else {
                QString error_msg = "Connection test failed: " + reply->errorString();
                reply->deleteLater();
                return qtplugin::make_error<void>(
                    PluginErrorCode::NetworkError, error_msg.toStdString());
            }
        } else {
            reply->abort();
            reply->deleteLater();
            return qtplugin::make_error<void>(
                PluginErrorCode::NetworkError, "Connection test timed out");
        }
    }
    
    // For other source types, just return success for now
    // TODO: Implement specific connection tests for Git, FTP, etc.
    return qtplugin::make_success();
}

QJsonObject RemotePluginSource::to_json() const {
    QJsonObject json;
    json["url"] = m_url.toString();
    json["type"] = static_cast<int>(m_type);
    json["name"] = m_name;
    json["enabled"] = m_enabled;
    json["authentication"] = m_auth.to_json();
    json["configuration"] = m_config.to_json();
    return json;
}

RemotePluginSource RemotePluginSource::from_json(const QJsonObject& json) {
    RemotePluginSource source(
        QUrl(json["url"].toString()),
        static_cast<RemoteSourceType>(json["type"].toInt()),
        json["name"].toString()
    );
    
    source.m_enabled = json["enabled"].toBool();
    source.m_auth = AuthenticationCredentials::from_json(json["authentication"].toObject());
    source.m_config = RemoteSourceConfig::from_json(json["configuration"].toObject());
    
    return source;
}

QString RemotePluginSource::to_string() const {
    return QString("%1 (%2)").arg(m_name, m_url.toString());
}

RemoteSourceType RemotePluginSource::detect_source_type(const QUrl& url) {
    QString scheme = url.scheme().toLower();
    
    if (scheme == "http" || scheme == "https") {
        return RemoteSourceType::Http;
    } else if (scheme == "git" || scheme.startsWith("git+")) {
        return RemoteSourceType::Git;
    } else if (scheme == "ftp" || scheme == "ftps") {
        return RemoteSourceType::Ftp;
    } else if (scheme == "registry") {
        return RemoteSourceType::Registry;
    } else {
        return RemoteSourceType::Custom;
    }
}

bool RemotePluginSource::is_supported_url(const QUrl& url) {
    auto supported = supported_schemes();
    QString scheme = url.scheme().toLower();
    return std::find(supported.begin(), supported.end(), scheme) != supported.end();
}

std::vector<QString> RemotePluginSource::supported_schemes() {
    return {"http", "https", "git", "git+http", "git+https", "ftp", "ftps", "registry"};
}

bool RemotePluginSource::operator==(const RemotePluginSource& other) const {
    return m_url == other.m_url && m_type == other.m_type;
}

bool RemotePluginSource::operator!=(const RemotePluginSource& other) const {
    return !(*this == other);
}

void RemotePluginSource::initialize_defaults() {
    // Set reasonable defaults based on source type
    switch (m_type) {
        case RemoteSourceType::Http:
            m_config.timeout = std::chrono::seconds{30};
            m_config.max_retries = 3;
            break;
        case RemoteSourceType::Git:
            m_config.timeout = std::chrono::seconds{60};
            m_config.max_retries = 2;
            break;
        case RemoteSourceType::Ftp:
            m_config.timeout = std::chrono::seconds{45};
            m_config.max_retries = 2;
            break;
        default:
            // Use default values
            break;
    }
}

QString RemotePluginSource::generate_id() const {
    // Generate a consistent ID based on URL and type
    QString data = m_url.toString() + QString::number(static_cast<int>(m_type));
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(data.toUtf8());
    return hash.result().toHex().left(16);  // Use first 16 characters
}

// === RemoteSourceManager Implementation ===

RemoteSourceManager::RemoteSourceManager() = default;

RemoteSourceManager::~RemoteSourceManager() = default;

qtplugin::expected<void, PluginError> RemoteSourceManager::add_source(const RemotePluginSource& source) {
    // Validate source first
    auto validation_result = source.validate();
    if (!validation_result) {
        return validation_result;
    }

    std::unique_lock lock(m_sources_mutex);

    QString source_id = source.id();

    // Check if source already exists
    if (m_sources.find(source_id) != m_sources.end()) {
        return qtplugin::make_error<void>(
            PluginErrorCode::AlreadyExists,
            "Remote source already exists: " + source_id.toStdString());
    }

    m_sources[source_id] = source;
    return qtplugin::make_success();
}

qtplugin::expected<void, PluginError> RemoteSourceManager::remove_source(const QString& source_id) {
    std::unique_lock lock(m_sources_mutex);

    auto it = m_sources.find(source_id);
    if (it == m_sources.end()) {
        return qtplugin::make_error<void>(
            PluginErrorCode::NotFound,
            "Remote source not found: " + source_id.toStdString());
    }

    m_sources.erase(it);
    return qtplugin::make_success();
}

std::optional<RemotePluginSource> RemoteSourceManager::get_source(const QString& source_id) const {
    std::shared_lock lock(m_sources_mutex);

    auto it = m_sources.find(source_id);
    if (it != m_sources.end()) {
        return it->second;
    }

    return std::nullopt;
}

std::vector<RemotePluginSource> RemoteSourceManager::get_all_sources() const {
    std::shared_lock lock(m_sources_mutex);

    std::vector<RemotePluginSource> sources;
    sources.reserve(m_sources.size());

    for (const auto& pair : m_sources) {
        sources.push_back(pair.second);
    }

    return sources;
}

std::vector<RemotePluginSource> RemoteSourceManager::get_enabled_sources() const {
    std::shared_lock lock(m_sources_mutex);

    std::vector<RemotePluginSource> enabled_sources;

    for (const auto& pair : m_sources) {
        if (pair.second.is_enabled()) {
            enabled_sources.push_back(pair.second);
        }
    }

    return enabled_sources;
}

void RemoteSourceManager::clear() {
    std::unique_lock lock(m_sources_mutex);
    m_sources.clear();
}

qtplugin::expected<void, PluginError> RemoteSourceManager::load_from_config(const QJsonObject& config) {
    QJsonArray sources_array = config["sources"].toArray();

    std::unique_lock lock(m_sources_mutex);
    m_sources.clear();

    for (const auto& source_value : sources_array) {
        if (!source_value.isObject()) {
            continue;
        }

        try {
            RemotePluginSource source = RemotePluginSource::from_json(source_value.toObject());

            // Validate source
            auto validation_result = source.validate();
            if (!validation_result) {
                // Log warning but continue loading other sources
                continue;
            }

            m_sources[source.id()] = source;
        } catch (const std::exception& e) {
            // Log error but continue loading other sources
            continue;
        }
    }

    return qtplugin::make_success();
}

QJsonObject RemoteSourceManager::save_to_config() const {
    std::shared_lock lock(m_sources_mutex);

    QJsonArray sources_array;
    for (const auto& pair : m_sources) {
        sources_array.append(pair.second.to_json());
    }

    QJsonObject config;
    config["sources"] = sources_array;
    config["version"] = "1.0";

    return config;
}

}  // namespace qtplugin
