/**
 * @file remote_plugin_registry_extension.cpp
 * @brief Implementation of remote plugin registry extension
 */

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <algorithm>
#include <filesystem>
#include <qtplugin/remote/remote_plugin_registry_extension.hpp>

namespace qtplugin {

// === RemotePluginInfo Implementation ===

QJsonObject RemotePluginInfo::to_json() const {
    QJsonObject json = PluginInfo::to_json();

    // Add remote-specific fields
    if (remote_source) {
        json["remote_source"] = remote_source->to_json();
    }

    if (original_url) {
        json["original_url"] = original_url->toString();
    }

    if (cached_path) {
        json["cached_path"] = QString::fromStdString(cached_path->string());
    }

    json["download_time"] =
        static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(
                                download_time.time_since_epoch())
                                .count());

    json["last_update_check"] =
        static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(
                                last_update_check.time_since_epoch())
                                .count());

    if (remote_version) {
        json["remote_version"] = QString::fromStdString(*remote_version);
    }

    if (checksum) {
        json["checksum"] = QString::fromStdString(*checksum);
    }

    json["auto_update_enabled"] = auto_update_enabled;
    json["is_cached"] = is_cached;
    json["remote_metadata"] = remote_metadata;

    return json;
}

qtplugin::expected<RemotePluginInfo, PluginError> RemotePluginInfo::from_json(
    const QJsonObject& json) {
    RemotePluginInfo info;

    // Parse base PluginInfo fields
    info.id = json["id"].toString().toStdString();
    info.file_path = json["file_path"].toString().toStdString();
    // Note: metadata and other complex fields would need proper parsing

    // Parse remote-specific fields
    if (json.contains("remote_source")) {
        auto source_result =
            RemotePluginSource::from_json(json["remote_source"].toObject());
        if (source_result) {
            info.remote_source = source_result.value();
        }
    }

    if (json.contains("original_url")) {
        info.original_url = QUrl(json["original_url"].toString());
    }

    if (json.contains("cached_path")) {
        info.cached_path = json["cached_path"].toString().toStdString();
    }

    if (json.contains("download_time")) {
        auto timestamp =
            std::chrono::seconds(json["download_time"].toInteger());
        info.download_time = std::chrono::system_clock::time_point(timestamp);
    }

    if (json.contains("last_update_check")) {
        auto timestamp =
            std::chrono::seconds(json["last_update_check"].toInteger());
        info.last_update_check =
            std::chrono::system_clock::time_point(timestamp);
    }

    if (json.contains("remote_version")) {
        info.remote_version = json["remote_version"].toString().toStdString();
    }

    if (json.contains("checksum")) {
        info.checksum = json["checksum"].toString().toStdString();
    }

    info.auto_update_enabled = json["auto_update_enabled"].toBool();
    info.is_cached = json["is_cached"].toBool();
    info.remote_metadata = json["remote_metadata"].toObject();

    return info;
}

bool RemotePluginInfo::needs_update() const {
    if (!remote_version || !metadata.version.is_valid()) {
        return false;
    }

    Version current_version = metadata.version;
    Version latest_version(*remote_version);

    return latest_version > current_version;
}

std::chrono::seconds RemotePluginInfo::cache_age() const {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::seconds>(now -
                                                            download_time);
}

// === RemotePluginDiscoveryResult Implementation ===

QJsonObject RemotePluginDiscoveryResult::to_json() const {
    QJsonObject json;
    json["plugin_id"] = QString::fromStdString(plugin_id);
    json["name"] = QString::fromStdString(name);
    json["version"] = QString::fromStdString(version);
    json["description"] = QString::fromStdString(description);
    json["author"] = QString::fromStdString(author);
    json["category"] = QString::fromStdString(category);

    QJsonArray tags_array;
    for (const auto& tag : tags) {
        tags_array.append(QString::fromStdString(tag));
    }
    json["tags"] = tags_array;

    json["download_url"] = download_url.toString();
    json["source"] = source.to_json();
    json["metadata"] = metadata;

    if (checksum) {
        json["checksum"] = QString::fromStdString(*checksum);
    }

    if (file_size) {
        json["file_size"] = *file_size;
    }

    if (rating) {
        json["rating"] = *rating;
    }

    if (download_count) {
        json["download_count"] = *download_count;
    }

    return json;
}

