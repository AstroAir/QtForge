/**
 * @file service_plugin.hpp
 * @brief Comprehensive service plugin demonstrating background processing and
 * MessageBus integration
 * @version 3.0.0
 *
 * This service plugin demonstrates advanced QtPlugin system capabilities
 * including:
 * - Background processing with QTimer and QThread
 * - Inter-plugin communication using MessageBus
 * - Service registration and discovery
 * - Resource management and monitoring
 * - Real-world service patterns
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QReadWriteLock>
#include <QThread>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/communication/plugin_service_discovery.hpp>
#include <qtplugin/qtplugin.hpp>
#include <queue>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <vector>

/**
 * @brief Service status message for inter-plugin communication
 */
class ServiceStatusMessage : public qtplugin::Message<ServiceStatusMessage> {
public:
    ServiceStatusMessage(std::string_view sender, const QString& service_name,
                         const QString& status, const QJsonObject& data = {})
        : qtplugin::Message<ServiceStatusMessage>(sender),
          m_service_name(service_name),
          m_status(status),
          m_data(data) {}

    std::string_view type() const noexcept override { return "ServiceStatus"; }
    QJsonObject to_json() const override {
        return QJsonObject{
            {"type", "ServiceStatus"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"service_name", m_service_name},
            {"status", m_status},
            {"data", m_data},
            {"timestamp",
             QString::number(timestamp().time_since_epoch().count())}};
    }

    const QString& service_name() const { return m_service_name; }
    const QString& status() const { return m_status; }
    const QJsonObject& data() const { return m_data; }

private:
    QString m_service_name;
    QString m_status;
    QJsonObject m_data;
};

/**
 * @brief Service request message for inter-plugin communication
 */
class ServiceRequestMessage : public qtplugin::Message<ServiceRequestMessage> {
public:
    ServiceRequestMessage(std::string_view sender, const QString& service_name,
                          const QString& operation,
                          const QJsonObject& params = {})
        : qtplugin::Message<ServiceRequestMessage>(sender),
          m_service_name(service_name),
          m_operation(operation),
          m_params(params) {}

    std::string_view type() const noexcept override { return "ServiceRequest"; }
    QJsonObject to_json() const override {
        return QJsonObject{
            {"type", "ServiceRequest"},
            {"sender", QString::fromStdString(std::string(sender()))},
            {"service_name", m_service_name},
            {"operation", m_operation},
            {"params", m_params},
            {"timestamp",
             QString::number(timestamp().time_since_epoch().count())}};
    }

    const QString& service_name() const { return m_service_name; }
    const QString& operation() const { return m_operation; }
    const QJsonObject& params() const { return m_params; }

private:
    QString m_service_name;
    QString m_operation;
    QJsonObject m_params;
};

/**
 * @brief Background worker for processing tasks
 */
class ServiceWorker : public QObject {
    Q_OBJECT

public:
    explicit ServiceWorker(QObject* parent = nullptr);
    ~ServiceWorker() override;

public slots:
    void process_task(const QJsonObject& task);
    void start_processing();
    void stop_processing();

signals:
    void task_completed(const QJsonObject& result);
    void task_failed(const QString& error);
    void status_changed(const QString& status);

private:
    std::atomic<bool> m_running{false};
    std::queue<QJsonObject> m_task_queue;
    mutable std::mutex m_queue_mutex;
    std::atomic<uint64_t> m_processed_tasks{0};
    std::atomic<uint64_t> m_failed_tasks{0};
};

/**
 * @brief Comprehensive service plugin
 *
 * This plugin demonstrates advanced service patterns including:
 * - Background processing with worker threads
 * - MessageBus integration for inter-plugin communication
 * - Service registration and discovery
 * - Resource monitoring and management
 * - Real-world service lifecycle management
 */
class ServicePlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IPlugin/3.0" FILE "service_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit ServicePlugin(QObject* parent = nullptr);
    ~ServicePlugin() override;

    // === IPlugin Interface ===

    // Metadata
    std::string_view name() const noexcept override {
        return "Service Example Plugin";
    }

    std::string_view description() const noexcept override {
        return "A comprehensive service plugin demonstrating background "
               "processing and MessageBus integration";
    }

    qtplugin::Version version() const noexcept override { return {1, 0, 0}; }

    std::string_view author() const noexcept override {
        return "QtPlugin Development Team";
    }

    std::string id() const noexcept override {
        return "com.example.service_plugin";
    }

    std::string_view category() const noexcept override { return "Service"; }

    std::string_view license() const noexcept override { return "MIT"; }

    std::string_view homepage() const noexcept override {
        return "https://github.com/example/qtplugin";
    }

    // === Lifecycle Management ===
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override { return m_state; }
    bool is_initialized() const noexcept override;

    qtplugin::expected<void, qtplugin::PluginError> pause() override;
    qtplugin::expected<void, qtplugin::PluginError> resume() override;
    qtplugin::expected<void, qtplugin::PluginError> restart() override;

    // === Capabilities ===
    qtplugin::PluginCapabilities capabilities() const noexcept override {
        return qtplugin::PluginCapability::Service |
               qtplugin::PluginCapability::Configuration |
               qtplugin::PluginCapability::Logging |
               qtplugin::PluginCapability::Monitoring |
               qtplugin::PluginCapability::Threading |
               qtplugin::PluginCapability::AsyncInit;
    }

    qtplugin::PluginPriority priority() const noexcept override {
        return qtplugin::PluginPriority::High;
    }

    // === Configuration ===
    std::optional<QJsonObject> default_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> configure(
        const QJsonObject& config) override;
    QJsonObject current_configuration() const override;
    bool validate_configuration(const QJsonObject& config) const override;

    // === Commands ===
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override;

    std::vector<std::string> available_commands() const override;

    // === Dependencies ===
    std::vector<std::string> dependencies() const override;
    std::vector<std::string> optional_dependencies() const override;
    bool dependencies_satisfied() const override;

    // === Error Handling ===
    std::string last_error() const override { return m_last_error; }
    std::vector<std::string> error_log() const override { return m_error_log; }
    void clear_errors() override;

    // === Monitoring ===
    std::chrono::milliseconds uptime() const override;
    QJsonObject performance_metrics() const override;
    QJsonObject resource_usage() const override;

    // === Threading ===
    bool is_thread_safe() const noexcept override { return true; }
    std::string_view thread_model() const noexcept override {
        return "multi-threaded";
    }

private slots:
    void on_processing_timer_timeout();
    void on_heartbeat_timer_timeout();
    void on_task_completed(const QJsonObject& result);
    void on_task_failed(const QString& error);
    void on_worker_status_changed(const QString& status);
    void on_service_request_received(
        std::shared_ptr<ServiceRequestMessage> message);

private:
    // === State Management ===
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    std::chrono::system_clock::time_point m_initialization_time;
    mutable std::shared_mutex m_state_mutex;

    // === Configuration ===
    QJsonObject m_configuration;
    mutable std::mutex m_config_mutex;
    int m_processing_interval = 5000;  // Default 5 seconds
    int m_heartbeat_interval = 30000;  // Default 30 seconds
    bool m_logging_enabled = true;
    QString m_service_name = "ExampleService";
    int m_max_concurrent_tasks = 10;

    // === Background Processing ===
    std::unique_ptr<QTimer> m_processing_timer;
    std::unique_ptr<QTimer> m_heartbeat_timer;
    std::unique_ptr<QThread> m_worker_thread;
    std::unique_ptr<ServiceWorker> m_worker;

    // === MessageBus Integration ===
    qtplugin::MessageBus* m_message_bus = nullptr;

    // === Service Registration ===
    qtplugin::PluginServiceDiscovery* m_service_discovery = nullptr;
    QString m_registered_service_id;

    // === Error Handling ===
    mutable std::string m_last_error;
    mutable std::vector<std::string> m_error_log;
    mutable std::mutex m_error_mutex;
    static constexpr size_t MAX_ERROR_LOG_SIZE = 100;

    // === Monitoring ===
    std::atomic<uint64_t> m_command_count{0};
    std::atomic<uint64_t> m_message_count{0};
    std::atomic<uint64_t> m_error_count{0};
    std::atomic<uint64_t> m_tasks_submitted{0};
    std::atomic<uint64_t> m_tasks_processed{0};
    std::atomic<uint64_t> m_tasks_failed{0};
    mutable std::mutex m_metrics_mutex;

    // === Dependencies ===
    std::vector<std::string> m_required_dependencies;
    std::vector<std::string> m_optional_dependencies;
    std::atomic<bool> m_dependencies_satisfied{true};

    // === Helper Methods ===
    void log_error(const std::string& error);
    void log_info(const std::string& message);
    void update_metrics();

    // === Service Management ===
    qtplugin::expected<void, qtplugin::PluginError> register_service();
    qtplugin::expected<void, qtplugin::PluginError> unregister_service();
    void publish_status_update(const QString& status,
                               const QJsonObject& data = {});
    void setup_message_subscriptions();

    // === Command Handlers ===
    QJsonObject handle_status_command(const QJsonObject& params);
    QJsonObject handle_service_command(const QJsonObject& params);
    QJsonObject handle_task_command(const QJsonObject& params);
    QJsonObject handle_message_command(const QJsonObject& params);
    QJsonObject handle_monitoring_command(const QJsonObject& params);
};
