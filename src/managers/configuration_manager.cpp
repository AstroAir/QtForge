/**
 * @file configuration_manager.cpp
 * @brief Implementation of configuration management system
 * @version 3.0.0
 */

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <QStandardPaths>
#include <mutex>
#include <random>
#include "qtplugin/managers/configuration_manager_impl.hpp"

Q_LOGGING_CATEGORY(configLog, "qtplugin.configuration")

namespace qtplugin {

ConfigurationManager::ConfigurationManager(QObject* parent) : QObject(parent) {
    qCDebug(configLog) << "Configuration manager initialized";

    // Initialize global configurations
    for (auto scope : {ConfigurationScope::Global, ConfigurationScope::User,
                       ConfigurationScope::Session}) {
        m_global_configs[scope] = std::make_unique<ConfigurationData>();
    }
}

ConfigurationManager::~ConfigurationManager() {
    // Save any dirty configurations
    if (m_auto_persist.load()) {
        for (const auto& [scope, config] : m_global_configs) {
            if (config && config->is_dirty) {
                save_to_file(config->file_path, scope);
            }
        }

        for (const auto& [plugin_id, plugin_configs] : m_plugin_configs) {
            for (const auto& [scope, config] : plugin_configs) {
                if (config && config->is_dirty) {
                    save_to_file(config->file_path, scope, plugin_id);
                }
            }
        }
    }

    qCDebug(configLog) << "Configuration manager destroyed";
}

qtplugin::expected<QJsonValue, PluginError> ConfigurationManager::get_value(
    std::string_view key, ConfigurationScope scope,
    std::string_view plugin_id) const {
    const_cast<std::atomic<size_t>&>(m_access_count).fetch_add(1);

    auto* config = get_config_data(scope, plugin_id);
    if (!config) {
        return make_error<QJsonValue>(PluginErrorCode::ConfigurationError,
                                      "Configuration not found for scope");
    }

    std::shared_lock lock(config->mutex);
    auto value = get_nested_value(config->data, key);

    if (value.isUndefined()) {
        return make_error<QJsonValue>(
            PluginErrorCode::ConfigurationError,
            std::format("Configuration key '{}' not found", key));
    }

    return value;
}

QJsonValue ConfigurationManager::get_value_or_default(
    std::string_view key, const QJsonValue& default_value,
    ConfigurationScope scope, std::string_view plugin_id) const {
    auto result = get_value(key, scope, plugin_id);
    return result ? result.value() : default_value;
}

qtplugin::expected<void, PluginError> ConfigurationManager::set_value(
    std::string_view key, const QJsonValue& value, ConfigurationScope scope,
    std::string_view plugin_id) {
    auto* config = get_or_create_config_data(scope, plugin_id);
    if (!config) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "Failed to create configuration data");
    }

    QJsonValue old_value;
    {
        std::unique_lock lock(config->mutex);
        old_value = get_nested_value(config->data, key);

        if (!set_nested_value(config->data, key, value)) {
            return make_error<void>(
                PluginErrorCode::ConfigurationError,
                std::format("Failed to set configuration key '{}'", key));
        }

        config->is_dirty = true;
    }

    // Notify change
    ConfigurationChangeEvent event(old_value.isUndefined()
                                       ? ConfigurationChangeType::Added
                                       : ConfigurationChangeType::Modified,
                                   key, old_value, value, scope, plugin_id);
    notify_change(event);

    m_change_count.fetch_add(1);

    // Auto-persist if enabled
    if (m_auto_persist.load()) {
        return persist_if_needed(scope, plugin_id);
    }

    return make_success();
}

