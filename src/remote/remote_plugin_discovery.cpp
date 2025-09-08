/**
 * @file remote_plugin_discovery.cpp
 * @brief Implementation of remote plugin discovery system
 */

#include <qtplugin/remote/remote_plugin_discovery.hpp>
#include <QJsonArray>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QUrlQuery>
#include <QUuid>
#include <algorithm>

namespace qtplugin {

// === PluginDiscoveryFilter Implementation ===

QJsonObject PluginDiscoveryFilter::to_json() const {
    QJsonObject json;
    
    if (name_pattern) {
        json["name_pattern"] = QString::fromStdString(*name_pattern);
    }
    
    if (category) {
        json["category"] = QString::fromStdString(*category);
    }
    
    if (!required_tags.empty()) {
        QJsonArray tags_array;
        for (const auto& tag : required_tags) {
            tags_array.append(QString::fromStdString(tag));
        }
        json["required_tags"] = tags_array;
    }
    
    if (!excluded_tags.empty()) {
        QJsonArray tags_array;
        for (const auto& tag : excluded_tags) {
            tags_array.append(QString::fromStdString(tag));
        }
        json["excluded_tags"] = tags_array;
    }
    
    if (author_pattern) {
        json["author_pattern"] = QString::fromStdString(*author_pattern);
    }
    
    if (license) {
        json["license"] = QString::fromStdString(*license);
    }
    
    if (min_rating) {
        json["min_rating"] = *min_rating;
    }
    
    if (version_range) {
        json["version_range"] = QString::fromStdString(*version_range);
    }
    
    if (max_size_bytes) {
        json["max_size_bytes"] = *max_size_bytes;
    }
    
    json["verified_only"] = verified_only;
    json["free_only"] = free_only;
    
    return json;
}

PluginDiscoveryFilter PluginDiscoveryFilter::from_json(const QJsonObject& json) {
    PluginDiscoveryFilter filter;
    
    if (json.contains("name_pattern")) {
        filter.name_pattern = json["name_pattern"].toString().toStdString();
    }
    
    if (json.contains("category")) {
        filter.category = json["category"].toString().toStdString();
    }
    
    if (json.contains("required_tags")) {
        QJsonArray tags_array = json["required_tags"].toArray();
        for (const auto& tag_value : tags_array) {
            filter.required_tags.push_back(tag_value.toString().toStdString());
        }
    }
    
    if (json.contains("excluded_tags")) {
        QJsonArray tags_array = json["excluded_tags"].toArray();
        for (const auto& tag_value : tags_array) {
            filter.excluded_tags.push_back(tag_value.toString().toStdString());
        }
    }
    
    if (json.contains("author_pattern")) {
        filter.author_pattern = json["author_pattern"].toString().toStdString();
    }
    
    if (json.contains("license")) {
        filter.license = json["license"].toString().toStdString();
    }
    
    if (json.contains("min_rating")) {
        filter.min_rating = json["min_rating"].toDouble();
    }
    
    if (json.contains("version_range")) {
        filter.version_range = json["version_range"].toString().toStdString();
    }
    
    if (json.contains("max_size_bytes")) {
        filter.max_size_bytes = json["max_size_bytes"].toInteger();
    }
    
    filter.verified_only = json["verified_only"].toBool();
    filter.free_only = json["free_only"].toBool();
    
    return filter;
}

bool PluginDiscoveryFilter::matches(const RemotePluginDiscoveryResult& result) const {
    // Check name pattern
    if (name_pattern) {
        QRegularExpression name_regex(QString::fromStdString(*name_pattern));
        if (!name_regex.match(QString::fromStdString(result.name)).hasMatch()) {
            return false;
        }
    }
    
    // Check category
    if (category && result.category != *category) {
        return false;
    }
    
    // Check required tags
    for (const auto& required_tag : required_tags) {
        if (std::find(result.tags.begin(), result.tags.end(), required_tag) == result.tags.end()) {
            return false;
        }
    }
    
    // Check excluded tags
    for (const auto& excluded_tag : excluded_tags) {
        if (std::find(result.tags.begin(), result.tags.end(), excluded_tag) != result.tags.end()) {
            return false;
        }
    }
    
    // Check author pattern
    if (author_pattern) {
        QRegularExpression author_regex(QString::fromStdString(*author_pattern));
        if (!author_regex.match(QString::fromStdString(result.author)).hasMatch()) {
            return false;
        }
    }
    
    // Check minimum rating
    if (min_rating && result.rating && *result.rating < *min_rating) {
        return false;
    }
    
    // Check file size
    if (max_size_bytes && result.file_size && *result.file_size > *max_size_bytes) {
        return false;
    }
    
    // Check verified only
    if (verified_only) {
        bool is_verified = result.metadata.value("verified").toBool();
        if (!is_verified) {
            return false;
        }
    }
    
    // Check free only
    if (free_only) {
        bool is_free = result.metadata.value("free").toBool(true);  // Default to free
        if (!is_free) {
            return false;
        }
    }
    
    return true;
}

// === DiscoveryProgress Implementation ===

QJsonObject DiscoveryProgress::to_json() const {
    QJsonObject json;
    json["sources_total"] = sources_total;
    json["sources_completed"] = sources_completed;
    json["plugins_found"] = plugins_found;
    json["current_source"] = current_source;
    json["status_message"] = status_message;
    json["progress_percentage"] = progress_percentage;
    return json;
}

// === DiscoveryResult Implementation ===

QJsonObject DiscoveryResult::to_json() const {
    QJsonObject json;
    
    QJsonArray plugins_array;
    for (const auto& plugin : plugins) {
        plugins_array.append(plugin.to_json());
    }
    json["plugins"] = plugins_array;
    
    QJsonArray failed_sources_array;
    for (const auto& source : failed_sources) {
        failed_sources_array.append(source);
    }
    json["failed_sources"] = failed_sources_array;
    
    QJsonArray errors_array;
    for (const auto& error : error_messages) {
        errors_array.append(error);
    }
    json["error_messages"] = errors_array;
    
    json["total_time_ms"] = static_cast<qint64>(total_time.count());
    json["total_sources_queried"] = total_sources_queried;
    json["success_rate"] = success_rate();
    
    return json;
}

// === HttpDiscoveryEngine Implementation ===

HttpDiscoveryEngine::HttpDiscoveryEngine(QObject* parent) 
    : QObject(parent), m_network_manager(new QNetworkAccessManager(this)) {
}

HttpDiscoveryEngine::~HttpDiscoveryEngine() {
    // Cancel all active operations
    for (auto& [operation_id, operation] : m_active_operations) {
        if (operation->reply) {
            operation->reply->abort();
        }
        if (operation->timeout_timer) {
            operation->timeout_timer->stop();
        }
    }
}

qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
HttpDiscoveryEngine::discover_from_source(const RemotePluginSource& source, 
                                         const PluginDiscoveryFilter& filter) {
    
    if (!supports_source(source)) {
        return qtplugin::make_error<std::vector<RemotePluginDiscoveryResult>>(
            PluginErrorCode::NotSupported,
            "HTTP discovery engine does not support source type");
    }

    // Determine discovery method based on source type
    if (source.type() == RemoteSourceType::Registry) {
        return discover_from_registry_api(source, filter);
    } else if (source.type() == RemoteSourceType::Http) {
        return discover_from_direct_url(source, filter);
    }

    return qtplugin::make_error<std::vector<RemotePluginDiscoveryResult>>(
        PluginErrorCode::NotSupported,
        "Unsupported HTTP source type");
}

QString HttpDiscoveryEngine::discover_from_source_async(
    const RemotePluginSource& source,
    const PluginDiscoveryFilter& filter,
    DiscoveryProgressCallback progress_callback,
    DiscoveryCompletionCallback completion_callback) {
    
    if (!supports_source(source)) {
        if (completion_callback) {
            PluginError error{PluginErrorCode::NotSupported,
                            "HTTP discovery engine does not support source type"};
            completion_callback(qtplugin::unexpected(error));
        }
        return QString();
    }
    
    QString operation_id = generate_operation_id();
    
    auto operation = std::make_unique<DiscoveryOperation>();
    operation->operation_id = operation_id;
    operation->source = source;
    operation->filter = filter;
    operation->progress_callback = progress_callback;
    operation->completion_callback = completion_callback;
    operation->start_time = std::chrono::steady_clock::now();
    
    // Create network request
    QNetworkRequest request = create_discovery_request(source, filter);
    
    // Start the request
    operation->reply = m_network_manager->get(request);
    connect(operation->reply, &QNetworkReply::finished, 
            this, &HttpDiscoveryEngine::on_network_reply_finished);
    
    // Set up timeout timer
    operation->timeout_timer = new QTimer(this);
    operation->timeout_timer->setSingleShot(true);
    operation->timeout_timer->setInterval(static_cast<int>(m_timeout.count() * 1000));
    connect(operation->timeout_timer, &QTimer::timeout, 
            this, &HttpDiscoveryEngine::on_discovery_timeout);
    operation->timeout_timer->start();
    
    // Store operation
    m_active_operations[operation_id] = std::move(operation);
    
    // Report initial progress
    if (progress_callback) {
        DiscoveryProgress progress;
        progress.sources_total = 1;
        progress.sources_completed = 0;
        progress.plugins_found = 0;
        progress.current_source = source.id();
        progress.status_message = "Starting discovery from " + source.id();
        progress.progress_percentage = 0.0;
        progress_callback(progress);
    }
    
    return operation_id;
}

bool HttpDiscoveryEngine::supports_source(const RemotePluginSource& source) const {
    RemoteSourceType type = source.type();
    return type == RemoteSourceType::Http || type == RemoteSourceType::Registry;
}

std::vector<QString> HttpDiscoveryEngine::supported_source_types() const {
    return {"http", "https", "registry"};
}

void HttpDiscoveryEngine::on_network_reply_finished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    // Find the operation for this reply
    QString operation_id;
    for (const auto& [id, operation] : m_active_operations) {
        if (operation->reply == reply) {
            operation_id = id;
            break;
        }
    }

