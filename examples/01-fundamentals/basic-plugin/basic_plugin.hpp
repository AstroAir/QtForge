/**
 * @file basic_plugin.hpp
 * @brief Core QtForge plugin demonstrating essential IPlugin interface
 * @version 2.0.0
 *
 * This plugin demonstrates the core IPlugin interface methods that every
 * plugin developer needs to understand:
 * - Complete lifecycle management
 * - Configuration handling with validation
 * - Command execution with parameters
 * - Error handling with expected<T,E>
 * - Plugin metadata and state management
 *
 * This is the foundation for all QtForge plugin development.
 */

#pragma once

#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QTimer>
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "qtplugin/core/plugin_interface.hpp"
#include "qtplugin/utils/error_handling.hpp"

/**
 * @brief Basic plugin demonstrating core IPlugin interface
 *
 * Focuses on essential plugin development concepts:
 * - Proper lifecycle management
 * - Configuration with validation
 * - Multiple commands with parameters
 * - Thread-safe state management
 * - Comprehensive error handling
 */
class Q_DECL_EXPORT BasicPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "basic_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit BasicPlugin(QObject* parent = nullptr);
    ~BasicPlugin() override;

    // === Core IPlugin Interface ===

    /**
     * @brief Get plugin name
     * @return Plugin name
     */
    std::string_view name() const noexcept override;

    /**
     * @brief Get plugin description
     * @return Plugin description
     */
    std::string_view description() const noexcept override;

    /**
     * @brief Get plugin version
     * @return Plugin version
     */
    qtplugin::Version version() const noexcept override;

    /**
     * @brief Get plugin author
     * @return Plugin author
     */
    std::string_view author() const noexcept override;

    /**
     * @brief Get plugin ID
     * @return Plugin ID
     */
    std::string id() const noexcept override;

    /**
     * @brief Get plugin capabilities
     * @return Plugin capabilities
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
     * @brief Configure the plugin
     * @param config Configuration object
     * @return Success or error information
     */
    qtplugin::expected<void, qtplugin::PluginError> configure(
        const QJsonObject& config) override;

    /**
     * @brief Execute a plugin command
     * @param command Command name to execute
     * @param params Command parameters
     * @return Command result or error
     */
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override;

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

    /**
     * @brief Check if plugin is initialized
     * @return True if initialized
     */
    bool is_initialized() const noexcept override;

    /**
     * @brief Get default configuration
     * @return Default configuration object
     */
    std::optional<QJsonObject> default_configuration() const override;

    /**
     * @brief Get current configuration
     * @return Current configuration object
     */
    QJsonObject current_configuration() const override;

private slots:
    /**
     * @brief Handle timer events for background processing
     */
    void on_timer_timeout();

private:
    // === Configuration Validation ===
    bool validate_configuration(const QJsonObject& config) const;

    // === Command Implementations ===
    QJsonObject execute_status_command(const QJsonObject& params);
    QJsonObject execute_echo_command(const QJsonObject& params);
    QJsonObject execute_config_command(const QJsonObject& params);
    QJsonObject execute_timer_command(const QJsonObject& params);

    // === State Management ===
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    mutable QMutex m_config_mutex;
    QJsonObject m_configuration;

    // === Background Processing ===
    std::unique_ptr<QTimer> m_timer;
    std::atomic<int> m_timer_count{0};

    // === Plugin Information ===
    static constexpr const char* PLUGIN_NAME = "BasicPlugin";
    static constexpr const char* PLUGIN_ID = "com.qtforge.examples.basic";
    static constexpr const char* PLUGIN_DESCRIPTION =
        "Core QtForge plugin demonstrating essential IPlugin interface";
};
