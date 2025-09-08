/**
 * @file remote_plugin_loader.cpp
 * @brief Implementation of remote plugin loader base class
 */

#include <qtplugin/remote/remote_plugin_loader.hpp>
#include <qtplugin/core/qt_plugin_loader.hpp>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <algorithm>

namespace qtplugin {

// === RemotePluginLoadOptions Implementation ===

QJsonObject RemotePluginLoadOptions::to_json() const {
    QJsonObject json;
    json["download_options"] = download_options.to_json();
    json["security_level"] = static_cast<int>(security_level);
    json["validate_source"] = validate_source;
    json["validate_plugin"] = validate_plugin;
    json["cache_plugin"] = cache_plugin;
    json["auto_update"] = auto_update;
    json["validation_timeout"] = static_cast<qint64>(validation_timeout.count());
    return json;
}

RemotePluginLoadOptions RemotePluginLoadOptions::from_json(const QJsonObject& json) {
    RemotePluginLoadOptions options;
    options.download_options = DownloadOptions::from_json(json["download_options"].toObject());
    options.security_level = static_cast<RemoteSecurityLevel>(json["security_level"].toInt());
    options.validate_source = json["validate_source"].toBool();
    options.validate_plugin = json["validate_plugin"].toBool();
    options.cache_plugin = json["cache_plugin"].toBool();
    options.auto_update = json["auto_update"].toBool();
    options.validation_timeout = std::chrono::seconds(json["validation_timeout"].toInt());
    return options;
}

// === RemotePluginLoadResult Implementation ===

QJsonObject RemotePluginLoadResult::to_json() const {
    QJsonObject json;
    if (plugin) {
        json["plugin_id"] = QString::fromStdString(plugin->id());
        json["plugin_name"] = QString::fromStdString(plugin->name());
        json["plugin_version"] = QString::fromStdString(plugin->version());
    }
    json["source"] = source.to_json();
    json["download_result"] = download_result.to_json();
    json["validation_result"] = validation_result.to_json();
    json["cached_path"] = QString::fromStdString(cached_path.string());
    json["load_time"] = QDateTime::fromSecsSinceEpoch(
        std::chrono::duration_cast<std::chrono::seconds>(load_time.time_since_epoch()).count()
    ).toString(Qt::ISODate);
    json["metadata"] = metadata;
    return json;
}

// === RemotePluginLoaderBase Implementation ===

RemotePluginLoaderBase::RemotePluginLoaderBase(
    std::shared_ptr<RemotePluginConfiguration> configuration,
    std::shared_ptr<PluginDownloadManager> download_manager,
    std::shared_ptr<RemotePluginValidator> validator)
    : m_configuration(configuration),
      m_download_manager(download_manager),
      m_validator(validator) {
    
    initialize_components();
}

RemotePluginLoaderBase::~RemotePluginLoaderBase() = default;

bool RemotePluginLoaderBase::can_load(const std::filesystem::path& file_path) const {
    // Check if it's a URL string in a file or a local plugin file
    if (m_local_loader) {
        return m_local_loader->can_load(file_path);
    }
    
    // Check if file contains a URL
    if (std::filesystem::exists(file_path)) {
        std::ifstream file(file_path);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        QUrl url(QString::fromStdString(content).trimmed());
        return can_load_remote(url);
    }
    
    return false;
}

qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> RemotePluginLoaderBase::load(
    const std::filesystem::path& file_path) {
    
    // Try loading as local plugin first
    if (m_local_loader && m_local_loader->can_load(file_path)) {
        return m_local_loader->load(file_path);
    }
    
    // Try loading as URL file
    if (std::filesystem::exists(file_path)) {
        std::ifstream file(file_path);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        
        QUrl url(QString::fromStdString(content).trimmed());
        if (can_load_remote(url)) {
            auto result = load_remote(url);
            if (result) {
                return result->plugin;
            } else {
                return qtplugin::unexpected(result.error());
            }
        }
    }
    
    return qtplugin::make_error<std::shared_ptr<IPlugin>>(
        PluginErrorCode::UnsupportedFormat,
        "File format not supported by remote plugin loader");
}

qtplugin::expected<void, PluginError> RemotePluginLoaderBase::unload(std::string_view plugin_id) {
    if (m_local_loader) {
        return m_local_loader->unload(plugin_id);
    }
    
    return qtplugin::make_error<void>(
        PluginErrorCode::NotImplemented,
        "Unload not implemented for remote plugin loader");
}

std::vector<std::string> RemotePluginLoaderBase::supported_extensions() const {
    std::vector<std::string> extensions = {".qtplugin", ".url", ".link"};
    
    if (m_local_loader) {
        auto local_extensions = m_local_loader->supported_extensions();
        extensions.insert(extensions.end(), local_extensions.begin(), local_extensions.end());
    }
    
    return extensions;
}

std::string_view RemotePluginLoaderBase::name() const noexcept {
    return "RemotePluginLoader";
}

bool RemotePluginLoaderBase::supports_hot_reload() const noexcept {
    return false;  // Remote plugins don't support hot reload by default
}

bool RemotePluginLoaderBase::can_load_remote(const QUrl& url) const {
    if (!url.isValid()) {
        return false;
    }
    
    // Check if URL scheme is supported
    return RemotePluginSource::is_supported_url(url);
}

qtplugin::expected<void, PluginError> RemotePluginLoaderBase::add_source(const RemotePluginSource& source) {
    if (!m_configuration) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "No configuration available");
    }
    
    return m_configuration->add_trusted_source(source);
}

qtplugin::expected<void, PluginError> RemotePluginLoaderBase::remove_source(const QString& source_id) {
    if (!m_configuration) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "No configuration available");
    }
    
    return m_configuration->remove_source(source_id);
}

