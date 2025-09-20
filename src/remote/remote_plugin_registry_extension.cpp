/**
 * @file remote_plugin_registry_extension.cpp
 * @brief Implementation of remote plugin registry extension
 */

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QStandardPaths>
#include <algorithm>
#include <filesystem>
#include <qtplugin/remote/remote_plugin_registry_extension.hpp>
#include <qtplugin/remote/remote_plugin_discovery.hpp>

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
        info.remote_source = RemotePluginSource::from_json(json["remote_source"].toObject());
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
    if (!remote_version) {
        return false;
    }

    // Parse both versions and compare
    auto current_version_opt = Version::parse(metadata.version.to_string());
    auto remote_version_opt = Version::parse(*remote_version);

    if (!current_version_opt || !remote_version_opt) {
        // If we can't parse versions, fall back to string comparison
        return *remote_version != metadata.version.to_string();
    }

    // Compare using semantic versioning
    return *remote_version_opt > *current_version_opt;
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
    result.source = RemotePluginSource::from_json(json["source"].toObject());

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
    auto base_info = std::make_unique<PluginInfo>();
    base_info->id = remote_plugin_info->id;
    base_info->file_path = remote_plugin_info->file_path;
    base_info->metadata = remote_plugin_info->metadata;
    base_info->state = remote_plugin_info->state;
    base_info->load_time = remote_plugin_info->load_time;
    base_info->last_activity = remote_plugin_info->last_activity;
    base_info->hot_reload_enabled = remote_plugin_info->hot_reload_enabled;

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

        if (latest_version_result && latest_version_result.value().has_value()) {
            std::string current_version_str = plugin_info->metadata.version.to_string();
            std::string latest_version_str = *latest_version_result.value();

            // Use our version comparison helper
            if (compare_versions(latest_version_str, current_version_str) > 0) {
                plugins_with_updates.push_back(plugin_id);

                // Note: Would update cached version info and emit signal in non-const version
                // plugin_info->remote_version = latest_version_str;
                // plugin_info->last_update_check = std::chrono::system_clock::now();
                // emit remote_plugin_update_available(plugin_id, latest_version_str);
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

    if (!latest_version_result.value().has_value()) {
        return false;  // No version information available
    }

    std::string current_version_str = it->second->metadata.version.to_string();
    std::string latest_version_str = *latest_version_result.value();

    bool has_update = compare_versions(latest_version_str, current_version_str) > 0;

    if (has_update) {
        // Note: Would update cached information and emit signal in non-const version
        // it->second->remote_version = latest_version_str;
        // it->second->last_update_check = std::chrono::system_clock::now();
        // emit remote_plugin_update_available(plugin_id, latest_version_str);
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
            // Create a new PluginInfo with the base information
            PluginInfo info;
            info.id = remote_info->id;
            info.file_path = remote_info->file_path;
            info.metadata = remote_info->metadata;
            info.state = remote_info->state;
            info.load_time = remote_info->load_time;
            info.last_activity = remote_info->last_activity;
            info.hot_reload_enabled = remote_info->hot_reload_enabled;
            return info;
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
    std::vector<PluginInfo> all_plugins;

    // Copy base plugins manually
    for (const auto& base_plugin : base_plugins) {
        PluginInfo info;
        info.id = base_plugin.id;
        info.file_path = base_plugin.file_path;
        info.metadata = base_plugin.metadata;
        info.state = base_plugin.state;
        info.load_time = base_plugin.load_time;
        info.last_activity = base_plugin.last_activity;
        info.hot_reload_enabled = base_plugin.hot_reload_enabled;
        all_plugins.push_back(std::move(info));
    }

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
            // Create a new PluginInfo with the base information
            PluginInfo info;
            info.id = remote_plugin.id;
            info.file_path = remote_plugin.file_path;
            info.metadata = remote_plugin.metadata;
            info.state = remote_plugin.state;
            info.load_time = remote_plugin.load_time;
            info.last_activity = remote_plugin.last_activity;
            info.hot_reload_enabled = remote_plugin.hot_reload_enabled;
            all_plugins.push_back(std::move(info));
        }
    }

    return all_plugins;
}

