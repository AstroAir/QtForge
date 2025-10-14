/**
 * @file service_plugin_interface.hpp
 * @brief Service plugin interface definitions
 * @version 3.2.0
 */

#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <functional>
#include "plugin_interface.hpp"

namespace qtplugin {

/**
 * @brief Service execution modes
 */
enum class ServiceExecutionMode {
    MainThread = 0,    ///< Execute in main thread
    WorkerThread = 1,  ///< Execute in dedicated worker thread
    ThreadPool = 2,    ///< Execute using thread pool
    Async = 3,         ///< Asynchronous execution
    Custom = 4         ///< Custom execution mode
};

/**
 * @brief Service states
 */
enum class ServiceState {
    Stopped = 0,    ///< Service is stopped
    Starting = 1,   ///< Service is starting
    Running = 2,    ///< Service is running
    Pausing = 3,    ///< Service is pausing
    Paused = 4,     ///< Service is paused
    Resuming = 5,   ///< Service is resuming
    Stopping = 6,   ///< Service is stopping
    Error = 7,      ///< Service is in error state
    Restarting = 8  ///< Service is restarting
};

/**
 * @brief Service plugin interface
 */
class IServicePlugin : public IPlugin {
public:
    virtual ~IServicePlugin() = default;

    /**
     * @brief Start the service
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> start_service() = 0;

    /**
     * @brief Stop the service
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> stop_service() = 0;

    /**
     * @brief Pause the service
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> pause_service() = 0;

    /**
     * @brief Resume the service
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> resume_service() = 0;

    /**
     * @brief Restart the service
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> restart_service() = 0;

    /**
     * @brief Get service state
     * @return Current service state
     */
    virtual ServiceState service_state() const noexcept = 0;

    /**
     * @brief Get service execution mode
     * @return Execution mode
     */
    virtual ServiceExecutionMode execution_mode() const noexcept = 0;

    /**
     * @brief Check if service is running
     * @return True if running
     */
    virtual bool is_service_running() const noexcept = 0;

    /**
     * @brief Get service uptime
     * @return Uptime in milliseconds
     */
    virtual std::chrono::milliseconds service_uptime() const noexcept = 0;

    /**
     * @brief Get service metrics
     * @return Service metrics as JSON
     */
    virtual QJsonObject service_metrics() const = 0;

    /**
     * @brief Set service configuration
     * @param config Service configuration
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> configure_service(
        const QJsonObject& config) = 0;

    /**
     * @brief Get service configuration
     * @return Current service configuration
     */
    virtual QJsonObject service_configuration() const = 0;
};

}  // namespace qtplugin
