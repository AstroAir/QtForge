/**
 * @file configuration_watcher.cpp
 * @brief Implementation of configuration watcher
 * @version 3.0.0
 */

#include <qtplugin/managers/components/configuration_watcher.hpp>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QTimer>
#include <chrono>

Q_LOGGING_CATEGORY(configWatcherLog, "qtplugin.config.watcher")

namespace qtplugin {

ConfigurationWatcher::ConfigurationWatcher(QObject* parent)
    : QObject(parent),
      m_file_watcher(std::make_unique<QFileSystemWatcher>(this)) {
    // Connect file system watcher signals
    connect(m_file_watcher.get(), &QFileSystemWatcher::fileChanged, this,
            &ConfigurationWatcher::on_file_changed);
    connect(m_file_watcher.get(), &QFileSystemWatcher::directoryChanged, this,
            &ConfigurationWatcher::on_directory_changed);

    qCDebug(configWatcherLog) << "Configuration watcher initialized";
}

ConfigurationWatcher::~ConfigurationWatcher() {
    clear_watches();
    qCDebug(configWatcherLog) << "Configuration watcher destroyed";
}

qtplugin::expected<void, PluginError> ConfigurationWatcher::watch_file(
    const std::filesystem::path& file_path, ConfigurationScope scope,
    std::string_view plugin_id, bool auto_reload) {
    try {
        std::string path_str = path_to_string(file_path);

        // Check if already watching
        if (m_watches.find(path_str) != m_watches.end()) {
            return make_error<void>(
                PluginErrorCode::ConfigurationError,
                "File is already being watched: " + path_str);
        }

        // Create watch info
        auto watch = std::make_unique<ConfigurationWatch>();
        watch->file_path = file_path;
        watch->scope = scope;
        watch->plugin_id = std::string(plugin_id);
        watch->auto_reload = auto_reload;
        watch->last_modified = get_file_modification_time(file_path);

        // Add to file system watcher
        QString qt_path = QString::fromStdString(path_str);
        if (std::filesystem::exists(file_path)) {
            m_file_watcher->addPath(qt_path);
        } else {
            // Watch the directory if file doesn't exist yet
            ensure_directory_watched(file_path);
        }

        // Store watch info
        m_watches[path_str] = std::move(watch);

        qCDebug(configWatcherLog)
            << "Started watching configuration file:" << qt_path;

        return make_success();

    } catch (const std::exception& e) {
        return make_error<void>(
            PluginErrorCode::ConfigurationError,
            "Exception watching file: " + std::string(e.what()));
    }
}

qtplugin::expected<void, PluginError> ConfigurationWatcher::unwatch_file(
    const std::filesystem::path& file_path) {
    try {
        std::string path_str = path_to_string(file_path);

        auto it = m_watches.find(path_str);
        if (it == m_watches.end()) {
            return make_error<void>(PluginErrorCode::ConfigurationError,
                                    "File is not being watched: " + path_str);
        }

        // Remove from file system watcher
        QString qt_path = QString::fromStdString(path_str);
        m_file_watcher->removePath(qt_path);

        // Remove watch info
        m_watches.erase(it);

        qCDebug(configWatcherLog)
            << "Stopped watching configuration file:" << qt_path;

        return make_success();

    } catch (const std::exception& e) {
        return make_error<void>(
            PluginErrorCode::ConfigurationError,
            "Exception unwatching file: " + std::string(e.what()));
    }
}

bool ConfigurationWatcher::is_watching(
    const std::filesystem::path& file_path) const {
    std::string path_str = path_to_string(file_path);
    return m_watches.find(path_str) != m_watches.end();
}

std::vector<std::filesystem::path> ConfigurationWatcher::get_watched_files()
    const {
    std::vector<std::filesystem::path> files;
    files.reserve(m_watches.size());

    for (const auto& [path_str, watch] : m_watches) {
        files.push_back(watch->file_path);
    }

    return files;
}

qtplugin::expected<void, PluginError> ConfigurationWatcher::set_auto_reload(
    const std::filesystem::path& file_path, bool auto_reload) {
    std::string path_str = path_to_string(file_path);

    auto it = m_watches.find(path_str);
    if (it == m_watches.end()) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "File is not being watched: " + path_str);
    }

    it->second->auto_reload = auto_reload;

    qCDebug(configWatcherLog)
        << "Auto-reload" << (auto_reload ? "enabled" : "disabled") << "for"
        << QString::fromStdString(path_str);

    return make_success();
}

qtplugin::expected<void, PluginError> ConfigurationWatcher::reload_file(
    const std::filesystem::path& file_path) {
    std::string path_str = path_to_string(file_path);

    auto it = m_watches.find(path_str);
    if (it == m_watches.end()) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "File is not being watched: " + path_str);
    }

    try {
        // Update last modified time
        it->second->last_modified = get_file_modification_time(file_path);

        // Call change callback if set
        if (m_change_callback) {
            m_change_callback(file_path, it->second->scope,
                              it->second->plugin_id);
        }

        emit file_reloaded(QString::fromStdString(path_str), true);
        qCDebug(configWatcherLog) << "Manually reloaded configuration file:"
                                  << QString::fromStdString(path_str);

        return make_success();

    } catch (const std::exception& e) {
        emit file_reloaded(QString::fromStdString(path_str), false);
        return make_error<void>(
            PluginErrorCode::ConfigurationError,
            "Exception reloading file: " + std::string(e.what()));
    }
}