qtplugin::expected<void, PluginError> ConfigurationManager::remove_key(
    std::string_view key, ConfigurationScope scope,
    std::string_view plugin_id) {
    auto* config = get_config_data(scope, plugin_id);
    if (!config) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "Configuration not found for scope");
    }

    QJsonValue old_value;
    {
        std::unique_lock lock(config->mutex);
        old_value = get_nested_value(config->data, key);

        if (old_value.isUndefined()) {
            return make_error<void>(
                PluginErrorCode::ConfigurationError,
                std::format("Configuration key '{}' not found", key));
        }

        if (!remove_nested_key(config->data, key)) {
            return make_error<void>(
                PluginErrorCode::ConfigurationError,
                std::format("Failed to remove configuration key '{}'", key));
        }

        config->is_dirty = true;
    }

    // Notify change
    ConfigurationChangeEvent event(ConfigurationChangeType::Removed, key,
                                   old_value, QJsonValue(), scope, plugin_id);
    notify_change(event);

    m_change_count.fetch_add(1);

    // Auto-persist if enabled
    if (m_auto_persist.load()) {
        return persist_if_needed(scope, plugin_id);
    }

    return make_success();
}

bool ConfigurationManager::has_key(std::string_view key,
                                   ConfigurationScope scope,
                                   std::string_view plugin_id) const {
    auto* config = get_config_data(scope, plugin_id);
    if (!config) {
        return false;
    }

    std::shared_lock lock(config->mutex);
    return !get_nested_value(config->data, key).isUndefined();
}

qtplugin::expected<QJsonObject, PluginError>
ConfigurationManager::get_configuration(ConfigurationScope scope,
                                        std::string_view plugin_id) const {
    auto* config = get_config_data(scope, plugin_id);
    if (!config) {
        return make_error<QJsonObject>(PluginErrorCode::ConfigurationError,
                                       "Configuration not found for scope");
    }

    std::shared_lock lock(config->mutex);
    return config->data;
}

qtplugin::expected<void, PluginError> ConfigurationManager::set_configuration(
    const QJsonObject& configuration, ConfigurationScope scope,
    std::string_view plugin_id, bool merge) {
    auto* config = get_or_create_config_data(scope, plugin_id);
    if (!config) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "Failed to create configuration data");
    }

    QJsonObject old_config;
    {
        std::unique_lock lock(config->mutex);
        old_config = config->data;

        if (merge) {
            // Merge configurations
            for (auto it = configuration.begin(); it != configuration.end();
                 ++it) {
                config->data[it.key()] = it.value();
            }
        } else {
            config->data = configuration;
        }

        config->is_dirty = true;
    }

    // Notify change
    ConfigurationChangeEvent event(ConfigurationChangeType::Reloaded, "",
                                   QJsonValue(old_config),
                                   QJsonValue(configuration), scope, plugin_id);
    notify_change(event);

    m_change_count.fetch_add(1);

    // Auto-persist if enabled
    if (m_auto_persist.load()) {
        return persist_if_needed(scope, plugin_id);
    }

    return make_success();
}

qtplugin::expected<void, PluginError> ConfigurationManager::clear_configuration(
    ConfigurationScope scope, std::string_view plugin_id) {
    auto* config = get_config_data(scope, plugin_id);
    if (!config) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "Configuration not found for scope");
    }

    QJsonObject old_config;
    {
        std::unique_lock lock(config->mutex);
        old_config = config->data;
        config->data = QJsonObject();
        config->is_dirty = true;
    }

    // Notify change
    ConfigurationChangeEvent event(ConfigurationChangeType::Reloaded, "",
                                   QJsonValue(old_config),
                                   QJsonValue(QJsonObject()), scope, plugin_id);
    notify_change(event);

    m_change_count.fetch_add(1);

    // Auto-persist if enabled
    if (m_auto_persist.load()) {
        return persist_if_needed(scope, plugin_id);
    }

    return make_success();
}

// Helper method implementations
ConfigurationManager::ConfigurationData* ConfigurationManager::get_config_data(
    ConfigurationScope scope, std::string_view plugin_id) const {
    if (scope == ConfigurationScope::Plugin && plugin_id.empty()) {
        return nullptr;
    }

    if (scope == ConfigurationScope::Plugin) {
        std::shared_lock lock(m_global_mutex);
        auto plugin_it = m_plugin_configs.find(std::string(plugin_id));
        if (plugin_it != m_plugin_configs.end()) {
            auto scope_it = plugin_it->second.find(scope);
            if (scope_it != plugin_it->second.end()) {
                return scope_it->second.get();
            }
        }
        return nullptr;
    } else {
        std::shared_lock lock(m_global_mutex);
        auto it = m_global_configs.find(scope);
        return (it != m_global_configs.end()) ? it->second.get() : nullptr;
    }
}