qtplugin::expected<RemotePluginDiscoveryResult, PluginError>
RemotePluginDiscoveryResult::from_json(const QJsonObject& json) {
    RemotePluginDiscoveryResult result;

    result.plugin_id = json["plugin_id"].toString().toStdString();
    result.name = json["name"].toString().toStdString();
    result.version = json["version"].toString().toStdString();
    result.description = json["description"].toString().toStdString();
    result.author = json["author"].toString().toStdString();
    result.category = json["category"].toString().toStdString();

    // Parse tags
    QJsonArray tags_array = json["tags"].toArray();
    for (const auto& tag_value : tags_array) {
        result.tags.push_back(tag_value.toString().toStdString());
    }

    result.download_url = QUrl(json["download_url"].toString());

    // Parse source
    auto source_result =
        RemotePluginSource::from_json(json["source"].toObject());
    if (!source_result) {
        return qtplugin::unexpected(source_result.error());
    }
    result.source = source_result.value();

    result.metadata = json["metadata"].toObject();

    // Parse optional fields
    if (json.contains("checksum")) {
        result.checksum = json["checksum"].toString().toStdString();
    }

    if (json.contains("file_size")) {
        result.file_size = json["file_size"].toInteger();
    }

    if (json.contains("rating")) {
        result.rating = json["rating"].toDouble();
    }

    if (json.contains("download_count")) {
        result.download_count = json["download_count"].toInt();
    }

    return result;
}

// === RemotePluginSearchCriteria Implementation ===

QJsonObject RemotePluginSearchCriteria::to_json() const {
    QJsonObject json;

    if (query) {
        json["query"] = QString::fromStdString(*query);
    }

    if (category) {
        json["category"] = QString::fromStdString(*category);
    }

    if (!tags.empty()) {
        QJsonArray tags_array;
        for (const auto& tag : tags) {
            tags_array.append(QString::fromStdString(tag));
        }
        json["tags"] = tags_array;
    }

    if (author) {
        json["author"] = QString::fromStdString(*author);
    }

    if (min_rating) {
        json["min_rating"] = *min_rating;
    }

    if (license) {
        json["license"] = QString::fromStdString(*license);
    }

    if (version_range) {
        json["version_range"] = QString::fromStdString(*version_range);
    }

    json["max_results"] = max_results;
    json["offset"] = offset;
    json["sort_by"] = QString::fromStdString(sort_by);
    json["sort_ascending"] = sort_ascending;

    return json;
}

RemotePluginSearchCriteria RemotePluginSearchCriteria::from_json(
    const QJsonObject& json) {
    RemotePluginSearchCriteria criteria;

    if (json.contains("query")) {
        criteria.query = json["query"].toString().toStdString();
    }

    if (json.contains("category")) {
        criteria.category = json["category"].toString().toStdString();
    }

    if (json.contains("tags")) {
        QJsonArray tags_array = json["tags"].toArray();
        for (const auto& tag_value : tags_array) {
            criteria.tags.push_back(tag_value.toString().toStdString());
        }
    }

    if (json.contains("author")) {
        criteria.author = json["author"].toString().toStdString();
    }

    if (json.contains("min_rating")) {
        criteria.min_rating = json["min_rating"].toDouble();
    }

    if (json.contains("license")) {
        criteria.license = json["license"].toString().toStdString();
    }

    if (json.contains("version_range")) {
        criteria.version_range = json["version_range"].toString().toStdString();
    }

    criteria.max_results = json["max_results"].toInt(50);
    criteria.offset = json["offset"].toInt(0);
    criteria.sort_by = json["sort_by"].toString("relevance").toStdString();
    criteria.sort_ascending = json["sort_ascending"].toBool(false);

    return criteria;
}

// === RemotePluginRegistry Implementation ===

RemotePluginRegistry::RemotePluginRegistry(QObject* parent)
    : PluginRegistry(parent) {
    initialize_cache_directory();
}

RemotePluginRegistry::~RemotePluginRegistry() {
    cleanup_expired_cache_entries();
}

qtplugin::expected<void, PluginError>
RemotePluginRegistry::register_remote_plugin(
    const std::string& plugin_id,
    std::unique_ptr<RemotePluginInfo> remote_plugin_info) {
    if (plugin_id.empty()) {
        return qtplugin::make_error<void>(PluginErrorCode::InvalidParameters,
                                          "Plugin ID cannot be empty");
    }

    if (!remote_plugin_info) {
        return qtplugin::make_error<void>(PluginErrorCode::InvalidParameters,
                                          "Remote plugin info cannot be null");
    }

    // First register with base registry
    auto base_info = std::make_unique<PluginInfo>(
        static_cast<const PluginInfo&>(*remote_plugin_info));
    auto base_result =
        PluginRegistry::register_plugin(plugin_id, std::move(base_info));
    if (!base_result) {
        return base_result;
    }

    // Then store remote-specific information
    std::unique_lock lock(m_remote_plugins_mutex);
    m_remote_plugins[plugin_id] = std::move(remote_plugin_info);

    return qtplugin::expected<void, PluginError>{};
}

