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
    if (m_initialized) {
        shutdown();
    }
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
    m_state = qtplugin::PluginState::Active;
    m_counter = 0;
    
    qDebug() << "SimpleTestPlugin initialized successfully";
    
    return qtplugin::make_success();
}

qtplugin::expected<void, qtplugin::PluginError> 
SimpleTestPlugin::shutdown() {
    if (!m_initialized) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::NotLoaded,
            "Plugin is not initialized"
        );
    }
    
    qDebug() << "SimpleTestPlugin::shutdown() called";
    
    // Cleanup
    m_initialized = false;
    m_state = qtplugin::PluginState::Unloaded;
    m_counter = 0;
    m_config_value.clear();
    m_current_config = QJsonObject();
    
    qDebug() << "SimpleTestPlugin shutdown successfully";
    
    return qtplugin::make_success();
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

qtplugin::PluginMetadata SimpleTestPlugin::metadata() const {
    qtplugin::PluginMetadata meta;
    
    meta.id = id();
    meta.name = name();
    meta.description = "A simple test plugin for validating the plugin system";
    meta.author = "QtForge Test Suite";
    meta.version = qtplugin::PluginVersion{1, 0, 0};
    meta.category = "Testing";
    meta.tags = {"test", "simple", "example"};
    meta.license = "MIT";
    meta.homepage = "https://qtforge.example.com/plugins/simple";
    meta.dependencies = dependencies();
    
    // Capabilities
    meta.capabilities["hot_reload"] = true;
    meta.capabilities["health_check"] = true;
    meta.capabilities["configuration"] = true;
    meta.capabilities["thread_safe"] = true;
    
    // Requirements
    meta.min_qt_version = qtplugin::PluginVersion{5, 15, 0};
    meta.min_system_version = qtplugin::PluginVersion{1, 0, 0};
    
    return meta;
}

qtplugin::expected<qtplugin::PluginHealthStatus, qtplugin::PluginError> 
SimpleTestPlugin::check_health() const {
    qtplugin::PluginHealthStatus status;
    
    // Increment health check counter
    const_cast<SimpleTestPlugin*>(this)->m_health_check_count++;
    
    // Simulate occasional failures for testing
    if (m_current_config.contains("simulate_unhealthy")) {
        int failure_rate = m_current_config["simulate_unhealthy"].toInt();
        if ((m_health_check_count % failure_rate) == 0) {
            m_simulated_failures++;
            status.is_healthy = false;
            status.status_message = "Simulated failure for testing";
            status.consecutive_failures = m_simulated_failures;
        } else {
            m_simulated_failures = 0;
            status.is_healthy = true;
            status.status_message = "Plugin is healthy";
            status.consecutive_failures = 0;
        }
    } else {
        // Normal health check
        if (!m_initialized) {
            status.is_healthy = false;
            status.status_message = "Plugin not initialized";
        } else if (m_state != qtplugin::PluginState::Active) {
            status.is_healthy = false;
            status.status_message = "Plugin not in active state";
        } else {
            status.is_healthy = true;
            status.status_message = "All systems operational";
        }
        status.consecutive_failures = 0;
    }
    
    status.last_check_time = std::chrono::steady_clock::now();
    
    return status;
}

} // namespace qtforge::test