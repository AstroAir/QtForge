/**
 * @file service_plugin_interface.hpp
 * @brief Service plugin interface definitions using modern C++ features
 * @version 3.0.0
 */

#pragma once

#include <QThread>
#include <QTimer>
#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include "plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Service execution modes
 */
enum class ServiceExecutionMode {
    MainThread,    ///< Service runs in the main thread
    WorkerThread,  ///< Service runs in a dedicated worker thread
    ThreadPool,    ///< Service uses a thread pool for tasks
    Async,         ///< Service uses async/await patterns
    Custom         ///< Service manages its own threading
};

/**
 * @brief Service lifecycle states
 */
enum class ServiceState {
    Stopped,    ///< Service is stopped
    Starting,   ///< Service is starting up
    Running,    ///< Service is running normally
    Pausing,    ///< Service is being paused
    Paused,     ///< Service is paused
    Resuming,   ///< Service is resuming from pause
    Stopping,   ///< Service is shutting down
    Error,      ///< Service is in error state
    Restarting  ///< Service is restarting
};

/**
 * @brief Service priority levels
 */
enum class ServicePriority {
    Idle = 0,     ///< Lowest priority, runs when system is idle
    Low = 1,      ///< Low priority background service
    Normal = 2,   ///< Normal priority service
    High = 3,     ///< High priority service
    Critical = 4  ///< Critical system service
};

/**
 * @brief Service health status
 */
enum class ServiceHealth {
    Unknown,   ///< Health status unknown
    Healthy,   ///< Service is healthy
    Warning,   ///< Service has warnings but is functional
    Critical,  ///< Service has critical issues
    Unhealthy  ///< Service is not functioning properly
};

/**
 * @brief Service plugin interface
 *
 * This interface extends the base plugin interface with service-specific
 * functionality for background services, scheduled tasks, and long-running
 * operations.
 */
class IServicePlugin : public virtual IPlugin {
public:
    ~IServicePlugin() override = default;

    // === Service Lifecycle ===

    /**
     * @brief Start the service
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> start_service() = 0;

    /**
     * @brief Stop the service
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> stop_service() = 0;

    /**
     * @brief Pause the service
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> pause_service() {
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Pause not supported");
    }

    /**
     * @brief Resume the service from paused state
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> resume_service() {
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Resume not supported");
    }

    /**
     * @brief Restart the service
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> restart_service() {
        if (auto result = stop_service(); !result) {
            return result;
        }
        return start_service();
    }

    /**
     * @brief Check if service is running
     * @return true if service is running
     */
    virtual bool is_service_running() const noexcept = 0;

    /**
     * @brief Get current service state
     * @return Current service state
     */
    virtual ServiceState service_state() const noexcept = 0;

    // === Service Configuration ===

    /**
     * @brief Get service execution mode
     * @return Service execution mode
     */
    virtual ServiceExecutionMode execution_mode() const noexcept {
        return ServiceExecutionMode::MainThread;
    }

    /**
     * @brief Get service priority
     * @return Service priority level
     */
    virtual ServicePriority service_priority() const noexcept {
        return ServicePriority::Normal;
    }

    /**
     * @brief Check if service supports pausing
     * @return true if service can be paused and resumed
     */
    virtual bool supports_pause() const noexcept { return false; }

    /**
     * @brief Check if service is auto-start enabled
     * @return true if service should start automatically
     */
    virtual bool is_auto_start() const noexcept { return false; }

    /**
     * @brief Set auto-start behavior
     * @param enabled true to enable auto-start
     */
    virtual void set_auto_start(bool enabled) {
        Q_UNUSED(enabled)
        // Default implementation does nothing
    }

    // === Service Status and Monitoring ===

    /**
     * @brief Get service status information
     * @return Service status as JSON object
     */
    virtual QJsonObject service_status() const = 0;

    /**
     * @brief Get service health status
     * @return Current health status
     */
    virtual ServiceHealth health_status() const noexcept {
        return is_service_running() ? ServiceHealth::Healthy
                                    : ServiceHealth::Unknown;
    }

    /**
     * @brief Get service uptime
     * @return Duration since service was started
     */
    virtual std::chrono::milliseconds service_uptime() const {
        return std::chrono::milliseconds{0};
    }

    /**
     * @brief Get service performance metrics
     * @return Performance metrics as JSON object
     */
    virtual QJsonObject service_metrics() const { return QJsonObject{}; }

    /**
     * @brief Get service resource usage
     * @return Resource usage information as JSON object
     */
    virtual QJsonObject resource_usage() const { return QJsonObject{}; }

    // === Service Dependencies ===

    /**
     * @brief Get required service dependencies
     * @return Vector of required service identifiers
     */
    virtual std::vector<std::string> service_dependencies() const { return {}; }

    /**
     * @brief Get optional service dependencies
     * @return Vector of optional service identifiers
     */
    virtual std::vector<std::string> optional_service_dependencies() const {
        return {};
    }

    /**
     * @brief Check if service dependencies are satisfied
     * @return true if all required dependencies are available
     */
    virtual bool service_dependencies_satisfied() const {
        return true;  // Override in derived classes
    }

    // === Threading Support ===

    /**
     * @brief Get service thread (if running in worker thread)
     * @return Pointer to service thread, or nullptr if not applicable
     */
    virtual QThread* service_thread() const { return nullptr; }

    /**
     * @brief Check if service runs in separate thread
     * @return true if service uses a separate thread
     */
    virtual bool runs_in_separate_thread() const noexcept {
        return execution_mode() == ServiceExecutionMode::WorkerThread;
    }

