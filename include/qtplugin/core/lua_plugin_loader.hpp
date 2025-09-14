/**
 * @file lua_plugin_loader.hpp
 * @brief Lua plugin loader for loading Lua script plugins
 * @version 3.2.0
 * @author QtPlugin Development Team
 *
 * This file provides a plugin loader specifically for Lua script plugins,
 * extending the plugin system to support Lua-based plugins alongside
 * native Qt plugins.
 */

#pragma once

#include <QObject>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include "../bridges/lua_plugin_bridge.hpp"
#include "plugin_loader.hpp"

namespace qtplugin {

/**
 * @brief Plugin loader for Lua script plugins
 *
 * This loader handles .lua files and creates LuaPluginBridge instances
 * to execute Lua scripts as plugins within the QtForge plugin system.
 */
class LuaPluginLoader : public IPluginLoader {
public:
    LuaPluginLoader();
    ~LuaPluginLoader() override;

    // === IPluginLoader Implementation ===

    /**
     * @brief Check if the loader can load the specified file
     * @param file_path Path to the plugin file
     * @return True if the file can be loaded (is a .lua file)
     */
    bool can_load(const std::filesystem::path& file_path) const override;

    /**
     * @brief Load a Lua plugin from file
     * @param file_path Path to the Lua plugin file
     * @return Plugin instance or error
     */
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> load(
        const std::filesystem::path& file_path) override;

    /**
     * @brief Unload a Lua plugin
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unload(
        std::string_view plugin_id) override;

    /**
     * @brief Get supported file extensions
     * @return List of supported extensions
     */
    std::vector<std::string> supported_extensions() const override;

    /**
     * @brief Get loader name
     * @return Loader name
     */
    std::string_view name() const noexcept override;

    /**
     * @brief Get loader description
     * @return Loader description
     */
    std::string_view description() const noexcept override;

    /**
     * @brief Get loader version
     * @return Loader version
     */
    Version version() const noexcept override;

    // === Lua-specific Methods ===

    /**
     * @brief Check if Lua bindings are available
     * @return True if Lua support is compiled in
     */
    static bool is_lua_available();

    /**
     * @brief Get number of loaded Lua plugins
     * @return Number of loaded plugins
     */
    size_t loaded_plugin_count() const;

    /**
     * @brief Get list of loaded plugin IDs
     * @return List of plugin IDs
     */
    std::vector<std::string> loaded_plugin_ids() const;

    /**
     * @brief Get Lua plugin bridge for a specific plugin
     * @param plugin_id Plugin identifier
     * @return Plugin bridge or nullptr if not found
     */
    std::shared_ptr<LuaPluginBridge> get_lua_bridge(
        std::string_view plugin_id) const;

private:
    struct LuaPluginInfo {
        std::string id;
        std::filesystem::path file_path;
        std::shared_ptr<LuaPluginBridge> bridge;
        std::chrono::system_clock::time_point load_time;
    };

    mutable std::shared_mutex m_plugins_mutex;
    std::unordered_map<std::string, std::unique_ptr<LuaPluginInfo>>
        m_loaded_plugins;

    /**
     * @brief Validate Lua plugin file
     * @param file_path Path to the file
     * @return True if valid
     */
    bool is_valid_lua_file(const std::filesystem::path& file_path) const;

    /**
     * @brief Extract plugin metadata from Lua file
     * @param file_path Path to the Lua file
     * @return Plugin metadata or error
     */
    qtplugin::expected<PluginMetadata, PluginError> extract_lua_metadata(
        const std::filesystem::path& file_path) const;

    /**
     * @brief Generate unique plugin ID for Lua plugin
     * @param file_path Path to the Lua file
     * @return Generated plugin ID
     */
    std::string generate_lua_plugin_id(
        const std::filesystem::path& file_path) const;
};

/**
 * @brief Factory for creating Lua plugin loaders
 */
class LuaPluginLoaderFactory {
public:
    /**
     * @brief Create a new Lua plugin loader
     * @return Lua plugin loader instance
     */
    static std::unique_ptr<LuaPluginLoader> create();

    /**
     * @brief Check if Lua plugin loader can be created
     * @return True if Lua support is available
     */
    static bool is_available();

    /**
     * @brief Register Lua plugin loader with the plugin loader factory
     */
    static void register_with_factory();

private:
    static std::unique_ptr<IPluginLoader> create_lua_loader();
};

/**
 * @brief Composite plugin loader that supports both Qt and Lua plugins
 *
 * This loader combines QtPluginLoader and LuaPluginLoader to provide
 * unified loading of both native Qt plugins and Lua script plugins.
 */
class CompositePluginLoader : public IPluginLoader {
public:
    CompositePluginLoader();
    ~CompositePluginLoader() override;

    // === IPluginLoader Implementation ===
    bool can_load(const std::filesystem::path& file_path) const override;
    qtplugin::expected<std::shared_ptr<IPlugin>, PluginError> load(
        const std::filesystem::path& file_path) override;
    qtplugin::expected<void, PluginError> unload(
        std::string_view plugin_id) override;
    std::vector<std::string> supported_extensions() const override;
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    Version version() const noexcept override;

    // === Composite-specific Methods ===

    /**
     * @brief Get the Qt plugin loader
     * @return Qt plugin loader instance
     */
    std::shared_ptr<QtPluginLoader> qt_loader() const { return m_qt_loader; }

    /**
     * @brief Get the Lua plugin loader
     * @return Lua plugin loader instance (may be null if Lua not available)
     */
    std::shared_ptr<LuaPluginLoader> lua_loader() const { return m_lua_loader; }

    /**
     * @brief Check if Lua support is enabled
     * @return True if Lua loader is available
     */
    bool has_lua_support() const { return m_lua_loader != nullptr; }

private:
    std::shared_ptr<QtPluginLoader> m_qt_loader;
    std::shared_ptr<LuaPluginLoader> m_lua_loader;

    /**
     * @brief Determine which loader to use for a file
     * @param file_path Path to the file
     * @return Appropriate loader or nullptr
     */
    IPluginLoader* select_loader(const std::filesystem::path& file_path) const;
};

}  // namespace qtplugin
