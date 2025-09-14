/**
 * @file plugin_manager_python.hpp
 * @brief Python-specific PluginManager declarations
 * @version 3.0.0
 *
 * This header provides declarations for the Python bindings without
 * including the problematic main PluginManager header.
 */

#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <chrono>
#include <QObject>
#include <QString>

// Include the existing types - we'll use them directly
#include <qtplugin/core/plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>
#include <qtplugin/utils/version.hpp>

namespace qtplugin {

// Additional structures for Python bindings that don't conflict
struct PythonPluginLoadOptions {
    bool validate_signature = true;
    bool check_dependencies = true;
    bool initialize_immediately = true;
    bool enable_hot_reload = false;
};

struct PythonPluginInfo {
    std::string id;
    std::filesystem::path file_path;
    PluginState state = PluginState::Unloaded;
    std::chrono::system_clock::time_point load_time;
    std::chrono::system_clock::time_point last_activity;
    bool hot_reload_enabled = false;

    std::string to_json() const {
        return "{\"id\":\"" + id + "\",\"state\":" + std::to_string(static_cast<int>(state)) + "}";
    }

    std::string __repr__() const {
        return "<PluginInfo: " + id + " (" + std::to_string(static_cast<int>(state)) + ")>";
    }
};

// Python-specific PluginManager class that doesn't conflict with the main one
// This class doesn't inherit from QObject to avoid MOC complications
class PythonPluginManager {
public:
    // Simple constructor
    PythonPluginManager();
    ~PythonPluginManager();

    // Plugin management methods
    qtplugin::expected<std::string, PluginError> load_plugin(
        const std::filesystem::path& file_path, const PythonPluginLoadOptions& options = {});

    qtplugin::expected<void, PluginError> unload_plugin(
        std::string_view plugin_id, bool force = false);

    std::vector<std::string> loaded_plugins() const;
    std::vector<std::filesystem::path> search_paths() const;
    void add_search_path(const std::filesystem::path& path);
    void remove_search_path(const std::filesystem::path& path);
    std::shared_ptr<IPlugin> get_plugin(std::string_view plugin_id) const;
    std::vector<PythonPluginInfo> all_plugin_info() const;
    std::vector<std::filesystem::path> discover_plugins(
        const std::filesystem::path& directory, bool recursive = false) const;

    // Utility methods for Python bindings
    void on_file_changed(const std::string& path);
    void on_monitoring_timer();
};

} // namespace qtplugin
