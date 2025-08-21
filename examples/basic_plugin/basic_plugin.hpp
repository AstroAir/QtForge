/**
 * @file basic_plugin.hpp
 * @brief Example basic plugin demonstrating the QtPlugin system
 * @version 3.0.0
 */

#pragma once

#include <qtplugin/qtplugin.hpp>
#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <string_view>
#include <vector>
#include <memory>
#include <memory>
#include <string>
#include <string_view>
#include <chrono>
#include <atomic>

/**
 * @brief Basic example plugin
 * 
 * This plugin demonstrates the basic functionality of the QtPlugin system
 * including lifecycle management, configuration, commands, and messaging.
 */
class BasicPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "basic_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit BasicPlugin(QObject* parent = nullptr);
    ~BasicPlugin() override;

    // === IPlugin Interface ===
    
    // Metadata
    std::string_view name() const noexcept override {
        return "Basic Example Plugin";
    }
    
    std::string_view description() const noexcept override {
        return "A basic plugin demonstrating the QtPlugin system capabilities";
    }
    
    qtplugin::Version version() const noexcept override {
        return {1, 0, 0};
    }
    
    std::string_view author() const noexcept override {
        return "QtPlugin Development Team";
    }
    
    std::string id() const noexcept override {
        return "com.example.basic_plugin";
    }
    
    std::string_view category() const noexcept override {
        return "Example";
    }
    
    std::string_view license() const noexcept override {
        return "MIT";
    }
    
    std::string_view homepage() const noexcept override {
        return "https://github.com/example/qtplugin";
    }
    
    // Lifecycle
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override { return m_state; }
    
    // Capabilities
    qtplugin::PluginCapabilities capabilities() const noexcept override {
        return qtplugin::PluginCapability::Service | 
               qtplugin::PluginCapability::Configuration |
               qtplugin::PluginCapability::Logging;
    }
    
    // Configuration
    std::optional<QJsonObject> default_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;
    bool validate_configuration(const QJsonObject& config) const override;
    
    // Commands
    qtplugin::expected<QJsonObject, qtplugin::PluginError>
    execute_command(std::string_view command, const QJsonObject& params = {}) override;
    
    std::vector<std::string> available_commands() const override;
    
    // Error handling
    std::string last_error() const override { return m_last_error; }
    std::vector<std::string> error_log() const override { return m_error_log; }
    void clear_errors() override;
    
    // Monitoring
    std::chrono::milliseconds uptime() const override;
    QJsonObject performance_metrics() const override;
    QJsonObject resource_usage() const override;
    
    // Threading
    bool is_thread_safe() const noexcept override { return true; }
    std::string_view thread_model() const noexcept override { return "thread-safe"; }

private slots:
    void on_timer_timeout();
    void on_message_received();

private:
    // State management
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    std::chrono::system_clock::time_point m_initialization_time;
    
    // Configuration
    QJsonObject m_configuration;
    int m_timer_interval = 5000;  // Default 5 seconds
    bool m_logging_enabled = true;
    std::string m_custom_message = "Hello from BasicPlugin!";
    
    // Error handling
    mutable std::string m_last_error;
    mutable std::vector<std::string> m_error_log;
    static constexpr size_t MAX_ERROR_LOG_SIZE = 100;
    
    // Monitoring
    std::atomic<uint64_t> m_command_count{0};
    std::atomic<uint64_t> m_message_count{0};
    std::atomic<uint64_t> m_error_count{0};
    
    // Timer for periodic tasks
    std::unique_ptr<QTimer> m_timer;
    
    // Helper methods
    void log_error(const std::string& error);
    void log_info(const std::string& message);
    void update_metrics();
    
    // Command handlers
    QJsonObject handle_status_command(const QJsonObject& params);
    QJsonObject handle_echo_command(const QJsonObject& params);
    QJsonObject handle_config_command(const QJsonObject& params);
    QJsonObject handle_metrics_command(const QJsonObject& params);
    QJsonObject handle_test_command(const QJsonObject& params);
};