    if (operation_id.isEmpty()) {
        reply->deleteLater();
        return;
    }

    auto operation_it = m_active_operations.find(operation_id);
    if (operation_it == m_active_operations.end()) {
        reply->deleteLater();
        return;
    }

    auto& operation = operation_it->second;

    // Stop timeout timer
    if (operation->timeout_timer) {
        operation->timeout_timer->stop();
        operation->timeout_timer->deleteLater();
        operation->timeout_timer = nullptr;
    }

    // Process the response
    qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError> result =
        qtplugin::make_error<std::vector<RemotePluginDiscoveryResult>>(
            PluginErrorCode::NetworkError, "Unknown error");

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response_data = reply->readAll();
        result = parse_discovery_response(response_data, operation->source);

        // Report progress
        if (operation->progress_callback && result) {
            DiscoveryProgress progress;
            progress.sources_total = 1;
            progress.sources_completed = 1;
            progress.plugins_found = static_cast<int>(result.value().size());
            progress.current_source = operation->source.id();
            progress.status_message = QString("Found %1 plugins").arg(result.value().size());
            progress.progress_percentage = 100.0;
            operation->progress_callback(progress);
        }
    } else {
        result = qtplugin::make_error<std::vector<RemotePluginDiscoveryResult>>(
            PluginErrorCode::NetworkError,
            "Network error: " + reply->errorString().toStdString());
    }

    // Call completion callback
    if (operation->completion_callback) {
        operation->completion_callback(result);
    }

    // Clean up
    reply->deleteLater();
    cleanup_operation(operation_id);
}

