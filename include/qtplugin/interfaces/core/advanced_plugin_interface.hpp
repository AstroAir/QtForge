#pragma once

#include <QJsonObject>
#include <QVariant>
#include <functional>
#include <memory>
#include "plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Advanced plugin interface with extended capabilities
 *
 * This interface extends the basic plugin interface with advanced features
 * such as configuration management, event handling, and lifecycle hooks.
 */
class IAdvancedPlugin : public IPlugin {
public:
    virtual ~IAdvancedPlugin() = default;

    // === Configuration Management ===

    /**
     * @brief Apply configuration to the plugin
     * @param config Configuration object
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> apply_configuration(
        const QJsonObject& config) = 0;

    /**
     * @brief Get current plugin configuration
     * @return Current configuration
     */
    virtual QJsonObject get_configuration() const = 0;

    /**
     * @brief Validate configuration before applying
     * @param config Configuration to validate
     * @return True if valid, false otherwise
     */
    virtual bool validate_configuration(const QJsonObject& config) const = 0;

    // === Event Handling ===

    /**
     * @brief Handle plugin event
     * @param event_type Type of event
     * @param event_data Event data
     * @return Event handling result
     */
    virtual qtplugin::expected<QVariant, PluginError> handle_event(
        const QString& event_type, const QVariant& event_data) = 0;

    /**
     * @brief Get list of supported event types
     * @return List of event types this plugin can handle
     */
    virtual QStringList supported_event_types() const = 0;

    // === Lifecycle Hooks ===

    /**
     * @brief Called before plugin initialization
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> pre_initialize() = 0;

    /**
     * @brief Called after plugin initialization
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> post_initialize() = 0;

    /**
     * @brief Called before plugin shutdown
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> pre_shutdown() = 0;

    /**
     * @brief Called after plugin shutdown
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> post_shutdown() = 0;

    // === Resource Management ===

    /**
     * @brief Get plugin resource usage
     * @return Resource usage information
     */
    virtual QJsonObject get_resource_usage() const = 0;

    /**
     * @brief Set resource limits for the plugin
     * @param limits Resource limits configuration
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> set_resource_limits(
        const QJsonObject& limits) = 0;

    // === Plugin Communication ===

    /**
     * @brief Send message to another plugin
     * @param target_plugin_id Target plugin identifier
     * @param message Message to send
     * @return Response or error
     */
    virtual qtplugin::expected<QVariant, PluginError> send_message(
        const QString& target_plugin_id, const QVariant& message) = 0;

    /**
     * @brief Handle incoming message from another plugin
     * @param sender_plugin_id Sender plugin identifier
     * @param message Received message
     * @return Response or error
     */
    virtual qtplugin::expected<QVariant, PluginError> receive_message(
        const QString& sender_plugin_id, const QVariant& message) = 0;

    // === Plugin State Management ===

    /**
     * @brief Save plugin state
     * @return Serialized state or error
     */
    virtual qtplugin::expected<QJsonObject, PluginError> save_state() = 0;

    /**
     * @brief Restore plugin state
     * @param state Previously saved state
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> restore_state(
        const QJsonObject& state) = 0;

    // === Plugin Capabilities ===

    /**
     * @brief Get plugin capabilities
     * @return List of capabilities this plugin provides
     */
    virtual QStringList get_capabilities() const = 0;

    /**
     * @brief Check if plugin has specific capability
     * @param capability Capability to check
     * @return True if plugin has the capability
     */
    virtual bool has_capability(const QString& capability) const = 0;

    // === Plugin Services ===

    /**
     * @brief Register a service provided by this plugin
     * @param service_name Name of the service
     * @param service_interface Service interface
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> register_service(
        const QString& service_name, const QVariant& service_interface) = 0;

    /**
     * @brief Unregister a service
     * @param service_name Name of the service to unregister
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> unregister_service(
        const QString& service_name) = 0;

    /**
     * @brief Get list of services provided by this plugin
     * @return List of service names
     */
    virtual QStringList get_provided_services() const = 0;

    // === Plugin Metrics ===

    /**
     * @brief Get plugin performance metrics
     * @return Performance metrics
     */
    virtual QJsonObject get_performance_metrics() const = 0;

    /**
     * @brief Reset plugin metrics
     */
    virtual void reset_metrics() = 0;

    // === Plugin Validation ===

    /**
     * @brief Validate plugin integrity
     * @return Validation result
     */
    virtual qtplugin::expected<bool, PluginError> validate_integrity()
        const = 0;

    /**
     * @brief Get plugin health status
     * @return Health status information
     */
    virtual QJsonObject get_health_status() const = 0;
};

/**
 * @brief Base implementation of advanced plugin interface
 *
 * Provides default implementations for advanced plugin functionality.
 * Plugins can inherit from this class and override specific methods.
 */
class AdvancedPluginBase : public IAdvancedPlugin {
public:
    AdvancedPluginBase() = default;
    virtual ~AdvancedPluginBase() = default;

    // Default implementations
    qtplugin::expected<void, PluginError> apply_configuration(
        const QJsonObject& config) override;

    QJsonObject get_configuration() const override;

    bool validate_configuration(const QJsonObject& config) const override;

    qtplugin::expected<QVariant, PluginError> handle_event(
        const QString& event_type, const QVariant& event_data) override;

    QStringList supported_event_types() const override;

    qtplugin::expected<void, PluginError> pre_initialize() override;
    qtplugin::expected<void, PluginError> post_initialize() override;
    qtplugin::expected<void, PluginError> pre_shutdown() override;
    qtplugin::expected<void, PluginError> post_shutdown() override;

    QJsonObject get_resource_usage() const override;

    qtplugin::expected<void, PluginError> set_resource_limits(
        const QJsonObject& limits) override;

    qtplugin::expected<QVariant, PluginError> send_message(
        const QString& target_plugin_id, const QVariant& message) override;

    qtplugin::expected<QVariant, PluginError> receive_message(
        const QString& sender_plugin_id, const QVariant& message) override;

    qtplugin::expected<QJsonObject, PluginError> save_state() override;

    qtplugin::expected<void, PluginError> restore_state(
        const QJsonObject& state) override;

    QStringList get_capabilities() const override;

    bool has_capability(const QString& capability) const override;

    qtplugin::expected<void, PluginError> register_service(
        const QString& service_name,
        const QVariant& service_interface) override;

    qtplugin::expected<void, PluginError> unregister_service(
        const QString& service_name) override;

    QStringList get_provided_services() const override;

    QJsonObject get_performance_metrics() const override;

    void reset_metrics() override;

    qtplugin::expected<bool, PluginError> validate_integrity() const override;

    QJsonObject get_health_status() const override;

protected:
    QJsonObject m_configuration;
    QStringList m_capabilities;
    QStringList m_provided_services;
    mutable QJsonObject m_performance_metrics;
    QJsonObject m_resource_limits;
};

}  // namespace qtplugin
