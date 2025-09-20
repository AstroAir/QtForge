/**
 * @file plugin_hot_reload_manager.cpp
 * @brief Implementation of plugin hot reload manager
 * @version 3.0.0
 */

#include "qtplugin/monitoring/plugin_hot_reload_manager.hpp"

#include <QDebug>
#include <QLoggingCategory>
#include <QMutexLocker>

#include <future>

Q_LOGGING_CATEGORY(hotReloadLog, "qtplugin.hotreload")

namespace qtplugin {

PluginHotReloadManager::PluginHotReloadManager(QObject* parent)
    : QObject(parent),
      m_file_watcher(std::make_unique<QFileSystemWatcher>(this)) {
    // Connect file watcher signals
    connect(m_file_watcher.get(), &QFileSystemWatcher::fileChanged, this,
            &PluginHotReloadManager::on_file_changed);

    qCDebug(hotReloadLog) << "Plugin hot reload manager initialized";
}

PluginHotReloadManager::~PluginHotReloadManager() {
    clear();
    qCDebug(hotReloadLog) << "Plugin hot reload manager destroyed";
}

qtplugin::expected<void, PluginError> PluginHotReloadManager::enable_hot_reload(
    const std::string& plugin_id, const std::filesystem::path& file_path) {
    if (plugin_id.empty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Plugin ID cannot be empty");
    }

    if (file_path.empty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Plugin file path cannot be empty");
    }

    if (!std::filesystem::exists(file_path)) {
        return make_error<void>(
            PluginErrorCode::FileNotFound,
            "Plugin file does not exist: " + file_path.string());
    }

    if (!std::filesystem::is_regular_file(file_path)) {
        return make_error<void>(
            PluginErrorCode::InvalidParameters,
            "Path is not a regular file: " + file_path.string());
    }

    // Check if already watching this plugin
    {
        QMutexLocker locker(&m_watched_files_mutex);
        if (m_watched_files.find(plugin_id) != m_watched_files.end()) {
            qCDebug(hotReloadLog) << "Hot reload already enabled for plugin:"
                                  << QString::fromStdString(plugin_id);
            return make_success();
        }
    }

    // Add file to watcher
    QString qt_path = QString::fromStdString(file_path.string());
    if (!m_file_watcher->addPath(qt_path)) {
        return make_error<void>(PluginErrorCode::LoadFailed,
                                "Failed to watch file: " + file_path.string());
    }

    // Store the mapping
    {
        QMutexLocker locker(&m_watched_files_mutex);
        m_watched_files[plugin_id] = file_path;
    }

    qCDebug(hotReloadLog) << "Hot reload enabled for plugin:"
                          << QString::fromStdString(plugin_id)
                          << "watching file:" << qt_path;

    emit hot_reload_enabled(QString::fromStdString(plugin_id));

    return make_success();
}

void PluginHotReloadManager::disable_hot_reload(const std::string& plugin_id) {
    QMutexLocker locker(&m_watched_files_mutex);

    auto it = m_watched_files.find(plugin_id);
    if (it == m_watched_files.end()) {
        qCDebug(hotReloadLog) << "Hot reload not enabled for plugin:"
                              << QString::fromStdString(plugin_id);
        return;
    }

    // Remove file from watcher
    QString qt_path = QString::fromStdString(it->second.string());
    m_file_watcher->removePath(qt_path);

    // Remove from our mapping
    m_watched_files.erase(it);

    qCDebug(hotReloadLog) << "Hot reload disabled for plugin:"
                          << QString::fromStdString(plugin_id);

    emit hot_reload_disabled(QString::fromStdString(plugin_id));
}

bool PluginHotReloadManager::is_hot_reload_enabled(
    const std::string& plugin_id) const {
    QMutexLocker locker(&m_watched_files_mutex);
    return m_watched_files.find(plugin_id) != m_watched_files.end();
}

void PluginHotReloadManager::set_reload_callback(
    std::function<void(const std::string&)> callback) {
    if (!callback) {
        qCDebug(hotReloadLog) << "Reload callback cleared";
    } else {
        qCDebug(hotReloadLog) << "Reload callback set";
    }
    m_reload_callback = std::move(callback);
}

std::vector<std::string> PluginHotReloadManager::get_hot_reload_plugins()
    const {
    QMutexLocker locker(&m_watched_files_mutex);

    std::vector<std::string> plugin_ids;
    plugin_ids.reserve(m_watched_files.size());

    for (const auto& [plugin_id, file_path] : m_watched_files) {
        plugin_ids.push_back(plugin_id);
    }

    return plugin_ids;
}

void PluginHotReloadManager::clear() {
    QMutexLocker locker(&m_watched_files_mutex);

    if (!m_watched_files.empty()) {
        // Remove all paths from watcher
        QStringList paths;
        for (const auto& [plugin_id, file_path] : m_watched_files) {
            paths.append(QString::fromStdString(file_path.string()));
        }

        if (!paths.isEmpty()) {
            m_file_watcher->removePaths(paths);
        }

        size_t count = m_watched_files.size();
        m_watched_files.clear();

        qCDebug(hotReloadLog)
            << "Hot reload cleared," << count << "watchers removed";
    }
}

void PluginHotReloadManager::set_global_hot_reload_enabled(bool enabled) {
    m_global_enabled = enabled;
    qCDebug(hotReloadLog) << "Global hot reload"
                          << (enabled ? "enabled" : "disabled");
}

bool PluginHotReloadManager::is_global_hot_reload_enabled() const {
    return m_global_enabled;
}

void PluginHotReloadManager::on_file_changed(const QString& path) {
    if (!m_global_enabled) {
        qCDebug(hotReloadLog)
            << "File changed but global hot reload is disabled:" << path;
        return;
    }

    std::filesystem::path file_path = path.toStdString();
    std::string plugin_id = find_plugin_by_path(file_path);

    if (plugin_id.empty()) {
        qCWarning(hotReloadLog)
            << "File changed but no plugin found for path:" << path;
        return;
    }

    qCDebug(hotReloadLog) << "Plugin file changed:" << path
                          << "for plugin:" << QString::fromStdString(plugin_id);

    emit plugin_file_changed(QString::fromStdString(plugin_id), path);

    // Call reload callback if set
    if (m_reload_callback) {
        // Execute reload asynchronously to avoid blocking the file watcher
        auto future = std::async(std::launch::async, [this, plugin_id]() {
            try {
                m_reload_callback(plugin_id);
            } catch (const std::exception& e) {
                qCWarning(hotReloadLog)
                    << "Exception in reload callback:" << e.what();
            } catch (...) {
                qCWarning(hotReloadLog)
                    << "Unknown exception in reload callback";
            }
        });
        // Detach the future to avoid blocking
        future.wait_for(std::chrono::milliseconds(0));
    }
}

std::string PluginHotReloadManager::find_plugin_by_path(
    const std::filesystem::path& file_path) const {
    QMutexLocker locker(&m_watched_files_mutex);

    for (const auto& [plugin_id, watched_path] : m_watched_files) {
        if (watched_path == file_path) {
            return plugin_id;
        }
    }
    return {};
}

}  // namespace qtplugin
