/**
 * @file remote_plugin_manager_extension.cpp
 * @brief Implementation of PluginManager extension for remote plugin support
 */

#include <qtplugin/remote/remote_plugin_manager_extension.hpp>
#include <QRegularExpression>
#include <QUuid>
#include <algorithm>

namespace qtplugin {

// === RemotePluginLoadOptions Implementation ===

qtplugin::RemotePluginLoadOptions RemotePluginLoadOptions::to_remote_options() const {
    qtplugin::RemotePluginLoadOptions remote_opts;
    remote_opts.download_options.timeout = download_timeout;
    remote_opts.download_options.use_cache = cache_remote_plugin;
    remote_opts.download_options.verify_checksum = validate_signature;
    remote_opts.security_level = remote_security_level;
    remote_opts.validate_source = validate_remote_source;
    remote_opts.validate_plugin = validate_signature;
    remote_opts.cache_plugin = cache_remote_plugin;
    remote_opts.auto_update = auto_update_remote;
    remote_opts.validation_timeout = std::chrono::duration_cast<std::chrono::seconds>(timeout);
    return remote_opts;
}

// === RemotePluginManagerExtension Implementation ===

RemotePluginManagerExtension::RemotePluginManagerExtension(std::shared_ptr<PluginManager> plugin_manager)
    : m_plugin_manager(plugin_manager) {
    
    if (!m_plugin_manager) {
        throw std::invalid_argument("Plugin manager cannot be null");
    }
    
    initialize_remote_components();
    register_remote_loaders();
    setup_async_callbacks();
}

RemotePluginManagerExtension::~RemotePluginManagerExtension() = default;

qtplugin::expected<std::string, PluginError> RemotePluginManagerExtension::load_remote_plugin(
    const QUrl& url,
    const RemotePluginLoadOptions& options) {
    
    if (!m_remote_plugins_enabled) {
        return qtplugin::make_error<std::string>(
            PluginErrorCode::InvalidConfiguration,
            "Remote plugin support is disabled");
    }
    
    if (!url.isValid()) {
        return qtplugin::make_error<std::string>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid URL: " + url.toString().toStdString());
    }
    
    // Create temporary source from URL
    RemotePluginSource source(url);
    return load_remote_plugin(source, options);
}

qtplugin::expected<std::string, PluginError> RemotePluginManagerExtension::load_remote_plugin(
    const RemotePluginSource& source,
    const RemotePluginLoadOptions& options) {
    
    if (!m_remote_plugins_enabled) {
        return qtplugin::make_error<std::string>(
            PluginErrorCode::InvalidConfiguration,
            "Remote plugin support is disabled");
    }
    
    // Use HTTP loader for HTTP/HTTPS sources
    if (HttpPluginLoader::is_http_url(source.url())) {
        auto remote_options = options.to_remote_options();
        auto result = m_http_loader->load_remote(source, remote_options);
        
        if (!result) {
            return qtplugin::unexpected(result.error());
        }
        
        // Load the downloaded plugin using the base manager
        auto base_options = convert_to_base_options(options);
        auto load_result = m_plugin_manager->load_plugin(result->cached_path, base_options);
        
        if (load_result) {
            // Track the remote source for this plugin
            std::lock_guard<std::mutex> lock(m_remote_plugins_mutex);
            m_remote_plugin_sources[*load_result] = source;
        }
        
        return load_result;
    }
    
    return qtplugin::make_error<std::string>(
        PluginErrorCode::UnsupportedFormat,
        "Unsupported remote plugin source type");
}

QString RemotePluginManagerExtension::load_remote_plugin_async(
    const QUrl& url,
    const RemotePluginLoadOptions& options,
    std::function<void(const DownloadProgress&)> progress_callback,
    std::function<void(const qtplugin::expected<std::string, PluginError>&)> completion_callback) {
    
    if (!m_remote_plugins_enabled) {
        if (completion_callback) {
            PluginError error{PluginErrorCode::InvalidConfiguration, "Remote plugin support is disabled"};
            completion_callback(qtplugin::unexpected(error));
        }
        return QString();
    }
    
    if (!url.isValid()) {
        if (completion_callback) {
            PluginError error{PluginErrorCode::InvalidConfiguration, "Invalid URL: " + url.toString().toStdString()};
            completion_callback(qtplugin::unexpected(error));
        }
        return QString();
    }
    
    // Create temporary source from URL
    RemotePluginSource source(url);
    
    QString operation_id = generate_operation_id();
    
    // Use HTTP loader for HTTP/HTTPS sources
    if (HttpPluginLoader::is_http_url(source.url())) {
        auto remote_options = options.to_remote_options();
        
        // Create completion wrapper that loads the plugin using base manager
        auto wrapped_completion = [this, options, operation_id, source, completion_callback]
            (const qtplugin::expected<RemotePluginLoadResult, PluginError>& result) {
            
            if (!result) {
                if (completion_callback) {
                    completion_callback(qtplugin::unexpected(result.error()));
                }
                untrack_async_operation(operation_id);
                return;
            }
            
            // Load the downloaded plugin using the base manager
            auto base_options = convert_to_base_options(options);
            auto load_result = m_plugin_manager->load_plugin(result->cached_path, base_options);
            
            if (load_result) {
                // Track the remote source for this plugin
                std::lock_guard<std::mutex> lock(m_remote_plugins_mutex);
                m_remote_plugin_sources[*load_result] = source;
            }
            
            if (completion_callback) {
                completion_callback(load_result);
            }
            
            untrack_async_operation(operation_id);
        };
        
        QString remote_operation_id = m_http_loader->load_remote_async(
            source, remote_options, progress_callback, wrapped_completion);
        
        if (!remote_operation_id.isEmpty()) {
            track_async_operation(operation_id, remote_operation_id.toStdString());
            return operation_id;
        }
    }
    
    if (completion_callback) {
        PluginError error{PluginErrorCode::UnsupportedFormat, "Unsupported remote plugin source type"};
        completion_callback(qtplugin::unexpected(error));
    }
    
    return QString();
}

qtplugin::expected<void, PluginError> RemotePluginManagerExtension::cancel_remote_load(const QString& operation_id) {
    std::lock_guard<std::mutex> lock(m_remote_plugins_mutex);
    
    auto it = m_async_operations.find(operation_id);
    if (it == m_async_operations.end()) {
        return qtplugin::make_error<void>(
            PluginErrorCode::NotFound,
            "Operation not found: " + operation_id.toStdString());
    }
    
    // Cancel the underlying remote operation
    QString remote_operation_id = QString::fromStdString(it->second);
    auto cancel_result = m_http_loader->cancel_remote_load(remote_operation_id);
    
    // Clean up tracking
    m_async_operations.erase(it);
    
    return cancel_result;
}

qtplugin::expected<std::string, PluginError> RemotePluginManagerExtension::load_plugin(
    const std::string& path_or_url,
    const RemotePluginLoadOptions& options) {
    
    if (is_url(path_or_url)) {
        // Load as remote plugin
        QUrl url = parse_url(path_or_url);
        return load_remote_plugin(url, options);
    } else {
        // Load as local plugin using base manager
        std::filesystem::path file_path(path_or_url);
        auto base_options = convert_to_base_options(options);
        return m_plugin_manager->load_plugin(file_path, base_options);
    }
}

qtplugin::expected<void, PluginError> RemotePluginManagerExtension::add_remote_source(const RemotePluginSource& source) {
    if (!m_http_loader) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Remote plugin loader not available");
    }
    