ConfigurationManager::ConfigurationData*
ConfigurationManager::get_or_create_config_data(ConfigurationScope scope,
                                                std::string_view plugin_id) {
    if (scope == ConfigurationScope::Plugin && plugin_id.empty()) {
        return nullptr;
    }

    if (scope == ConfigurationScope::Plugin) {
        std::unique_lock lock(m_global_mutex);
        auto& plugin_configs = m_plugin_configs[std::string(plugin_id)];
        auto& config = plugin_configs[scope];
        if (!config) {
            config = std::make_unique<ConfigurationData>();
            config->file_path = get_default_config_path(scope, plugin_id);
        }
        return config.get();
    } else {
        std::unique_lock lock(m_global_mutex);
        auto& config = m_global_configs[scope];
        if (!config) {
            config = std::make_unique<ConfigurationData>();
            config->file_path = get_default_config_path(scope, plugin_id);
        }
        return config.get();
    }
}

std::string ConfigurationManager::generate_subscription_id() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::string id;
    id.reserve(32);
    for (int i = 0; i < 32; ++i) {
        id += "0123456789abcdef"[dis(gen)];
    }
    return id;
}

QJsonValue ConfigurationManager::get_nested_value(const QJsonObject& obj,
                                                  std::string_view key) const {
    if (key.find('.') == std::string_view::npos) {
        return obj.value(QString::fromUtf8(key.data(), key.size()));
    }

    // Handle nested keys
    QJsonObject current = obj;
    std::string key_str(key);
    size_t start = 0;
    size_t pos = 0;

    while ((pos = key_str.find('.', start)) != std::string::npos) {
        std::string part = key_str.substr(start, pos - start);
        QJsonValue value = current.value(QString::fromStdString(part));
        if (!value.isObject()) {
            return QJsonValue();
        }
        current = value.toObject();
        start = pos + 1;
    }

    std::string final_part = key_str.substr(start);
    return current.value(QString::fromStdString(final_part));
}

bool ConfigurationManager::set_nested_value(QJsonObject& obj,
                                            std::string_view key,
                                            const QJsonValue& value) {
    QString qkey = QString::fromUtf8(key.data(), key.size());

    if (!qkey.contains('.')) {
        obj[qkey] = value;
        return true;
    }

    // For nested keys, we'll use a recursive approach with QJsonDocument
    QStringList parts = qkey.split('.');

    std::function<void(QJsonObject&, const QStringList&, int,
                       const QJsonValue&)>
        setNested = [&](QJsonObject& current, const QStringList& keyParts,
                        int index, const QJsonValue& val) {
            if (index == keyParts.size() - 1) {
                current[keyParts[index]] = val;
                return;
            }

            QString part = keyParts[index];
            if (!current.contains(part) || !current[part].isObject()) {
                current[part] = QJsonObject();
            }

            QJsonObject nested = current[part].toObject();
            setNested(nested, keyParts, index + 1, val);
            current[part] = nested;
        };

    setNested(obj, parts, 0, value);
    return true;
}

std::filesystem::path ConfigurationManager::get_default_config_path(
    ConfigurationScope scope, std::string_view plugin_id) const {
    QString base_path;

    switch (scope) {
        case ConfigurationScope::Global:
            base_path = QStandardPaths::writableLocation(
                QStandardPaths::AppConfigLocation);
            return std::filesystem::path(base_path.toStdString()) /
                   "global.json";

        case ConfigurationScope::User:
            base_path = QStandardPaths::writableLocation(
                QStandardPaths::AppConfigLocation);
            return std::filesystem::path(base_path.toStdString()) / "user.json";

        case ConfigurationScope::Session:
            base_path =
                QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            return std::filesystem::path(base_path.toStdString()) /
                   "session.json";

        case ConfigurationScope::Plugin:
            base_path = QStandardPaths::writableLocation(
                QStandardPaths::AppConfigLocation);
            return std::filesystem::path(base_path.toStdString()) / "plugins" /
                   (std::string(plugin_id) + ".json");

        case ConfigurationScope::Runtime:
        default:
            return {};
    }
}

