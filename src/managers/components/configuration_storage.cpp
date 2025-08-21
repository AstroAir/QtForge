/**
 * @file configuration_storage.cpp
 * @brief Implementation of configuration storage
 * @version 3.0.0
 */

#include "../../../include/qtplugin/managers/components/configuration_storage.hpp"
#include <QObject>
#include <QLoggingCategory>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QString>
#include <mutex>

Q_LOGGING_CATEGORY(configStorageLog, "qtplugin.config.storage")

namespace qtplugin {

ConfigurationStorage::ConfigurationStorage(QObject* parent)
    : QObject(parent) {
    
    // Initialize global configurations
    for (auto scope : {ConfigurationScope::Global, ConfigurationScope::User, ConfigurationScope::Session}) {
        m_global_configs[scope] = std::make_unique<ConfigurationData>();
    }
    
    qCDebug(configStorageLog) << "Configuration storage initialized";
}

ConfigurationStorage::~ConfigurationStorage() {
    qCDebug(configStorageLog) << "Configuration storage destroyed";
}

qtplugin::expected<void, PluginError>
ConfigurationStorage::load_from_file(const std::filesystem::path& file_path,
                                    ConfigurationScope scope,
                                    std::string_view plugin_id,
                                    bool merge) {
    if (!std::filesystem::exists(file_path)) {
        return make_error<void>(PluginErrorCode::FileNotFound,
                               "Configuration file not found: " + file_path.string());
    }
    
    // Parse JSON file
    auto json_result = parse_json_file(file_path);
    if (!json_result) {
        return qtplugin::unexpected<PluginError>{json_result.error()};
    }
    
    QJsonObject config = json_result.value();
    
    // Set configuration
    auto result = set_configuration(config, scope, plugin_id, merge);
    if (!result) {
        return result;
    }
    
    // Update file path for the configuration
    auto* config_data = get_config_data(scope, plugin_id);
    if (config_data) {
        std::unique_lock lock(config_data->mutex);
        config_data->file_path = file_path;
        config_data->is_dirty = false;
    }
    
    emit configuration_loaded(static_cast<int>(scope), QString::fromStdString(std::string(plugin_id)));
    qCDebug(configStorageLog) << "Configuration loaded from" << QString::fromStdString(file_path.string());
    
    return make_success();
}

qtplugin::expected<void, PluginError>
ConfigurationStorage::save_to_file(const std::filesystem::path& file_path,
                                  ConfigurationScope scope,
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
    
    // Ensure directory exists
    ensure_directory_exists(file_path);
    
    // Write JSON file
    auto result = write_json_file(file_path, data);
    if (!result) {
        return result;
    }
    
    // Mark as clean
    {
        std::unique_lock lock(config->mutex);
        config->file_path = file_path;
        config->is_dirty = false;
    }
    
    // Note: Cannot emit signals from const method
    // emit configuration_saved(static_cast<int>(scope), QString::fromStdString(std::string(plugin_id)));
    qCDebug(configStorageLog) << "Configuration saved to" << QString::fromStdString(file_path.string());
    
    return make_success();
}

ConfigurationData* ConfigurationStorage::get_config_data(ConfigurationScope scope,
                                                        std::string_view plugin_id) const {
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
        if (it != m_global_configs.end()) {
            return it->second.get();
        }
        return nullptr;
    }
}

ConfigurationData* ConfigurationStorage::get_or_create_config_data(ConfigurationScope scope,
                                                                  std::string_view plugin_id) {
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

QJsonObject ConfigurationStorage::get_configuration(ConfigurationScope scope,
                                                   std::string_view plugin_id) const {
    auto* config = get_config_data(scope, plugin_id);
    if (!config) {
        return QJsonObject{};
    }
    
    std::shared_lock lock(config->mutex);
    return config->data;
}

qtplugin::expected<void, PluginError>
ConfigurationStorage::set_configuration(const QJsonObject& configuration,
                                       ConfigurationScope scope,
                                       std::string_view plugin_id,
                                       bool merge) {
    auto* config = get_or_create_config_data(scope, plugin_id);
    if (!config) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                               "Failed to create configuration data");
    }
    
    {
        std::unique_lock lock(config->mutex);
        
        if (merge) {
            // Merge configurations
            for (auto it = configuration.begin(); it != configuration.end(); ++it) {
                config->data[it.key()] = it.value();
            }
        } else {
            config->data = configuration;
        }
        
        config->is_dirty = true;
    }
    
    return make_success();
}

void ConfigurationStorage::clear() {
    std::unique_lock lock(m_global_mutex);
    
    // Clear global configurations
    for (auto& [scope, config] : m_global_configs) {
        if (config) {
            std::unique_lock config_lock(config->mutex);
            config->data = QJsonObject{};
            config->is_dirty = false;
        }
    }
    
    // Clear plugin configurations
    for (auto& [plugin_id, plugin_configs] : m_plugin_configs) {
        for (auto& [scope, config] : plugin_configs) {
            if (config) {
                std::unique_lock config_lock(config->mutex);
                config->data = QJsonObject{};
                config->is_dirty = false;
            }
        }
    }
    
    qCDebug(configStorageLog) << "All configurations cleared";
}

std::filesystem::path ConfigurationStorage::get_default_config_path(ConfigurationScope scope,
                                                                   std::string_view plugin_id) const {
    QString base_path;
    
    switch (scope) {
        case ConfigurationScope::Global:
            base_path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
            return std::filesystem::path(base_path.toStdString()) / "global.json";
            
        case ConfigurationScope::User:
            base_path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
            return std::filesystem::path(base_path.toStdString()) / "user.json";
            
        case ConfigurationScope::Session:
            base_path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            return std::filesystem::path(base_path.toStdString()) / "session.json";
            
        case ConfigurationScope::Plugin:
            base_path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
            return std::filesystem::path(base_path.toStdString()) / "plugins" / (std::string(plugin_id) + ".json");
            
        case ConfigurationScope::Runtime:
        default:
            return {};
    }
}

qtplugin::expected<QJsonObject, PluginError> 
ConfigurationStorage::parse_json_file(const std::filesystem::path& file_path) const {
    QFile file(QString::fromStdString(file_path.string()));
    if (!file.open(QIODevice::ReadOnly)) {
        return make_error<QJsonObject>(PluginErrorCode::FileSystemError,
                                      "Failed to open configuration file: " + file.errorString().toStdString());
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidFormat,
                                      "Failed to parse configuration JSON: " + parseError.errorString().toStdString());
    }
    
    if (!doc.isObject()) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidFormat,
                                      "Configuration file must contain a JSON object");
    }
    
    return doc.object();
}

qtplugin::expected<void, PluginError> 
ConfigurationStorage::write_json_file(const std::filesystem::path& file_path,
                                     const QJsonObject& data) const {
    QFile file(QString::fromStdString(file_path.string()));
    if (!file.open(QIODevice::WriteOnly)) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                               "Failed to create configuration file: " + file.errorString().toStdString());
    }
    
    QJsonDocument doc(data);
    qint64 bytes_written = file.write(doc.toJson());
    file.close();
    
    if (bytes_written == -1) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                               "Failed to write configuration file");
    }
    
    return make_success();
}

void ConfigurationStorage::ensure_directory_exists(const std::filesystem::path& file_path) const {
    auto parent_path = file_path.parent_path();
    if (!parent_path.empty() && !std::filesystem::exists(parent_path)) {
        std::filesystem::create_directories(parent_path);
    }
}

} // namespace qtplugin
