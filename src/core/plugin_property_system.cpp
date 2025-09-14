/**
 * @file plugin_property_system.cpp
 * @brief Implementation of Qt property system integration for plugins
 * @version 3.0.0
 */

#include "qtplugin/core/plugin_property_system.hpp"
#include <QDateTime>
#include <QLoggingCategory>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMutex>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QTimer>
#include <QUuid>
#include <algorithm>
#include <memory>
#include <unordered_map>

Q_LOGGING_CATEGORY(propertySystemLog, "qtplugin.property.system")

namespace qtplugin {

// === PropertyMetadata Implementation ===

QJsonObject PropertyMetadata::to_json() const {
    QJsonObject json;
    json["name"] = name;
    json["display_name"] = display_name;
    json["description"] = description;
    json["category"] = category;
    json["default_value"] = QJsonValue::fromVariant(default_value);
    json["minimum_value"] = QJsonValue::fromVariant(minimum_value);
    json["maximum_value"] = QJsonValue::fromVariant(maximum_value);
    json["enum_values"] = QJsonArray::fromStringList(enum_values);
    json["regex_pattern"] = regex_pattern;
    json["validation_type"] = static_cast<int>(validation_type);
    json["is_required"] = is_required;
    json["is_readonly"] = is_readonly;
    json["is_advanced"] = is_advanced;
    json["units"] = units;
    json["custom_attributes"] = custom_attributes;
    return json;
}

PropertyMetadata PropertyMetadata::from_json(const QJsonObject& json) {
    PropertyMetadata metadata;
    metadata.name = json["name"].toString();
    metadata.display_name = json["display_name"].toString();
    metadata.description = json["description"].toString();
    metadata.category = json["category"].toString();
    metadata.default_value = json["default_value"].toVariant();
    metadata.minimum_value = json["minimum_value"].toVariant();
    metadata.maximum_value = json["maximum_value"].toVariant();

    auto enum_array = json["enum_values"].toArray();
    for (const auto& value : enum_array) {
        metadata.enum_values.append(value.toString());
    }

    metadata.regex_pattern = json["regex_pattern"].toString();
    metadata.validation_type =
        static_cast<PropertyValidationType>(json["validation_type"].toInt());
    metadata.is_required = json["is_required"].toBool();
    metadata.is_readonly = json["is_readonly"].toBool();
    metadata.is_advanced = json["is_advanced"].toBool();
    metadata.units = json["units"].toString();
    metadata.custom_attributes = json["custom_attributes"].toObject();

    return metadata;
}

// === PropertyBinding Implementation ===

QJsonObject PropertyBinding::to_json() const {
    QJsonObject json;
    json["binding_id"] = binding_id;
    json["source_plugin_id"] = source_plugin_id;
    json["source_property"] = source_property;
    json["target_plugin_id"] = target_plugin_id;
    json["target_property"] = target_property;
    json["binding_type"] = static_cast<int>(binding_type);
    json["is_active"] = is_active;
    json["metadata"] = metadata;
    return json;
}

// === PropertyChangeEvent Implementation ===

QJsonObject PropertyChangeEvent::to_json() const {
    QJsonObject json;
    json["plugin_id"] = plugin_id;
    json["property_name"] = property_name;
    json["old_value"] = QJsonValue::fromVariant(old_value);
    json["new_value"] = QJsonValue::fromVariant(new_value);
    json["timestamp"] = QDateTime::fromSecsSinceEpoch(
                            std::chrono::duration_cast<std::chrono::seconds>(
                                timestamp.time_since_epoch())
                                .count())
                            .toString(Qt::ISODate);
    json["source"] = source;
    json["metadata"] = metadata;
    return json;
}

// === PropertyValidationResult Implementation ===

QJsonObject PropertyValidationResult::to_json() const {
    QJsonObject json;
    json["is_valid"] = is_valid;
    json["error_message"] = error_message;
    json["corrected_value"] = QJsonValue::fromVariant(corrected_value);
    json["warnings"] = QJsonArray::fromStringList(warnings);
    return json;
}

// === PluginPropertySystem Private Implementation ===

struct PluginPropertyInfo {
    std::shared_ptr<IPlugin> plugin;
    QObject* plugin_object = nullptr;
    std::unordered_map<QString, PropertyMetadata> property_metadata;
    std::unordered_map<QString, PropertyValidationCallback> custom_validators;
    std::vector<PropertyChangeEvent> change_history;
};

struct PropertyChangeCallbackInfo {
    QString id;
    QString plugin_id_filter;
    QString property_name_filter;
    PropertyChangeCallback callback;
    PropertyNotificationMode notification_mode;
    int delay_ms;
    std::unique_ptr<QTimer> debounce_timer;
    std::unique_ptr<QTimer> throttle_timer;
    std::vector<PropertyChangeEvent> pending_events;
    std::chrono::steady_clock::time_point last_notification;
};

class PluginPropertySystem::Private {
public:
    mutable QMutex mutex;
    std::unordered_map<QString, std::unique_ptr<PluginPropertyInfo>> plugins;
    std::unordered_map<QString, PropertyBinding> property_bindings;
    std::unordered_map<QString, std::unique_ptr<PropertyChangeCallbackInfo>>
        change_callbacks;