    return m_http_loader->add_source(source);
}

qtplugin::expected<void, PluginError> RemotePluginManagerExtension::remove_remote_source(const QString& source_id) {
    if (!m_http_loader) {
        return qtplugin::make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Remote plugin loader not available");
    }
    
    return m_http_loader->remove_source(source_id);
}

std::vector<RemotePluginSource> RemotePluginManagerExtension::get_remote_sources() const {
    if (!m_http_loader) {
        return {};
    }
    
    return m_http_loader->get_sources();
}

qtplugin::expected<std::vector<QJsonObject>, PluginError> RemotePluginManagerExtension::discover_remote_plugins(
    const QString& source_id) {
    
    if (!m_http_loader) {
        return qtplugin::make_error<std::vector<QJsonObject>>(
            PluginErrorCode::InvalidConfiguration,
            "Remote plugin loader not available");
    }
    
    std::vector<QJsonObject> all_plugins;
    auto sources = m_http_loader->get_sources();
    
    for (const auto& source : sources) {
        // Filter by source ID if specified
        if (!source_id.isEmpty() && source.id() != source_id) {
            continue;
        }
        
        auto result = m_http_loader->discover_plugins(source);
        if (result) {
            auto& plugins = result.value();
            all_plugins.insert(all_plugins.end(), plugins.begin(), plugins.end());
        }
    }
    
    return all_plugins;
}

qtplugin::expected<std::vector<QJsonObject>, PluginError> RemotePluginManagerExtension::search_remote_plugins(
    const QString& query, int max_results) {
    
    if (!m_http_loader) {
        return qtplugin::make_error<std::vector<QJsonObject>>(
            PluginErrorCode::InvalidConfiguration,
            "Remote plugin loader not available");
    }
    
    return m_http_loader->search_plugins(query, max_results);
}