// Add remaining method implementations
qtplugin::expected<void, PluginError> ConfigurationManager::set_schema(
    const ConfigurationSchema& schema, ConfigurationScope scope,
    std::string_view plugin_id) {
    auto* config = get_or_create_config_data(scope, plugin_id);
    if (!config) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "Failed to create configuration data");
    }

    std::unique_lock lock(config->mutex);
    config->schema = schema;
    return make_success();
}

ConfigurationValidationResult ConfigurationManager::validate_configuration(
    ConfigurationScope scope, std::string_view plugin_id) const {
    auto* config = get_config_data(scope, plugin_id);
    if (!config) {
        return ConfigurationValidationResult{
            false, {"Configuration not found"}, {}};
    }

    std::shared_lock lock(config->mutex);
    if (!config->schema) {
        return ConfigurationValidationResult{
            true, {}, {"No schema defined for validation"}};
    }

    return validate_configuration(config->data, config->schema.value());
}

ConfigurationValidationResult ConfigurationManager::validate_configuration(
    const QJsonObject& configuration, const ConfigurationSchema& schema) const {
    ConfigurationValidationResult result;
    result.is_valid = true;

    // Basic JSON schema validation
    if (schema.schema.isEmpty()) {
        result.warnings.push_back("Empty schema provided");
        return result;
    }

    // Validate required properties
    if (schema.schema.contains("required") &&
        schema.schema["required"].isArray()) {
        QJsonArray required = schema.schema["required"].toArray();
        for (const auto& req : required) {
            QString key = req.toString();
            if (!configuration.contains(key)) {
                result.is_valid = false;
                result.errors.push_back("Required property '" +
                                        key.toStdString() + "' is missing");
            }
        }
    }

    // Validate properties against their schemas
    if (schema.schema.contains("properties") &&
        schema.schema["properties"].isObject()) {
        QJsonObject properties = schema.schema["properties"].toObject();

        for (auto it = configuration.begin(); it != configuration.end(); ++it) {
            QString key = it.key();
            QJsonValue value = it.value();

            if (properties.contains(key)) {
                QJsonObject propSchema = properties[key].toObject();
                auto validation =
                    validate_property(value, propSchema, key.toStdString());
                if (!validation.is_valid) {
                    result.is_valid = false;
                    result.errors.insert(result.errors.end(),
                                         validation.errors.begin(),
                                         validation.errors.end());
                }
                result.warnings.insert(result.warnings.end(),
                                       validation.warnings.begin(),
                                       validation.warnings.end());
            } else if (schema.strict_mode) {
                result.is_valid = false;
                result.errors.push_back("Unknown property '" +
                                        key.toStdString() + "' in strict mode");
            }
        }
    }

    return result;
}

// Configuration persistence implementation
qtplugin::expected<void, PluginError> ConfigurationManager::load_from_file(
    const std::filesystem::path& file_path, ConfigurationScope scope,
    std::string_view plugin_id, bool merge) {
    if (!std::filesystem::exists(file_path)) {
        return make_error<void>(
            PluginErrorCode::FileNotFound,
            "Configuration file not found: " + file_path.string());
    }

    // Read file
    QFile file(QString::fromStdString(file_path.string()));
    if (!file.open(QIODevice::ReadOnly)) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to open configuration file: " +
                                    file.errorString().toStdString());
    }

    QByteArray data = file.readAll();
    file.close();

    // Parse JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return make_error<void>(PluginErrorCode::InvalidFormat,
                                "Failed to parse configuration JSON: " +
                                    parseError.errorString().toStdString());
    }

    if (!doc.isObject()) {
        return make_error<void>(
            PluginErrorCode::InvalidFormat,
            "Configuration file must contain a JSON object");
    }

    QJsonObject config = doc.object();

    // Set configuration
    auto result = set_configuration(config, scope, plugin_id, merge);
    if (!result) {
        return result;
    }

    // Update file path for this configuration
    auto* config_data = get_config_data(scope, plugin_id);
    if (config_data) {
        std::unique_lock lock(config_data->mutex);
        config_data->file_path = file_path;
        config_data->is_dirty = false;  // Just loaded, so not dirty
    }

    emit configuration_loaded(static_cast<int>(scope),
                              QString::fromStdString(std::string(plugin_id)));

    qCDebug(configLog) << "Configuration loaded from"
                       << QString::fromStdString(file_path.string());

    return make_success();
}