    /**
     * @brief Get thread affinity mask (for multi-core systems)
     * @return Thread affinity mask, or 0 for no specific affinity
     */
    virtual uint64_t thread_affinity() const noexcept {
        return 0;  // No specific affinity
    }

    // === Async Operations ===

    /**
     * @brief Start service asynchronously
     * @return Future that completes when service is started
     */
    virtual std::future<qtplugin::expected<void, PluginError>>
    start_service_async() {
        std::promise<qtplugin::expected<void, PluginError>> promise;
        auto future = promise.get_future();
        promise.set_value(start_service());
        return future;
    }

    /**
     * @brief Stop service asynchronously
     * @return Future that completes when service is stopped
     */
    virtual std::future<qtplugin::expected<void, PluginError>>
    stop_service_async() {
        std::promise<qtplugin::expected<void, PluginError>> promise;
        auto future = promise.get_future();
        promise.set_value(stop_service());
        return future;
    }

    // === Service Events ===

    /**
     * @brief Handle service start event
     *
     * Called when the service is starting. Override to perform
     * custom initialization logic.
     */
    virtual void on_service_starting() {
        // Default implementation does nothing
    }

    /**
     * @brief Handle service started event
     *
     * Called when the service has successfully started.
     */
    virtual void on_service_started() {
        // Default implementation does nothing
    }

    /**
     * @brief Handle service stopping event
     *
     * Called when the service is stopping. Override to perform
     * custom cleanup logic.
     */
    virtual void on_service_stopping() {
        // Default implementation does nothing
    }

    /**
     * @brief Handle service stopped event
     *
     * Called when the service has been stopped.
     */
    virtual void on_service_stopped() {
        // Default implementation does nothing
    }

    /**
     * @brief Handle service error event
     * @param error Error that occurred
     */
    virtual void on_service_error(const PluginError& error) {
        Q_UNUSED(error)
        // Default implementation does nothing
    }

    // === Scheduled Operations ===

    /**
     * @brief Check if service supports scheduled operations
     * @return true if service can perform scheduled tasks
     */
    virtual bool supports_scheduling() const noexcept { return false; }

    /**
     * @brief Schedule a task to run at specified interval
     * @param task_name Name of the task
     * @param interval Interval between executions
     * @param immediate true to run immediately, false to wait for first
     * interval
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> schedule_task(
        std::string_view task_name, std::chrono::milliseconds interval,
        bool immediate = false) {
        Q_UNUSED(task_name)
        Q_UNUSED(interval)
        Q_UNUSED(immediate)
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Scheduling not supported");
    }

    /**
     * @brief Cancel a scheduled task
     * @param task_name Name of the task to cancel
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> cancel_task(
        std::string_view task_name) {
        Q_UNUSED(task_name)
        return make_error<void>(PluginErrorCode::CommandNotFound,
                                "Scheduling not supported");
    }

    /**
     * @brief Get list of scheduled tasks
     * @return Vector of scheduled task names
     */
    virtual std::vector<std::string> scheduled_tasks() const { return {}; }

    // === Health Monitoring ===

    /**
     * @brief Perform health check
     * @return Health check result with details
     */
    virtual qtplugin::expected<ServiceHealth, PluginError>
    perform_health_check() {
        return ServiceHealth::Healthy;
    }

    /**
     * @brief Get health check interval
     * @return Interval between automatic health checks
     */
    virtual std::chrono::milliseconds health_check_interval() const {
        return std::chrono::minutes{5};  // Default: 5 minutes
    }

    /**
     * @brief Check if automatic health monitoring is enabled
     * @return true if health monitoring is enabled
     */
    virtual bool is_health_monitoring_enabled() const noexcept { return false; }

    /**
     * @brief Enable or disable health monitoring
     * @param enabled true to enable health monitoring
     */
    virtual void set_health_monitoring_enabled(bool enabled) {
        Q_UNUSED(enabled)
        // Default implementation does nothing
    }

    // === Service Recovery ===

    /**
     * @brief Check if service supports automatic recovery
     * @return true if service can recover from errors automatically
     */
    virtual bool supports_auto_recovery() const noexcept { return false; }

    /**
     * @brief Attempt to recover from error state
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> recover() {
        return restart_service();
    }

    /**
     * @brief Get maximum recovery attempts
     * @return Maximum number of automatic recovery attempts
     */
    virtual int max_recovery_attempts() const noexcept { return 3; }

    /**
     * @brief Get recovery delay
     * @return Delay between recovery attempts
     */
    virtual std::chrono::milliseconds recovery_delay() const {
        return std::chrono::seconds{30};
    }
};

/**
 * @brief Service plugin factory interface
 */
class IServicePluginFactory {
public:
    virtual ~IServicePluginFactory() = default;

    /**
     * @brief Create service plugin instance
     * @param config Service configuration
     * @return Unique pointer to the created service plugin
     */
    virtual std::unique_ptr<IServicePlugin> create_service_plugin(
        const QJsonObject& config = {}) = 0;

    /**
     * @brief Check if factory can create service with given requirements
     * @param requirements Service requirements
     * @return true if service can be created with the requirements
     */
    virtual bool can_create_with_requirements(
        const QJsonObject& requirements) const = 0;

    /**
     * @brief Get supported execution modes
     * @return Vector of supported execution modes
     */
    virtual std::vector<ServiceExecutionMode> supported_execution_modes()
        const = 0;
};

}  // namespace qtplugin

Q_DECLARE_INTERFACE(qtplugin::IServicePlugin, "qtplugin.IServicePlugin/3.0")
Q_DECLARE_INTERFACE(qtplugin::IServicePluginFactory,
                    "qtplugin.IServicePluginFactory/3.0")
