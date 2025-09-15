/**
 * @file dynamic_plugin_interface.hpp
 * @brief Dynamic plugin interface system with runtime adaptation
 * @version 3.2.0
 * @author QtPlugin Development Team
 *
 * This file provides a dynamic plugin interface system that supports:
 * - Runtime interface discovery and adaptation
 * - Interface versioning and compatibility
 * - Optional interface extensions
 * - Capability negotiation
 * - Multi-language plugin support
 */

#pragma once

#include <QJsonObject>
#include <QString>
#include <QVariant>
#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "../utils/error_handling.hpp"
#include "../utils/version.hpp"
#include "plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Interface capability descriptor
 */
struct InterfaceCapability {
    QString name;           ///< Capability name
    Version version;        ///< Capability version
    QJsonObject metadata;   ///< Additional metadata
    bool required = false;  ///< Whether this capability is required

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static qtplugin::expected<InterfaceCapability, PluginError> from_json(
        const QJsonObject& json);
};

/**
 * @brief Interface descriptor for dynamic interfaces
 */
struct InterfaceDescriptor {
    QString interface_id;  ///< Unique interface identifier
    Version version;       ///< Interface version
    QString description;   ///< Human-readable description
    std::vector<InterfaceCapability> capabilities;  ///< Supported capabilities
    QJsonObject schema;    ///< Interface schema (JSON Schema)
    QJsonObject metadata;  ///< Additional metadata

    /**
     * @brief Check if this interface is compatible with another
     */
    bool is_compatible_with(const InterfaceDescriptor& other) const;

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON representation
     */
    static qtplugin::expected<InterfaceDescriptor, PluginError> from_json(
        const QJsonObject& json);
};

/**
 * @brief Plugin type enumeration for multi-language support
 */
enum class PluginType {
    Native,      ///< Native C++ plugin
    Python,      ///< Python script plugin
    JavaScript,  ///< JavaScript plugin
    Lua,         ///< Lua script plugin
    Remote,      ///< Remote plugin (network-based)
    Composite    ///< Composite plugin (combination of others)
};

/**
 * @brief Plugin execution context for different plugin types
 */
struct PluginExecutionContext {
    PluginType type = PluginType::Native;
    QString interpreter_path;  ///< Path to interpreter (for scripted plugins)
    QJsonObject environment;   ///< Environment variables
    QJsonObject security_policy;               ///< Security policy settings
    std::chrono::milliseconds timeout{30000};  ///< Execution timeout

    /**
     * @brief Convert to JSON representation
     */
    QJsonObject to_json() const;
};

/**
 * @brief Dynamic plugin interface with runtime adaptation
 */
class IDynamicPlugin : public virtual IPlugin {
public:
    ~IDynamicPlugin() override = default;

    // === Interface Discovery ===

    /**
     * @brief Get supported interface descriptors
     * @return Vector of interface descriptors
     */
    virtual std::vector<InterfaceDescriptor> get_interface_descriptors()
        const = 0;

    /**
     * @brief Check if plugin supports a specific interface
     * @param interface_id Interface identifier
     * @param min_version Minimum required version
     * @return True if interface is supported
     */
    virtual bool supports_interface(
        const QString& interface_id,
        const Version& min_version = Version{}) const = 0;

    /**
     * @brief Get interface descriptor by ID
     * @param interface_id Interface identifier
     * @return Interface descriptor or nullopt if not found
     */
    virtual std::optional<InterfaceDescriptor> get_interface_descriptor(
        const QString& interface_id) const = 0;

    // === Runtime Adaptation ===

    /**
     * @brief Adapt to a specific interface version
     * @param interface_id Interface identifier
     * @param target_version Target version to adapt to
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> adapt_to_interface(
        const QString& interface_id, const Version& target_version) = 0;

    /**
     * @brief Negotiate capabilities with another plugin
     * @param other_plugin_id Other plugin identifier
     * @param requested_capabilities Requested capabilities
     * @return Negotiated capabilities or error
     */
    virtual qtplugin::expected<std::vector<InterfaceCapability>, PluginError>
    negotiate_capabilities(
        const QString& other_plugin_id,
        const std::vector<InterfaceCapability>& requested_capabilities) = 0;

    // === Multi-Language Support ===

    /**
     * @brief Get plugin type
     * @return Plugin type
     */
    virtual PluginType get_plugin_type() const = 0;

    /**
     * @brief Get execution context
     * @return Plugin execution context
     */
    virtual PluginExecutionContext get_execution_context() const = 0;

