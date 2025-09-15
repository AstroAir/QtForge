/**
 * @file plugin_capability_discovery.hpp
 * @brief Plugin capability discovery using Qt's meta-object system
 * @version 3.0.0
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "../utils/error_handling.hpp"
#include "plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Plugin capability information
 */
struct PluginCapabilityInfo {
    QString name;                      ///< Capability name
    QString description;               ///< Capability description
    PluginCapability capability_flag;  ///< Capability flag
    QStringList required_methods;      ///< Required methods
    QStringList optional_methods;      ///< Optional methods
    QStringList required_properties;   ///< Required properties
    QStringList optional_properties;   ///< Optional properties
    QJsonObject metadata;              ///< Additional metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static PluginCapabilityInfo from_json(const QJsonObject& json);
};

/**
 * @brief Plugin method information
 */
struct PluginMethodInfo {
    QString name;                 ///< Method name
    QString signature;            ///< Method signature
    QString return_type;          ///< Return type
    QStringList parameter_types;  ///< Parameter types
    QStringList parameter_names;  ///< Parameter names
    bool is_invokable = false;    ///< Whether method is invokable
    bool is_slot = false;         ///< Whether method is a slot
    bool is_signal = false;       ///< Whether method is a signal
    QMetaMethod::Access access = QMetaMethod::Public;  ///< Method access level
    QJsonObject annotations;                           ///< Method annotations

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Plugin property information
 */
struct PluginPropertyInfo {
    QString name;                    ///< Property name
    QString type;                    ///< Property type
    QVariant default_value;          ///< Default value
    bool is_readable = true;         ///< Whether property is readable
    bool is_writable = true;         ///< Whether property is writable
    bool is_resettable = false;      ///< Whether property is resettable
    bool has_notify_signal = false;  ///< Whether property has notify signal
    QString notify_signal;           ///< Notify signal name
    QJsonObject annotations;         ///< Property annotations

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Plugin interface information
 */
struct PluginInterfaceInfo {
    QString interface_id;                        ///< Interface identifier
    QString interface_name;                      ///< Interface name
    QString version;                             ///< Interface version
    QStringList parent_interfaces;               ///< Parent interfaces
    std::vector<PluginMethodInfo> methods;       ///< Interface methods
    std::vector<PluginPropertyInfo> properties;  ///< Interface properties
    QJsonObject metadata;                        ///< Interface metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Plugin discovery result
 */
struct PluginDiscoveryResult {
    QString plugin_id;                ///< Plugin identifier
    QString plugin_name;              ///< Plugin name
    PluginCapabilities capabilities;  ///< Plugin capabilities
    std::vector<PluginCapabilityInfo>
        capability_details;                       ///< Detailed capability info
    std::vector<PluginInterfaceInfo> interfaces;  ///< Implemented interfaces
    std::vector<PluginMethodInfo> methods;        ///< Available methods
    std::vector<PluginPropertyInfo> properties;   ///< Available properties
    QJsonObject metadata;                         ///< Discovery metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Plugin capability matcher
 */
class PluginCapabilityMatcher {
public:
    /**
     * @brief Match plugins by capability
     * @param required_capability Required capability
     * @param plugins Vector of plugins to match
     * @return Vector of matching plugins
     */
    static std::vector<std::shared_ptr<IPlugin>> match_by_capability(
        PluginCapability required_capability,
        const std::vector<std::shared_ptr<IPlugin>>& plugins);

    /**
     * @brief Match plugins by multiple capabilities
     * @param required_capabilities Required capabilities (bitfield)
     * @param plugins Vector of plugins to match
     * @param require_all Whether all capabilities are required (AND) or any
     * (OR)
     * @return Vector of matching plugins
     */
    static std::vector<std::shared_ptr<IPlugin>> match_by_capabilities(
        PluginCapabilities required_capabilities,
        const std::vector<std::shared_ptr<IPlugin>>& plugins,
        bool require_all = true);

    /**
     * @brief Match plugins by interface
     * @param interface_id Interface identifier
     * @param plugins Vector of plugins to match
     * @return Vector of matching plugins
     */
    static std::vector<std::shared_ptr<IPlugin>> match_by_interface(
        const QString& interface_id,
        const std::vector<std::shared_ptr<IPlugin>>& plugins);

    /**
     * @brief Match plugins by method
     * @param method_signature Method signature
     * @param plugins Vector of plugins to match
     * @return Vector of matching plugins
     */
    static std::vector<std::shared_ptr<IPlugin>> match_by_method(
        const QString& method_signature,
        const std::vector<std::shared_ptr<IPlugin>>& plugins);

    /**
     * @brief Score plugin compatibility
     * @param plugin Plugin to score
     * @param requirements Requirements to match against
     * @return Compatibility score (0-100)
     */
    static int score_compatibility(std::shared_ptr<IPlugin> plugin,
                                   const QJsonObject& requirements);
};

/**
 * @brief Plugin capability discovery engine
 */
class PluginCapabilityDiscovery : public QObject {
    Q_OBJECT

public:
    explicit PluginCapabilityDiscovery(QObject* parent = nullptr);
    ~PluginCapabilityDiscovery() override;

