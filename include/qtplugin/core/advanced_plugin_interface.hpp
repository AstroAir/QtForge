/**
 * @file advanced_plugin_interface.hpp
 * @brief Advanced plugin interface with service contracts support
 * @version 3.1.0
 * @author QtPlugin Development Team
 *
 * This file extends the base plugin interface with advanced features including
 * service contracts, advanced communication patterns, and improved lifecycle
 * management.
 */

#pragma once

#include <QJsonObject>
#include <QString>
#include <functional>
#include <future>
#include <memory>
#include <vector>
#include "../communication/plugin_service_contracts.hpp"
#include "../utils/error_handling.hpp"
#include "plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Advanced plugin interface with service contracts support
 *
 * This interface extends IPlugin with advanced features:
 * - Service contract registration and management
 * - Advanced inter-plugin communication
 * - Asynchronous operation support
 * - Transaction support
 * - Health monitoring
 */
class IAdvancedPlugin : public virtual IPlugin {
public:
    ~IAdvancedPlugin() override = default;

    // === Service Contract Management ===

    /**
     * @brief Get service contracts provided by this plugin
     * @return Vector of service contracts
     */
    virtual std::vector<contracts::ServiceContract> get_service_contracts()
        const = 0;

    /**
     * @brief Get service dependencies required by this plugin
     * @return Vector of required service names and minimum versions
     */
    virtual std::vector<std::pair<QString, contracts::ServiceVersion>>
    get_service_dependencies() const {
        return {};
    }

    /**
     * @brief Register service contracts with the registry
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> register_services() {
        auto& registry = contracts::ServiceContractRegistry::instance();

        for (const auto& contract : get_service_contracts()) {
            auto result = registry.register_contract(
                QString::fromStdString(id()), contract);
            if (!result) {
                return result;
            }
        }

        return make_success();
    }

    /**
     * @brief Unregister service contracts from the registry
     */
    virtual void unregister_services() {
        auto& registry = contracts::ServiceContractRegistry::instance();

        for (const auto& contract : get_service_contracts()) {
            registry.unregister_contract(QString::fromStdString(id()),
                                         contract.service_name());
        }
    }

    // === Advanced Communication ===

    /**
     * @brief Call a service method on another plugin
     * @param service_name Name of the service
     * @param method_name Name of the method
     * @param parameters Method parameters
     * @param timeout Operation timeout
     * @return Method result or error
     */
    virtual qtplugin::expected<QJsonObject, PluginError> call_service(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters = {},
        std::chrono::milliseconds timeout = std::chrono::milliseconds{
            30000}) = 0;

    /**
     * @brief Call a service method asynchronously
     * @param service_name Name of the service
     * @param method_name Name of the method
     * @param parameters Method parameters
     * @param timeout Operation timeout
     * @return Future with method result
     */
    virtual std::future<qtplugin::expected<QJsonObject, PluginError>>
    call_service_async(const QString& service_name, const QString& method_name,
                       const QJsonObject& parameters = {},
                       std::chrono::milliseconds timeout =
                           std::chrono::milliseconds{30000}) = 0;

    /**
     * @brief Handle incoming service calls
     * @param service_name Name of the service
     * @param method_name Name of the method
     * @param parameters Method parameters
     * @return Method result or error
     */
    virtual qtplugin::expected<QJsonObject, PluginError> handle_service_call(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters) = 0;

    // === Transaction Support ===

    /**
     * @brief Begin a transaction
     * @param transaction_id Unique transaction identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> begin_transaction(
        const QString& transaction_id) {
        Q_UNUSED(transaction_id)
        // Default implementation - no transaction support
        return make_error<void>(PluginErrorCode::NotSupported,
                                "Transactions not supported");
    }

    /**
     * @brief Commit a transaction
     * @param transaction_id Transaction identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> commit_transaction(
        const QString& transaction_id) {
        Q_UNUSED(transaction_id)
        return make_error<void>(PluginErrorCode::NotSupported,
                                "Transactions not supported");
    }

    /**
     * @brief Rollback a transaction
     * @param transaction_id Transaction identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> rollback_transaction(
        const QString& transaction_id) {
        Q_UNUSED(transaction_id)
        return make_error<void>(PluginErrorCode::NotSupported,
                                "Transactions not supported");
    }

    // === Health Monitoring ===

    /**
     * @brief Get plugin health status
     * @return Health information
     */
    virtual QJsonObject get_health_status() const {
        QJsonObject health;
        health["status"] = "healthy";
        health["state"] = static_cast<int>(state());
        health["uptime"] = 0;  // Should be overridden by implementations
        return health;
    }