qtplugin::expected<void, PluginError> ConfigurationManager::save_to_file(
    const std::filesystem::path& file_path, ConfigurationScope scope,
    std::string_view plugin_id) const {
    auto* config = get_config_data(scope, plugin_id);
    if (!config) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "Configuration not found for scope");
    }

    QJsonObject data;
    {
        std::shared_lock lock(config->mutex);
        data = config->data;
    }

    // Create directory if it doesn't exist
    std::filesystem::path dir = file_path.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir)) {
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        if (ec) {
            return make_error<void>(
                PluginErrorCode::FileSystemError,
                "Failed to create directory: " + ec.message());
        }
    }

    // Create JSON document
    QJsonDocument doc(data);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    // Write to file
    QFile file(QString::fromStdString(file_path.string()));
    if (!file.open(QIODevice::WriteOnly)) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to open file for writing: " +
                                    file.errorString().toStdString());
    }

    qint64 written = file.write(jsonData);
    file.close();

    if (written != jsonData.size()) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to write complete configuration data");
    }

    // Update file path and mark as clean
    {
        std::unique_lock lock(config->mutex);
        config->file_path = file_path;
        config->is_dirty = false;
    }

    emit const_cast<ConfigurationManager*>(this)->configuration_saved(
        static_cast<int>(scope),
        QString::fromStdString(std::string(plugin_id)));

    qCDebug(configLog) << "Configuration saved to"
                       << QString::fromStdString(file_path.string());

    return make_success();
}

qtplugin::expected<void, PluginError>
ConfigurationManager::reload_configuration(ConfigurationScope scope,
                                           std::string_view plugin_id) {
    auto* config = get_config_data(scope, plugin_id);
    if (!config) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "Configuration not found for scope");
    }

    std::filesystem::path file_path;
    {
        std::shared_lock lock(config->mutex);
        file_path = config->file_path;
    }

    if (file_path.empty()) {
        // Use default path if no specific path is set
        file_path = get_default_config_path(scope, plugin_id);
    }

    if (file_path.empty() || !std::filesystem::exists(file_path)) {
        return make_error<void>(PluginErrorCode::FileNotFound,
                                "No configuration file found to reload from");
    }

    // Load from file (merge=false to replace completely)
    auto result = load_from_file(file_path, scope, plugin_id, false);
    if (result) {
        qCDebug(configLog) << "Configuration reloaded for scope"
                           << static_cast<int>(scope);
    }

    return result;
}

std::string ConfigurationManager::subscribe_to_changes(
    std::function<void(const ConfigurationChangeEvent&)> callback,
    std::optional<std::string> key_filter,
    std::optional<ConfigurationScope> scope_filter,
    std::optional<std::string> plugin_filter) {
    Q_UNUSED(callback)
    Q_UNUSED(key_filter)
    Q_UNUSED(scope_filter)
    Q_UNUSED(plugin_filter)
    return generate_subscription_id();
}

qtplugin::expected<void, PluginError>
ConfigurationManager::unsubscribe_from_changes(
    const std::string& subscription_id) {
    Q_UNUSED(subscription_id)
    return make_success();
}

std::vector<std::string> ConfigurationManager::get_keys(
    ConfigurationScope scope, std::string_view plugin_id) const {
    Q_UNUSED(scope)
    Q_UNUSED(plugin_id)
    return {};
}