void ConfigurationWatcher::clear_watches() {
    // Remove all paths from file system watcher
    QStringList watched_files = m_file_watcher->files();
    QStringList watched_dirs = m_file_watcher->directories();

    if (!watched_files.isEmpty()) {
        m_file_watcher->removePaths(watched_files);
    }
    if (!watched_dirs.isEmpty()) {
        m_file_watcher->removePaths(watched_dirs);
    }

    // Clear internal data
    m_watches.clear();
    m_watched_directories.clear();

    qCDebug(configWatcherLog) << "Cleared all configuration watches";
}

void ConfigurationWatcher::set_change_callback(
    std::function<void(const std::filesystem::path&, ConfigurationScope,
                       const std::string&)>
        callback) {
    m_change_callback = callback;
}

void ConfigurationWatcher::on_file_changed(const QString& path) {
    std::string path_str = path.toStdString();
    std::filesystem::path file_path = string_to_path(path_str);

    auto it = m_watches.find(path_str);
    if (it == m_watches.end()) {
        return;  // Not watching this file
    }

    auto& watch = it->second;

    // Check if file was actually modified (avoid duplicate notifications)
    if (!is_file_modified(file_path, watch->last_modified)) {
        return;
    }

    // Update last modified time
    watch->last_modified = get_file_modification_time(file_path);

    qCDebug(configWatcherLog) << "Configuration file changed:" << path;

    // Emit signal
    emit file_changed(path, static_cast<int>(watch->scope),
                      QString::fromStdString(watch->plugin_id));

    // Auto-reload if enabled
    if (watch->auto_reload) {
        if (m_change_callback) {
            try {
                m_change_callback(file_path, watch->scope, watch->plugin_id);
                emit file_reloaded(path, true);
            } catch (const std::exception& e) {
                emit watch_error(path, QString::fromStdString(e.what()));
                emit file_reloaded(path, false);
            }
        }
    }
}

void ConfigurationWatcher::on_directory_changed(const QString& path) {
    // Check if any watched files in this directory were created/deleted
    QDir dir(path);

    for (const auto& [path_str, watch] : m_watches) {
        std::filesystem::path file_path = watch->file_path;
        std::filesystem::path parent_path = file_path.parent_path();

        if (parent_path == std::filesystem::path(path.toStdString())) {
            // Check if file now exists and should be added to watcher
            if (std::filesystem::exists(file_path)) {
                QString qt_file_path = QString::fromStdString(path_str);
                if (!m_file_watcher->files().contains(qt_file_path)) {
                    m_file_watcher->addPath(qt_file_path);
                    qCDebug(configWatcherLog)
                        << "Added newly created file to watcher:"
                        << qt_file_path;
                }
            }
        }
    }
}

std::string ConfigurationWatcher::path_to_string(
    const std::filesystem::path& path) const {
    return path.string();
}

std::filesystem::path ConfigurationWatcher::string_to_path(
    const std::string& path_str) const {
    return std::filesystem::path(path_str);
}

void ConfigurationWatcher::ensure_directory_watched(
    const std::filesystem::path& file_path) {
    auto parent_path = file_path.parent_path();
    if (parent_path.empty()) {
        return;
    }

    std::string dir_str = parent_path.string();
    if (m_watched_directories.find(dir_str) != m_watched_directories.end()) {
        return;  // Already watching this directory
    }

    if (std::filesystem::exists(parent_path)) {
        QString qt_dir_path = QString::fromStdString(dir_str);
        m_file_watcher->addPath(qt_dir_path);
        m_watched_directories.insert(dir_str);

        qCDebug(configWatcherLog)
            << "Started watching directory:" << qt_dir_path;
    }
}

bool ConfigurationWatcher::is_file_modified(
    const std::filesystem::path& file_path,
    const std::chrono::system_clock::time_point& last_known) const {
    try {
        auto current_time = get_file_modification_time(file_path);
        return current_time > last_known;
    } catch (...) {
        return false;
    }
}

std::chrono::system_clock::time_point
ConfigurationWatcher::get_file_modification_time(
    const std::filesystem::path& file_path) const {
    try {
        if (std::filesystem::exists(file_path)) {
            auto ftime = std::filesystem::last_write_time(file_path);
            // Convert file_time to system_clock time (simplified)
            auto sctp = std::chrono::time_point_cast<
                std::chrono::system_clock::duration>(
                ftime - std::filesystem::file_time_type::clock::now() +
                std::chrono::system_clock::now());
            return sctp;
        }
    } catch (...) {
        // Return epoch time on error
    }

    return std::chrono::system_clock::time_point{};
}

}  // namespace qtplugin
