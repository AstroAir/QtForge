/**
 * @file configuration_plugin.cpp
 * @brief Configuration management example plugin implementation
 * @version 1.0.0
 */

#include "configuration_plugin.hpp"
#include <QDebug>
#include <QFileSystemWatcher>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonParseError>

namespace qtplugin::examples {

ConfigurationPlugin::ConfigurationPlugin(QObject* parent)
    : QObject(parent)
    , m_config_file_path(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config.json")
{
    qDebug() << "ConfigurationPlugin: Constructed";
    
    // Setup default configuration
    m_default_configuration = QJsonObject{
        {"logging_enabled", true},
        {"log_level", "info"},
        {"auto_save", true},
        {"validation_interval", 30000},
        {"max_config_history", 10},
        {"theme", "default"},
        {"language", "en"},
        {"features", QJsonObject{
            {"advanced_mode", false},
            {"debug_mode", false},
            {"experimental", false}
        }}
    };
}

ConfigurationPlugin::~ConfigurationPlugin() {
    if (m_state.load() != qtplugin::PluginState::Unloaded) {
        shutdown();
    }
    qDebug() << "ConfigurationPlugin: Destroyed";
}

bool ConfigurationPlugin::initialize(const QJsonObject& config) {
    QMutexLocker locker(&m_config_mutex);
    
    if (m_state.load() != qtplugin::PluginState::Unloaded) {
        qWarning() << "ConfigurationPlugin: Already initialized";
        return false;
    }
    
    qDebug() << "ConfigurationPlugin: Initializing...";
    m_state.store(qtplugin::PluginState::Loading);
    
    try {
        // Initialize configuration manager
        m_config_manager = std::make_unique<qtplugin::ConfigurationManager>();
        
        // Load configuration
        if (!load_configuration()) {
            qWarning() << "ConfigurationPlugin: Failed to load configuration, using defaults";
            m_configuration = m_default_configuration;
        }
        
        // Apply any provided config overrides
        if (!config.isEmpty()) {
            for (auto it = config.begin(); it != config.end(); ++it) {
                m_configuration[it.key()] = it.value();
            }
        }
        
        // Validate configuration
        if (!validate_configuration(m_configuration)) {
            qWarning() << "ConfigurationPlugin: Configuration validation failed, using defaults";
            m_configuration = m_default_configuration;
        }
        
        // Setup configuration watching
        setup_configuration_watching();
        
        // Setup validation timer
        m_validation_timer = std::make_unique<QTimer>();
        m_validation_timer->setInterval(m_configuration["validation_interval"].toInt());
        connect(m_validation_timer.get(), &QTimer::timeout, this, &ConfigurationPlugin::validate_configuration_timer);
        m_validation_timer->start();
        
        m_state.store(qtplugin::PluginState::Initialized);
        qDebug() << "ConfigurationPlugin: Initialized successfully!";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "ConfigurationPlugin: Initialization failed:" << e.what();
        m_state.store(qtplugin::PluginState::Error);
        return false;
    }
}

void ConfigurationPlugin::shutdown() {
    QMutexLocker locker(&m_config_mutex);
    
    if (m_state.load() == qtplugin::PluginState::Unloaded) {
        return;
    }
    
    qDebug() << "ConfigurationPlugin: Shutting down...";
    
    // Stop validation timer
    if (m_validation_timer) {
        m_validation_timer->stop();
        m_validation_timer.reset();
    }
    
    // Save current configuration
    if (m_configuration["auto_save"].toBool()) {
        save_configuration();
    }
    
    // Cleanup
    m_config_manager.reset();
    
    m_state.store(qtplugin::PluginState::Unloaded);
    qDebug() << "ConfigurationPlugin: Shutdown complete.";
}

QJsonObject ConfigurationPlugin::metadata() const {
    return QJsonObject{
        {"name", name()},
        {"version", version()},
        {"description", description()},
        {"state", static_cast<int>(state())},
        {"config_changes", m_config_changes.load()},
        {"validation_runs", m_validation_runs.load()},
        {"reload_count", m_reload_count.load()},
        {"config_file", m_config_file_path},
        {"commands", QJsonArray{"get_config", "set_config", "validate_config", "reload_config", "save_config"}}
    };
}

QJsonObject ConfigurationPlugin::execute_command(const QString& command, const QJsonObject& params) {
    if (m_state.load() != qtplugin::PluginState::Initialized) {
        return QJsonObject{{"error", "Plugin not initialized"}};
    }
    
    if (command == "get_config") {
        return execute_get_config_command(params);
    } else if (command == "set_config") {
        return execute_set_config_command(params);
    } else if (command == "validate_config") {
        return execute_validate_config_command(params);
    } else if (command == "reload_config") {
        return execute_reload_config_command(params);
    } else if (command == "save_config") {
        return execute_save_config_command(params);
    }
    
    return QJsonObject{{"error", "Unknown command: " + command}};
}

bool ConfigurationPlugin::load_configuration() {
    QFile file(m_config_file_path);
    if (!file.exists()) {
        qDebug() << "ConfigurationPlugin: Config file does not exist, will create with defaults";
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ConfigurationPlugin: Cannot open config file for reading:" << file.errorString();
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "ConfigurationPlugin: JSON parse error:" << error.errorString();
        return false;
    }
    
    m_configuration = doc.object();
    qDebug() << "ConfigurationPlugin: Configuration loaded successfully";
    return true;
}

bool ConfigurationPlugin::save_configuration() {
    QDir().mkpath(QFileInfo(m_config_file_path).absolutePath());
    
    QFile file(m_config_file_path);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "ConfigurationPlugin: Cannot open config file for writing:" << file.errorString();
        return false;
    }
    
    QJsonDocument doc(m_configuration);
    file.write(doc.toJson());
    qDebug() << "ConfigurationPlugin: Configuration saved successfully";
    return true;
}

bool ConfigurationPlugin::validate_configuration(const QJsonObject& config) const {
    // Basic validation rules
    if (!config.contains("logging_enabled") || !config["logging_enabled"].isBool()) {
        return false;
    }
    
    if (config.contains("validation_interval")) {
        int interval = config["validation_interval"].toInt();
        if (interval < 1000 || interval > 300000) { // 1s to 5min
            return false;
        }
    }
    
    if (config.contains("max_config_history")) {
        int history = config["max_config_history"].toInt();
        if (history < 1 || history > 100) {
            return false;
        }
    }
    
    return true;
}

void ConfigurationPlugin::setup_configuration_watching() {
    // Implementation would setup file system watching
    qDebug() << "ConfigurationPlugin: Configuration watching setup complete";
}

QJsonObject ConfigurationPlugin::execute_get_config_command(const QJsonObject& params) {
    QMutexLocker locker(&m_config_mutex);
    
    QString key = params["key"].toString();
    if (key.isEmpty()) {
        return QJsonObject{
            {"config", m_configuration},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    } else {
        return QJsonObject{
            {"key", key},
            {"value", m_configuration[key]},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    }
}

QJsonObject ConfigurationPlugin::execute_set_config_command(const QJsonObject& params) {
    QMutexLocker locker(&m_config_mutex);
    
    QString key = params["key"].toString();
    QJsonValue value = params["value"];
    
    if (key.isEmpty()) {
        return QJsonObject{{"error", "Key is required"}};
    }
    
    QJsonObject new_config = m_configuration;
    new_config[key] = value;
    
    if (!validate_configuration(new_config)) {
        return QJsonObject{{"error", "Configuration validation failed"}};
    }
    
    m_configuration[key] = value;
    m_config_changes.fetch_add(1);
    
    if (m_configuration["auto_save"].toBool()) {
        save_configuration();
    }
    
    qDebug() << "ConfigurationPlugin: Configuration updated:" << key << "=" << value;
    
    return QJsonObject{
        {"success", true},
        {"key", key},
        {"value", value},
        {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };
}

QJsonObject ConfigurationPlugin::execute_validate_config_command(const QJsonObject& params) {
    QJsonObject config_to_validate = params.isEmpty() ? m_configuration : params;
    bool is_valid = validate_configuration(config_to_validate);
    m_validation_runs.fetch_add(1);
    
    return QJsonObject{
        {"valid", is_valid},
        {"config", config_to_validate},
        {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };
}

QJsonObject ConfigurationPlugin::execute_reload_config_command(const QJsonObject& params) {
    Q_UNUSED(params)
    
    if (load_configuration()) {
        m_reload_count.fetch_add(1);
        qDebug() << "ConfigurationPlugin: Configuration reloaded successfully";
        return QJsonObject{
            {"success", true},
            {"config", m_configuration},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    } else {
        return QJsonObject{{"error", "Failed to reload configuration"}};
    }
}

QJsonObject ConfigurationPlugin::execute_save_config_command(const QJsonObject& params) {
    Q_UNUSED(params)
    
    if (save_configuration()) {
        return QJsonObject{
            {"success", true},
            {"file", m_config_file_path},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    } else {
        return QJsonObject{{"error", "Failed to save configuration"}};
    }
}

void ConfigurationPlugin::on_configuration_changed(const QString& key, const QJsonValue& value) {
    qDebug() << "ConfigurationPlugin: Configuration changed signal received:" << key << "=" << value;
    m_config_changes.fetch_add(1);
}

void ConfigurationPlugin::on_configuration_file_changed() {
    qDebug() << "ConfigurationPlugin: Configuration file changed, reloading...";
    load_configuration();
    m_reload_count.fetch_add(1);
}

void ConfigurationPlugin::validate_configuration_timer() {
    if (validate_configuration(m_configuration)) {
        m_validation_runs.fetch_add(1);
        qDebug() << "ConfigurationPlugin: Periodic validation passed";
    } else {
        qWarning() << "ConfigurationPlugin: Periodic validation failed!";
    }
}

} // namespace qtplugin::examples
