/**
 * @file plugin_registry.cpp
 * @brief Implementation of plugin registry
 * @version 3.0.0
 */

#include "../../include/qtplugin/core/plugin_registry.hpp"
#include <QLoggingCategory>
#include <algorithm>
#include "../../include/qtplugin/core/plugin_manager.hpp"

Q_LOGGING_CATEGORY(pluginRegistryLog, "qtplugin.registry")

namespace qtplugin {

PluginRegistry::PluginRegistry(QObject* parent) : QObject(parent) {
    qCDebug(pluginRegistryLog) << "Plugin registry initialized";
}

PluginRegistry::~PluginRegistry() {
    clear();
    qCDebug(pluginRegistryLog) << "Plugin registry destroyed";
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

    std::unique_lock lock(m_plugins_mutex);

    // Check if plugin is already registered
    if (m_plugins.find(plugin_id) != m_plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                                "Plugin already registered: " + plugin_id);
    }

    // Store plugin info
    m_plugins[plugin_id] = std::move(plugin_info);

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

    std::unique_lock lock(m_plugins_mutex);

    auto it = m_plugins.find(plugin_id);
    if (it == m_plugins.end()) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                                "Plugin not found: " + plugin_id);
    }

    m_plugins.erase(it);

    lock.unlock();

    qCDebug(pluginRegistryLog)
        << "Plugin unregistered:" << QString::fromStdString(plugin_id);
    emit plugin_unregistered(QString::fromStdString(plugin_id));

    return make_success();
}

std::shared_ptr<IPlugin> PluginRegistry::get_plugin(
    const std::string& plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);

    auto it = m_plugins.find(plugin_id);
    if (it != m_plugins.end() && it->second) {
        return it->second->instance;
    }

    return nullptr;
}

std::optional<PluginInfo> PluginRegistry::get_plugin_info(
    const std::string& plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);

    auto it = m_plugins.find(plugin_id);
    if (it != m_plugins.end() && it->second) {
        return create_plugin_info_copy(*it->second);
    }

    return std::nullopt;
}

std::vector<std::string> PluginRegistry::get_all_plugin_ids() const {
    std::shared_lock lock(m_plugins_mutex);

    std::vector<std::string> plugin_ids;
    plugin_ids.reserve(m_plugins.size());

    for (const auto& [id, info] : m_plugins) {
        plugin_ids.push_back(id);
    }

    return plugin_ids;
}

std::vector<PluginInfo> PluginRegistry::get_all_plugin_info() const {
    std::shared_lock lock(m_plugins_mutex);

    std::vector<PluginInfo> plugin_infos;
    plugin_infos.reserve(m_plugins.size());

    for (const auto& [id, info] : m_plugins) {
        if (info) {
            plugin_infos.push_back(create_plugin_info_copy(*info));
        }
    }

    return plugin_infos;
}

bool PluginRegistry::is_plugin_registered(const std::string& plugin_id) const {
    std::shared_lock lock(m_plugins_mutex);
    return m_plugins.find(plugin_id) != m_plugins.end();
}

size_t PluginRegistry::plugin_count() const {
    std::shared_lock lock(m_plugins_mutex);
    return m_plugins.size();
}

void PluginRegistry::clear() {
    std::unique_lock lock(m_plugins_mutex);

    size_t count = m_plugins.size();
    m_plugins.clear();

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

    std::unique_lock lock(m_plugins_mutex);

    auto it = m_plugins.find(plugin_id);
    if (it == m_plugins.end()) {
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

PluginInfo PluginRegistry::create_plugin_info_copy(
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

}  // namespace qtplugin
