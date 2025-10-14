/**
 * @file plugin_property_system.hpp
 * @brief Qt property system integration for dynamic plugin configuration
 * @version 3.0.0
 */

#pragma once

#include <QEasingCurve>
#include <QJsonArray>
#include <QJsonObject>
#include <QMetaProperty>
#include <QMetaType>
#include <QObject>
#include <QPropertyAnimation>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>
#include "../interfaces/core/plugin_interface.hpp"
#include "../utils/error_handling.hpp"

namespace qtplugin {

/**
 * @brief Property binding types
 */
enum class PropertyBindingType {
    OneWay,  ///< One-way binding (source -> target)
    TwoWay,  ///< Two-way binding (bidirectional)
    OneTime  ///< One-time binding (set once)
};

/**
 * @brief Property validation types
 */
enum class PropertyValidationType {
    None,   ///< No validation
    Range,  ///< Range validation (min/max)
    Enum,   ///< Enumeration validation
    Regex,  ///< Regular expression validation
    Custom  ///< Custom validation function
};

/**
 * @brief Property change notification mode
 */
enum class PropertyNotificationMode {
    Immediate,  ///< Immediate notification
    Debounced,  ///< Debounced notification (delay after last change)
    Throttled,  ///< Throttled notification (maximum frequency)
    Batched     ///< Batched notification (collect multiple changes)
};

/**
 * @brief Property metadata
 */
struct PropertyMetadata {
    QString name;             ///< Property name
    QString display_name;     ///< Display name for UI
    QString description;      ///< Property description
    QString category;         ///< Property category
    QVariant default_value;   ///< Default value
    QVariant minimum_value;   ///< Minimum value (for range validation)
    QVariant maximum_value;   ///< Maximum value (for range validation)
    QStringList enum_values;  ///< Enumeration values
    QString regex_pattern;    ///< Regex pattern for validation
    PropertyValidationType validation_type =
        PropertyValidationType::None;  ///< Validation type
    bool is_required = false;          ///< Whether property is required
    bool is_readonly = false;          ///< Whether property is read-only
    bool is_specialized =
        false;      ///< Whether property is specialized (hidden by default)
    QString units;  ///< Property units (e.g., "ms", "px", "%")
    QJsonObject custom_attributes;  ///< Custom attributes

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static PropertyMetadata from_json(const QJsonObject& json);
};

/**
 * @brief Property binding information
 */
struct PropertyBinding {
    QString binding_id;                ///< Binding identifier
    QString source_plugin_id;          ///< Source plugin ID
    QString source_property;           ///< Source property name
    QString target_plugin_id;          ///< Target plugin ID
    QString target_property;           ///< Target property name
    PropertyBindingType binding_type;  ///< Binding type
    std::function<QVariant(const QVariant&)>
        transform_function;  ///< Value transformation function
    bool is_active = true;   ///< Whether binding is active
    QJsonObject metadata;    ///< Binding metadata

    /**
     * @brief Convert to JSON object (excluding transform function)
     */
    QJsonObject to_json() const;
};

/**
 * @brief Property change event
 */
struct PropertyChangeEvent {
    QString plugin_id;                                ///< Plugin identifier
    QString property_name;                            ///< Property name
    QVariant old_value;                               ///< Previous value
    QVariant new_value;                               ///< New value
    std::chrono::system_clock::time_point timestamp;  ///< Change timestamp
    QString source;  ///< Change source (e.g., "user", "binding", "system")
    QJsonObject metadata;  ///< Additional metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Property validation result
 */
struct PropertyValidationResult {
    bool is_valid = true;      ///< Whether value is valid
    QString error_message;     ///< Error message if invalid
    QVariant corrected_value;  ///< Corrected value if applicable
    QStringList warnings;      ///< Validation warnings

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Property change callback
 */
using PropertyChangeCallback = std::function<void(const PropertyChangeEvent&)>;

/**
 * @brief Property validation callback
 */
using PropertyValidationCallback =
    std::function<PropertyValidationResult(const QVariant&)>;

/**
 * @brief Plugin property system
 *
 * This class provides comprehensive property management for plugins using Qt's
 * property system, including dynamic configuration, property binding,
 * validation, and change notifications.
 */
class PluginPropertySystem : public QObject {
    Q_OBJECT

public:
    explicit PluginPropertySystem(QObject* parent = nullptr);
    ~PluginPropertySystem() override;

    // === Plugin Registration ===

    /**
     * @brief Register plugin for property management
     * @param plugin Plugin to register
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> register_plugin(
        std::shared_ptr<IPlugin> plugin);

    /**
     * @brief Unregister plugin from property management
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_plugin(
        const QString& plugin_id);

    /**
     * @brief Check if plugin is registered
     * @param plugin_id Plugin identifier
     * @return true if plugin is registered
     */
    bool is_plugin_registered(const QString& plugin_id) const;

    // === Property Metadata ===

    /**
     * @brief Set property metadata
     * @param plugin_id Plugin identifier
     * @param property_name Property name
     * @param metadata Property metadata
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_property_metadata(
        const QString& plugin_id, const QString& property_name,
        const PropertyMetadata& metadata);

    /**
     * @brief Get property metadata
     * @param plugin_id Plugin identifier
     * @param property_name Property name
     * @return Property metadata or error
     */
    qtplugin::expected<PropertyMetadata, PluginError> get_property_metadata(
        const QString& plugin_id, const QString& property_name) const;

    /**
     * @brief Get all properties for plugin
     * @param plugin_id Plugin identifier
     * @return Vector of property names
     */
    std::vector<QString> get_plugin_properties(const QString& plugin_id) const;

    /**
     * @brief Get properties by category
     * @param plugin_id Plugin identifier
     * @param category Property category
     * @return Vector of property names
     */
    std::vector<QString> get_properties_by_category(
        const QString& plugin_id, const QString& category) const;

