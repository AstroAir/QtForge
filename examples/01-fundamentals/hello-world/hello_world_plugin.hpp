/**
 * @file hello_world_plugin.hpp
 * @brief Minimal QtForge plugin for absolute beginners
 * @version 1.0.0
 *
 * This is the simplest possible QtForge plugin demonstrating:
 * - Basic IPlugin interface implementation
 * - Minimal lifecycle management
 * - Single command execution
 *
 * Perfect starting point for learning QtForge plugin development.
 */

#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>

#include "qtplugin/core/plugin_interface.hpp"
#include "qtplugin/utils/error_handling.hpp"

/**
 * @brief Minimal hello world plugin
 *
 * Demonstrates the absolute minimum required to create a working QtForge plugin.
 * Only implements essential IPlugin methods with simple functionality.
 */
class HelloWorldPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "hello_world_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit HelloWorldPlugin(QObject* parent = nullptr);
    ~HelloWorldPlugin() override = default;

    // === Essential IPlugin Interface ===

    /**
     * @brief Get plugin name
     */
    std::string_view name() const noexcept override;

    /**
     * @brief Get plugin description
     */
    std::string_view description() const noexcept override;

    /**
     * @brief Get plugin version
     */
    qtplugin::Version version() const noexcept override;

    /**
     * @brief Get plugin author
     */
    std::string_view author() const noexcept override;

    /**
     * @brief Get unique plugin identifier
     */
    std::string id() const noexcept override;

    /**
     * @brief Get plugin capabilities
     */
    qtplugin::PluginCapabilities capabilities() const noexcept override;

    /**
     * @brief Initialize the plugin
     * @return Success or error information
     */
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;

    /**
     * @brief Shutdown the plugin
     */
    void shutdown() noexcept override;

    /**
     * @brief Execute a plugin command
     * @param command Command name to execute
     * @param params Command parameters (optional)
     * @return Command result or error
     */
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}) override;

    /**
     * @brief Get list of available commands
     * @return Vector of command names
     */
    std::vector<std::string> available_commands() const override;

    /**
     * @brief Get plugin metadata
     * @return Plugin metadata information
     */
    qtplugin::PluginMetadata metadata() const override;

    /**
     * @brief Get current plugin state
     * @return Current state
     */
    qtplugin::PluginState state() const noexcept override;

private:
    qtplugin::PluginState m_state{qtplugin::PluginState::Unloaded};
};
