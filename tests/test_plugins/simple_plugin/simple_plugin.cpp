/**
 * @file simple_plugin.cpp
 * @brief Implementation of simple test plugin
 * @version 1.0.0
 * @date 2024-01-13
 */

#include "simple_plugin.hpp"
#include <QDebug>
#include <QThread>
#include <chrono>

namespace qtforge::test {

SimpleTestPlugin::SimpleTestPlugin()
    : m_state(qtplugin::PluginState::Unloaded)
    , m_initialized(false)
    , m_counter(0)
    , m_health_check_count(0)
    , m_simulated_failures(0) {
    qDebug() << "SimpleTestPlugin constructed";
}

SimpleTestPlugin::~SimpleTestPlugin() {
    qDebug() << "SimpleTestPlugin destroyed";
    shutdown(); // shutdown() is noexcept, so always safe to call
}

qtplugin::expected<void, qtplugin::PluginError> 
SimpleTestPlugin::initialize() {
    if (m_initialized) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::AlreadyLoaded,
            "Plugin is already initialized"
        );
    }
    
    qDebug() << "SimpleTestPlugin::initialize() called";
    
    // Simulate initialization work
    QThread::msleep(10);
    
    m_initialized = true;
    m_state = qtplugin::PluginState::Running;
    m_counter = 0;
    
    qDebug() << "SimpleTestPlugin initialized successfully";
    
    return qtplugin::make_success();
}

void SimpleTestPlugin::shutdown() noexcept {
    try {
    if (!m_initialized) {
        return; // Already shut down, nothing to do
    }
    
    qDebug() << "SimpleTestPlugin::shutdown() called";
    
    // Cleanup
    m_initialized = false;
    m_state = qtplugin::PluginState::Unloaded;
    m_counter = 0;
    m_config_value.clear();
    m_current_config = QJsonObject();
    
    qDebug() << "SimpleTestPlugin shutdown successfully";
    } catch (...) {
        // Ignore all exceptions in shutdown
    }
}

qtplugin::expected<void, qtplugin::PluginError> 
SimpleTestPlugin::configure(const QJsonObject& config) {
    if (!m_initialized) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::NotLoaded,
            "Plugin must be initialized before configuration"
        );
    }
    
    qDebug() << "SimpleTestPlugin::configure() called with:" << config;
    
    m_current_config = config;
    
    // Process configuration
    if (config.contains("test_value")) {
        QString old_value = m_config_value;
        m_config_value = config["test_value"].toString();
        
        if (old_value != m_config_value) {
            emit configuration_changed("test_value", m_config_value);
        }
    }
    
    if (config.contains("counter_init")) {
        int new_counter = config["counter_init"].toInt();
        if (new_counter != m_counter) {
            m_counter = new_counter;
            emit counter_changed(m_counter);
        }
    }
    
    // Simulate configuration validation
    if (config.contains("fail_config") && config["fail_config"].toBool()) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError,
            "Configuration validation failed: fail_config is true"
        );
    }
    
    // Support hot reload testing
    if (config.contains("hot_reload")) {
        qDebug() << "Hot reload configuration applied";
        // Apply configuration without restart
        if (config["hot_reload"].toObject().contains("increment_counter")) {
            int increment = config["hot_reload"].toObject()["increment_counter"].toInt();
            m_counter += increment;
            emit counter_changed(m_counter);
        }
    }
    
    qDebug() << "SimpleTestPlugin configured successfully";
    
    return qtplugin::make_success();
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> 
SimpleTestPlugin::execute_command(std::string_view command, const QJsonObject& params) {
    if (!m_initialized) {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::InvalidState,
            "Plugin must be initialized before executing commands"
        );
    }
    
    QJsonObject result;
    
    if (command == "increment") {
        increment_counter();
        result["counter"] = m_counter;
        result["success"] = true;
    } else if (command == "get_counter") {
        result["counter"] = m_counter;
        result["success"] = true;
    } else if (command == "set_config") {
        if (params.contains("value")) {
            m_config_value = params["value"].toString();
            result["config_value"] = m_config_value;
            result["success"] = true;
        } else {
            return qtplugin::make_error<QJsonObject>(
                qtplugin::PluginErrorCode::InvalidArgument,
                "Missing 'value' parameter for set_config command"
            );
        }
    } else {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            std::string("Unknown command: ") + std::string(command)
        );
    }
    
    return result;
}


} // namespace qtforge::test