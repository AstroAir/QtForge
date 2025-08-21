/**
 * @file configuration_manager.hpp
 * @brief Configuration management system for plugins
 * @version 3.0.0
 */

#pragma once

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <atomic>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "../utils/error_handling.hpp"

namespace qtplugin {

/**
 * @brief Configuration scope levels
 */
enum class ConfigurationScope {
    Global,   ///< Application-wide configuration
    Plugin,   ///< Plugin-specific configuration
    User,     ///< User-specific configuration
    Session,  ///< Session-specific configuration
    Runtime   ///< Runtime-only configuration (not persisted)
};

/**
 * @brief Configuration change event types
 */
enum class ConfigurationChangeType {
    Added,     ///< New configuration key added
    Modified,  ///< Existing configuration value changed
    Removed,   ///< Configuration key removed
    Reloaded   ///< Configuration reloaded from source
};

/**
 * @brief Configuration validation result
 */
struct ConfigurationValidationResult {
    bool is_valid = true;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    explicit operator bool() const noexcept { return is_valid; }
};

/**
 * @brief Configuration change event
 */
struct ConfigurationChangeEvent {
    ConfigurationChangeType type;
    std::string key;
    QJsonValue old_value;
    QJsonValue new_value;
    ConfigurationScope scope;
    std::string plugin_id;  // Empty for global scope
    std::chrono::system_clock::time_point timestamp;

    ConfigurationChangeEvent(ConfigurationChangeType t, std::string_view k,
                             const QJsonValue& old_val,
                             const QJsonValue& new_val, ConfigurationScope s,
                             std::string_view plugin = {})
        : type(t),
          key(k),
          old_value(old_val),
          new_value(new_val),
          scope(s),
          plugin_id(plugin),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @brief Configuration schema for validation
 */
struct ConfigurationSchema {
    QJsonObject schema;
    bool strict_mode = false;  // If true, only allow keys defined in schema

    ConfigurationSchema() = default;
    explicit ConfigurationSchema(const QJsonObject& s, bool strict = false)
        : schema(s), strict_mode(strict) {}
};

/**
 * @brief Configuration manager interface
 *
 * Provides comprehensive configuration management with hierarchical
 * configurations, validation, persistence, and change notifications.
 */
class IConfigurationManager {
public:
    virtual ~IConfigurationManager() = default;

    // === Configuration Access ===

    /**
     * @brief Get configuration value
     * @param key Configuration key (supports dot notation for nested values)
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return Configuration value or error
     */
    virtual qtplugin::expected<QJsonValue, PluginError> get_value(
        std::string_view key,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Get configuration value with default
     * @param key Configuration key
     * @param default_value Default value if key not found
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return Configuration value or default
     */
    virtual QJsonValue get_value_or_default(
        std::string_view key, const QJsonValue& default_value,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Set configuration value
     * @param key Configuration key
     * @param value Configuration value
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> set_value(
        std::string_view key, const QJsonValue& value,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) = 0;

    /**
     * @brief Remove configuration key
     * @param key Configuration key
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> remove_key(
        std::string_view key,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) = 0;

    /**
     * @brief Check if configuration key exists
     * @param key Configuration key
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return true if key exists
     */
    virtual bool has_key(std::string_view key,
                         ConfigurationScope scope = ConfigurationScope::Global,
                         std::string_view plugin_id = {}) const = 0;

    // === Bulk Operations ===

    /**
     * @brief Get entire configuration for scope
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return Configuration object or error
     */
    virtual qtplugin::expected<QJsonObject, PluginError> get_configuration(
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Set entire configuration for scope
     * @param configuration Configuration object
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @param merge If true, merge with existing configuration
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> set_configuration(
        const QJsonObject& configuration,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}, bool merge = true) = 0;

    /**
     * @brief Clear all configuration for scope
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> clear_configuration(
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) = 0;

    // === Schema and Validation ===

    /**
     * @brief Set configuration schema for validation
     * @param schema Configuration schema
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> set_schema(
        const ConfigurationSchema& schema,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) = 0;

    /**
     * @brief Validate configuration against schema
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return Validation result
     */
    virtual ConfigurationValidationResult validate_configuration(
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Validate specific configuration object
     * @param configuration Configuration to validate
     * @param schema Schema to validate against
     * @return Validation result
     */
    virtual ConfigurationValidationResult validate_configuration(
        const QJsonObject& configuration,
        const ConfigurationSchema& schema) const = 0;

    // === Persistence ===

    /**
     * @brief Load configuration from file
     * @param file_path Path to configuration file
     * @param scope Configuration scope to load into
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @param merge If true, merge with existing configuration
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> load_from_file(
        const std::filesystem::path& file_path,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}, bool merge = true) = 0;

    /**
     * @brief Save configuration to file
     * @param file_path Path to save configuration
     * @param scope Configuration scope to save
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> save_to_file(
        const std::filesystem::path& file_path,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Reload configuration from persistent storage
     * @param scope Configuration scope to reload
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> reload_configuration(
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) = 0;

    // === Change Notifications ===

    /**
     * @brief Subscribe to configuration changes
     * @param callback Callback function for change events
     * @param key_filter Optional key filter (supports wildcards)
     * @param scope_filter Optional scope filter
     * @param plugin_filter Optional plugin filter
     * @return Subscription ID for unsubscribing
     */
    virtual std::string subscribe_to_changes(
        std::function<void(const ConfigurationChangeEvent&)> callback,
        std::optional<std::string> key_filter = std::nullopt,
        std::optional<ConfigurationScope> scope_filter = std::nullopt,
        std::optional<std::string> plugin_filter = std::nullopt) = 0;

    /**
     * @brief Unsubscribe from configuration changes
     * @param subscription_id Subscription ID returned by subscribe_to_changes
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> unsubscribe_from_changes(
        const std::string& subscription_id) = 0;

    // === Utility Functions ===

    /**
     * @brief Get all configuration keys for scope
     * @param scope Configuration scope
     * @param plugin_id Plugin ID (required for Plugin scope)
     * @return List of configuration keys
     */
    virtual std::vector<std::string> get_keys(
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Get configuration statistics
     * @return Statistics as JSON object
     */
    virtual QJsonObject get_statistics() const = 0;

    /**
     * @brief Enable or disable automatic persistence
     * @param enabled If true, automatically save changes to persistent storage
     */
    virtual void set_auto_persist(bool enabled) = 0;

    /**
     * @brief Check if automatic persistence is enabled
     * @return true if auto-persistence is enabled
     */
    virtual bool is_auto_persist_enabled() const = 0;
};

}  // namespace qtplugin
