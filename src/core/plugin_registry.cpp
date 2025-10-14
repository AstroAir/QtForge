/**
 * @file plugin_registry.cpp
 * @brief Implementation of plugin registry
 * @version 3.0.0
 */

#include "../../include/qtplugin/core/plugin_registry.hpp"
#include <QLoggingCategory>
#include <algorithm>
#include <shared_mutex>
#include <unordered_map>
#include "../../include/qtplugin/core/plugin_manager.hpp"

Q_LOGGING_CATEGORY(pluginRegistryLog, "qtplugin.registry")

namespace qtplugin {

/**
 * @brief Private implementation class for PluginRegistry
 *
 * Contains all private members and implementation details to reduce
 * compilation dependencies and improve encapsulation.
 */
class PluginRegistry::Impl {
public:
    // Thread-safe storage for plugin information
    mutable std::shared_mutex plugins_mutex;
    std::unordered_map<std::string, std::unique_ptr<PluginInfo>> plugins;

    // Helper methods
    PluginInfo create_plugin_info_copy(const PluginInfo& original) const;
};

PluginInfo PluginRegistry::Impl::create_plugin_info_copy(
    const PluginInfo& original) const {
    PluginInfo copy;
    copy.id = original.id;
    copy.file_path = original.file_path;
    copy.metadata = original.metadata;
    copy.state = original.state;
    copy.load_time = original.load_time;
    copy.last_activity = original.last_activity;
    copy.instance = original.instance;
    // Skip loader (unique_ptr)
    copy.configuration = original.configuration;
    copy.error_log = original.error_log;
    copy.metrics = original.metrics;
    copy.hot_reload_enabled = original.hot_reload_enabled;

    return copy;
}

PluginRegistry::PluginRegistry(QObject* parent)
    : QObject(parent), d(std::make_unique<Impl>()) {
    qCDebug(pluginRegistryLog) << "Plugin registry initialized";
}

PluginRegistry::~PluginRegistry() {
    clear();
    qCDebug(pluginRegistryLog) << "Plugin registry destroyed";
}

PluginRegistry::PluginRegistry(const PluginRegistry& other)
    : QObject(other.parent()), d(std::make_unique<Impl>()) {
    std::shared_lock lock(other.d->plugins_mutex);
    // Deep copy all plugins (excluding unique_ptr members)
    for (const auto& [id, info] : other.d->plugins) {
        if (info) {
            auto new_info = std::make_unique<PluginInfo>();
            new_info->id = info->id;
            new_info->file_path = info->file_path;
            new_info->metadata = info->metadata;
            new_info->state = info->state;
            new_info->load_time = info->load_time;
            new_info->last_activity = info->last_activity;
            new_info->instance = info->instance;
            // Skip loader (unique_ptr) - cannot be copied
            new_info->configuration = info->configuration;
            new_info->error_log = info->error_log;
            new_info->metrics = info->metrics;
            new_info->hot_reload_enabled = info->hot_reload_enabled;

            d->plugins[id] = std::move(new_info);
        }
    }
}

PluginRegistry& PluginRegistry::operator=(const PluginRegistry& other) {
    if (this != &other) {
        std::unique_lock this_lock(d->plugins_mutex);
        std::shared_lock other_lock(other.d->plugins_mutex);

        // Clear current plugins
        d->plugins.clear();

        // Deep copy all plugins from other (excluding unique_ptr members)
        for (const auto& [id, info] : other.d->plugins) {
            if (info) {
                auto new_info = std::make_unique<PluginInfo>();
                new_info->id = info->id;
                new_info->file_path = info->file_path;
                new_info->metadata = info->metadata;
                new_info->state = info->state;
                new_info->load_time = info->load_time;
                new_info->last_activity = info->last_activity;
                new_info->instance = info->instance;
                // Skip loader (unique_ptr) - cannot be copied
                new_info->configuration = info->configuration;
                new_info->error_log = info->error_log;
                new_info->metrics = info->metrics;
                new_info->hot_reload_enabled = info->hot_reload_enabled;

                d->plugins[id] = std::move(new_info);
            }
        }
    }
    return *this;
}

PluginRegistry::PluginRegistry(PluginRegistry&& other) noexcept
    : QObject(other.parent()), d(std::move(other.d)) {
    // other.d is now nullptr, which is fine for moved-from object
}

PluginRegistry& PluginRegistry::operator=(PluginRegistry&& other) noexcept {
    if (this != &other) {
        d = std::move(other.d);
        // other.d is now nullptr, which is fine for moved-from object
    }
    return *this;
}

qtplugin::expected<void, PluginError> PluginRegistry::register_plugin(
    const std::string& plugin_id, std::unique_ptr<PluginInfo> plugin_info) {
    if (plugin_id.empty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Plugin ID cannot be empty");
    }

    if (!plugin_info) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Plugin info cannot be null");
    }

