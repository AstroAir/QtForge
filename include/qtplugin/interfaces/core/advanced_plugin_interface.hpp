/**
 * @file advanced_plugin_interface.hpp
 * @brief Advanced plugin interface with service contracts and enhanced capabilities
 * @version 3.2.0
 * @author QtPlugin Development Team
 *
 * This interface extends the base IPlugin interface with advanced features including:
 * - Service contract support for formal inter-plugin communication
 * - Advanced communication capabilities
 * - Dynamic capability negotiation
 * - Enhanced metadata and versioning
 * - Hot reload support
 * - Dependency change handling
 * - Configuration schema validation
 */

#pragma once

#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QStringList>
#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include "plugin_interface.hpp"
#include "../../communication/plugin_service_contracts.hpp"

namespace qtplugin {

/**
 * @brief Advanced plugin interface extending IPlugin with service contracts
 *
 * This interface provides advanced plugin functionality including service contracts,
 * enhanced communication, and dynamic capability management. It is designed for
 * plugins that need to provide or consume services from other plugins with
 * formal contracts and type safety.
 */
class IAdvancedPlugin : public virtual IPlugin {
public:
    ~IAdvancedPlugin() override = default;

    // === Service Contract System ===

    /**
     * @brief Get service contracts provided by this plugin
     * @return Vector of service contracts this plugin provides
     */
    virtual std::vector<contracts::ServiceContract> get_service_contracts() const = 0;

    /**
     * @brief Get service dependencies required by this plugin
     * @return Vector of service contracts this plugin depends on
     */
    virtual std::vector<contracts::ServiceContract> get_service_dependencies() const {
        return {};
    }

    /**
     * @brief Register services with the plugin system
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> register_services() {
        return make_success();
    }

    /**
     * @brief Unregister services from the plugin system
     */
    virtual void unregister_services() {}

    /**
     * @brief Call a service method on another plugin
     * @param service_name Name of the service to call
     * @param method_name Name of the method to invoke
     * @param parameters Method parameters as JSON object
     * @param timeout Maximum time to wait for response
     * @return Service response or error
     */
    virtual qtplugin::expected<QJsonObject, PluginError> call_service(
        const QString& service_name,
        const QString& method_name,
        const QJsonObject& parameters = {},
        std::chrono::milliseconds timeout = std::chrono::milliseconds{30000}) = 0;

    // === Configuration Management ===

    /**
     * @brief Validate configuration against plugin's schema
     * @param config Configuration to validate
     * @return True if configuration is valid
     */
    virtual bool validate_configuration(const QJsonObject& config) const {
        Q_UNUSED(config)
        return true;
    }

    /**
     * @brief Get configuration schema for this plugin
     * @return JSON schema describing valid configuration
     */
    virtual QJsonObject get_configuration_schema() const {
        return QJsonObject{};
    }

    // === Hot Reload Support ===

    /**
     * @brief Support hot reload of plugin
     * @return True if hot reload was successful
     */
    virtual bool hot_reload() {
        return false;
    }

    /**
     * @brief Handle dependency state changes
     * @param dependency_id ID of the dependency that changed
     * @param new_state New state of the dependency
     * @return True if change was handled successfully
     */
    virtual bool handle_dependency_change(const QString& dependency_id, 
                                        PluginState new_state) {
        Q_UNUSED(dependency_id)
        Q_UNUSED(new_state)
        return true;
    }

    // === Enhanced Metadata ===

    /**
     * @brief Get extended plugin information
     * @return Extended metadata including service information
     */
    virtual QJsonObject get_extended_metadata() const {
        QJsonObject extended;
        
        // Add service contracts
        QJsonArray contracts;
        for (const auto& contract : get_service_contracts()) {
            contracts.append(contract.to_json());
        }
        extended["service_contracts"] = contracts;
        
        // Add service dependencies
        QJsonArray dependencies;
        for (const auto& dependency : get_service_dependencies()) {
            dependencies.append(dependency.to_json());
        }
        extended["service_dependencies"] = dependencies;
        
        // Add configuration schema
        extended["configuration_schema"] = get_configuration_schema();
        
        return extended;
    }

    // === Plugin Lifecycle Events ===

    /**
     * @brief Called before plugin initialization
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> pre_initialize() {
        return make_success();
    }

    /**
     * @brief Called after plugin initialization
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> post_initialize() {
        return make_success();
    }

    /**
     * @brief Called before plugin shutdown
     */
    virtual void pre_shutdown() {}

    /**
     * @brief Called after plugin shutdown
     */
    virtual void post_shutdown() {}

    // === Service Discovery ===

    /**
     * @brief Check if a service is available
     * @param service_name Name of the service to check
     * @return True if service is available
     */
    virtual bool is_service_available(const QString& service_name) const {
        Q_UNUSED(service_name)
        return false;
    }

    /**
     * @brief Get list of available services
     * @return List of available service names
     */
    virtual QStringList get_available_services() const {
        return {};
    }

    // === Transaction Support ===

    /**
     * @brief Begin a transaction
     * @param transaction_id Unique transaction identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> begin_transaction(
        const QString& transaction_id) {
        Q_UNUSED(transaction_id)
        return make_success();
    }

    /**
     * @brief Commit a transaction
     * @param transaction_id Transaction identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> commit_transaction(
        const QString& transaction_id) {
        Q_UNUSED(transaction_id)
        return make_success();
    }

    /**
     * @brief Rollback a transaction
     * @param transaction_id Transaction identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> rollback_transaction(
        const QString& transaction_id) {
        Q_UNUSED(transaction_id)
        return make_success();
    }
};

}  // namespace qtplugin

// Register interface with Qt's meta-object system
Q_DECLARE_INTERFACE(qtplugin::IAdvancedPlugin, "qtplugin.IAdvancedPlugin/3.2")

// Register interface with validator
#include "../interface_validator.hpp"
namespace {
    static const bool advanced_plugin_interface_registered = 
        qtplugin::InterfaceRegistry::instance().register_interface(
            "qtplugin.IAdvancedPlugin/3.2",
            qtplugin::InterfaceMetadata{
                .interface_id = "qtplugin.IAdvancedPlugin/3.2",
                .version = qtplugin::Version{3, 2, 0},
                .name = "Advanced Plugin Interface",
                .description = "Advanced plugin interface with service contracts and enhanced capabilities",
                .required_methods = {
                    "get_service_contracts() const",
                    "call_service(const QString&, const QString&, const QJsonObject&, std::chrono::milliseconds)"
                },
                .optional_methods = {
                    "get_service_dependencies() const",
                    "register_services()",
                    "unregister_services()",
                    "validate_configuration(const QJsonObject&) const",
                    "get_configuration_schema() const",
                    "hot_reload()",
                    "handle_dependency_change(const QString&, PluginState)"
                },
                .dependencies = {"qtplugin.IPlugin/3.2"}
            }
        );
}