void HttpDiscoveryEngine::on_discovery_timeout() {
    QTimer* timer = qobject_cast<QTimer*>(sender());
    if (!timer) {
        return;
    }

    // Find the operation for this timer
    QString operation_id;
    for (const auto& [id, operation] : m_active_operations) {
        if (operation->timeout_timer == timer) {
            operation_id = id;
            break;
        }
    }

    if (operation_id.isEmpty()) {
        return;
    }

    auto operation_it = m_active_operations.find(operation_id);
    if (operation_it == m_active_operations.end()) {
        return;
    }

    auto& operation = operation_it->second;

    // Abort the network request
    if (operation->reply) {
        operation->reply->abort();
    }

    // Call completion callback with timeout error
    if (operation->completion_callback) {
        PluginError error{PluginErrorCode::TimeoutError,
                        "Discovery operation timed out for source: " + operation->source.id().toStdString()};
        operation->completion_callback(qtplugin::unexpected(error));
    }

    // Clean up
    cleanup_operation(operation_id);
}

QString HttpDiscoveryEngine::generate_operation_id() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QNetworkRequest HttpDiscoveryEngine::create_discovery_request(const RemotePluginSource& source,
                                                            const PluginDiscoveryFilter& filter) const {
    QNetworkRequest request;

    // Set URL based on source type
    if (source.type() == RemoteSourceType::Registry) {
        // For registry APIs, construct search URL
        QUrl url = source.url();
        QUrlQuery query;

        if (filter.name_pattern) {
            query.addQueryItem("q", QString::fromStdString(*filter.name_pattern));
        }

        if (filter.category) {
            query.addQueryItem("category", QString::fromStdString(*filter.category));
        }

        if (!filter.required_tags.empty()) {
            QString tags = QString::fromStdString(filter.required_tags[0]);
            for (size_t i = 1; i < filter.required_tags.size(); ++i) {
                tags += "," + QString::fromStdString(filter.required_tags[i]);
            }
            query.addQueryItem("tags", tags);
        }

        if (filter.min_rating) {
            query.addQueryItem("min_rating", QString::number(*filter.min_rating));
        }

        url.setQuery(query);
        request.setUrl(url);
    } else {
        // For direct HTTP sources, use the URL as-is
        request.setUrl(source.url());
    }

    // Set headers
    request.setHeader(QNetworkRequest::UserAgentHeader, m_user_agent);
    request.setRawHeader("Accept", "application/json");

    // Add authentication if configured
    auto auth = source.authentication();
    if (auth.type == AuthenticationType::ApiKey) {
        if (!auth.api_key_header.isEmpty()) {
            request.setRawHeader(auth.api_key_header.toUtf8(), auth.api_key.toUtf8());
        } else {
            request.setRawHeader("Authorization", ("Bearer " + auth.api_key).toUtf8());
        }
    } else if (auth.type == AuthenticationType::Basic) {
        QString credentials = auth.username + ":" + auth.password;
        QByteArray encoded = credentials.toUtf8().toBase64();
        request.setRawHeader("Authorization", "Basic " + encoded);
    }

    // TODO: Add custom headers support when available in RemotePluginSource interface

    return request;
}

qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
HttpDiscoveryEngine::parse_discovery_response(const QByteArray& response, const RemotePluginSource& source) const {

    std::vector<RemotePluginDiscoveryResult> results;

    // Parse JSON response
    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(response, &parse_error);

    if (parse_error.error != QJsonParseError::NoError) {
        return qtplugin::make_error<std::vector<RemotePluginDiscoveryResult>>(
            PluginErrorCode::InvalidFormat,
            "Failed to parse JSON response: " + parse_error.errorString().toStdString());
    }

    QJsonObject root = doc.object();

    // Handle different response formats
    QJsonArray plugins_array;

    if (root.contains("plugins")) {
        plugins_array = root["plugins"].toArray();
    } else if (root.contains("results")) {
        plugins_array = root["results"].toArray();
    } else if (root.contains("data")) {
        plugins_array = root["data"].toArray();
    } else if (doc.isArray()) {
        plugins_array = doc.array();
    } else {
        // Single plugin object
        plugins_array.append(root);
    }

    // Parse each plugin
    for (const auto& plugin_value : plugins_array) {
        QJsonObject plugin_obj = plugin_value.toObject();

        // Parse download URL first
        QString download_url_str = plugin_obj["download_url"].toString();
        if (download_url_str.isEmpty()) {
            download_url_str = plugin_obj["url"].toString();
        }
        QUrl download_url(download_url_str);

        // Create result with initialized source
        RemotePluginDiscoveryResult result;
        result.source = source;  // Copy the source
        result.plugin_id = plugin_obj["id"].toString().toStdString();
        result.name = plugin_obj["name"].toString().toStdString();
        result.version = plugin_obj["version"].toString().toStdString();
        result.description = plugin_obj["description"].toString().toStdString();
        result.author = plugin_obj["author"].toString().toStdString();
        result.category = plugin_obj["category"].toString().toStdString();

        // Parse tags
        QJsonArray tags_array = plugin_obj["tags"].toArray();
        for (const auto& tag_value : tags_array) {
            result.tags.push_back(tag_value.toString().toStdString());
        }

        result.download_url = download_url;
        result.metadata = plugin_obj;

        // Parse optional fields
        if (plugin_obj.contains("checksum")) {
            result.checksum = plugin_obj["checksum"].toString().toStdString();
        }

        if (plugin_obj.contains("size")) {
            result.file_size = plugin_obj["size"].toInteger();
        }

        if (plugin_obj.contains("rating")) {
            result.rating = plugin_obj["rating"].toDouble();
        }

        if (plugin_obj.contains("downloads")) {
            result.download_count = plugin_obj["downloads"].toInt();
        }

        results.push_back(result);
    }

    return results;
}

qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
HttpDiscoveryEngine::discover_from_registry_api(const RemotePluginSource& /* source */,
                                               const PluginDiscoveryFilter& /* filter */) const {

    // This is a placeholder implementation
    // In a real implementation, this would make HTTP requests to registry APIs

    std::vector<RemotePluginDiscoveryResult> results;

    // TODO: Implement actual registry API discovery
    // This would involve:
    // 1. Constructing the appropriate API endpoint URL
    // 2. Making HTTP requests with proper authentication
    // 3. Parsing the registry-specific response format
    // 4. Applying filters to the results

    return results;
}

qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
HttpDiscoveryEngine::discover_from_direct_url(const RemotePluginSource& /* source */,
                                             const PluginDiscoveryFilter& /* filter */) const {

    // This is a placeholder implementation
    // In a real implementation, this would handle direct URL discovery

    std::vector<RemotePluginDiscoveryResult> results;

    // TODO: Implement direct URL discovery
    // This would involve:
    // 1. Fetching the content from the URL
    // 2. Parsing plugin metadata (could be JSON, XML, or HTML)
    // 3. Extracting plugin information
    // 4. Creating discovery results

    return results;
}

void HttpDiscoveryEngine::cleanup_operation(const QString& operation_id) {
    auto it = m_active_operations.find(operation_id);
    if (it != m_active_operations.end()) {
        m_active_operations.erase(it);
    }
}