/**
 * @brief Plugin factory for creating BasicPlugin instances
 */
class BasicPluginFactory : public QObject {
    Q_OBJECT
    
public:
    explicit BasicPluginFactory(QObject* parent = nullptr) : QObject(parent) {}
    
    /**
     * @brief Create a new BasicPlugin instance
     * @param parent Parent QObject
     * @param config Initial configuration
     * @return Unique pointer to the created plugin
     */
    static std::unique_ptr<BasicPlugin> create_plugin(QObject* parent = nullptr,
                                                     const QJsonObject& config = {}) {
        auto plugin = std::make_unique<BasicPlugin>(parent);
        
        if (!config.isEmpty()) {
            auto result = plugin->configure(config);
            if (!result) {
                // Log configuration error but still return the plugin
                qWarning() << "Failed to configure BasicPlugin:" << result.error().message.c_str();
            }
        }
        
        return plugin;
    }
    
    /**
     * @brief Check if factory can create plugin with given requirements
     * @param requirements Plugin requirements
     * @return true if plugin can be created with the requirements
     */
    static bool can_create_with_requirements(const QJsonObject& requirements) {
        // Check if we support the required capabilities
        if (requirements.contains("required_capabilities")) {
            auto required_caps = requirements["required_capabilities"].toArray();
            for (const auto& cap : required_caps) {
                QString cap_str = cap.toString();
                if (cap_str != "Service" && cap_str != "Configuration" && cap_str != "Logging") {
                    return false;
                }
            }
        }
        
        // Check version requirements
        if (requirements.contains("min_version")) {
            auto min_version = qtplugin::Version::parse(requirements["min_version"].toString().toStdString());
            if (min_version && *min_version > qtplugin::Version{1, 0, 0}) {
                return false;
            }
        }
        
        return true;
    }
};

// Example usage function
namespace qtplugin::examples {

/**
 * @brief Demonstrate basic plugin usage
 * 
 * This function shows how to create, configure, and use a basic plugin
 * in a pure C++ application.
 */
inline void demonstrate_basic_plugin() {
    // Initialize the QtPlugin library
    qtplugin::LibraryInitializer init;
    if (!init.is_initialized()) {
        qCritical() << "Failed to initialize QtPlugin library";
        return;
    }
    
    // Create plugin manager
    qtplugin::PluginManager manager;
    
    // Create and configure the plugin
    auto plugin = BasicPluginFactory::create_plugin();
    
    // Initialize the plugin
    auto init_result = plugin->initialize();
    if (!init_result) {
        qCritical() << "Failed to initialize plugin:" << init_result.error().message.c_str();
        return;
    }
    
    qInfo() << "Plugin initialized successfully";
    qInfo() << "Plugin name:" << plugin->name().data();
    qInfo() << "Plugin version:" << plugin->version().to_string().c_str();
    qInfo() << "Plugin description:" << plugin->description().data();
    
    // Configure the plugin
    QJsonObject config{
        {"timer_interval", 3000},
        {"logging_enabled", true},
        {"custom_message", "Hello from configured plugin!"}
    };
    
    auto config_result = plugin->configure(config);
    if (config_result) {
        qInfo() << "Plugin configured successfully";
    }
    
    // Execute some commands
    auto status_result = plugin->execute_command("status");
    if (status_result) {
        qInfo() << "Status command result:" << QJsonDocument(status_result.value()).toJson();
    }
    
    auto echo_result = plugin->execute_command("echo", QJsonObject{{"message", "Test message"}});
    if (echo_result) {
        qInfo() << "Echo command result:" << QJsonDocument(echo_result.value()).toJson();
    }
    
    // Get performance metrics
    auto metrics = plugin->performance_metrics();
    qInfo() << "Plugin metrics:" << QJsonDocument(metrics).toJson();
    
    // Shutdown the plugin
    plugin->shutdown();
    qInfo() << "Plugin shutdown completed";
}

} // namespace qtplugin::examples

// MOC include will be generated automatically