QJsonObject ConfigurationManager::get_statistics() const {
    return QJsonObject{
        {"access_count", static_cast<qint64>(m_access_count.load())},
        {"change_count", static_cast<qint64>(m_change_count.load())},
        {"auto_persist", m_auto_persist.load()}};
}

void ConfigurationManager::set_auto_persist(bool enabled) {
    m_auto_persist.store(enabled);
}

bool ConfigurationManager::is_auto_persist_enabled() const {
    return m_auto_persist.load();
}

void ConfigurationManager::notify_change(
    const ConfigurationChangeEvent& event) {
    std::shared_lock lock(m_subscriptions_mutex);

    // Increment change counter
    m_change_count.fetch_add(1);

    // Notify all matching subscriptions
    for (const auto& [subscription_id, subscription] : m_subscriptions) {
        if (subscription && matches_filter(*subscription, event)) {
            try {
                // Call the callback in a safe manner
                subscription->callback(event);
            } catch (const std::exception& e) {
                qCWarning(configLog) << "Exception in configuration change "
                                        "callback for subscription"
                                     << QString::fromStdString(subscription_id)
                                     << ":" << e.what();
            } catch (...) {
                qCWarning(configLog) << "Unknown exception in configuration "
                                        "change callback for subscription"
                                     << QString::fromStdString(subscription_id);
            }
        }
    }
}

bool ConfigurationManager::matches_filter(
    const ChangeSubscription& subscription,
    const ConfigurationChangeEvent& event) const {
    // Check key filter
    if (subscription.key_filter.has_value()) {
        const std::string& key_filter = subscription.key_filter.value();

        // Support wildcard matching
        if (key_filter.back() == '*') {
            // Prefix matching
            std::string prefix = key_filter.substr(0, key_filter.length() - 1);
            if (event.key.substr(0, prefix.length()) != prefix) {
                return false;
            }
        } else if (key_filter != event.key) {
            // Exact matching
            return false;
        }
    }

    // Check scope filter
    if (subscription.scope_filter.has_value()) {
        if (subscription.scope_filter.value() != event.scope) {
            return false;
        }
    }

    // Check plugin filter
    if (subscription.plugin_filter.has_value()) {
        if (subscription.plugin_filter.value() != event.plugin_id) {
            return false;
        }
    }

    return true;
}

qtplugin::expected<void, PluginError> ConfigurationManager::persist_if_needed(
    ConfigurationScope scope, std::string_view plugin_id) {
    if (scope == ConfigurationScope::Runtime) {
        // Runtime configurations are not persisted
        return make_success();
    }

    auto* config = get_config_data(scope, plugin_id);
    if (!config) {
        return make_success();  // Nothing to persist
    }

    std::filesystem::path file_path;
    bool is_dirty = false;
    {
        std::shared_lock lock(config->mutex);
        file_path = config->file_path;
        is_dirty = config->is_dirty;
    }

    if (!is_dirty) {
        return make_success();  // No changes to persist
    }

    if (file_path.empty()) {
        file_path = get_default_config_path(scope, plugin_id);
    }

    if (file_path.empty()) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "No file path available for persistence");
    }

    return save_to_file(file_path, scope, plugin_id);
}

bool ConfigurationManager::remove_nested_key(QJsonObject& obj,
                                             std::string_view key) {
    QString qkey = QString::fromUtf8(key.data(), key.size());

    if (!qkey.contains('.')) {
        if (obj.contains(qkey)) {
            obj.remove(qkey);
            return true;
        }
        return false;
    }

    // For nested keys, use a recursive approach similar to set_nested_value
    QStringList parts = qkey.split('.');

    std::function<bool(QJsonObject&, const QStringList&, int)> removeNested =
        [&](QJsonObject& current, const QStringList& keyParts,
            int index) -> bool {
        if (index == keyParts.size() - 1) {
            // Last part - remove the key
            if (current.contains(keyParts[index])) {
                current.remove(keyParts[index]);
                return true;
            }
            return false;
        }

        QString part = keyParts[index];
        if (!current.contains(part) || !current[part].isObject()) {
            return false;  // Path doesn't exist
        }

        QJsonObject nested = current[part].toObject();
        bool result = removeNested(nested, keyParts, index + 1);
        if (result) {
            current[part] = nested;  // Update the modified nested object
        }
        return result;
    };

    return removeNested(obj, parts, 0);
}