void RemotePluginManagerExtension::set_remote_configuration(std::shared_ptr<RemotePluginConfiguration> configuration) {
    m_remote_configuration = configuration;
    
    if (m_http_loader) {
        m_http_loader->set_configuration(configuration);
    }
    
    if (m_validator) {
        m_validator->set_configuration(configuration);
    }
}

std::shared_ptr<RemotePluginConfiguration> RemotePluginManagerExtension::remote_configuration() const {
    return m_remote_configuration;
}

void RemotePluginManagerExtension::set_remote_plugins_enabled(bool enabled) {
    m_remote_plugins_enabled = enabled;
}

bool RemotePluginManagerExtension::is_remote_plugins_enabled() const {
    return m_remote_plugins_enabled;
}

QJsonObject RemotePluginManagerExtension::get_remote_statistics() const {
    QJsonObject stats;
    
    if (m_http_loader) {
        stats["http_loader"] = m_http_loader->get_statistics();
    }
    
    if (m_download_manager) {
        stats["download_manager"] = m_download_manager->get_statistics();
    }
    
    if (m_validator) {
        stats["validator"] = m_validator->get_validation_statistics();
    }
    
    {
        std::lock_guard<std::mutex> lock(m_remote_plugins_mutex);
        stats["remote_plugins_loaded"] = static_cast<int>(m_remote_plugin_sources.size());
        stats["active_operations"] = static_cast<int>(m_async_operations.size());
    }
    
    return stats;
}

std::vector<QString> RemotePluginManagerExtension::get_active_remote_operations() const {
    std::lock_guard<std::mutex> lock(m_remote_plugins_mutex);
    
    std::vector<QString> operations;
    operations.reserve(m_async_operations.size());
    
    for (const auto& pair : m_async_operations) {
        operations.push_back(pair.first);
    }
    
    return operations;
}

// === Helper Methods ===

bool RemotePluginManagerExtension::is_url(const std::string& path_or_url) const {
    return is_plugin_url(path_or_url);
}

QUrl RemotePluginManagerExtension::parse_url(const std::string& url_string) const {
    return QUrl(QString::fromStdString(url_string));
}

qtplugin::expected<std::string, PluginError> RemotePluginManagerExtension::load_from_cached_file(
    const std::filesystem::path& cached_path, const RemotePluginLoadOptions& options) {

    auto base_options = convert_to_base_options(options);
    return m_plugin_manager->load_plugin(cached_path, base_options);
}

void RemotePluginManagerExtension::initialize_remote_components() {
    // Create default remote configuration if not provided
    if (!m_remote_configuration) {
        m_remote_configuration = std::make_shared<RemotePluginConfiguration>(
            RemotePluginConfiguration::create_default());
    }

    // Create download manager
    m_download_manager = std::make_shared<PluginDownloadManager>();

    // Create validator
    m_validator = std::make_shared<RemotePluginValidator>(nullptr, m_remote_configuration);

    // Create HTTP loader
    m_http_loader = std::make_unique<HttpPluginLoader>(
        m_remote_configuration, m_download_manager, m_validator);
}

void RemotePluginManagerExtension::register_remote_loaders() {
    // Note: In a full implementation, we would register the remote loaders
    // with the base PluginManager's loader registry. For now, we handle
    // remote loading separately in this extension.
}

void RemotePluginManagerExtension::setup_async_callbacks() {
    // Async callbacks are handled in the load_remote_plugin_async method
    // through the completion wrapper function.
}

QString RemotePluginManagerExtension::generate_operation_id() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void RemotePluginManagerExtension::track_async_operation(const QString& operation_id, const std::string& plugin_id) {
    std::lock_guard<std::mutex> lock(m_remote_plugins_mutex);
    m_async_operations[operation_id] = plugin_id;
}

void RemotePluginManagerExtension::untrack_async_operation(const QString& operation_id) {
    std::lock_guard<std::mutex> lock(m_remote_plugins_mutex);
    m_async_operations.erase(operation_id);
}