    /**
     * @brief Execute code in plugin's runtime environment
     * @param code Code to execute
     * @param context Execution context
     * @return Execution result or error
     */
    virtual qtplugin::expected<QVariant, PluginError> execute_code(
        const QString& code, const QJsonObject& context = {}) = 0;

    // === Dynamic Method Invocation ===

    /**
     * @brief Invoke a method dynamically
     * @param method_name Method name
     * @param parameters Method parameters
     * @param interface_id Target interface (optional)
     * @return Method result or error
     */
    virtual qtplugin::expected<QVariant, PluginError> invoke_method(
        const QString& method_name, const QVariantList& parameters = {},
        const QString& interface_id = {}) = 0;

    /**
     * @brief Get available methods for an interface
     * @param interface_id Interface identifier
     * @return Vector of method names
     */
    virtual std::vector<QString> get_available_methods(
        const QString& interface_id = {}) const = 0;

    /**
     * @brief Get method signature
     * @param method_name Method name
     * @param interface_id Interface identifier
     * @return Method signature as JSON schema
     */
    virtual std::optional<QJsonObject> get_method_signature(
        const QString& method_name, const QString& interface_id = {}) const = 0;

    // === Property System ===

    /**
     * @brief Get property value
     * @param property_name Property name
     * @param interface_id Interface identifier
     * @return Property value or error
     */
    virtual qtplugin::expected<QVariant, PluginError> get_property(
        const QString& property_name, const QString& interface_id = {}) = 0;

    /**
     * @brief Set property value
     * @param property_name Property name
     * @param value Property value
     * @param interface_id Interface identifier
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> set_property(
        const QString& property_name, const QVariant& value,
        const QString& interface_id = {}) = 0;

    /**
     * @brief Get available properties for an interface
     * @param interface_id Interface identifier
     * @return Vector of property names
     */
    virtual std::vector<QString> get_available_properties(
        const QString& interface_id = {}) const = 0;

    // === Event System ===

    /**
     * @brief Subscribe to events from another plugin
     * @param source_plugin_id Source plugin identifier
     * @param event_types Event types to subscribe to
     * @param callback Event callback function
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> subscribe_to_events(
        const QString& source_plugin_id,
        const std::vector<QString>& event_types,
        std::function<void(const QString&, const QJsonObject&)> callback) = 0;

    /**
     * @brief Unsubscribe from events
     * @param source_plugin_id Source plugin identifier
     * @param event_types Event types to unsubscribe from
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> unsubscribe_from_events(
        const QString& source_plugin_id,
        const std::vector<QString>& event_types = {}) = 0;

    /**
     * @brief Emit an event
     * @param event_type Event type
     * @param event_data Event data
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> emit_event(
        const QString& event_type, const QJsonObject& event_data) = 0;
};

/**
 * @brief Interface registry for managing dynamic interfaces
 */
class InterfaceRegistry {
public:
    static InterfaceRegistry& instance();

    /**
     * @brief Register an interface descriptor
     * @param descriptor Interface descriptor
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> register_interface(
        const InterfaceDescriptor& descriptor);

    /**
     * @brief Unregister an interface
     * @param interface_id Interface identifier
     */
    void unregister_interface(const QString& interface_id);

    /**
     * @brief Get interface descriptor
     * @param interface_id Interface identifier
     * @return Interface descriptor or nullopt if not found
     */
    std::optional<InterfaceDescriptor> get_interface(
        const QString& interface_id) const;

    /**
     * @brief Find compatible interfaces
     * @param requirements Interface requirements
     * @return Vector of compatible interface descriptors
     */
    std::vector<InterfaceDescriptor> find_compatible_interfaces(
        const InterfaceDescriptor& requirements) const;

    /**
     * @brief Get all registered interfaces
     * @return Vector of all interface descriptors
     */
    std::vector<InterfaceDescriptor> get_all_interfaces() const;

private:
    InterfaceRegistry() = default;
    std::unordered_map<QString, InterfaceDescriptor> m_interfaces;
    mutable std::shared_mutex m_mutex;
};

/**
 * @brief Plugin type utilities
 */
class PluginTypeUtils {
public:
    /**
     * @brief Convert plugin type to string
     */
    static QString plugin_type_to_string(PluginType type);

    /**
     * @brief Convert string to plugin type
     */
    static std::optional<PluginType> string_to_plugin_type(const QString& str);

    /**
     * @brief Check if plugin type supports feature
     */
    static bool supports_feature(PluginType type, const QString& feature);

    /**
     * @brief Get default execution context for plugin type
     */
    static PluginExecutionContext get_default_context(PluginType type);
};

}  // namespace qtplugin

Q_DECLARE_INTERFACE(qtplugin::IDynamicPlugin, "qtplugin.IDynamicPlugin/3.2")