// === Helper Methods ===

RemotePluginInfo RemotePluginRegistry::create_remote_plugin_info_copy(
    const RemotePluginInfo& original) const {
    // Create a new RemotePluginInfo by copying fields manually
    RemotePluginInfo copy;

    // Copy base PluginInfo fields
    copy.id = original.id;
    copy.file_path = original.file_path;
    copy.metadata = original.metadata;
    copy.state = original.state;
    copy.load_time = original.load_time;
    copy.last_activity = original.last_activity;
    copy.hot_reload_enabled = original.hot_reload_enabled;

    // Copy remote-specific fields
    copy.remote_source = original.remote_source;
    copy.original_url = original.original_url;
    copy.cached_path = original.cached_path;
    copy.download_time = original.download_time;
    copy.last_update_check = original.last_update_check;
    copy.remote_version = original.remote_version;
    copy.checksum = original.checksum;
    copy.auto_update_enabled = original.auto_update_enabled;
    copy.is_cached = original.is_cached;
    copy.remote_metadata = original.remote_metadata;

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
    Q_UNUSED(source)
    Q_UNUSED(criteria)

    std::vector<RemotePluginDiscoveryResult> results;

    try {
        // For now, return empty results as this would require a full discovery implementation
        // In a complete implementation, this would:
        // 1. Create HTTP requests based on the source type
        // 2. Parse the response to extract plugin information
        // 3. Apply the search criteria filtering

        // Placeholder implementation - return empty results
        return results;

    } catch (const std::exception& e) {
        return qtplugin::make_error<std::vector<RemotePluginDiscoveryResult>>(
            PluginErrorCode::SystemError,
            "Plugin discovery from source failed: " + std::string(e.what()));
    }
}

qtplugin::expected<std::optional<std::string>, PluginError>
RemotePluginRegistry::get_latest_version_from_source(
    const std::string& plugin_id, const RemotePluginSource& source) const {
    Q_UNUSED(plugin_id)
    Q_UNUSED(source)

    try {
        // Placeholder implementation - in a real implementation this would:
        // 1. Use the discovery system to find the plugin
        // 2. Parse version information from the source
        // 3. Return the latest version string

        // For now, return no version information
        return std::optional<std::string>{};

    } catch (const std::exception& e) {
        return qtplugin::make_error<std::optional<std::string>>(
            PluginErrorCode::SystemError,
            "Failed to get latest version from source: " + std::string(e.what()));
    }
}

int RemotePluginRegistry::compare_versions(const std::string& version1, const std::string& version2) const {
    // Simple semantic version comparison
    // Returns: 1 if version1 > version2, -1 if version1 < version2, 0 if equal

    auto parse_version = [](const std::string& version) -> std::vector<int> {
        std::vector<int> parts;
        QString version_str = QString::fromStdString(version);
        QStringList version_parts = version_str.split('.');

        for (const QString& part : version_parts) {
            bool ok;
            int num = part.toInt(&ok);
            if (ok) {
                parts.push_back(num);
            } else {
                parts.push_back(0); // Default for non-numeric parts
            }
        }

        return parts;
    };

    auto v1_parts = parse_version(version1);
    auto v2_parts = parse_version(version2);

    // Pad shorter version with zeros
    size_t max_size = std::max(v1_parts.size(), v2_parts.size());
    v1_parts.resize(max_size, 0);
    v2_parts.resize(max_size, 0);

    // Compare each part
    for (size_t i = 0; i < max_size; ++i) {
        if (v1_parts[i] > v2_parts[i]) {
            return 1;
        } else if (v1_parts[i] < v2_parts[i]) {
            return -1;
        }
    }

    return 0; // Versions are equal
}

}  // namespace qtplugin