PluginLoadOptions RemotePluginManagerExtension::convert_to_base_options(const RemotePluginLoadOptions& remote_options) const {
    // Since RemotePluginLoadOptions inherits from PluginLoadOptions, we can use the base part
    PluginLoadOptions base_options;

    // Set default values for base options
    base_options.validate_signature = remote_options.validate_plugin;  // Map remote plugin validation to signature validation
    base_options.check_dependencies = true;  // Default to true for remote plugins
    base_options.initialize_immediately = true;  // Default to true
    base_options.enable_hot_reload = false;  // Remote plugins don't support hot reload by default
    base_options.timeout = std::chrono::duration_cast<std::chrono::milliseconds>(remote_options.validation_timeout);
    base_options.configuration = QJsonObject();  // Empty configuration by default

    // Convert remote security level to base security level
    switch (remote_options.security_level) {
        case RemoteSecurityLevel::Minimal:
            base_options.security_level = SecurityLevel::None;
            break;
        case RemoteSecurityLevel::Standard:
            base_options.security_level = SecurityLevel::Standard;
            break;
        case RemoteSecurityLevel::High:
            base_options.security_level = SecurityLevel::Strict;
            break;
        case RemoteSecurityLevel::Paranoid:
            base_options.security_level = SecurityLevel::Maximum;
            break;
    }

    return base_options;
}

RemotePluginLoadOptions RemotePluginManagerExtension::convert_from_base_options(const PluginLoadOptions& base_options) const {
    RemotePluginLoadOptions remote_options;

    // Map base options to remote options
    remote_options.validate_source = true;  // Always validate remote sources
    remote_options.validate_plugin = base_options.validate_signature;
    remote_options.cache_plugin = true;  // Default to caching remote plugins
    remote_options.auto_update = false;  // Default to no auto-update
    remote_options.validation_timeout = std::chrono::duration_cast<std::chrono::seconds>(base_options.timeout);

    // Convert base security level to remote security level
    switch (base_options.security_level) {
        case SecurityLevel::None:
            remote_options.security_level = RemoteSecurityLevel::Minimal;
            break;
        case SecurityLevel::Basic:
        case SecurityLevel::Permissive:
            remote_options.security_level = RemoteSecurityLevel::Standard;
            break;
        case SecurityLevel::Standard:
        case SecurityLevel::Moderate:
            remote_options.security_level = RemoteSecurityLevel::Standard;
            break;
        case SecurityLevel::Strict:
            remote_options.security_level = RemoteSecurityLevel::High;
            break;
        case SecurityLevel::Maximum:
            remote_options.security_level = RemoteSecurityLevel::Paranoid;
            break;
    }

    return remote_options;
}

// === Factory Implementation ===

std::unique_ptr<RemotePluginManagerExtension> RemotePluginManagerFactory::create_with_remote_support() {
    // Create base plugin manager
    auto base_manager = std::make_shared<PluginManager>();

    // Create extension
    return std::make_unique<RemotePluginManagerExtension>(base_manager);
}

std::unique_ptr<RemotePluginManagerExtension> RemotePluginManagerFactory::create_with_remote_config(
    std::shared_ptr<RemotePluginConfiguration> remote_config) {

    // Create base plugin manager
    auto base_manager = std::make_shared<PluginManager>();

    // Create extension
    auto extension = std::make_unique<RemotePluginManagerExtension>(base_manager);
    extension->set_remote_configuration(remote_config);

    return extension;
}

std::unique_ptr<RemotePluginManagerExtension> RemotePluginManagerFactory::enhance_existing_manager(
    std::shared_ptr<PluginManager> base_manager) {

    return std::make_unique<RemotePluginManagerExtension>(base_manager);
}

std::unique_ptr<RemotePluginManagerExtension> RemotePluginManagerFactory::create_enterprise() {
    // Create base plugin manager
    auto base_manager = std::make_shared<PluginManager>();

    // Create enterprise remote configuration
    auto remote_config = std::make_shared<RemotePluginConfiguration>(
        RemotePluginConfiguration::create_enterprise());

    // Create extension
    auto extension = std::make_unique<RemotePluginManagerExtension>(base_manager);
    extension->set_remote_configuration(remote_config);

    return extension;
}

// === Utility Functions ===

bool is_plugin_url(const std::string& path_or_url) {
    // Simple URL detection - check for common URL schemes
    QRegularExpression url_regex(R"(^(https?|ftp|git(\+https?)?|registry)://)",
                                QRegularExpression::CaseInsensitiveOption);

    QString str = QString::fromStdString(path_or_url);
    return url_regex.match(str).hasMatch();
}

qtplugin::expected<std::string, PluginError> load_plugin_from_path_or_url(
    PluginManager& manager,
    const std::string& path_or_url,
    const PluginLoadOptions& options) {

    if (is_plugin_url(path_or_url)) {
        // This is a URL - we need remote plugin support
        return qtplugin::make_error<std::string>(
            PluginErrorCode::UnsupportedFormat,
            "Remote plugin URLs require RemotePluginManagerExtension");
    } else {
        // This is a local file path
        std::filesystem::path file_path(path_or_url);
        return manager.load_plugin(file_path, options);
    }
}

}  // namespace qtplugin
