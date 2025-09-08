/**
 * @file http_plugin_loader.cpp
 * @brief Implementation of HTTP/HTTPS plugin loader
 */

#include <qtplugin/remote/http_plugin_loader.hpp>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrlQuery>

namespace qtplugin {

HttpPluginLoader::HttpPluginLoader(
    std::shared_ptr<RemotePluginConfiguration> configuration,
    std::shared_ptr<PluginDownloadManager> download_manager,
    std::shared_ptr<RemotePluginValidator> validator,
    QObject* parent)
    : RemotePluginLoaderBase(configuration, download_manager, validator),
      m_network_manager(std::make_unique<QNetworkAccessManager>(this)) {
    
    setParent(parent);
    
    // Connect download manager signals for async operations
    if (m_download_manager) {
        connect(m_download_manager.get(), &PluginDownloadManager::download_progress,
                this, &HttpPluginLoader::on_async_load_progress);
        connect(m_download_manager.get(), &PluginDownloadManager::download_completed,
                this, &HttpPluginLoader::on_async_load_completed);
        connect(m_download_manager.get(), &PluginDownloadManager::download_failed,
                this, &HttpPluginLoader::on_async_load_failed);
    }
}

HttpPluginLoader::~HttpPluginLoader() = default;

bool HttpPluginLoader::can_load_remote(const QUrl& url) const {
    return is_http_url(url) && RemotePluginLoaderBase::can_load_remote(url);
}

qtplugin::expected<RemotePluginLoadResult, PluginError> HttpPluginLoader::load_remote(
    const RemotePluginSource& source,
    const RemotePluginLoadOptions& options) {
    
    m_remote_loads_attempted++;
    
    try {
        return perform_remote_load(source, options);
    } catch (const std::exception& e) {
        m_remote_loads_failed++;
        return qtplugin::make_error<RemotePluginLoadResult>(
            PluginErrorCode::LoadingFailed,
            "Remote plugin loading failed: " + std::string(e.what()));
    }
}

qtplugin::expected<RemotePluginLoadResult, PluginError> HttpPluginLoader::load_remote(
    const QUrl& url,
    const RemotePluginLoadOptions& options) {
    
    // Create a temporary source from URL
    RemotePluginSource source = create_source_from_url(url);
    return load_remote(source, options);
}

QString HttpPluginLoader::load_remote_async(
    const RemotePluginSource& source,
    const RemotePluginLoadOptions& options,
    std::function<void(const DownloadProgress&)> progress_callback,
    std::function<void(const qtplugin::expected<RemotePluginLoadResult, PluginError>&)> completion_callback) {
    
    m_remote_loads_attempted++;
    
    QString operation_id = register_async_operation(source, options, progress_callback, completion_callback);
    
    // Start async validation and download
    QTimer::singleShot(0, this, [this, operation_id, source, options]() {
        // Validate source first
        auto validation_result = validate_http_source(source, options);
        if (!validation_result) {
            complete_async_operation(operation_id, qtplugin::unexpected(validation_result.error()));
            return;
        }
        
        if (validation_result->is_failed()) {
            PluginError error{PluginErrorCode::SecurityViolation, validation_result->message};
            complete_async_operation(operation_id, qtplugin::unexpected(error));
            return;
        }
        
        // Check cache first
        if (options.cache_plugin && m_download_manager->is_cached(source.url())) {
            auto cached_path = m_download_manager->get_cached_path(source.url());
            if (cached_path) {
                auto plugin_result = load_from_cache(*cached_path);
                if (plugin_result) {
                    RemotePluginLoadResult result;
                    result.plugin = *plugin_result;
                    result.source = source;
                    result.cached_path = *cached_path;
                    result.load_time = std::chrono::system_clock::now();
                    result.validation_result = *validation_result;
                    
                    m_cache_hits++;
                    complete_async_operation(operation_id, result);
                    return;
                }
            }
        }
        
        // Start async download
        QString download_id = m_download_manager->download_plugin_async(
            source, QUrl(), options.download_options,
            nullptr,  // Progress handled by our slot
            nullptr   // Completion handled by our slot
        );
        
        if (download_id.isEmpty()) {
            PluginError error{PluginErrorCode::NetworkError, "Failed to start download"};
            complete_async_operation(operation_id, qtplugin::unexpected(error));
            return;
        }
        
        // Map download ID to operation ID
        {
            std::lock_guard<std::mutex> lock(m_async_operations_mutex);
            m_download_to_operation_map[download_id] = operation_id;
        }
    });
    
    return operation_id;
}

qtplugin::expected<void, PluginError> HttpPluginLoader::cancel_remote_load(const QString& operation_id) {
    std::lock_guard<std::mutex> lock(m_async_operations_mutex);
    
    auto it = m_async_operations.find(operation_id);
    if (it == m_async_operations.end()) {
        return qtplugin::make_error<void>(
            PluginErrorCode::NotFound,
            "Operation not found: " + operation_id.toStdString());
    }
    
    // Cancel download if active
    if (!it->second->download_id.isEmpty()) {
        m_download_manager->cancel_download(it->second->download_id);
    }
    
    // Clean up operation
    cleanup_async_operation(operation_id);
    
    return qtplugin::make_success();
}

qtplugin::expected<std::vector<QJsonObject>, PluginError> HttpPluginLoader::discover_plugins(
    const RemotePluginSource& source) {
    
    if (!is_http_url(source.url())) {
        return qtplugin::make_error<std::vector<QJsonObject>>(
            PluginErrorCode::UnsupportedFormat,
            "Source is not HTTP/HTTPS");
    }
    
    // Check if source supports registry API
    if (supports_registry_api(source)) {
        QUrl endpoint = get_registry_endpoint(source, "list");
        auto response = make_registry_request(endpoint);
        if (response) {
            auto plugins = parse_plugin_list_response(*response);
            if (plugins) {
                std::vector<QJsonObject> result;
                for (const auto& plugin_value : *plugins) {
                    result.push_back(plugin_value.toObject());
                }
                return result;
            } else {
                return qtplugin::unexpected(plugins.error());
            }
        } else {
            return qtplugin::unexpected(response.error());
        }
    } else {
        // Try to discover plugins from a directory listing or index page
        QUrl discovery_url = build_discovery_url(source);
        auto response = make_http_request(discovery_url);
        if (response) {
            if (is_valid_discovery_response(*response)) {
                auto plugins = parse_plugin_list_response(*response);
                if (plugins) {
                    std::vector<QJsonObject> result;
                    for (const auto& plugin_value : *plugins) {
                        result.push_back(plugin_value.toObject());
                    }
                    return result;
                }
            }
        }
        
        // Fallback: return empty list if discovery is not supported
        return std::vector<QJsonObject>{};
    }
}

qtplugin::expected<std::vector<QJsonObject>, PluginError> HttpPluginLoader::search_plugins(
    const QString& query, int max_results) {
    
    std::vector<QJsonObject> all_results;
    auto sources = get_sources();
    
    for (const auto& source : sources) {
        if (!is_http_url(source.url())) {
            continue;
        }
        
        try {
            if (supports_registry_api(source)) {
                QUrl endpoint = get_registry_endpoint(source, "search");
                QJsonObject params;
                params["query"] = query;
                params["limit"] = max_results;
                
                auto response = make_registry_request(endpoint, params);
                if (response) {
                    auto plugins = parse_plugin_list_response(*response);
                    if (plugins) {
                        for (const auto& plugin_value : *plugins) {
                            all_results.push_back(plugin_value.toObject());
                            if (all_results.size() >= static_cast<size_t>(max_results)) {
                                break;
                            }
                        }
                    }
                }
            } else {
                // Try basic search using URL parameters
                QUrl search_url = build_search_url(source, query, max_results);
                auto response = make_http_request(search_url);
                if (response && is_valid_search_response(*response)) {
                    auto plugins = parse_plugin_list_response(*response);
                    if (plugins) {
                        for (const auto& plugin_value : *plugins) {
                            all_results.push_back(plugin_value.toObject());
                            if (all_results.size() >= static_cast<size_t>(max_results)) {
                                break;
                            }
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            // Continue with other sources if one fails
            continue;
        }
        
        if (all_results.size() >= static_cast<size_t>(max_results)) {
            break;
        }
    }
    
    return all_results;
}

void HttpPluginLoader::set_network_manager(std::unique_ptr<QNetworkAccessManager> network_manager) {
    m_network_manager = std::move(network_manager);
    m_network_manager->setParent(this);
}

bool HttpPluginLoader::is_http_url(const QUrl& url) {
    QString scheme = url.scheme().toLower();
    return scheme == "http" || scheme == "https";
}

qtplugin::expected<QJsonObject, PluginError> HttpPluginLoader::get_plugin_metadata(
    const QUrl& url, std::chrono::seconds timeout) {
    
    if (!is_http_url(url)) {
        return qtplugin::make_error<QJsonObject>(
            PluginErrorCode::UnsupportedFormat,
            "URL is not HTTP/HTTPS");
    }
    
    // Try to get metadata from a .meta or .json file alongside the plugin
    QUrl metadata_url = url;
    QString path = metadata_url.path();
    
    // Try different metadata file extensions
    QStringList metadata_extensions = {".meta", ".json", ".info"};
    
    for (const QString& ext : metadata_extensions) {
        QUrl test_url = metadata_url;
        test_url.setPath(path + ext);
        
        auto response = make_http_request(test_url, "GET", {}, {}, timeout);
        if (response) {
            return *response;
        }
    }
    
    // If no metadata file found, return basic metadata
    QJsonObject basic_metadata;
    basic_metadata["url"] = url.toString();
    basic_metadata["name"] = QFileInfo(url.path()).baseName();
    basic_metadata["source"] = "http";
    
    return basic_metadata;
}

bool HttpPluginLoader::supports_registry_api(const RemotePluginSource& source) const {
    // Check if source has registry API configuration
    QJsonObject custom_options = source.configuration().custom_options;
    return custom_options.contains("registry_api") && custom_options["registry_api"].toBool();
}

QUrl HttpPluginLoader::get_registry_endpoint(const RemotePluginSource& source, const QString& endpoint) const {
    QUrl base_url = source.url();
    
    // Get API base path from configuration
    QJsonObject custom_options = source.configuration().custom_options;
    QString api_base = custom_options.value("api_base").toString("/api/v1");
    
    // Build endpoint URL
    QString path = api_base + "/" + endpoint;
    base_url.setPath(path);
    
    return base_url;
}

qtplugin::expected<RemotePluginLoadResult, PluginError> HttpPluginLoader::perform_remote_load(
    const RemotePluginSource& source,
    const RemotePluginLoadOptions& options) {

    // Validate source
    auto validation_result = validate_http_source(source, options);
    if (!validation_result) {
        return qtplugin::unexpected(validation_result.error());
    }

    if (validation_result.value().is_failed()) {
        m_remote_loads_failed++;
        return qtplugin::make_error<RemotePluginLoadResult>(
            PluginErrorCode::SecurityViolation,
            validation_result.value().message);
    }

    // Check cache first
    if (options.cache_plugin && m_download_manager->is_cached(source.url())) {
        auto cached_path = m_download_manager->get_cached_path(source.url());
        if (cached_path) {
            auto plugin_result = load_from_cache(*cached_path);
            if (plugin_result) {
                RemotePluginLoadResult result;
                result.plugin = *plugin_result;
                result.source = source;
                result.cached_path = *cached_path;
                result.load_time = std::chrono::system_clock::now();
                result.validation_result = validation_result.value();

                m_cache_hits++;
                m_remote_loads_successful++;
                return result;
            }
        }
    }

    // Download plugin
    auto download_result = download_plugin_file(source, options);
    if (!download_result) {
        m_remote_loads_failed++;
        return qtplugin::unexpected(download_result.error());
    }

    // Load downloaded plugin
    auto plugin_result = load_downloaded_plugin(*download_result, source);
    if (!plugin_result) {
        m_remote_loads_failed++;
        return qtplugin::unexpected(plugin_result.error());
    }

    // Create result
    RemotePluginLoadResult result;
    result.plugin = *plugin_result;
    result.source = source;
    result.download_result = *download_result;
    result.validation_result = validation_result.value();
    result.cached_path = download_result->file_path;
    result.load_time = std::chrono::system_clock::now();

    m_remote_loads_successful++;
    return result;
}

// === Slot Implementations ===

void HttpPluginLoader::on_async_load_progress(const QString& download_id, const DownloadProgress& progress) {
    std::lock_guard<std::mutex> lock(m_async_operations_mutex);

    auto map_it = m_download_to_operation_map.find(download_id);
    if (map_it == m_download_to_operation_map.end()) {
        return;
    }

    auto op_it = m_async_operations.find(map_it->second);
    if (op_it != m_async_operations.end() && op_it->second->progress_callback) {
        op_it->second->progress_callback(progress);
    }
}

void HttpPluginLoader::on_async_load_completed(const QString& download_id, const DownloadResult& result) {
    std::lock_guard<std::mutex> lock(m_async_operations_mutex);

    auto map_it = m_download_to_operation_map.find(download_id);
    if (map_it == m_download_to_operation_map.end()) {
        return;
    }

    QString operation_id = map_it->second;
    auto op_it = m_async_operations.find(operation_id);
    if (op_it == m_async_operations.end()) {
        return;
    }

    // Load the downloaded plugin
    auto plugin_result = load_downloaded_plugin(result, op_it->second->source);
    if (plugin_result) {
        RemotePluginLoadResult load_result;
        load_result.plugin = *plugin_result;
        load_result.source = op_it->second->source;
        load_result.download_result = result;
        load_result.cached_path = result.file_path;
        load_result.load_time = std::chrono::system_clock::now();

        m_remote_loads_successful++;
        complete_async_operation(operation_id, load_result);
    } else {
        m_remote_loads_failed++;
        complete_async_operation(operation_id, qtplugin::unexpected(plugin_result.error()));
    }
}

void HttpPluginLoader::on_async_load_failed(const QString& download_id, const PluginError& error) {
    std::lock_guard<std::mutex> lock(m_async_operations_mutex);

    auto map_it = m_download_to_operation_map.find(download_id);
    if (map_it == m_download_to_operation_map.end()) {
        return;
    }

    QString operation_id = map_it->second;
    m_remote_loads_failed++;
    complete_async_operation(operation_id, qtplugin::unexpected(error));
}

// === Helper Methods ===

RemotePluginSource HttpPluginLoader::create_source_from_url(const QUrl& url) const {
    RemotePluginSource source(url, RemoteSourceType::Http);

    // Apply default configuration if available
    if (m_configuration) {
        RemoteSourceConfig config;
        config.security_level = m_configuration->security_policy().default_security_level;
        config.cache_policy = m_configuration->cache_config().default_cache_policy;
        config.timeout = m_configuration->network_config().connection_timeout;
        config.max_retries = m_configuration->network_config().max_retries;
        config.verify_ssl = m_configuration->network_config().verify_ssl_certificates;

        source.set_configuration(config);
    }

    return source;
}

qtplugin::expected<ValidationResult, PluginError> HttpPluginLoader::validate_http_source(
    const RemotePluginSource& source, const RemotePluginLoadOptions& options) {

    if (!options.validate_source) {
        ValidationResult result;
        result.level = ValidationLevel::Passed;
        result.message = "Source validation skipped";
        result.timestamp = std::chrono::system_clock::now();
        return result;
    }

    if (!m_validator) {
        return qtplugin::make_error<ValidationResult>(
            PluginErrorCode::InvalidConfiguration,
            "No validator available");
    }

    return m_validator->validate_source(source);
}

qtplugin::expected<DownloadResult, PluginError> HttpPluginLoader::download_plugin_file(
    const RemotePluginSource& source, const RemotePluginLoadOptions& options) {

    if (!m_download_manager) {
        return qtplugin::make_error<DownloadResult>(
            PluginErrorCode::InvalidConfiguration,
            "No download manager available");
    }

    return m_download_manager->download_plugin(source, QUrl(), options.download_options);
}

qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> HttpPluginLoader::load_downloaded_plugin(
    const DownloadResult& download_result, const RemotePluginSource& source) {

    // Validate downloaded file if required
    if (m_validator) {
        auto validation_result = m_validator->validate_plugin_file(
            download_result.file_path, source, download_result.checksum);

        if (!validation_result) {
            return qtplugin::unexpected(validation_result.error());
        }

        if (validation_result->is_failed()) {
            return qtplugin::make_error<std::shared_ptr<IPlugin>>(
                PluginErrorCode::SecurityViolation,
                validation_result->message);
        }
    }

    // Load plugin from downloaded file
    return load_from_cache(download_result.file_path);
}

QString HttpPluginLoader::register_async_operation(
    const RemotePluginSource& source,
    const RemotePluginLoadOptions& options,
    std::function<void(const DownloadProgress&)> progress_callback,
    std::function<void(const qtplugin::expected<RemotePluginLoadResult, PluginError>&)> completion_callback) {

    QString operation_id = generate_operation_id();

    auto operation = std::make_unique<AsyncOperation>();
    operation->operation_id = operation_id;
    operation->source = source;
    operation->options = options;
    operation->progress_callback = progress_callback;
    operation->completion_callback = completion_callback;
    operation->start_time = std::chrono::system_clock::now();

    {
        std::lock_guard<std::mutex> lock(m_async_operations_mutex);
        m_async_operations[operation_id] = std::move(operation);
    }

    // Track operation
    QJsonObject info;
    info["type"] = "async_load";
    info["source_url"] = source.url().toString();
    info["start_time"] = QDateTime::fromSecsSinceEpoch(
        std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count()
    ).toString(Qt::ISODate);

    track_operation(operation_id, info);

    return operation_id;
}

void HttpPluginLoader::complete_async_operation(const QString& operation_id,
                                               const qtplugin::expected<RemotePluginLoadResult, PluginError>& result) {
    std::unique_ptr<AsyncOperation> operation;

    {
        std::lock_guard<std::mutex> lock(m_async_operations_mutex);
        auto it = m_async_operations.find(operation_id);
        if (it != m_async_operations.end()) {
            operation = std::move(it->second);
            m_async_operations.erase(it);
        }
    }

    if (operation && operation->completion_callback) {
        operation->completion_callback(result);
    }

    cleanup_async_operation(operation_id);
}

void HttpPluginLoader::cleanup_async_operation(const QString& operation_id) {
    {
        std::lock_guard<std::mutex> lock(m_async_operations_mutex);

        // Remove from download mapping
        for (auto it = m_download_to_operation_map.begin(); it != m_download_to_operation_map.end(); ++it) {
            if (it->second == operation_id) {
                m_download_to_operation_map.erase(it);
                break;
            }
        }
    }

    untrack_operation(operation_id);
}

// === Registry API Helpers ===

qtplugin::expected<QJsonObject, PluginError> HttpPluginLoader::make_registry_request(
    const QUrl& endpoint, const QJsonObject& params, std::chrono::seconds timeout) {

    QJsonObject headers;
    headers["Content-Type"] = "application/json";
    headers["Accept"] = "application/json";

    return make_http_request(endpoint, "POST", params, headers, timeout);
}

qtplugin::expected<QJsonArray, PluginError> HttpPluginLoader::parse_plugin_list_response(const QJsonObject& response) {
    if (is_error_response(response)) {
        return qtplugin::unexpected(extract_error_from_response(response));
    }

    if (response.contains("plugins") && response["plugins"].isArray()) {
        return response["plugins"].toArray();
    } else if (response.contains("data") && response["data"].isArray()) {
        return response["data"].toArray();
    } else if (response.contains("results") && response["results"].isArray()) {
        return response["results"].toArray();
    }

    return qtplugin::make_error<QJsonArray>(
        PluginErrorCode::InvalidFormat,
        "Invalid plugin list response format");
}

qtplugin::expected<QJsonObject, PluginError> HttpPluginLoader::parse_plugin_info_response(const QJsonObject& response) {
    if (is_error_response(response)) {
        return qtplugin::unexpected(extract_error_from_response(response));
    }

    if (response.contains("plugin") && response["plugin"].isObject()) {
        return response["plugin"].toObject();
    } else if (response.contains("data") && response["data"].isObject()) {
        return response["data"].toObject();
    }

    return response;  // Assume the whole response is plugin info
}

// === HTTP Request Helpers ===

qtplugin::expected<QJsonObject, PluginError> HttpPluginLoader::make_http_request(
    const QUrl& url, const QString& method, const QJsonObject& data,
    const QJsonObject& headers, std::chrono::seconds timeout) {

    QNetworkRequest request(url);

    // Apply headers
    apply_custom_headers(request, headers);

    // Set timeout
    request.setTransferTimeout(static_cast<int>(timeout.count() * 1000));

    QNetworkReply* reply = nullptr;

    if (method.toUpper() == "GET") {
        reply = m_network_manager->get(request);
    } else if (method.toUpper() == "POST") {
        QJsonDocument doc(data);
        QByteArray post_data = doc.toJson();
        reply = m_network_manager->post(request, post_data);
    } else {
        return qtplugin::make_error<QJsonObject>(
            PluginErrorCode::UnsupportedFormat,
            "Unsupported HTTP method: " + method.toStdString());
    }

    // Wait for response
    QEventLoop loop;
    QTimer timeout_timer;
    timeout_timer.setSingleShot(true);
    timeout_timer.start(static_cast<int>(timeout.count() * 1000));

    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    connect(&timeout_timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    loop.exec();

    if (timeout_timer.isActive()) {
        timeout_timer.stop();

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response_data = reply->readAll();
            reply->deleteLater();

            return parse_json_response(response_data);
        } else {
            QString error_msg = reply->errorString();
            reply->deleteLater();

            return qtplugin::make_error<QJsonObject>(
                PluginErrorCode::NetworkError,
                "HTTP request failed: " + error_msg.toStdString());
        }
    } else {
        reply->abort();
        reply->deleteLater();

        return qtplugin::make_error<QJsonObject>(
            PluginErrorCode::NetworkError,
            "HTTP request timed out");
    }
}

void HttpPluginLoader::setup_network_request(QNetworkRequest& request, const RemotePluginSource& source) const {
    // Set user agent
    QString user_agent = "QtForge-HttpPluginLoader/3.0.0";
    if (m_configuration) {
        user_agent = m_configuration->network_config().user_agent;
    }
    request.setHeader(QNetworkRequest::UserAgentHeader, user_agent);

    // Apply authentication
    apply_authentication(request, source);

    // Apply custom headers from source configuration
    const auto& custom_headers = source.configuration().custom_headers;
    apply_custom_headers(request, custom_headers);
}

void HttpPluginLoader::apply_authentication(QNetworkRequest& request, const RemotePluginSource& source) const {
    if (!source.has_authentication()) {
        return;
    }

    const auto& auth = source.authentication();

    switch (auth.type) {
        case AuthenticationType::Basic: {
            QString credentials = auth.username + ":" + auth.password;
            QString encoded = credentials.toUtf8().toBase64();
            request.setRawHeader("Authorization", "Basic " + encoded.toUtf8());
            break;
        }
        case AuthenticationType::Bearer:
            request.setRawHeader("Authorization", "Bearer " + auth.token.toUtf8());
            break;
        case AuthenticationType::ApiKey:
            request.setRawHeader("X-API-Key", auth.api_key.toUtf8());
            break;
        default:
            // Other authentication types not implemented for HTTP requests
            break;
    }
}

void HttpPluginLoader::apply_custom_headers(QNetworkRequest& request, const QJsonObject& headers) const {
    for (auto it = headers.begin(); it != headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
    }
}

// === URL Manipulation Helpers ===

QUrl HttpPluginLoader::resolve_plugin_url(const RemotePluginSource& source, const QString& plugin_path) const {
    QUrl base_url = source.url();

    if (plugin_path.startsWith("http://") || plugin_path.startsWith("https://")) {
        return QUrl(plugin_path);
    }

    QString resolved_path = base_url.path();
    if (!resolved_path.endsWith('/') && !plugin_path.startsWith('/')) {
        resolved_path += '/';
    }
    resolved_path += plugin_path;

    base_url.setPath(resolved_path);
    return base_url;
}

QUrl HttpPluginLoader::build_search_url(const RemotePluginSource& source, const QString& query, int max_results) const {
    QUrl search_url = source.url();

    QUrlQuery url_query;
    url_query.addQueryItem("q", query);
    url_query.addQueryItem("query", query);
    url_query.addQueryItem("search", query);
    url_query.addQueryItem("limit", QString::number(max_results));
    url_query.addQueryItem("max", QString::number(max_results));

    search_url.setQuery(url_query);
    return search_url;
}

QUrl HttpPluginLoader::build_discovery_url(const RemotePluginSource& source) const {
    QUrl discovery_url = source.url();

    // Try common discovery endpoints
    QString path = discovery_url.path();
    if (!path.endsWith('/')) {
        path += '/';
    }

    // Common discovery paths
    QStringList discovery_paths = {"plugins", "list", "index", "directory"};

    // For now, just use the base URL - in a real implementation,
    // we might try multiple paths or use configuration
    return discovery_url;
}

// === Response Parsing Helpers ===

qtplugin::expected<QJsonObject, PluginError> HttpPluginLoader::parse_json_response(const QByteArray& data) const {
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);

    if (parse_error.error != QJsonParseError::NoError) {
        return qtplugin::make_error<QJsonObject>(
            PluginErrorCode::InvalidFormat,
            "Failed to parse JSON response: " + parse_error.errorString().toStdString());
    }

    if (!doc.isObject()) {
        return qtplugin::make_error<QJsonObject>(
            PluginErrorCode::InvalidFormat,
            "Response is not a JSON object");
    }

    return doc.object();
}

bool HttpPluginLoader::is_error_response(const QJsonObject& response) const {
    return response.contains("error") ||
           response.contains("errors") ||
           (response.contains("status") && response["status"].toString() == "error");
}

PluginError HttpPluginLoader::extract_error_from_response(const QJsonObject& response) const {
    QString error_message = "Unknown error";

    if (response.contains("error")) {
        QJsonValue error_value = response["error"];
        if (error_value.isString()) {
            error_message = error_value.toString();
        } else if (error_value.isObject()) {
            QJsonObject error_obj = error_value.toObject();
            error_message = error_obj.value("message").toString("Unknown error");
        }
    } else if (response.contains("message")) {
        error_message = response["message"].toString();
    }

    return PluginError{PluginErrorCode::NetworkError, error_message.toStdString()};
}

// === Validation Helpers ===

bool HttpPluginLoader::is_valid_plugin_response(const QJsonObject& response) const {
    return !is_error_response(response) &&
           (response.contains("plugin") || response.contains("data") ||
            response.contains("name"));
}

bool HttpPluginLoader::is_valid_search_response(const QJsonObject& response) const {
    return !is_error_response(response) &&
           (response.contains("results") || response.contains("plugins") ||
            response.contains("data"));
}

bool HttpPluginLoader::is_valid_discovery_response(const QJsonObject& response) const {
    return !is_error_response(response) &&
           (response.contains("plugins") || response.contains("list") ||
            response.contains("data"));
}

// === Factory Implementation ===

std::unique_ptr<HttpPluginLoader> HttpPluginLoaderFactory::create_default(QObject* parent) {
    auto config = std::make_shared<RemotePluginConfiguration>(
        RemotePluginConfiguration::create_default());

    return std::make_unique<HttpPluginLoader>(config, nullptr, nullptr, parent);
}

std::unique_ptr<HttpPluginLoader> HttpPluginLoaderFactory::create_with_config(
    std::shared_ptr<RemotePluginConfiguration> configuration, QObject* parent) {

    return std::make_unique<HttpPluginLoader>(configuration, nullptr, nullptr, parent);
}

std::unique_ptr<HttpPluginLoader> HttpPluginLoaderFactory::create_enterprise(QObject* parent) {
    auto config = std::make_shared<RemotePluginConfiguration>(
        RemotePluginConfiguration::create_enterprise());

    return std::make_unique<HttpPluginLoader>(config, nullptr, nullptr, parent);
}

std::unique_ptr<HttpPluginLoader> HttpPluginLoaderFactory::create_custom(
    std::shared_ptr<RemotePluginConfiguration> configuration,
    std::shared_ptr<PluginDownloadManager> download_manager,
    std::shared_ptr<RemotePluginValidator> validator,
    QObject* parent) {

    return std::make_unique<HttpPluginLoader>(configuration, download_manager, validator, parent);
}

}  // namespace qtplugin

#include "http_plugin_loader.moc"