    PropertyValidationResult validate_property_internal(
        const PropertyMetadata& metadata, const QVariant& value,
        PropertyValidationCallback custom_validator = nullptr);
    void notify_property_change(const PropertyChangeEvent& event);
    void execute_property_bindings(const QString& source_plugin_id,
                                   const QString& source_property);
    void setup_property_monitoring(PluginPropertyInfo* info);
    std::vector<QString> discover_plugin_properties(QObject* plugin_object);
    PropertyMetadata create_default_metadata(QObject* plugin_object,
                                             const QString& property_name);
};

PropertyValidationResult
PluginPropertySystem::Private::validate_property_internal(
    const PropertyMetadata& metadata, const QVariant& value,
    PropertyValidationCallback custom_validator) {
    PropertyValidationResult result;
    result.is_valid = true;

    // Custom validation first
    if (custom_validator) {
        result = custom_validator(value);
        if (!result.is_valid) {
            return result;
        }
    }

    // Built-in validation
    switch (metadata.validation_type) {
        case PropertyValidationType::None:
            break;

        case PropertyValidationType::Range:
            if (metadata.minimum_value.isValid() &&
                value < metadata.minimum_value) {
                result.is_valid = false;
                result.error_message =
                    QString("Value %1 is below minimum %2")
                        .arg(value.toString())
                        .arg(metadata.minimum_value.toString());
                result.corrected_value = metadata.minimum_value;
            } else if (metadata.maximum_value.isValid() &&
                       value > metadata.maximum_value) {
                result.is_valid = false;
                result.error_message =
                    QString("Value %1 is above maximum %2")
                        .arg(value.toString())
                        .arg(metadata.maximum_value.toString());
                result.corrected_value = metadata.maximum_value;
            }
            break;

        case PropertyValidationType::Enum:
            if (!metadata.enum_values.contains(value.toString())) {
                result.is_valid = false;
                result.error_message =
                    QString("Value '%1' is not in allowed values: %2")
                        .arg(value.toString())
                        .arg(metadata.enum_values.join(", "));
                if (!metadata.enum_values.isEmpty()) {
                    result.corrected_value = metadata.enum_values.first();
                }
            }
            break;

        case PropertyValidationType::Regex:
            if (!metadata.regex_pattern.isEmpty()) {
                QRegularExpression regex(metadata.regex_pattern);
                if (!regex.match(value.toString()).hasMatch()) {
                    result.is_valid = false;
                    result.error_message =
                        QString("Value '%1' does not match pattern '%2'")
                            .arg(value.toString())
                            .arg(metadata.regex_pattern);
                }
            }
            break;

        case PropertyValidationType::Custom:
            // Already handled above
            break;
    }

    return result;
}

void PluginPropertySystem::Private::notify_property_change(
    const PropertyChangeEvent& event) {
    for (const auto& [callback_id, callback_info] : change_callbacks) {
        bool should_notify = false;

        // Check filters
        if (callback_info->plugin_id_filter.isEmpty() ||
            callback_info->plugin_id_filter == event.plugin_id) {
            if (callback_info->property_name_filter.isEmpty() ||
                callback_info->property_name_filter == event.property_name) {
                should_notify = true;
            }
        }

        if (!should_notify)
            continue;

        // Handle notification mode
        switch (callback_info->notification_mode) {
            case PropertyNotificationMode::Immediate:
                if (callback_info->callback) {
                    callback_info->callback(event);
                }
                break;

            case PropertyNotificationMode::Debounced:
                callback_info->pending_events.clear();
                callback_info->pending_events.push_back(event);
                if (callback_info->debounce_timer) {
                    callback_info->debounce_timer->start();
                }
                break;

            case PropertyNotificationMode::Throttled: {
                auto now = std::chrono::steady_clock::now();
                auto time_since_last =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - callback_info->last_notification);

                if (time_since_last.count() >= callback_info->delay_ms) {
                    if (callback_info->callback) {
                        callback_info->callback(event);
                    }
                    callback_info->last_notification = now;
                } else {
                    // Schedule for later
                    callback_info->pending_events.clear();
                    callback_info->pending_events.push_back(event);
                    if (callback_info->throttle_timer) {
                        callback_info->throttle_timer->start();
                    }
                }
                break;
            }

            case PropertyNotificationMode::Batched:
                callback_info->pending_events.push_back(event);
                if (callback_info->debounce_timer) {
                    callback_info->debounce_timer->start();
                }
                break;
        }
    }
}