    // === Property Access ===

    /**
     * @brief Get property value
     * @param plugin_id Plugin identifier
     * @param property_name Property name
     * @return Property value or error
     */
    qtplugin::expected<QVariant, PluginError> get_property_value(
        const QString& plugin_id, const QString& property_name) const;

    /**
     * @brief Set property value
     * @param plugin_id Plugin identifier
     * @param property_name Property name
     * @param value Property value
     * @param source Change source identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_property_value(
        const QString& plugin_id, const QString& property_name,
        const QVariant& value, const QString& source = "user");

    /**
     * @brief Set multiple property values
     * @param plugin_id Plugin identifier
     * @param properties Map of property names to values
     * @param source Change source identifier
     * @return Vector of results for each property
     */
    std::vector<qtplugin::expected<void, PluginError>> set_property_values(
        const QString& plugin_id, const QJsonObject& properties,
        const QString& source = "user");

    /**
     * @brief Reset property to default value
     * @param plugin_id Plugin identifier
     * @param property_name Property name
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> reset_property(
        const QString& plugin_id, const QString& property_name);

    // === Property Validation ===

    /**
     * @brief Validate property value
     * @param plugin_id Plugin identifier
     * @param property_name Property name
     * @param value Value to validate
     * @return Validation result
     */
    PropertyValidationResult validate_property_value(
        const QString& plugin_id, const QString& property_name,
        const QVariant& value) const;

    /**
     * @brief Set custom validation callback
     * @param plugin_id Plugin identifier
     * @param property_name Property name
     * @param callback Validation callback
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_property_validator(
        const QString& plugin_id, const QString& property_name,
        PropertyValidationCallback callback);

    /**
     * @brief Remove custom validation callback
     * @param plugin_id Plugin identifier
     * @param property_name Property name
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> remove_property_validator(
        const QString& plugin_id, const QString& property_name);

    // === Property Binding ===

    /**
     * @brief Create property binding
     * @param source_plugin_id Source plugin ID
     * @param source_property Source property name
     * @param target_plugin_id Target plugin ID
     * @param target_property Target property name
     * @param binding_type Binding type
     * @param transform_function Optional value transformation function
     * @return Binding ID or error
     */
    qtplugin::expected<QString, PluginError> create_property_binding(
        const QString& source_plugin_id, const QString& source_property,
        const QString& target_plugin_id, const QString& target_property,
        PropertyBindingType binding_type,
        std::function<QVariant(const QVariant&)> transform_function = nullptr);

    /**
     * @brief Remove property binding
     * @param binding_id Binding identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> remove_property_binding(
        const QString& binding_id);

    /**
     * @brief Get property bindings for plugin
     * @param plugin_id Plugin identifier
     * @return Vector of binding information
     */
    std::vector<PropertyBinding> get_plugin_bindings(
        const QString& plugin_id) const;

    /**
     * @brief Enable/disable property binding
     * @param binding_id Binding identifier
     * @param enabled Whether binding is enabled
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_binding_enabled(
        const QString& binding_id, bool enabled);

    // === Change Notifications ===

    /**
     * @brief Register property change callback
     * @param plugin_id Plugin identifier (empty for all plugins)
     * @param property_name Property name (empty for all properties)
     * @param callback Change callback
     * @param notification_mode Notification mode
     * @param delay_ms Delay for debounced/throttled notifications
     * @return Callback ID for unregistration
     */
    QString register_change_callback(
        const QString& plugin_id, const QString& property_name,
        PropertyChangeCallback callback,
        PropertyNotificationMode notification_mode =
            PropertyNotificationMode::Immediate,
        int delay_ms = 100);

    /**
     * @brief Unregister property change callback
     * @param callback_id Callback identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_change_callback(
        const QString& callback_id);

    // === Configuration Management ===

    /**
     * @brief Export plugin configuration
     * @param plugin_id Plugin identifier
     * @param include_specialized Whether to include specialized properties
     * @return Configuration as JSON object
     */
    QJsonObject export_plugin_configuration(
        const QString& plugin_id, bool include_specialized = false) const;

    /**
     * @brief Import plugin configuration
     * @param plugin_id Plugin identifier
     * @param configuration Configuration as JSON object
     * @param validate Whether to validate values
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> import_plugin_configuration(
        const QString& plugin_id, const QJsonObject& configuration,
        bool validate = true);

    /**
     * @brief Create configuration template
     * @param plugin_id Plugin identifier
     * @return Configuration template with metadata
     */
    QJsonObject create_configuration_template(const QString& plugin_id) const;

signals:
    /**
     * @brief Emitted when property value changes
     * @param event Property change event
     */
    void property_changed(const PropertyChangeEvent& event);

    /**
     * @brief Emitted when property binding is created
     * @param binding_id Binding identifier
     * @param binding Binding information
     */
    void binding_created(const QString& binding_id,
                         const PropertyBinding& binding);

    /**
     * @brief Emitted when property binding is removed
     * @param binding_id Binding identifier
     */
    void binding_removed(const QString& binding_id);

private slots:
    void on_property_changed();
    void on_debounce_timer();
    void on_throttle_timer();

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::PropertyBindingType)
Q_DECLARE_METATYPE(qtplugin::PropertyValidationType)
Q_DECLARE_METATYPE(qtplugin::PropertyNotificationMode)
Q_DECLARE_METATYPE(qtplugin::PropertyMetadata)
Q_DECLARE_METATYPE(qtplugin::PropertyBinding)
Q_DECLARE_METATYPE(qtplugin::PropertyChangeEvent)
Q_DECLARE_METATYPE(qtplugin::PropertyValidationResult)