std::optional<RemotePluginInfo> RemotePluginRegistry::get_remote_plugin_info(
    const std::string& plugin_id) const {
    std::shared_lock lock(m_remote_plugins_mutex);

    auto it = m_remote_plugins.find(plugin_id);
    if (it != m_remote_plugins.end()) {
        return create_remote_plugin_info_copy(*it->second);
    }

    return std::nullopt;
}

std::vector<RemotePluginInfo> RemotePluginRegistry::get_all_remote_plugin_info()
    const {
    std::shared_lock lock(m_remote_plugins_mutex);

    std::vector<RemotePluginInfo> result;
    result.reserve(m_remote_plugins.size());

    for (const auto& [plugin_id, plugin_info] : m_remote_plugins) {
        result.push_back(create_remote_plugin_info_copy(*plugin_info));
    }

    return result;
}

qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
RemotePluginRegistry::discover_remote_plugins(
    const RemotePluginSearchCriteria& criteria) const {
    std::vector<RemotePluginDiscoveryResult> all_results;

    std::shared_lock lock(m_remote_sources_mutex);

    for (const auto& [source_id, source] : m_remote_sources) {
        auto source_results = discover_from_source(source, criteria);
        if (source_results) {
            auto& results = source_results.value();
            all_results.insert(all_results.end(), results.begin(),
                               results.end());
        }
        // Continue with other sources even if one fails
    }

    // Sort and limit results
    if (!criteria.sort_by.empty()) {
        std::sort(all_results.begin(), all_results.end(),
                  [&criteria](const RemotePluginDiscoveryResult& a,
                              const RemotePluginDiscoveryResult& b) {
                      if (criteria.sort_by == "name") {
                          return criteria.sort_ascending ? a.name < b.name
                                                         : a.name > b.name;
                      } else if (criteria.sort_by == "rating" && a.rating &&
                                 b.rating) {
                          return criteria.sort_ascending
                                     ? *a.rating < *b.rating
                                     : *a.rating > *b.rating;
                      } else if (criteria.sort_by == "downloads" &&
                                 a.download_count && b.download_count) {
                          return criteria.sort_ascending
                                     ? *a.download_count < *b.download_count
                                     : *a.download_count > *b.download_count;
                      }
                      // Default to name sorting
                      return criteria.sort_ascending ? a.name < b.name
                                                     : a.name > b.name;
                  });
    }

    // Apply pagination
    if (criteria.offset > 0 &&
        criteria.offset < static_cast<int>(all_results.size())) {
        all_results.erase(all_results.begin(),
                          all_results.begin() + criteria.offset);
    }

    if (criteria.max_results > 0 &&
        static_cast<int>(all_results.size()) > criteria.max_results) {
        all_results.resize(criteria.max_results);
    }

    return all_results;
}

qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
RemotePluginRegistry::search_remote_plugins(const std::string& query,
                                            int max_results) const {
    RemotePluginSearchCriteria criteria;
    criteria.query = query;
    criteria.max_results = max_results;
    criteria.sort_by = "relevance";

    return discover_remote_plugins(criteria);
}

qtplugin::expected<void, PluginError> RemotePluginRegistry::add_remote_source(
    const RemotePluginSource& source) {
    if (source.id().isEmpty()) {
        return qtplugin::make_error<void>(PluginErrorCode::InvalidParameters,
                                          "Remote source ID cannot be empty");
    }

    std::unique_lock lock(m_remote_sources_mutex);

    std::string source_id = source.id().toStdString();
    m_remote_sources[source_id] = source;

    lock.unlock();

    emit remote_source_added(source.id());

    return qtplugin::expected<void, PluginError>{};
}

qtplugin::expected<void, PluginError>
RemotePluginRegistry::remove_remote_source(const std::string& source_id) {
    if (source_id.empty()) {
        return qtplugin::make_error<void>(PluginErrorCode::InvalidParameters,
                                          "Source ID cannot be empty");
    }

    std::unique_lock lock(m_remote_sources_mutex);

    auto it = m_remote_sources.find(source_id);
    if (it == m_remote_sources.end()) {
        return qtplugin::make_error<void>(
            PluginErrorCode::NotFound, "Remote source not found: " + source_id);
    }

    m_remote_sources.erase(it);

    lock.unlock();

    emit remote_source_removed(QString::fromStdString(source_id));

    return qtplugin::expected<void, PluginError>{};
}