    // === Discovery Operations ===

    /**
     * @brief Discover plugin capabilities
     * @param plugin Plugin to analyze
     * @return Discovery result or error
     */
    qtplugin::expected<PluginDiscoveryResult, PluginError>
    discover_capabilities(std::shared_ptr<IPlugin> plugin);

    /**
     * @brief Discover capabilities for multiple plugins
     * @param plugins Vector of plugins to analyze
     * @return Vector of discovery results
     */
    std::vector<PluginDiscoveryResult> discover_batch_capabilities(
        const std::vector<std::shared_ptr<IPlugin>>& plugins);

    /**
     * @brief Analyze plugin interface
     * @param plugin Plugin to analyze
     * @param interface_id Interface identifier
     * @return Interface information or error
     */
    qtplugin::expected<PluginInterfaceInfo, PluginError> analyze_interface(
        std::shared_ptr<IPlugin> plugin, const QString& interface_id);

    /**
     * @brief Get plugin methods
     * @param plugin Plugin to analyze
     * @return Vector of method information
     */
    std::vector<PluginMethodInfo> get_plugin_methods(
        std::shared_ptr<IPlugin> plugin);

    /**
     * @brief Get plugin properties
     * @param plugin Plugin to analyze
     * @return Vector of property information
     */
    std::vector<PluginPropertyInfo> get_plugin_properties(
        std::shared_ptr<IPlugin> plugin);

    // === Capability Registry ===

    /**
     * @brief Register capability definition
     * @param capability_info Capability information
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> register_capability(
        const PluginCapabilityInfo& capability_info);

    /**
     * @brief Unregister capability definition
     * @param capability_name Capability name
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_capability(
        const QString& capability_name);

    /**
     * @brief Get registered capabilities
     * @return Vector of registered capability information
     */
    std::vector<PluginCapabilityInfo> get_registered_capabilities() const;

    /**
     * @brief Get capability definition
     * @param capability_name Capability name
     * @return Capability information or error
     */
    qtplugin::expected<PluginCapabilityInfo, PluginError>
    get_capability_definition(const QString& capability_name) const;

    // === Interface Registry ===

    /**
     * @brief Register interface definition
     * @param interface_info Interface information
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> register_interface(
        const PluginInterfaceInfo& interface_info);

    /**
     * @brief Unregister interface definition
     * @param interface_id Interface identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_interface(
        const QString& interface_id);

    /**
     * @brief Get registered interfaces
     * @return Vector of registered interface information
     */
    std::vector<PluginInterfaceInfo> get_registered_interfaces() const;

    // === Validation ===

    /**
     * @brief Validate plugin against capability requirements
     * @param plugin Plugin to validate
     * @param capability_name Capability name
     * @return Validation result or error
     */
    qtplugin::expected<bool, PluginError> validate_capability(
        std::shared_ptr<IPlugin> plugin, const QString& capability_name);

    /**
     * @brief Validate plugin against interface requirements
     * @param plugin Plugin to validate
     * @param interface_id Interface identifier
     * @return Validation result or error
     */
    qtplugin::expected<bool, PluginError> validate_interface(
        std::shared_ptr<IPlugin> plugin, const QString& interface_id);

    // === Introspection ===

    /**
     * @brief Invoke plugin method dynamically
     * @param plugin Plugin instance
     * @param method_name Method name
     * @param arguments Method arguments
     * @return Method result or error
     */
    qtplugin::expected<QVariant, PluginError> invoke_method(
        std::shared_ptr<IPlugin> plugin, const QString& method_name,
        const QVariantList& arguments = {});

    /**
     * @brief Get plugin property value
     * @param plugin Plugin instance
     * @param property_name Property name
     * @return Property value or error
     */
    qtplugin::expected<QVariant, PluginError> get_property(
        std::shared_ptr<IPlugin> plugin, const QString& property_name);

    /**
     * @brief Set plugin property value
     * @param plugin Plugin instance
     * @param property_name Property name
     * @param value Property value
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_property(
        std::shared_ptr<IPlugin> plugin, const QString& property_name,
        const QVariant& value);

signals:
    /**
     * @brief Emitted when capability discovery is completed
     * @param plugin_id Plugin identifier
     * @param result Discovery result
     */
    void capability_discovered(const QString& plugin_id,
                               const PluginDiscoveryResult& result);

    /**
     * @brief Emitted when capability registration changes
     * @param capability_name Capability name
     * @param registered Whether capability was registered or unregistered
     */
    void capability_registration_changed(const QString& capability_name,
                                         bool registered);

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::PluginCapabilityInfo)
Q_DECLARE_METATYPE(qtplugin::PluginMethodInfo)
Q_DECLARE_METATYPE(qtplugin::PluginPropertyInfo)
Q_DECLARE_METATYPE(qtplugin::PluginInterfaceInfo)
Q_DECLARE_METATYPE(qtplugin::PluginDiscoveryResult)