    std::unique_lock lock(d->plugins_mutex);

    // Check if plugin is already registered
    if (d->plugins.find(plugin_id) != d->plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                                "Plugin already registered: " + plugin_id);
    }

    // Store plugin info
    d->plugins[plugin_id] = std::move(plugin_info);

    lock.unlock();

    qCDebug(pluginRegistryLog)
        << "Plugin registered:" << QString::fromStdString(plugin_id);
    emit plugin_registered(QString::fromStdString(plugin_id));

    return make_success();
}

qtplugin::expected<void, PluginError> PluginRegistry::unregister_plugin(
    const std::string& plugin_id) {
    if (plugin_id.empty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Plugin ID cannot be empty");
    }

    std::unique_lock lock(d->plugins_mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                                "Plugin not found: " + plugin_id);
    }

    d->plugins.erase(it);

    lock.unlock();

    qCDebug(pluginRegistryLog)
        << "Plugin unregistered:" << QString::fromStdString(plugin_id);
    emit plugin_unregistered(QString::fromStdString(plugin_id));

    return make_success();
}

std::shared_ptr<IPlugin> PluginRegistry::get_plugin(
    const std::string& plugin_id) const {
    std::shared_lock lock(d->plugins_mutex);

    auto it = d->plugins.find(plugin_id);
    if (it != d->plugins.end() && it->second) {
        return it->second->instance;
    }

    return nullptr;
}

std::optional<PluginInfo> PluginRegistry::get_plugin_info(
    const std::string& plugin_id) const {
    std::shared_lock lock(d->plugins_mutex);

    auto it = d->plugins.find(plugin_id);
    if (it != d->plugins.end() && it->second) {
        return d->create_plugin_info_copy(*it->second);
    }

    return std::nullopt;
}

std::vector<std::string> PluginRegistry::get_all_plugin_ids() const {
    std::shared_lock lock(d->plugins_mutex);

    std::vector<std::string> plugin_ids;
    plugin_ids.reserve(d->plugins.size());

    for (const auto& [id, info] : d->plugins) {
        plugin_ids.push_back(id);
    }

    return plugin_ids;
}

std::vector<PluginInfo> PluginRegistry::get_all_plugin_info() const {
    std::shared_lock lock(d->plugins_mutex);

    std::vector<PluginInfo> plugin_infos;
    plugin_infos.reserve(d->plugins.size());

    for (const auto& [id, info] : d->plugins) {
        if (info) {
            plugin_infos.push_back(d->create_plugin_info_copy(*info));
        }
    }

    return plugin_infos;
}

bool PluginRegistry::is_plugin_registered(const std::string& plugin_id) const {
    std::shared_lock lock(d->plugins_mutex);
    return d->plugins.find(plugin_id) != d->plugins.end();
}

size_t PluginRegistry::plugin_count() const {
    std::shared_lock lock(d->plugins_mutex);
    return d->plugins.size();
}

void PluginRegistry::clear() {
    std::unique_lock lock(d->plugins_mutex);

    size_t count = d->plugins.size();
    d->plugins.clear();

    lock.unlock();

    qCDebug(pluginRegistryLog)
        << "Registry cleared," << count << "plugins removed";
}

qtplugin::expected<void, PluginError> PluginRegistry::update_plugin_info(
    const std::string& plugin_id, const PluginInfo& plugin_info) {
    if (plugin_id.empty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Plugin ID cannot be empty");
    }

    std::unique_lock lock(d->plugins_mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                                "Plugin not found: " + plugin_id);
    }

    // Update the plugin info (excluding the instance pointer)
    if (it->second) {
        it->second->metadata = plugin_info.metadata;
        it->second->state = plugin_info.state;
        it->second->last_activity = plugin_info.last_activity;
        it->second->configuration = plugin_info.configuration;
        it->second->error_log = plugin_info.error_log;
        it->second->metrics = plugin_info.metrics;
        it->second->hot_reload_enabled = plugin_info.hot_reload_enabled;
    }

    lock.unlock();

    qCDebug(pluginRegistryLog)
        << "Plugin info updated:" << QString::fromStdString(plugin_id);
    emit plugin_info_updated(QString::fromStdString(plugin_id));

    return make_success();
}

}  // namespace qtplugin