std::vector<RemotePluginSource> RemotePluginRegistry::get_remote_sources()
    const {
    std::shared_lock lock(m_remote_sources_mutex);

    std::vector<RemotePluginSource> sources;
    sources.reserve(m_remote_sources.size());

    for (const auto& [source_id, source] : m_remote_sources) {
        sources.push_back(source);
    }

    return sources;
}

int RemotePluginRegistry::clear_remote_cache(int older_than_days) {
    std::unique_lock cache_lock(m_cache_mutex);
    std::unique_lock plugins_lock(m_remote_plugins_mutex);

    int cleared_count = 0;
    auto cutoff_time = std::chrono::system_clock::now() -
                       std::chrono::hours(24 * older_than_days);

    auto it = m_remote_plugins.begin();
    while (it != m_remote_plugins.end()) {
        bool should_clear = false;

        if (older_than_days == 0) {
            // Clear all cached plugins
            should_clear = it->second->is_cached;
        } else {
            // Clear plugins older than specified days
            should_clear = it->second->is_cached &&
                           it->second->download_time < cutoff_time;
        }

        if (should_clear) {
            // Remove cached file if it exists
            if (it->second->cached_path &&
                std::filesystem::exists(*it->second->cached_path)) {
                std::filesystem::remove(*it->second->cached_path);
            }

            // Remove from registry
            it = m_remote_plugins.erase(it);
            cleared_count++;
        } else {
            ++it;
        }
    }

    return cleared_count;
}

QJsonObject RemotePluginRegistry::get_cache_statistics() const {
    std::shared_lock cache_lock(m_cache_mutex);
    std::shared_lock plugins_lock(m_remote_plugins_mutex);

    QJsonObject stats;

    int cached_count = 0;
    int total_count = static_cast<int>(m_remote_plugins.size());
    qint64 total_cache_size = 0;

    for (const auto& [plugin_id, plugin_info] : m_remote_plugins) {
        if (plugin_info->is_cached) {
            cached_count++;

            if (plugin_info->cached_path &&
                std::filesystem::exists(*plugin_info->cached_path)) {
                try {
                    total_cache_size +=
                        std::filesystem::file_size(*plugin_info->cached_path);
                } catch (const std::filesystem::filesystem_error&) {
                    // Ignore errors getting file size
                }
            }
        }
    }

    stats["total_remote_plugins"] = total_count;
    stats["cached_plugins"] = cached_count;
    stats["cache_hit_ratio"] =
        total_count > 0 ? static_cast<double>(cached_count) / total_count : 0.0;
    stats["total_cache_size_bytes"] = total_cache_size;
    stats["cache_directory"] =
        QString::fromStdString(m_cache_directory.string());

    return stats;
}

qtplugin::expected<std::vector<std::string>, PluginError>
RemotePluginRegistry::check_for_updates() const {
    std::vector<std::string> plugins_with_updates;

    std::shared_lock plugins_lock(m_remote_plugins_mutex);
    std::shared_lock sources_lock(m_remote_sources_mutex);

    for (const auto& [plugin_id, plugin_info] : m_remote_plugins) {
        if (!plugin_info->remote_source) {
            continue;
        }

        auto latest_version_result = get_latest_version_from_source(
            plugin_id, *plugin_info->remote_source);

        if (latest_version_result && latest_version_result->has_value()) {
            Version current_version = plugin_info->metadata.version;
            Version latest_version(**latest_version_result);

            if (latest_version > current_version) {
                plugins_with_updates.push_back(plugin_id);

                // Update cached remote version
                plugin_info->remote_version = **latest_version_result;
                plugin_info->last_update_check =
                    std::chrono::system_clock::now();

                emit remote_plugin_update_available(
                    QString::fromStdString(plugin_id),
                    QString::fromStdString(**latest_version_result));
            }
        }
    }

    return plugins_with_updates;
}

qtplugin::expected<bool, PluginError> RemotePluginRegistry::check_plugin_update(
    const std::string& plugin_id) const {
    std::shared_lock plugins_lock(m_remote_plugins_mutex);

    auto it = m_remote_plugins.find(plugin_id);
    if (it == m_remote_plugins.end()) {
        return qtplugin::make_error<bool>(
            PluginErrorCode::NotFound, "Remote plugin not found: " + plugin_id);
    }

    if (!it->second->remote_source) {
        return false;  // No remote source, can't check for updates
    }

    std::shared_lock sources_lock(m_remote_sources_mutex);

    auto latest_version_result =
        get_latest_version_from_source(plugin_id, *it->second->remote_source);

    if (!latest_version_result) {
        return qtplugin::unexpected(latest_version_result.error());
    }

    if (!latest_version_result->has_value()) {
        return false;  // No version information available
    }

    Version current_version = it->second->metadata.version;
    Version latest_version(**latest_version_result);

    bool has_update = latest_version > current_version;

    if (has_update) {
        // Update cached information
        it->second->remote_version = **latest_version_result;
        it->second->last_update_check = std::chrono::system_clock::now();

        emit remote_plugin_update_available(
            QString::fromStdString(plugin_id),
            QString::fromStdString(**latest_version_result));
    }

    return has_update;
}