    /**
     * @brief Perform health check
     * @return Success if healthy, error otherwise
     */
    virtual qtplugin::expected<void, PluginError> health_check() const {
        return make_success();
    }

    // === Advanced Lifecycle ===

    /**
     * @brief Prepare for shutdown (called before shutdown)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> prepare_shutdown() {
        return make_success();
    }

    /**
     * @brief Handle configuration change
     * @param new_config New configuration
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> reconfigure(
        const QJsonObject& new_config) {
        // Default implementation - reload configuration
        return configure(new_config);
    }

    /**
     * @brief Pause plugin operations
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> pause() {
        return make_error<void>(PluginErrorCode::NotSupported,
                                "Pause not supported");
    }

    /**
     * @brief Resume plugin operations
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> resume() {
        return make_error<void>(PluginErrorCode::NotSupported,
                                "Resume not supported");
    }

    // === Plugin Composition Support ===

    /**
     * @brief Check if this plugin can be composed with another
     * @param other_plugin_id ID of the other plugin
     * @return True if composition is possible
     */
    virtual bool can_compose_with(const QString& other_plugin_id) const {
        Q_UNUSED(other_plugin_id)
        return false;
    }

    /**
     * @brief Get composition requirements
     * @return Requirements for plugin composition
     */
    virtual QJsonObject get_composition_requirements() const {
        return QJsonObject();
    }

    // === Event Handling ===

    /**
     * @brief Handle plugin events
     * @param event_type Type of event
     * @param event_data Event data
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> handle_event(
        const QString& event_type, const QJsonObject& event_data) {
        Q_UNUSED(event_type)
        Q_UNUSED(event_data)
        return make_success();
    }

    /**
     * @brief Get supported event types
     * @return Vector of supported event types
     */
    virtual std::vector<QString> get_supported_events() const { return {}; }
};

/**
 * @brief Base implementation of advanced plugin interface
 *
 * Provides default implementations for common functionality.
 */
class AdvancedPluginBase : public QObject, public virtual IAdvancedPlugin {
    Q_OBJECT

public:
    explicit AdvancedPluginBase(QObject* parent = nullptr);
    ~AdvancedPluginBase() override;

    // === IPlugin Implementation ===
    qtplugin::expected<void, PluginError> initialize() override;
    void shutdown() noexcept override;
    PluginState state() const noexcept override { return m_state; }

    // === IAdvancedPlugin Implementation ===
    qtplugin::expected<QJsonObject, PluginError> call_service(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters = {},
        std::chrono::milliseconds timeout = std::chrono::milliseconds{
            30000}) override;

    std::future<qtplugin::expected<QJsonObject, PluginError>>
    call_service_async(const QString& service_name, const QString& method_name,
                       const QJsonObject& parameters = {},
                       std::chrono::milliseconds timeout =
                           std::chrono::milliseconds{30000}) override;

    qtplugin::expected<QJsonObject, PluginError> handle_service_call(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters) override;

    QJsonObject get_health_status() const override;

protected:
    void set_state(PluginState new_state) { m_state = new_state; }

    /**
     * @brief Initialize plugin-specific functionality
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> do_initialize() = 0;

    /**
     * @brief Shutdown plugin-specific functionality
     */
    virtual void do_shutdown() = 0;

private:
    PluginState m_state{PluginState::Unloaded};
    std::chrono::system_clock::time_point m_start_time;
};

}  // namespace qtplugin

Q_DECLARE_INTERFACE(qtplugin::IAdvancedPlugin, "qtplugin.IAdvancedPlugin/3.1")
