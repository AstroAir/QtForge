/**
 * @file plugin_types_only.hpp
 * @brief Plugin types for Python bindings without PluginManager
 * @version 3.0.0
 *
 * This header includes only the type definitions needed for Python bindings,
 * without including the PluginManager class that causes incomplete type issues.
 */

#pragma once

// Include the basic types we need
#include <qtplugin/core/plugin_interface.hpp>
// Security manager removed
#include <qtplugin/utils/version.hpp>
#include <qtplugin/utils/error_handling.hpp>

// Include specific parts of plugin_manager.hpp that we need
// We'll copy the type definitions to avoid including the full header

#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace qtplugin {

/**
 * @brief Plugin loading options
 */
struct PluginLoadOptions {
    bool validate_sha256 = false;        ///< Validate plugin SHA256 checksum
    std::string expected_sha256;          ///< Expected SHA256 hash (if validation enabled)
    bool check_dependencies = true;      ///< Check plugin dependencies
    bool initialize_immediately = true;  ///< Initialize plugin after loading
    bool enable_hot_reload = false;      ///< Enable hot reloading for this plugin
    std::chrono::milliseconds timeout = std::chrono::milliseconds(5000);  ///< Loading timeout
    std::vector<std::string> allowed_paths;  ///< Allowed paths for plugin files
    bool sandbox_enabled = true;  ///< Enable sandboxing for plugin
};

/**
 * @brief Plugin information structure
 */
struct PluginInfo {
    std::string id;
    std::filesystem::path file_path;
    PluginMetadata metadata;
    PluginState state = PluginState::Unloaded;
    std::chrono::system_clock::time_point load_time;
    std::chrono::system_clock::time_point last_activity;
    bool hot_reload_enabled = false;

    /**
     * @brief Convert plugin info to JSON representation
     */
    std::string to_json() const {
        return "{\"id\":\"" + id + "\",\"state\":" + std::to_string(static_cast<int>(state)) + "}";
    }

    /**
     * @brief String representation for Python
     */
    std::string __repr__() const {
        return "<PluginInfo: " + id + " (" + std::to_string(static_cast<int>(state)) + ")>";
    }
};

} // namespace qtplugin