void ConfigurationManager::collect_keys(const QJsonObject& obj,
                                        std::vector<std::string>& keys,
                                        const std::string& prefix) const {
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        const QString& key = it.key();
        const QJsonValue& value = it.value();

        // Build the full key path
        std::string full_key = prefix.empty()
                                   ? key.toStdString()
                                   : prefix + "." + key.toStdString();

        // Add this key to the collection
        keys.push_back(full_key);

        // If the value is an object, recursively collect its keys
        if (value.isObject()) {
            collect_keys(value.toObject(), keys, full_key);
        }
        // If the value is an array, collect keys for array elements that are
        // objects
        else if (value.isArray()) {
            const QJsonArray array = value.toArray();
            for (int i = 0; i < array.size(); ++i) {
                const QJsonValue& element = array[i];
                if (element.isObject()) {
                    std::string array_key =
                        full_key + "[" + std::to_string(i) + "]";
                    keys.push_back(array_key);
                    collect_keys(element.toObject(), keys, array_key);
                }
            }
        }
    }
}

ConfigurationValidationResult ConfigurationManager::validate_property(
    const QJsonValue& value, const QJsonObject& schema,
    const std::string& property_name) const {
    ConfigurationValidationResult result;
    result.is_valid = true;

    // Check type
    if (schema.contains("type")) {
        QString expectedType = schema["type"].toString();
        QString actualType = json_value_type_name(value);

        if (expectedType != actualType) {
            result.is_valid = false;
            result.errors.push_back("Property '" + property_name +
                                    "' expected type '" +
                                    expectedType.toStdString() + "' but got '" +
                                    actualType.toStdString() + "'");
        }
    }

    // Check string constraints
    if (value.isString() && schema.contains("minLength")) {
        int minLength = schema["minLength"].toInt();
        if (value.toString().length() < minLength) {
            result.is_valid = false;
            result.errors.push_back("Property '" + property_name +
                                    "' string too short (minimum " +
                                    std::to_string(minLength) + " characters)");
        }
    }

    if (value.isString() && schema.contains("maxLength")) {
        int maxLength = schema["maxLength"].toInt();
        if (value.toString().length() > maxLength) {
            result.is_valid = false;
            result.errors.push_back("Property '" + property_name +
                                    "' string too long (maximum " +
                                    std::to_string(maxLength) + " characters)");
        }
    }

    // Check numeric constraints
    if (value.isDouble() && schema.contains("minimum")) {
        double minimum = schema["minimum"].toDouble();
        if (value.toDouble() < minimum) {
            result.is_valid = false;
            result.errors.push_back("Property '" + property_name +
                                    "' value too small (minimum " +
                                    std::to_string(minimum) + ")");
        }
    }

    if (value.isDouble() && schema.contains("maximum")) {
        double maximum = schema["maximum"].toDouble();
        if (value.toDouble() > maximum) {
            result.is_valid = false;
            result.errors.push_back("Property '" + property_name +
                                    "' value too large (maximum " +
                                    std::to_string(maximum) + ")");
        }
    }

    return result;
}

QString ConfigurationManager::json_value_type_name(
    const QJsonValue& value) const {
    switch (value.type()) {
        case QJsonValue::Null:
            return "null";
        case QJsonValue::Bool:
            return "boolean";
        case QJsonValue::Double:
            return "number";
        case QJsonValue::String:
            return "string";
        case QJsonValue::Array:
            return "array";
        case QJsonValue::Object:
            return "object";
        case QJsonValue::Undefined:
            return "undefined";
    }
    return "unknown";
}

// Factory function
std::unique_ptr<IConfigurationManager> create_configuration_manager(
    QObject* parent) {
    return std::make_unique<ConfigurationManager>(parent);
}

}  // namespace qtplugin
