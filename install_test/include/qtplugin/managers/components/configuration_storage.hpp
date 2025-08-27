/**
 * @file configuration_storage.hpp
 * @brief Configuration storage interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include "../../utils/error_handling.hpp"
#include "../configuration_manager.hpp"

namespace qtplugin {

// Forward declarations
enum class ConfigurationScope;

/**
 * @brief Configuration data container
 */
struct ConfigurationData {
    QJsonObject data;
    std::optional<ConfigurationSchema> schema;
    std::filesystem::path file_path;
    bool is_dirty = false;
    mutable std::shared_mutex mutex;
};

/**
 * @brief Interface for configuration storage and persistence
 *
 * The configuration storage handles file I/O operations, data persistence,
 * and configuration data management.
 */
class IConfigurationStorage {
public:
    virtual ~IConfigurationStorage() = default;

    /**
     * @brief Load configuration from file
     * @param file_path Path to configuration file
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     * @param merge Whether to merge with existing configuration
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> load_from_file(
        const std::filesystem::path& file_path, ConfigurationScope scope,
        std::string_view plugin_id = {}, bool merge = true) = 0;

    /**
     * @brief Save configuration to file
     * @param file_path Path to save configuration
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> save_to_file(
        const std::filesystem::path& file_path, ConfigurationScope scope,
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Get configuration data for scope
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     * @return Configuration data or nullptr
     */
    virtual ConfigurationData* get_config_data(
        ConfigurationScope scope, std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Get or create configuration data for scope
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     * @return Configuration data or nullptr
     */
    virtual ConfigurationData* get_or_create_config_data(
        ConfigurationScope scope, std::string_view plugin_id = {}) = 0;

    /**
     * @brief Get entire configuration for scope
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     * @return Configuration object
     */
    virtual QJsonObject get_configuration(
        ConfigurationScope scope, std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Set entire configuration for scope
     * @param configuration Configuration object
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     * @param merge Whether to merge with existing configuration
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> set_configuration(
        const QJsonObject& configuration, ConfigurationScope scope,
        std::string_view plugin_id = {}, bool merge = false) = 0;

    /**
     * @brief Clear all configurations
     */
    virtual void clear() = 0;

    /**
     * @brief Get default configuration file path
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     * @return Default file path
     */
    virtual std::filesystem::path get_default_config_path(
        ConfigurationScope scope, std::string_view plugin_id = {}) const = 0;
};

/**
 * @brief Configuration storage implementation
 *
 * Manages configuration data storage, file I/O operations, and persistence.
 * Provides thread-safe access to configuration data.
 */
class ConfigurationStorage : public QObject, public IConfigurationStorage {
    Q_OBJECT

public:
    explicit ConfigurationStorage(QObject* parent = nullptr);
    ~ConfigurationStorage() override;

    // IConfigurationStorage interface
    qtplugin::expected<void, PluginError> load_from_file(
        const std::filesystem::path& file_path, ConfigurationScope scope,
        std::string_view plugin_id = {}, bool merge = true) override;

    qtplugin::expected<void, PluginError> save_to_file(
        const std::filesystem::path& file_path, ConfigurationScope scope,
        std::string_view plugin_id = {}) const override;

    ConfigurationData* get_config_data(
        ConfigurationScope scope,
        std::string_view plugin_id = {}) const override;

    ConfigurationData* get_or_create_config_data(
        ConfigurationScope scope, std::string_view plugin_id = {}) override;

    QJsonObject get_configuration(
        ConfigurationScope scope,
        std::string_view plugin_id = {}) const override;

    qtplugin::expected<void, PluginError> set_configuration(
        const QJsonObject& configuration, ConfigurationScope scope,
        std::string_view plugin_id = {}, bool merge = false) override;

    void clear() override;

    std::filesystem::path get_default_config_path(
        ConfigurationScope scope,
        std::string_view plugin_id = {}) const override;

signals:
    /**
     * @brief Emitted when configuration is loaded
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     */
    void configuration_loaded(int scope, const QString& plugin_id);

    /**
     * @brief Emitted when configuration is saved
     * @param scope Configuration scope
     * @param plugin_id Plugin identifier
     */
    void configuration_saved(int scope, const QString& plugin_id);

    /**
     * @brief Emitted when storage error occurs
     * @param error Error message
     */
    void storage_error(const QString& error);

private:
    // Configuration storage
    mutable std::shared_mutex m_global_mutex;
    std::unordered_map<ConfigurationScope, std::unique_ptr<ConfigurationData>>
        m_global_configs;
    std::unordered_map<std::string,
                       std::unordered_map<ConfigurationScope,
                                          std::unique_ptr<ConfigurationData>>>
        m_plugin_configs;

    // Helper methods
    qtplugin::expected<QJsonObject, PluginError> parse_json_file(
        const std::filesystem::path& file_path) const;
    qtplugin::expected<void, PluginError> write_json_file(
        const std::filesystem::path& file_path, const QJsonObject& data) const;
    void ensure_directory_exists(const std::filesystem::path& file_path) const;
};

}  // namespace qtplugin
