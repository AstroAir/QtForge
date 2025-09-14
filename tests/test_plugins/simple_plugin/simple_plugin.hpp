/**
 * @file simple_plugin.hpp
 * @brief Simple test plugin for testing plugin system
 * @version 1.0.0
 * @date 2024-01-13
 */

#ifndef QTFORGE_SIMPLE_TEST_PLUGIN_HPP
#define QTFORGE_SIMPLE_TEST_PLUGIN_HPP

#include <QObject>
#include <QJsonObject>
#include <QtPlugin>
#include <qtplugin/interfaces/plugin_interface.hpp>
#include <qtplugin/core/plugin_metadata.hpp>

namespace qtforge::test {

/**
 * @class SimpleTestPlugin
 * @brief A simple plugin for testing basic plugin functionality
 */
class SimpleTestPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qtforge.test.SimplePlugin" FILE "simple_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)
    
public:
    SimpleTestPlugin();
    virtual ~SimpleTestPlugin();
    
    // IPlugin interface implementation
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    qtplugin::expected<void, qtplugin::PluginError> shutdown() override;
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    
    std::string id() const override { return "simple_test_plugin"; }
    std::string name() const override { return "Simple Test Plugin"; }
    qtplugin::PluginMetadata metadata() const override;
    qtplugin::PluginState state() const override { return m_state; }
    std::vector<std::string> dependencies() const override { return {}; }
    
    // Test-specific methods
    bool is_initialized() const { return m_initialized; }
    int get_counter() const { return m_counter; }
    void increment_counter() { ++m_counter; }
    QString get_config_value() const { return m_config_value; }
    
    // Health check interface (optional)
    qtplugin::expected<qtplugin::PluginHealthStatus, qtplugin::PluginError> 
    check_health() const override;
    
signals:
    void counter_changed(int value);
    void configuration_changed(const QString& key, const QVariant& value);
    
private:
    qtplugin::PluginState m_state;
    bool m_initialized;
    int m_counter;
    QString m_config_value;
    QJsonObject m_current_config;
    int m_health_check_count;
    mutable int m_simulated_failures;
};

} // namespace qtforge::test

#endif // QTFORGE_SIMPLE_TEST_PLUGIN_HPP