void PluginPropertySystem::Private::execute_property_bindings(
    const QString& source_plugin_id, const QString& source_property) {
    for (const auto& [binding_id, binding] : property_bindings) {
        if (!binding.is_active)
            continue;

        if (binding.source_plugin_id == source_plugin_id &&
            binding.source_property == source_property) {
            // Get source value
            auto source_it = plugins.find(source_plugin_id);
            if (source_it == plugins.end())
                continue;

            QObject* source_obj = source_it->second->plugin_object;
            if (!source_obj)
                continue;

            QVariant source_value =
                source_obj->property(source_property.toUtf8().constData());

            // Apply transformation if available
            QVariant target_value = source_value;
            if (binding.transform_function) {
                try {
                    target_value = binding.transform_function(source_value);
                } catch (const std::exception& e) {
                    qCWarning(propertySystemLog)
                        << "Property binding transformation failed:"
                        << binding_id << "error:" << e.what();
                    continue;
                } catch (...) {
                    qCWarning(propertySystemLog)
                        << "Unknown exception in property binding "
                           "transformation:"
                        << binding_id;
                    continue;
                }
            }

            // Set target value
            auto target_it = plugins.find(binding.target_plugin_id);
            if (target_it == plugins.end())
                continue;

            QObject* target_obj = target_it->second->plugin_object;
            if (!target_obj)
                continue;

            bool success = target_obj->setProperty(
                binding.target_property.toUtf8().constData(), target_value);
            if (!success) {
                qCWarning(propertySystemLog)
                    << "Failed to set property in binding:" << binding_id
                    << "target:" << binding.target_plugin_id
                    << "property:" << binding.target_property;
            }
        }
    }
}

void PluginPropertySystem::Private::setup_property_monitoring(
    PluginPropertyInfo* info) {
    if (!info || !info->plugin_object)
        return;

    // Connect to property change signals
    const QMetaObject* meta = info->plugin_object->metaObject();
    for (int i = 0; i < meta->propertyCount(); ++i) {
        QMetaProperty property = meta->property(i);
        if (property.hasNotifySignal()) {
            QMetaMethod notify_signal = property.notifySignal();

            // Connect to a generic slot that handles property changes
            // Note: This is a simplified approach - in practice, you'd need
            // more sophisticated signal handling
            qCDebug(propertySystemLog)
                << "Property" << property.name()
                << "has notify signal:" << notify_signal.name();
        }
    }
}

std::vector<QString> PluginPropertySystem::Private::discover_plugin_properties(
    QObject* plugin_object) {
    std::vector<QString> properties;

    if (!plugin_object)
        return properties;

    const QMetaObject* meta = plugin_object->metaObject();
    for (int i = 0; i < meta->propertyCount(); ++i) {
        QMetaProperty property = meta->property(i);
        properties.push_back(QString(property.name()));
    }

    return properties;
}

