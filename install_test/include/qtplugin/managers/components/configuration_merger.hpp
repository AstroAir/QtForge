/**
 * @file configuration_merger.hpp
 * @brief Configuration merger interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <string>
#include <string_view>
#include <vector>
#include "../../utils/error_handling.hpp"
#include "../configuration_manager.hpp"

namespace qtplugin {

// Forward declarations
enum class ConfigurationScope;

/**
 * @brief Configuration merge strategy
 */
enum class ConfigurationMergeStrategy {
    Replace,    ///< Replace existing values
    Merge,      ///< Merge objects, replace primitives
    DeepMerge,  ///< Deep merge all nested objects
    Append,     ///< Append to arrays, merge objects
    Prepend     ///< Prepend to arrays, merge objects
};

/**
 * @brief Configuration merge conflict resolution
 */
enum class ConflictResolution {
    UseSource,  ///< Use source value in conflicts
    UseTarget,  ///< Use target value in conflicts
    Combine,    ///< Attempt to combine values
    Error       ///< Throw error on conflicts
};

/**
 * @brief Interface for configuration merging and inheritance
 *
 * The configuration merger handles merging configurations from different
 * sources, resolving conflicts, and implementing inheritance hierarchies.
 */
class IConfigurationMerger {
public:
    virtual ~IConfigurationMerger() = default;

    /**
     * @brief Merge two configurations
     * @param target Target configuration (modified in place)
     * @param source Source configuration
     * @param strategy Merge strategy to use
     * @param conflict_resolution How to resolve conflicts
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> merge_configurations(
        QJsonObject& target, const QJsonObject& source,
        ConfigurationMergeStrategy strategy = ConfigurationMergeStrategy::Merge,
        ConflictResolution conflict_resolution =
            ConflictResolution::UseSource) = 0;

    /**
     * @brief Get merged configuration with inheritance
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     * @param include_defaults Whether to include default values
     * @return Merged configuration
     */
    virtual QJsonObject get_merged_configuration(
        ConfigurationScope scope, std::string_view plugin_id = {},
        bool include_defaults = true) const = 0;

    /**
     * @brief Get configuration value with inheritance
     * @param key Configuration key
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     * @return Configuration value or error
     */
    virtual qtplugin::expected<QJsonValue, PluginError> get_inherited_value(
        std::string_view key, ConfigurationScope scope,
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Set merge strategy for scope
     * @param scope Configuration scope
     * @param strategy Merge strategy
     */
    virtual void set_merge_strategy(ConfigurationScope scope,
                                    ConfigurationMergeStrategy strategy) = 0;

    /**
     * @brief Get merge strategy for scope
     * @param scope Configuration scope
     * @return Current merge strategy
     */
    virtual ConfigurationMergeStrategy get_merge_strategy(
        ConfigurationScope scope) const = 0;

    /**
     * @brief Set inheritance hierarchy
     * @param hierarchy List of scopes in inheritance order (highest to lowest
     * priority)
     */
    virtual void set_inheritance_hierarchy(
        const std::vector<ConfigurationScope>& hierarchy) = 0;

    /**
     * @brief Get inheritance hierarchy
     * @return Current inheritance hierarchy
     */
    virtual std::vector<ConfigurationScope> get_inheritance_hierarchy()
        const = 0;
};

/**
 * @brief Configuration merger implementation
 *
 * Provides configuration merging capabilities with support for different
 * merge strategies, conflict resolution, and inheritance hierarchies.
 */
class ConfigurationMerger : public QObject, public IConfigurationMerger {
    Q_OBJECT

public:
    explicit ConfigurationMerger(QObject* parent = nullptr);
    ~ConfigurationMerger() override;

    // IConfigurationMerger interface
    qtplugin::expected<void, PluginError> merge_configurations(
        QJsonObject& target, const QJsonObject& source,
        ConfigurationMergeStrategy strategy = ConfigurationMergeStrategy::Merge,
        ConflictResolution conflict_resolution =
            ConflictResolution::UseSource) override;

    QJsonObject get_merged_configuration(
        ConfigurationScope scope, std::string_view plugin_id = {},
        bool include_defaults = true) const override;

    qtplugin::expected<QJsonValue, PluginError> get_inherited_value(
        std::string_view key, ConfigurationScope scope,
        std::string_view plugin_id = {}) const override;

    void set_merge_strategy(ConfigurationScope scope,
                            ConfigurationMergeStrategy strategy) override;
    ConfigurationMergeStrategy get_merge_strategy(
        ConfigurationScope scope) const override;

    void set_inheritance_hierarchy(
        const std::vector<ConfigurationScope>& hierarchy) override;
    std::vector<ConfigurationScope> get_inheritance_hierarchy() const override;

signals:
    /**
     * @brief Emitted when configurations are merged
     * @param target_scope Target configuration scope
     * @param source_scope Source configuration scope
     * @param strategy Merge strategy used
     */
    void configurations_merged(int target_scope, int source_scope,
                               int strategy);

    /**
     * @brief Emitted when merge conflict occurs
     * @param key Configuration key with conflict
     * @param resolution How conflict was resolved
     */
    void merge_conflict_resolved(const QString& key, int resolution);

private:
    std::unordered_map<ConfigurationScope, ConfigurationMergeStrategy>
        m_merge_strategies;
    std::vector<ConfigurationScope> m_inheritance_hierarchy;

    // Helper methods
    void merge_objects(QJsonObject& target, const QJsonObject& source,
                       ConfigurationMergeStrategy strategy,
                       ConflictResolution conflict_resolution);
    void merge_arrays(QJsonArray& target, const QJsonArray& source,
                      ConfigurationMergeStrategy strategy,
                      ConflictResolution conflict_resolution);
    QJsonValue resolve_conflict(const QJsonValue& target_value,
                                const QJsonValue& source_value,
                                ConflictResolution resolution,
                                const QString& key);
    QJsonValue get_nested_value(const QJsonObject& object,
                                std::string_view key) const;
    void initialize_default_strategies();
    void initialize_default_hierarchy();
};

}  // namespace qtplugin
