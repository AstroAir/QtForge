/**
 * @file configuration_manager_impl.hpp
 * @brief Concrete implementation of configuration management system
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <atomic>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include "configuration_manager.hpp"

namespace qtplugin {

/**
 * @brief Default configuration manager implementation
 */
class ConfigurationManager : public QObject, public IConfigurationManager {
    Q_OBJECT

public:
    explicit ConfigurationManager(QObject* parent = nullptr);
    ~ConfigurationManager() override;

    // IConfigurationManager implementation
    qtplugin::expected<QJsonValue, PluginError> get_value(
        std::string_view key,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const override;

    QJsonValue get_value_or_default(
        std::string_view key, const QJsonValue& default_value,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const override;

    qtplugin::expected<void, PluginError> set_value(
        std::string_view key, const QJsonValue& value,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) override;

    qtplugin::expected<void, PluginError> remove_key(
        std::string_view key,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) override;

    bool has_key(std::string_view key,
                 ConfigurationScope scope = ConfigurationScope::Global,
                 std::string_view plugin_id = {}) const override;

    qtplugin::expected<QJsonObject, PluginError> get_configuration(
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const override;

    qtplugin::expected<void, PluginError> set_configuration(
        const QJsonObject& configuration,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}, bool merge = true) override;

    qtplugin::expected<void, PluginError> clear_configuration(
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) override;

    qtplugin::expected<void, PluginError> set_schema(
        const ConfigurationSchema& schema,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) override;

    ConfigurationValidationResult validate_configuration(
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const override;

    ConfigurationValidationResult validate_configuration(
        const QJsonObject& configuration,
        const ConfigurationSchema& schema) const override;

    qtplugin::expected<void, PluginError> load_from_file(
        const std::filesystem::path& file_path,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}, bool merge = true) override;

    qtplugin::expected<void, PluginError> save_to_file(
        const std::filesystem::path& file_path,
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const override;

    qtplugin::expected<void, PluginError> reload_configuration(
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) override;

    std::string subscribe_to_changes(
        std::function<void(const ConfigurationChangeEvent&)> callback,
        std::optional<std::string> key_filter = std::nullopt,
        std::optional<ConfigurationScope> scope_filter = std::nullopt,
        std::optional<std::string> plugin_filter = std::nullopt) override;

    qtplugin::expected<void, PluginError> unsubscribe_from_changes(
        const std::string& subscription_id) override;

    std::vector<std::string> get_keys(
        ConfigurationScope scope = ConfigurationScope::Global,
        std::string_view plugin_id = {}) const override;

    QJsonObject get_statistics() const override;

    void set_auto_persist(bool enabled) override;
    bool is_auto_persist_enabled() const override;

signals:
    void configuration_changed(const QString& key, const QJsonValue& old_value,
                               const QJsonValue& new_value, int scope,
                               const QString& plugin_id);
    void configuration_loaded(int scope, const QString& plugin_id);
    void configuration_saved(int scope, const QString& plugin_id);

private:
    struct ConfigurationData {
        QJsonObject data;
        std::optional<ConfigurationSchema> schema;
        std::filesystem::path file_path;
        bool is_dirty = false;
        mutable std::shared_mutex mutex;
    };

    struct ChangeSubscription {
        std::string id;
        std::function<void(const ConfigurationChangeEvent&)> callback;
        std::optional<std::string> key_filter;
        std::optional<ConfigurationScope> scope_filter;
        std::optional<std::string> plugin_filter;
        QRegularExpression key_regex;  // Compiled from key_filter
    };

    // Configuration storage
    mutable std::shared_mutex m_global_mutex;
    std::unordered_map<ConfigurationScope, std::unique_ptr<ConfigurationData>>
        m_global_configs;
    std::unordered_map<std::string,
                       std::unordered_map<ConfigurationScope,
                                          std::unique_ptr<ConfigurationData>>>
        m_plugin_configs;

    // Change notifications
    mutable std::shared_mutex m_subscriptions_mutex;
    std::unordered_map<std::string, std::unique_ptr<ChangeSubscription>>
        m_subscriptions;

    // Settings
    std::atomic<bool> m_auto_persist{true};
    std::atomic<size_t> m_change_count{0};
    std::atomic<size_t> m_access_count{0};

    // Helper methods
    ConfigurationData* get_config_data(ConfigurationScope scope,
                                       std::string_view plugin_id) const;
    ConfigurationData* get_or_create_config_data(ConfigurationScope scope,
                                                 std::string_view plugin_id);
    std::string generate_subscription_id() const;
    QJsonValue get_nested_value(const QJsonObject& obj,
                                std::string_view key) const;
    bool set_nested_value(QJsonObject& obj, std::string_view key,
                          const QJsonValue& value);
    bool remove_nested_key(QJsonObject& obj, std::string_view key);
    void collect_keys(const QJsonObject& obj, std::vector<std::string>& keys,
                      const std::string& prefix = {}) const;
    void notify_change(const ConfigurationChangeEvent& event);
    bool matches_filter(const ChangeSubscription& subscription,
                        const ConfigurationChangeEvent& event) const;
    std::filesystem::path get_default_config_path(
        ConfigurationScope scope, std::string_view plugin_id) const;
    qtplugin::expected<void, PluginError> persist_if_needed(
        ConfigurationScope scope, std::string_view plugin_id);
    ConfigurationValidationResult validate_property(
        const QJsonValue& value, const QJsonObject& schema,
        const std::string& property_name) const;
    QString json_value_type_name(const QJsonValue& value) const;
};

/**
 * @brief Create a default configuration manager instance
 * @param parent Parent QObject
 * @return Unique pointer to configuration manager
 */
std::unique_ptr<IConfigurationManager> create_configuration_manager(
    QObject* parent = nullptr);

}  // namespace qtplugin
