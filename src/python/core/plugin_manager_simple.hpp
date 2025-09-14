/**
 * @file plugin_manager_simple.hpp
 * @brief Simplified PluginManager for Python bindings
 * @version 3.0.0
 * 
 * This header provides a simplified PluginManager that works with Python bindings
 * without the complex dependency injection issues.
 */

#pragma once

#include "plugin_types_only.hpp"
#include <memory>
#include <vector>
#include <string>
#include <filesystem>

namespace qtplugin {

/**
 * @brief Simplified PluginManager for Python bindings
 * 
 * This class provides a working PluginManager implementation that doesn't
 * require complex dependency injection, making it suitable for Python bindings.
 */
class SimplePluginManager {
private:
    std::vector<std::string> m_loaded_plugins;
    std::vector<std::filesystem::path> m_search_paths;
    std::vector<PluginInfo> m_plugin_info;

public:
    /**
     * @brief Constructor
     */
    SimplePluginManager();
    
    /**
     * @brief Destructor
     */
    ~SimplePluginManager() = default;

    /**
     * @brief Load a plugin from file path
     * @param file_path Path to plugin file
     * @param options Loading options
     * @return Plugin ID on success, error on failure
     */
    expected<std::string, PluginError> load_plugin(
        const std::filesystem::path& file_path, 
        const PluginLoadOptions& options = {});

    /**
     * @brief Unload a plugin
     * @param plugin_id Plugin identifier
     * @param force Force unload even if plugin is busy
     * @return Success or error
     */
    expected<void, PluginError> unload_plugin(
        std::string_view plugin_id, 
        bool force = false);

    /**
     * @brief Get list of loaded plugin IDs
     * @return Vector of plugin IDs
     */
    std::vector<std::string> loaded_plugins() const;

    /**
     * @brief Get plugin search paths
     * @return Vector of search paths
     */
    std::vector<std::filesystem::path> search_paths() const;

    /**
     * @brief Add a search path for plugins
     * @param path Path to add
     */
    void add_search_path(const std::filesystem::path& path);

    /**
     * @brief Remove a search path
     * @param path Path to remove
     */
    void remove_search_path(const std::filesystem::path& path);

    /**
     * @brief Get plugin by ID
     * @param plugin_id Plugin identifier
     * @return Plugin instance or nullptr
     */
    std::shared_ptr<IPlugin> get_plugin(std::string_view plugin_id) const;

    /**
     * @brief Get information about all plugins
     * @return Vector of plugin information
     */
    std::vector<PluginInfo> all_plugin_info() const;

    /**
     * @brief Discover plugins in a directory
     * @param directory Directory to search
     * @param recursive Whether to search recursively
     * @return Vector of discovered plugin paths
     */
    std::vector<std::filesystem::path> discover_plugins(
        const std::filesystem::path& directory, 
        bool recursive = false) const;

    /**
     * @brief Check if plugin manager is ready
     * @return true if ready
     */
    bool is_ready() const;

    /**
     * @brief Get plugin manager statistics
     * @return Statistics as JSON-like string
     */
    std::string get_statistics() const;

    /**
     * @brief Python representation
     */
    std::string __repr__() const;
};

/**
 * @brief Create a simple plugin manager instance
 * @return Unique pointer to plugin manager
 */
std::unique_ptr<SimplePluginManager> create_simple_plugin_manager();

} // namespace qtplugin
