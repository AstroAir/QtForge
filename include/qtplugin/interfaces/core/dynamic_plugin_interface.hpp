#pragma once

#include <QJsonObject>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <chrono>
#include <functional>
#include <future>
#include <optional>
#include <vector>
#include "advanced_plugin_interface.hpp"
#include "plugin_interface.hpp"
#include "qtplugin/utils/error_handling.hpp"
#include "qtplugin/utils/version.hpp"

namespace qtplugin {

// Forward declarations
namespace contracts {
struct ServiceContract;
}

// Undefine the enum versions from plugin_interface.hpp to allow struct
// definitions
#ifdef InterfaceCapability
#undef InterfaceCapability
#endif

/**
 * @brief Interface capability descriptor for dynamic plugins
 *
 * Note: This shadows the InterfaceCapability enum from plugin_interface.hpp
 * The struct version is used for dynamic plugin capability negotiation
 */
struct InterfaceCapability {
    QString name;           ///< Capability name
    Version version;        ///< Capability version
    QJsonObject metadata;   ///< Additional metadata
    bool required = false;  ///< Whether capability is required

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static qtplugin::expected<InterfaceCapability, PluginError> from_json(
        const QJsonObject& json);
};

/**
 * @brief Interface descriptor for dynamic plugin interfaces
 *
 * Note: This extends the InterfaceDescriptor struct from plugin_interface.hpp
 * with additional fields for dynamic plugin interface management
 */
struct InterfaceDescriptor {
    QString interface_id;                           ///< Interface identifier
    Version version;                                ///< Interface version
    QString description;                            ///< Interface description
    std::vector<InterfaceCapability> capabilities;  ///< Interface capabilities
    QJsonObject schema;                             ///< Interface schema
    QJsonObject metadata;                           ///< Additional metadata

    /**
     * @brief Check compatibility with another interface
     */
    bool is_compatible_with(const InterfaceDescriptor& other) const;

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static qtplugin::expected<InterfaceDescriptor, PluginError> from_json(
        const QJsonObject& json);
};

/**
 * @brief Interface for dynamically loadable plugins
 */
class DynamicPluginInterface : public IPlugin {
public:
    virtual ~DynamicPluginInterface() = default;

    /**
     * @brief Load plugin from dynamic library
     * @param library_path Path to the plugin library
     * @return True if loaded successfully
     */
    virtual bool load_from_library(const std::string& library_path) = 0;

    /**
     * @brief Unload the plugin
     */
    virtual void unload() = 0;

    /**
     * @brief Check if plugin is loaded
     * @return True if loaded
     */
    virtual bool is_loaded() const = 0;

    /**
     * @brief Get library path
     * @return Path to the loaded library
     */
    virtual std::string library_path() const = 0;
};

/**
 * @brief Dynamic plugin interface with runtime adaptation
 *
 * This interface extends IAdvancedPlugin with capabilities for:
 * - Dynamic interface discovery and adaptation
 * - Runtime method and property access
 * - Event subscription and emission
 * - Service contract management
 * - Capability negotiation
 */
class IDynamicPlugin : public IAdvancedPlugin {
public:
    virtual ~IDynamicPlugin() = default;

    // === Plugin Identity (Convenience Methods) ===

    /**
     * @brief Get plugin unique identifier
     */
    virtual std::string id() const noexcept = 0;

    /**
     * @brief Get plugin name
     */
    virtual std::string_view name() const noexcept = 0;

    /**
     * @brief Get plugin description
     */
    virtual std::string_view description() const noexcept = 0;

    /**
     * @brief Get plugin version
     */
    virtual Version version() const noexcept = 0;

    /**
     * @brief Get plugin author
     */
    virtual std::string_view author() const noexcept = 0;

    /**
     * @brief Get current configuration
     */
    virtual QJsonObject current_configuration() const = 0;

    // === Interface Management ===

    /**
     * @brief Get all interface descriptors supported by this plugin
     */
    virtual std::vector<InterfaceDescriptor> get_interface_descriptors()
        const = 0;

    /**
     * @brief Check if plugin supports a specific interface
     * @param interface_id Interface identifier
     * @param min_version Minimum required version
     */
    virtual bool supports_interface(
        const QString& interface_id,
        const Version& min_version = Version{}) const = 0;

    /**
     * @brief Get descriptor for a specific interface
     * @param interface_id Interface identifier
     */
    virtual std::optional<InterfaceDescriptor> get_interface_descriptor(
        const QString& interface_id) const = 0;

    /**
     * @brief Adapt plugin to a specific interface version
     * @param interface_id Interface identifier
     * @param target_version Target version to adapt to
     */
    virtual qtplugin::expected<void, PluginError> adapt_to_interface(
        const QString& interface_id, const Version& target_version) = 0;