PropertyMetadata PluginPropertySystem::Private::create_default_metadata(
    QObject* plugin_object, const QString& property_name) {
    PropertyMetadata metadata;
    metadata.name = property_name;
    metadata.display_name = property_name;
    metadata.description = QString("Property %1").arg(property_name);
    metadata.category = "General";

    if (!plugin_object)
        return metadata;

    const QMetaObject* meta = plugin_object->metaObject();
    int property_index =
        meta->indexOfProperty(property_name.toUtf8().constData());

    if (property_index >= 0) {
        QMetaProperty property = meta->property(property_index);
        metadata.default_value = plugin_object->property(property.name());
        metadata.is_readonly = !property.isWritable();

        // Try to infer validation type from property type
        QString type_name = QString(property.typeName());
        if (type_name == "int" || type_name == "double" ||
            type_name == "float") {
            metadata.validation_type = PropertyValidationType::Range;
        } else if (type_name.contains("Enum") || property.isEnumType()) {
            metadata.validation_type = PropertyValidationType::Enum;

            // Get enum values if available
            if (property.isEnumType()) {
                QMetaEnum meta_enum = property.enumerator();
                for (int i = 0; i < meta_enum.keyCount(); ++i) {
                    metadata.enum_values.append(QString(meta_enum.key(i)));
                }
            }
        }
    }

    return metadata;
}

// === PluginPropertySystem Implementation ===

PluginPropertySystem::PluginPropertySystem(QObject* parent)
    : QObject(parent), d(std::make_unique<Private>()) {
    qCDebug(propertySystemLog) << "Plugin property system initialized";
}

PluginPropertySystem::~PluginPropertySystem() = default;

qtplugin::expected<void, PluginError> PluginPropertySystem::register_plugin(
    std::shared_ptr<IPlugin> plugin) {
    if (!plugin) {
        return make_error<void>(PluginErrorCode::InvalidArgument,
                                "Plugin is null");
    }

    QString plugin_id = QString::fromStdString(plugin->id());

    QMutexLocker locker(&d->mutex);

    // Check if already registered
    if (d->plugins.find(plugin_id) != d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::AlreadyExists,
            "Plugin already registered: " + plugin_id.toStdString());
    }

    // Create plugin property info
    auto info = std::make_unique<PluginPropertyInfo>();
    info->plugin = plugin;
    info->plugin_object = dynamic_cast<QObject*>(plugin.get());

    if (info->plugin_object) {
        // Discover properties and create default metadata
        auto properties = d->discover_plugin_properties(info->plugin_object);
        for (const QString& property_name : properties) {
            PropertyMetadata metadata =
                d->create_default_metadata(info->plugin_object, property_name);
            info->property_metadata[property_name] = metadata;
        }

        // Set up property monitoring
        d->setup_property_monitoring(info.get());
    }

    // Store plugin info
    d->plugins[plugin_id] = std::move(info);

    qCDebug(propertySystemLog)
        << "Registered plugin for property management:" << plugin_id;

    return make_success();
}

qtplugin::expected<void, PluginError> PluginPropertySystem::unregister_plugin(
    const QString& plugin_id) {
    QMutexLocker locker(&d->mutex);

    auto it = d->plugins.find(plugin_id);
    if (it == d->plugins.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Plugin not registered: " + plugin_id.toStdString());
    }

    // Remove property bindings involving this plugin
    auto binding_it = d->property_bindings.begin();
    while (binding_it != d->property_bindings.end()) {
        const auto& binding = binding_it->second;
        if (binding.source_plugin_id == plugin_id ||
            binding.target_plugin_id == plugin_id) {
            binding_it = d->property_bindings.erase(binding_it);
        } else {
            ++binding_it;
        }
    }

    // Remove plugin
    d->plugins.erase(it);

    qCDebug(propertySystemLog)
        << "Unregistered plugin from property management:" << plugin_id;

    return make_success();
}

}  // namespace qtplugin