qtplugin::expected<void, PluginError> RemotePluginRegistry::set_auto_update(
    const std::string& plugin_id, bool enabled) {
    std::unique_lock lock(m_remote_plugins_mutex);

    auto it = m_remote_plugins.find(plugin_id);
    if (it == m_remote_plugins.end()) {
        return qtplugin::make_error<void>(
            PluginErrorCode::NotFound, "Remote plugin not found: " + plugin_id);
    }

    it->second->auto_update_enabled = enabled;

    return qtplugin::expected<void, PluginError>{};
}

std::optional<PluginInfo> RemotePluginRegistry::get_plugin_info(
    const std::string& plugin_id) const {
    // First check if it's a remote plugin
    if (is_remote_plugin(plugin_id)) {
        auto remote_info = get_remote_plugin_info(plugin_id);
        if (remote_info) {
            // Return the base PluginInfo part
            return static_cast<PluginInfo>(*remote_info);
        }
    }

    // Fall back to base registry
    return PluginRegistry::get_plugin_info(plugin_id);
}

std::vector<PluginInfo> RemotePluginRegistry::get_all_plugin_info() const {
    // Get base plugin info
    auto base_plugins = PluginRegistry::get_all_plugin_info();

    // Get remote plugin info
    auto remote_plugins = get_all_remote_plugin_info();

    // Combine results (avoiding duplicates)
    std::vector<PluginInfo> all_plugins = base_plugins;

    for (const auto& remote_plugin : remote_plugins) {
        // Check if this plugin is already in base_plugins
        bool found = false;
        for (const auto& base_plugin : base_plugins) {
            if (base_plugin.id == remote_plugin.id) {
                found = true;
                break;
            }
        }

        if (!found) {
            all_plugins.push_back(static_cast<PluginInfo>(remote_plugin));
        }
    }

    return all_plugins;
}

// === Helper Methods ===

RemotePluginInfo RemotePluginRegistry::create_remote_plugin_info_copy(
    const RemotePluginInfo& original) const {
    RemotePluginInfo copy = original;
    // Deep copy any shared resources if needed
    return copy;
}

bool RemotePluginRegistry::is_remote_plugin(
    const std::string& plugin_id) const {
    std::shared_lock lock(m_remote_plugins_mutex);
    return m_remote_plugins.find(plugin_id) != m_remote_plugins.end();
}

void RemotePluginRegistry::initialize_cache_directory() {
    // Use standard cache location
    QString cache_dir =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    m_cache_directory =
        std::filesystem::path(cache_dir.toStdString()) / "remote_plugins";

    // Create directory if it doesn't exist
    std::filesystem::create_directories(m_cache_directory);
}

void RemotePluginRegistry::cleanup_expired_cache_entries() {
    // This would be called periodically to clean up old cache entries
    // For now, just ensure the cache directory exists
    if (!std::filesystem::exists(m_cache_directory)) {
        std::filesystem::create_directories(m_cache_directory);
    }
}

qtplugin::expected<std::vector<RemotePluginDiscoveryResult>, PluginError>
RemotePluginRegistry::discover_from_source(
    const RemotePluginSource& source,
    const RemotePluginSearchCriteria& criteria) const {
    // This is a placeholder implementation
    // In a real implementation, this would:
    // 1. Connect to the remote source
    // 2. Query for plugins matching the criteria
    // 3. Parse the response and return discovery results

    std::vector<RemotePluginDiscoveryResult> results;

    // For now, return empty results
    // TODO: Implement actual discovery logic based on source type

    return results;
}

qtplugin::expected<std::optional<std::string>, PluginError>
RemotePluginRegistry::get_latest_version_from_source(
    const std::string& plugin_id, const RemotePluginSource& source) const {
    // This is a placeholder implementation
    // In a real implementation, this would:
    // 1. Connect to the remote source
    // 2. Query for the latest version of the specified plugin
    // 3. Return the version string

    // For now, return no version information
    return std::optional<std::string>{};
}

}  // namespace qtplugin
