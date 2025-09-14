/**
 * @file plugin_registry.hpp
 * @brief Plugin registry interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <memory>
#include <optional>
#include <qtplugin/core/plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace qtplugin {

// Forward declarations
struct PluginInfo;
class IPlugin;

/**
 * @brief Interface for plugin registry operations
 *
 * The plugin registry is responsible for storing and managing plugin
 * information, providing thread-safe access to plugin instances and metadata.
 */
class IPluginRegistry {
public:
    virtual ~IPluginRegistry() = default;

    /**
     * @brief Register a plugin in the registry
     * @param plugin_id Unique plugin identifier
     * @param plugin_info Plugin information structure
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> register_plugin(
        const std::string& plugin_id,
        std::unique_ptr<PluginInfo> plugin_info) = 0;

    /**
     * @brief Unregister a plugin from the registry
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> unregister_plugin(
        const std::string& plugin_id) = 0;

    /**
     * @brief Get plugin instance by ID
     * @param plugin_id Plugin identifier
     * @return Shared pointer to plugin, or nullptr if not found
     */
    virtual std::shared_ptr<IPlugin> get_plugin(
        const std::string& plugin_id) const = 0;

    /**
     * @brief Get plugin information by ID
     * @param plugin_id Plugin identifier
     * @return Plugin information or nullopt if not found
     */
    virtual std::optional<PluginInfo> get_plugin_info(
        const std::string& plugin_id) const = 0;

    /**
     * @brief Get all registered plugin IDs
     * @return Vector of plugin identifiers
     */
    virtual std::vector<std::string> get_all_plugin_ids() const = 0;

    /**
     * @brief Get all plugin information
     * @return Vector of plugin information structures
     */
    virtual std::vector<PluginInfo> get_all_plugin_info() const = 0;

    /**
     * @brief Check if plugin is registered
     * @param plugin_id Plugin identifier
     * @return true if plugin is registered
     */
    virtual bool is_plugin_registered(const std::string& plugin_id) const = 0;

    /**
     * @brief Get number of registered plugins
     * @return Plugin count
     */
    virtual size_t plugin_count() const = 0;

    /**
     * @brief Clear all registered plugins
     */
    virtual void clear() = 0;

    /**
     * @brief Update plugin information
     * @param plugin_id Plugin identifier
     * @param plugin_info Updated plugin information
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> update_plugin_info(
        const std::string& plugin_id, const PluginInfo& plugin_info) = 0;
};

/**
 * @brief Thread-safe plugin registry implementation
 *
 * Provides thread-safe storage and access to plugin information.
 * Uses shared_mutex for efficient concurrent read access.
 */
class PluginRegistry : public QObject, public IPluginRegistry {
    Q_OBJECT

public:
    explicit PluginRegistry(QObject* parent = nullptr);
    ~PluginRegistry() override;

    // IPluginRegistry interface
    qtplugin::expected<void, PluginError> register_plugin(
        const std::string& plugin_id,
        std::unique_ptr<PluginInfo> plugin_info) override;

    qtplugin::expected<void, PluginError> unregister_plugin(
        const std::string& plugin_id) override;

    std::shared_ptr<IPlugin> get_plugin(
        const std::string& plugin_id) const override;
    std::optional<PluginInfo> get_plugin_info(
        const std::string& plugin_id) const override;
    std::vector<std::string> get_all_plugin_ids() const override;
    std::vector<PluginInfo> get_all_plugin_info() const override;
    bool is_plugin_registered(const std::string& plugin_id) const override;
    size_t plugin_count() const override;
    void clear() override;

    qtplugin::expected<void, PluginError> update_plugin_info(
        const std::string& plugin_id, const PluginInfo& plugin_info) override;

signals:
    /**
     * @brief Emitted when a plugin is registered
     * @param plugin_id Plugin identifier
     */
    void plugin_registered(const QString& plugin_id);

    /**
     * @brief Emitted when a plugin is unregistered
     * @param plugin_id Plugin identifier
     */
    void plugin_unregistered(const QString& plugin_id);

    /**
     * @brief Emitted when plugin information is updated
     * @param plugin_id Plugin identifier
     */
    void plugin_info_updated(const QString& plugin_id);

private:
    mutable std::shared_mutex m_plugins_mutex;
    std::unordered_map<std::string, std::unique_ptr<PluginInfo>> m_plugins;

    // Helper methods
    PluginInfo create_plugin_info_copy(const PluginInfo& original) const;
};

}  // namespace qtplugin
