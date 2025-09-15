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
#include <qtplugin/core/plugin_interface.hpp>

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
    void shutdown() noexcept override;
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    
    std::string id() const noexcept override { return "simple_test_plugin"; }
    std::string_view name() const noexcept override { return "Simple Test Plugin"; }
    std::string_view description() const noexcept override { return "A simple test plugin"; }
    std::string_view author() const noexcept override { return "QtForge Test Suite"; }
    qtplugin::Version version() const noexcept override { return qtplugin::Version{1, 0, 0}; }
    qtplugin::PluginCapabilities capabilities() const noexcept override { return static_cast<qtplugin::PluginCapabilities>(qtplugin::PluginCapability::Configuration); }
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(std::string_view command, const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override { return {"increment", "get_counter", "set_config"}; }
    qtplugin::PluginState state() const noexcept override { return m_state; }
    std::vector<std::string> dependencies() const override { return {}; }
    
    // Test-specific methods
    bool is_initialized() const noexcept { return m_initialized; }
    int get_counter() const { return m_counter; }
    void increment_counter() { ++m_counter; }
    QString get_config_value() const { return m_config_value; }
    
    
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