std::vector<RemotePluginSource> RemotePluginLoaderBase::get_sources() const {
    if (!m_configuration) {
        return {};
    }
    
    return m_configuration->get_all_sources();
}

std::optional<RemotePluginSource> RemotePluginLoaderBase::find_source_for_url(const QUrl& url) const {
    auto sources = get_sources();
    
    for (const auto& source : sources) {
        // Check if URL matches source URL or is under source domain
        if (source.url().host() == url.host()) {
            return source;
        }
    }
    
    return std::nullopt;
}

void RemotePluginLoaderBase::set_configuration(std::shared_ptr<RemotePluginConfiguration> configuration) {
    m_configuration = configuration;
    
    // Update validator configuration
    if (m_validator && configuration) {
        m_validator->set_configuration(configuration);
    }
}

std::shared_ptr<RemotePluginConfiguration> RemotePluginLoaderBase::configuration() const {
    return m_configuration;
}

void RemotePluginLoaderBase::set_download_manager(std::shared_ptr<PluginDownloadManager> download_manager) {
    m_download_manager = download_manager;
}

std::shared_ptr<PluginDownloadManager> RemotePluginLoaderBase::download_manager() const {
    return m_download_manager;
}

void RemotePluginLoaderBase::set_validator(std::shared_ptr<RemotePluginValidator> validator) {
    m_validator = validator;
    
    // Update validator configuration
    if (validator && m_configuration) {
        validator->set_configuration(m_configuration);
    }
}

std::shared_ptr<RemotePluginValidator> RemotePluginLoaderBase::validator() const {
    return m_validator;
}

QJsonObject RemotePluginLoaderBase::get_statistics() const {
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    
    QJsonObject stats;
    stats["remote_loads_attempted"] = m_remote_loads_attempted.load();
    stats["remote_loads_successful"] = m_remote_loads_successful.load();
    stats["remote_loads_failed"] = m_remote_loads_failed.load();
    stats["cache_hits"] = m_cache_hits.load();
    
    int total = m_remote_loads_attempted.load();
    if (total > 0) {
        stats["success_rate"] = static_cast<double>(m_remote_loads_successful.load()) / total;
        stats["failure_rate"] = static_cast<double>(m_remote_loads_failed.load()) / total;
        stats["cache_hit_rate"] = static_cast<double>(m_cache_hits.load()) / total;
    }
    
    {
        std::lock_guard<std::mutex> ops_lock(m_operations_mutex);
        stats["active_operations"] = static_cast<int>(m_active_operations.size());
    }
    
    // Add component statistics
    if (m_download_manager) {
        stats["download_manager"] = m_download_manager->get_statistics();
    }
    
    if (m_validator) {
        stats["validator"] = m_validator->get_validation_statistics();
    }
    
    return stats;
}

void RemotePluginLoaderBase::reset_statistics() {
    std::lock_guard<std::mutex> lock(m_stats_mutex);
    
    m_remote_loads_attempted = 0;
    m_remote_loads_successful = 0;
    m_remote_loads_failed = 0;
    m_cache_hits = 0;
    
    if (m_download_manager) {
        m_download_manager->reset_statistics();
    }
    
    if (m_validator) {
        m_validator->clear_validation_cache();
    }
}

std::vector<QString> RemotePluginLoaderBase::get_active_operations() const {
    std::lock_guard<std::mutex> lock(m_operations_mutex);
    
    std::vector<QString> operation_ids;
    operation_ids.reserve(m_active_operations.size());
    
    for (const auto& pair : m_active_operations) {
        operation_ids.push_back(pair.first);
    }
    
    return operation_ids;
}

QString RemotePluginLoaderBase::generate_operation_id() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void RemotePluginLoaderBase::track_operation(const QString& operation_id, const QJsonObject& info) {
    std::lock_guard<std::mutex> lock(m_operations_mutex);
    m_active_operations[operation_id] = info;
}

void RemotePluginLoaderBase::untrack_operation(const QString& operation_id) {
    std::lock_guard<std::mutex> lock(m_operations_mutex);
    m_active_operations.erase(operation_id);
}

qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> RemotePluginLoaderBase::load_from_cache(
    const std::filesystem::path& cached_path) {
    
    if (!m_local_loader) {
        return qtplugin::make_error<std::shared_ptr<IPlugin>>(
            PluginErrorCode::InvalidConfiguration,
            "No local plugin loader available");
    }
    
    if (!std::filesystem::exists(cached_path)) {
        return qtplugin::make_error<std::shared_ptr<IPlugin>>(
            PluginErrorCode::FileNotFound,
            "Cached plugin file not found");
    }
    
    m_cache_hits++;
    return m_local_loader->load(cached_path);
}

void RemotePluginLoaderBase::initialize_components() {
    // Create default components if not provided
    if (!m_configuration) {
        m_configuration = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_default());
    }
    
    if (!m_download_manager) {
        m_download_manager = std::make_shared<PluginDownloadManager>();
    }
    
    if (!m_validator) {
        m_validator = std::make_shared<RemotePluginValidator>(nullptr, m_configuration);
    }
    
    // Create local plugin loader for handling downloaded plugins
    m_local_loader = std::make_unique<QtPluginLoader>();
    
    validate_configuration();
}

void RemotePluginLoaderBase::validate_configuration() const {
    if (!m_configuration) {
        throw std::runtime_error("Remote plugin configuration is required");
    }
    
    auto validation_result = m_configuration->validate();
    if (!validation_result) {
        throw std::runtime_error("Invalid remote plugin configuration: " + validation_result.error().message);
    }
}

}  // namespace qtplugin