    /**
     * @brief Negotiate capabilities with another plugin
     * @param other_plugin_id Other plugin identifier
     * @param requested_capabilities Requested capabilities
     * @return Negotiated capabilities
     */
    virtual qtplugin::expected<std::vector<InterfaceCapability>, PluginError>
    negotiate_capabilities(
        const QString& other_plugin_id,
        const std::vector<InterfaceCapability>& requested_capabilities) = 0;

    // === Plugin Type and Context ===

    /**
     * @brief Get plugin type
     */
    virtual PluginType get_plugin_type() const = 0;

    /**
     * @brief Get execution context
     */
    virtual PluginExecutionContext get_execution_context() const = 0;

    // === Dynamic Method Invocation ===

    /**
     * @brief Execute code dynamically
     * @param code Code to execute
     * @param context Execution context
     */
    virtual qtplugin::expected<QVariant, PluginError> execute_code(
        const QString& code, const QJsonObject& context = {}) = 0;

    /**
     * @brief Invoke a method dynamically
     * @param method_name Method name
     * @param parameters Method parameters
     * @param interface_id Interface identifier (optional)
     */
    virtual qtplugin::expected<QVariant, PluginError> invoke_method(
        const QString& method_name, const QVariantList& parameters = {},
        const QString& interface_id = {}) = 0;

    /**
     * @brief Get available methods
     * @param interface_id Interface identifier (optional)
     */
    virtual std::vector<QString> get_available_methods(
        const QString& interface_id = {}) const = 0;

    /**
     * @brief Get method signature
     * @param method_name Method name
     * @param interface_id Interface identifier (optional)
     */
    virtual std::optional<QJsonObject> get_method_signature(
        const QString& method_name, const QString& interface_id = {}) const = 0;

    // === Dynamic Property Access ===

    /**
     * @brief Get property value
     * @param property_name Property name
     * @param interface_id Interface identifier (optional)
     */
    virtual qtplugin::expected<QVariant, PluginError> get_property(
        const QString& property_name, const QString& interface_id = {}) = 0;

    /**
     * @brief Set property value
     * @param property_name Property name
     * @param value Property value
     * @param interface_id Interface identifier (optional)
     */
    virtual qtplugin::expected<void, PluginError> set_property(
        const QString& property_name, const QVariant& value,
        const QString& interface_id = {}) = 0;

    /**
     * @brief Get available properties
     * @param interface_id Interface identifier (optional)
     */
    virtual std::vector<QString> get_available_properties(
        const QString& interface_id = {}) const = 0;

    // === Event System ===

    /**
     * @brief Subscribe to events
     * @param event_source Event source identifier
     * @param event_types Event types to subscribe to
     * @param callback Event callback
     */
    virtual qtplugin::expected<void, PluginError> subscribe_to_events(
        const QString& event_source, const std::vector<QString>& event_types,
        std::function<void(const QString&, const QJsonObject&)> callback) = 0;

    /**
     * @brief Unsubscribe from events
     * @param event_source Event source identifier
     * @param event_types Event types to unsubscribe from
     */
    virtual qtplugin::expected<void, PluginError> unsubscribe_from_events(
        const QString& event_source,
        const std::vector<QString>& event_types) = 0;

    /**
     * @brief Emit an event
     * @param event_type Event type
     * @param event_data Event data
     */
    virtual qtplugin::expected<void, PluginError> emit_event(
        const QString& event_type, const QJsonObject& event_data) = 0;

    // === Service Contracts ===

    /**
     * @brief Get service contracts
     */
    virtual std::vector<contracts::ServiceContract> get_service_contracts()
        const = 0;

    /**
     * @brief Call a service
     * @param service_name Service name
     * @param method_name Method name
     * @param parameters Method parameters
     * @param timeout Timeout duration
     */
    virtual qtplugin::expected<QJsonObject, PluginError> call_service(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters,
        std::chrono::milliseconds timeout = std::chrono::seconds(30)) = 0;

    /**
     * @brief Call a service asynchronously
     * @param service_name Service name
     * @param method_name Method name
     * @param parameters Method parameters
     * @param timeout Timeout duration
     */
    virtual std::future<qtplugin::expected<QJsonObject, PluginError>>
    call_service_async(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters,
        std::chrono::milliseconds timeout = std::chrono::seconds(30)) = 0;

    /**
     * @brief Handle a service call
     * @param service_name Service name
     * @param method_name Method name
     * @param parameters Method parameters
     */
    virtual qtplugin::expected<QJsonObject, PluginError> handle_service_call(
        const QString& service_name, const QString& method_name,
        const QJsonObject& parameters) = 0;
};

}  // namespace qtplugin
