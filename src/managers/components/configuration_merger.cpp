/**
 * @file configuration_merger.cpp
 * @brief Implementation of configuration merger
 * @version 3.0.0
 */

#include "../../../include/qtplugin/managers/components/configuration_merger.hpp"
#include <QDebug>
#include <QJsonArray>
#include <QLoggingCategory>
#include <algorithm>

Q_LOGGING_CATEGORY(configMergerLog, "qtplugin.config.merger")

namespace qtplugin {

ConfigurationMerger::ConfigurationMerger(QObject* parent) : QObject(parent) {
    initialize_default_strategies();
    initialize_default_hierarchy();

    qCDebug(configMergerLog) << "Configuration merger initialized";
}

ConfigurationMerger::~ConfigurationMerger() {
    qCDebug(configMergerLog) << "Configuration merger destroyed";
}

qtplugin::expected<void, PluginError> ConfigurationMerger::merge_configurations(
    QJsonObject& target, const QJsonObject& source,
    ConfigurationMergeStrategy strategy,
    ConflictResolution conflict_resolution) {
    try {
        switch (strategy) {
            case ConfigurationMergeStrategy::Replace:
                target = source;
                break;

            case ConfigurationMergeStrategy::Merge:
            case ConfigurationMergeStrategy::DeepMerge:
                merge_objects(target, source, strategy, conflict_resolution);
                break;

            case ConfigurationMergeStrategy::Append:
            case ConfigurationMergeStrategy::Prepend:
                // For append/prepend, we still merge objects but handle arrays
                // differently
                merge_objects(target, source, strategy, conflict_resolution);
                break;
        }

        return make_success();

    } catch (const std::exception& e) {
        return make_error<void>(
            PluginErrorCode::ConfigurationError,
            "Exception during configuration merge: " + std::string(e.what()));
    }
}

QJsonObject ConfigurationMerger::get_merged_configuration(
    ConfigurationScope scope, std::string_view plugin_id,
    bool include_defaults) const {
    Q_UNUSED(scope);
    Q_UNUSED(plugin_id);
    Q_UNUSED(include_defaults);

    // This is a simplified implementation
    // In a real system, this would merge configurations from the inheritance
    // hierarchy
    QJsonObject merged;

    // TODO: Implement actual merging logic with storage component
    // For now, return empty object

    return merged;
}

qtplugin::expected<QJsonValue, PluginError>
ConfigurationMerger::get_inherited_value(std::string_view key,
                                         ConfigurationScope scope,
                                         std::string_view plugin_id) const {
    Q_UNUSED(key);
    Q_UNUSED(scope);
    Q_UNUSED(plugin_id);

    // This is a simplified implementation
    // In a real system, this would search through the inheritance hierarchy

    return make_error<QJsonValue>(PluginErrorCode::ConfigurationError,
                                  "Inherited value lookup not implemented");
}

void ConfigurationMerger::set_merge_strategy(
    ConfigurationScope scope, ConfigurationMergeStrategy strategy) {
    m_merge_strategies[scope] = strategy;
    qCDebug(configMergerLog)
        << "Merge strategy set for scope" << static_cast<int>(scope) << "to"
        << static_cast<int>(strategy);
}

ConfigurationMergeStrategy ConfigurationMerger::get_merge_strategy(
    ConfigurationScope scope) const {
    auto it = m_merge_strategies.find(scope);
    if (it != m_merge_strategies.end()) {
        return it->second;
    }
    return ConfigurationMergeStrategy::Merge;  // Default strategy
}

void ConfigurationMerger::set_inheritance_hierarchy(
    const std::vector<ConfigurationScope>& hierarchy) {
    m_inheritance_hierarchy = hierarchy;
    qCDebug(configMergerLog)
        << "Inheritance hierarchy updated with" << hierarchy.size() << "levels";
}

std::vector<ConfigurationScope> ConfigurationMerger::get_inheritance_hierarchy()
    const {
    return m_inheritance_hierarchy;
}

void ConfigurationMerger::merge_objects(
    QJsonObject& target, const QJsonObject& source,
    ConfigurationMergeStrategy strategy,
    ConflictResolution conflict_resolution) {
    for (auto it = source.begin(); it != source.end(); ++it) {
        const QString& key = it.key();
        const QJsonValue& source_value = it.value();

        if (target.contains(key)) {
            const QJsonValue& target_value = target[key];

            // Handle conflicts based on value types
            if (target_value.type() == source_value.type()) {
                if (source_value.isObject() &&
                    strategy == ConfigurationMergeStrategy::DeepMerge) {
                    // Deep merge objects
                    QJsonObject merged_object = target_value.toObject();
                    merge_objects(merged_object, source_value.toObject(),
                                  strategy, conflict_resolution);
                    target[key] = merged_object;
                } else if (source_value.isArray()) {
                    // Handle array merging
                    QJsonArray target_array = target_value.toArray();
                    QJsonArray source_array = source_value.toArray();
                    merge_arrays(target_array, source_array, strategy,
                                 conflict_resolution);
                    target[key] = target_array;
                } else {
                    // Resolve primitive value conflict
                    target[key] = resolve_conflict(target_value, source_value,
                                                   conflict_resolution, key);
                }
            } else {
                // Type mismatch - resolve conflict
                target[key] = resolve_conflict(target_value, source_value,
                                               conflict_resolution, key);
                emit merge_conflict_resolved(
                    key, static_cast<int>(conflict_resolution));
            }
        } else {
            // No conflict - add new value
            target[key] = source_value;
        }
    }
}

void ConfigurationMerger::merge_arrays(QJsonArray& target,
                                       const QJsonArray& source,
                                       ConfigurationMergeStrategy strategy,
                                       ConflictResolution conflict_resolution) {
    Q_UNUSED(conflict_resolution);

    switch (strategy) {
        case ConfigurationMergeStrategy::Replace:
            target = source;
            break;

        case ConfigurationMergeStrategy::Append:
            for (const auto& value : source) {
                target.append(value);
            }
            break;

        case ConfigurationMergeStrategy::Prepend:
            // Prepend in reverse order to maintain source order
            for (int i = source.size() - 1; i >= 0; --i) {
                target.prepend(source[i]);
            }
            break;

        case ConfigurationMergeStrategy::Merge:
        case ConfigurationMergeStrategy::DeepMerge:
        default:
            // For merge strategies, append by default
            for (const auto& value : source) {
                target.append(value);
            }
            break;
    }
}

QJsonValue ConfigurationMerger::resolve_conflict(const QJsonValue& target_value,
                                                 const QJsonValue& source_value,
                                                 ConflictResolution resolution,
                                                 const QString& key) {
    Q_UNUSED(key);

    switch (resolution) {
        case ConflictResolution::UseSource:
            return source_value;

        case ConflictResolution::UseTarget:
            return target_value;

        case ConflictResolution::Combine:
            // Attempt to combine values (simplified implementation)
            if (target_value.isString() && source_value.isString()) {
                return target_value.toString() + " " + source_value.toString();
            } else if (target_value.isDouble() && source_value.isDouble()) {
                return target_value.toDouble() + source_value.toDouble();
            } else {
                // Can't combine - use source
                return source_value;
            }

        case ConflictResolution::Error:
        default:
            // In a real implementation, this would throw an error
            // For now, use source value
            return source_value;
    }
}

QJsonValue ConfigurationMerger::get_nested_value(const QJsonObject& object,
                                                 std::string_view key) const {
    QString key_str = QString::fromStdString(std::string(key));
    QStringList key_parts = key_str.split('.');

    QJsonValue current = object;
    for (const QString& part : key_parts) {
        if (!current.isObject()) {
            return QJsonValue::Undefined;
        }

        QJsonObject current_obj = current.toObject();
        if (!current_obj.contains(part)) {
            return QJsonValue::Undefined;
        }

        current = current_obj[part];
    }

    return current;
}

void ConfigurationMerger::initialize_default_strategies() {
    // Set default merge strategies for each scope
    m_merge_strategies[ConfigurationScope::Global] =
        ConfigurationMergeStrategy::Merge;
    m_merge_strategies[ConfigurationScope::User] =
        ConfigurationMergeStrategy::DeepMerge;
    m_merge_strategies[ConfigurationScope::Session] =
        ConfigurationMergeStrategy::Merge;
    m_merge_strategies[ConfigurationScope::Plugin] =
        ConfigurationMergeStrategy::DeepMerge;
    m_merge_strategies[ConfigurationScope::Runtime] =
        ConfigurationMergeStrategy::Replace;
}

void ConfigurationMerger::initialize_default_hierarchy() {
    // Set default inheritance hierarchy (highest to lowest priority)
    m_inheritance_hierarchy = {
        ConfigurationScope::Runtime,  // Highest priority
        ConfigurationScope::Session, ConfigurationScope::User,
        ConfigurationScope::Plugin,
        ConfigurationScope::Global  // Lowest priority (defaults)
    };
}

}  // namespace qtplugin
