/**
 * @file configuration_plugin.hpp
 * @brief Configuration management example plugin
 * @version 1.0.0
 */

#pragma once

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QMutex>
#include <memory>
#include <atomic>

#include "qtplugin/core/plugin_interface.hpp"
#include "qtplugin/managers/configuration_manager.hpp"

namespace qtplugin::examples {

/**
 * @brief Configuration management example plugin
 * 
 * Demonstrates advanced configuration management patterns including:
 * - Dynamic configuration updates
 * - Configuration validation
 * - Configuration persistence
 * - Configuration watching and notifications
 */
class ConfigurationPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "configuration_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit ConfigurationPlugin(QObject* parent = nullptr);
    ~ConfigurationPlugin() override;

    // IPlugin interface
    bool initialize(const QJsonObject& config = {}) override;
    void shutdown() override;
    QString name() const override { return "ConfigurationPlugin"; }
    QString version() const override { return "1.0.0"; }
    QString description() const override { return "Advanced configuration management example"; }
    qtplugin::PluginState state() const override { return m_state.load(); }
    QJsonObject metadata() const override;
    QJsonObject execute_command(const QString& command, const QJsonObject& params = {}) override;

public slots:
    void on_configuration_changed(const QString& key, const QJsonValue& value);
    void on_configuration_file_changed();

private slots:
    void validate_configuration_timer();

private:
    // Configuration management
    bool load_configuration();
    bool save_configuration();
    bool validate_configuration(const QJsonObject& config) const;
    void setup_configuration_watching();
    void apply_configuration_changes(const QJsonObject& new_config);

    // Command implementations
    QJsonObject execute_get_config_command(const QJsonObject& params);
    QJsonObject execute_set_config_command(const QJsonObject& params);
    QJsonObject execute_validate_config_command(const QJsonObject& params);
    QJsonObject execute_reload_config_command(const QJsonObject& params);
    QJsonObject execute_save_config_command(const QJsonObject& params);

    // State management
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    mutable QMutex m_config_mutex;
    
    // Configuration data
    QJsonObject m_configuration;
    QJsonObject m_default_configuration;
    QString m_config_file_path;
    
    // Configuration management components
    std::unique_ptr<qtplugin::ConfigurationManager> m_config_manager;
    std::unique_ptr<QTimer> m_validation_timer;
    
    // Statistics
    std::atomic<int> m_config_changes{0};
    std::atomic<int> m_validation_runs{0};
    std::atomic<int> m_reload_count{0};
};

} // namespace qtplugin::examples
