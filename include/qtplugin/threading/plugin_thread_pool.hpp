/**
 * @file plugin_thread_pool.hpp
 * @brief Qt threading integration for plugin operations using QThreadPool
 * @version 3.0.0
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QMutex>
#include <QObject>
#include <QRunnable>
#include <QString>
#include <QThreadPool>
#include <QTimer>
#include <QWaitCondition>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <unordered_map>
#include <vector>
#include "../interfaces/core/plugin_interface.hpp"
#include "../utils/error_handling.hpp"

namespace qtplugin {

/**
 * @brief Thread pool task priority
 */
enum class TaskPriority {
    Lowest = 0,
    Low = 25,
    Normal = 50,
    High = 75,
    Highest = 100,
    Critical = 125
};

/**
 * @brief Thread pool task status
 */
enum class TaskStatus {
    Pending,    ///< Task is pending execution
    Running,    ///< Task is currently running
    Completed,  ///< Task completed successfully
    Failed,     ///< Task failed with error
    Cancelled,  ///< Task was cancelled
    Timeout     ///< Task timed out
};

/**
 * @brief Thread pool configuration
 */
struct ThreadPoolConfig {
    int max_thread_count = -1;    ///< Maximum thread count (-1 for auto)
    int ideal_thread_count = -1;  ///< Ideal thread count (-1 for auto)
    std::chrono::milliseconds thread_timeout{30000};  ///< Thread timeout
    std::chrono::milliseconds task_timeout{60000};    ///< Default task timeout
    bool enable_task_monitoring = true;      ///< Enable task monitoring
    bool enable_load_balancing = true;       ///< Enable load balancing
    bool enable_priority_scheduling = true;  ///< Enable priority scheduling
    QJsonObject custom_config;               ///< Custom configuration

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static ThreadPoolConfig from_json(const QJsonObject& json);
};

/**
 * @brief Thread pool task information
 */
struct TaskInfo {
    QString task_id;        ///< Task identifier
    QString task_name;      ///< Task name
    QString plugin_id;      ///< Associated plugin ID
    TaskPriority priority;  ///< Task priority
    TaskStatus status;      ///< Task status
    std::chrono::system_clock::time_point created_time;  ///< Task creation time
    std::chrono::system_clock::time_point start_time;    ///< Task start time
    std::chrono::system_clock::time_point end_time;      ///< Task end time
    std::chrono::milliseconds timeout;                   ///< Task timeout
    QString error_message;  ///< Error message if failed
    QJsonObject metadata;   ///< Task metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Get execution duration
     */
    std::chrono::milliseconds get_execution_duration() const {
        if (status == TaskStatus::Running || status == TaskStatus::Pending) {
            return std::chrono::milliseconds{0};
        }
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time);
    }
};

/**
 * @brief Thread pool statistics
 */
struct ThreadPoolStatistics {
    int active_thread_count = 0;         ///< Active thread count
    int max_thread_count = 0;            ///< Maximum thread count
    uint64_t total_tasks_executed = 0;   ///< Total tasks executed
    uint64_t total_tasks_failed = 0;     ///< Total tasks failed
    uint64_t total_tasks_cancelled = 0;  ///< Total tasks cancelled
    uint64_t total_tasks_timeout = 0;    ///< Total tasks timed out
    std::chrono::milliseconds average_execution_time{
        0};  ///< Average execution time
    std::chrono::milliseconds total_execution_time{
        0};  ///< Total execution time
    std::unordered_map<QString, uint64_t> tasks_by_plugin;  ///< Tasks by plugin
    std::unordered_map<TaskPriority, uint64_t>
        tasks_by_priority;  ///< Tasks by priority

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Task completion callback
 */
using TaskCompletionCallback = std::function<void(const TaskInfo&)>;

/**
 * @brief Task progress callback
 */
using TaskProgressCallback =
    std::function<void(const QString& task_id, int progress)>;

/**
 * @brief Plugin task runnable
 */
class PluginTaskRunnable : public QRunnable {
public:
    PluginTaskRunnable(const QString& task_id,
                       std::function<void()> task_function,
                       TaskCompletionCallback completion_callback = nullptr);

    ~PluginTaskRunnable() override;

    void run() override;

    QString get_task_id() const { return m_task_id; }
    TaskStatus get_status() const { return m_status; }
    QString get_error_message() const { return m_error_message; }

    void cancel();
    bool is_cancelled() const { return m_cancelled.load(); }

private:
    QString m_task_id;
    std::function<void()> m_task_function;
    TaskCompletionCallback m_completion_callback;
    std::atomic<TaskStatus> m_status{TaskStatus::Pending};
    std::atomic<bool> m_cancelled{false};
    QString m_error_message;
};

/**
 * @brief Plugin thread pool manager
 *
 * This class provides Qt threading integration for plugin operations
 * using QThreadPool with task management, priority scheduling, and monitoring.
 */
class PluginThreadPool : public QObject {
    Q_OBJECT

public:
    explicit PluginThreadPool(QObject* parent = nullptr);
    ~PluginThreadPool() override;

    // === Configuration ===

    /**
     * @brief Set thread pool configuration
     * @param config Thread pool configuration
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_config(
        const ThreadPoolConfig& config);

    /**
     * @brief Get thread pool configuration
     * @return Current configuration
     */
    ThreadPoolConfig get_config() const;

    /**
     * @brief Set maximum thread count
     * @param max_threads Maximum thread count
     */
    void set_max_thread_count(int max_threads);

    /**
     * @brief Get maximum thread count
     * @return Maximum thread count
     */
    int get_max_thread_count() const;

    /**
     * @brief Get active thread count
     * @return Active thread count
     */
    int get_active_thread_count() const;

    // === Task Execution ===

    /**
     * @brief Submit task for execution
     * @param task_name Task name
     * @param plugin_id Associated plugin ID
     * @param task_function Task function to execute
     * @param priority Task priority
     * @param timeout Task timeout
     * @param completion_callback Optional completion callback
     * @return Task ID or error
     */
    qtplugin::expected<QString, PluginError> submit_task(
        const QString& task_name, const QString& plugin_id,
        std::function<void()> task_function,
        TaskPriority priority = TaskPriority::Normal,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{60000},
        TaskCompletionCallback completion_callback = nullptr);

    /**
     * @brief Submit async task with future result
     * @param task_name Task name
     * @param plugin_id Associated plugin ID
     * @param task_function Task function to execute
     * @param priority Task priority
     * @param timeout Task timeout
     * @return Future with task result
     */
    template <typename T>
    std::future<qtplugin::expected<T, PluginError>> submit_async_task(
        const QString& task_name, const QString& plugin_id,
        std::function<T()> task_function,
        TaskPriority priority = TaskPriority::Normal,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{60000}) {
        auto promise = std::make_shared<
            std::promise<qtplugin::expected<T, PluginError>>>();
        auto future = promise->get_future();

        auto wrapper_function = [task_function, promise]() {
            try {
                T result = task_function();
                promise->set_value(
                    qtplugin::expected<T, PluginError>(std::move(result)));
            } catch (const std::exception& e) {
                PluginError error;
                error.code = PluginErrorCode::ExecutionFailed;
                error.message = e.what();
                promise->set_value(qtplugin::expected<T, PluginError>(error));
            } catch (...) {
                PluginError error;
                error.code = PluginErrorCode::ExecutionFailed;
                error.message = "Unknown exception in async task";
                promise->set_value(qtplugin::expected<T, PluginError>(error));
            }
        };

        auto task_result = submit_task(task_name, plugin_id, wrapper_function,
                                       priority, timeout);
        if (!task_result) {
            promise->set_value(
                qtplugin::expected<T, PluginError>(task_result.error()));
        }

        return future;
    }

    /**
     * @brief Submit batch of tasks
     * @param tasks Vector of task information and functions
     * @return Vector of task IDs or errors
     */
    std::vector<qtplugin::expected<QString, PluginError>> submit_batch_tasks(
        const std::vector<std::tuple<QString, QString, std::function<void()>,
                                     TaskPriority>>& tasks);

    /**
     * @brief Cancel task
     * @param task_id Task identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> cancel_task(const QString& task_id);

    /**
     * @brief Cancel all tasks for plugin
     * @param plugin_id Plugin identifier
     * @return Number of cancelled tasks
     */
    int cancel_plugin_tasks(const QString& plugin_id);

    /**
     * @brief Cancel all pending tasks
     * @return Number of cancelled tasks
     */
    int cancel_all_tasks();

    // === Task Management ===

    /**
     * @brief Get task information
     * @param task_id Task identifier
     * @return Task information or error
     */
    qtplugin::expected<TaskInfo, PluginError> get_task_info(
        const QString& task_id) const;

    /**
     * @brief Get all tasks
     * @param status_filter Optional status filter
     * @return Vector of task information
     */
    std::vector<TaskInfo> get_all_tasks(
        std::optional<TaskStatus> status_filter = std::nullopt) const;

    /**
     * @brief Get tasks for plugin
     * @param plugin_id Plugin identifier
     * @param status_filter Optional status filter
     * @return Vector of task information
     */
    std::vector<TaskInfo> get_plugin_tasks(
        const QString& plugin_id,
        std::optional<TaskStatus> status_filter = std::nullopt) const;

    /**
     * @brief Get pending tasks count
     * @return Number of pending tasks
     */
    int get_pending_tasks_count() const;

    /**
     * @brief Get running tasks count
     * @return Number of running tasks
     */
    int get_running_tasks_count() const;

    /**
     * @brief Wait for task completion
     * @param task_id Task identifier
     * @param timeout Wait timeout
     * @return true if task completed within timeout
     */
    bool wait_for_task(
        const QString& task_id,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{30000});

    /**
     * @brief Wait for all tasks completion
     * @param timeout Wait timeout
     * @return true if all tasks completed within timeout
     */
    bool wait_for_all_tasks(
        std::chrono::milliseconds timeout = std::chrono::milliseconds{60000});

    // === Plugin Integration ===

    /**
     * @brief Execute plugin method in thread pool
     * @param plugin Plugin instance
     * @param method_name Method name
     * @param arguments Method arguments
     * @param priority Task priority
     * @param timeout Task timeout
     * @return Future with method result
     */
    std::future<qtplugin::expected<QVariant, PluginError>>
    execute_plugin_method(
        std::shared_ptr<IPlugin> plugin, const QString& method_name,
        const QVariantList& arguments = {},
        TaskPriority priority = TaskPriority::Normal,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{30000});

    /**
     * @brief Initialize plugin in thread pool
     * @param plugin Plugin instance
     * @param priority Task priority
     * @param timeout Task timeout
     * @return Future with initialization result
     */
    std::future<qtplugin::expected<void, PluginError>> initialize_plugin_async(
        std::shared_ptr<IPlugin> plugin,
        TaskPriority priority = TaskPriority::High,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{30000});

    /**
     * @brief Shutdown plugin in thread pool
     * @param plugin Plugin instance
     * @param priority Task priority
     * @param timeout Task timeout
     * @return Future with shutdown result
     */
    std::future<qtplugin::expected<void, PluginError>> shutdown_plugin_async(
        std::shared_ptr<IPlugin> plugin,
        TaskPriority priority = TaskPriority::High,
        std::chrono::milliseconds timeout = std::chrono::milliseconds{10000});

    // === Load Balancing ===

    /**
     * @brief Enable load balancing
     * @param enabled Whether to enable load balancing
     */
    void set_load_balancing_enabled(bool enabled);

    /**
     * @brief Check if load balancing is enabled
     * @return true if load balancing is enabled
     */
    bool is_load_balancing_enabled() const;

    /**
     * @brief Get thread load distribution
     * @return Thread load distribution
     */
    QJsonObject get_thread_load_distribution() const;

    /**
     * @brief Balance thread load
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> balance_thread_load();

    // === Statistics and Monitoring ===

    /**
     * @brief Get thread pool statistics
     * @return Thread pool statistics
     */
    ThreadPoolStatistics get_statistics() const;

    /**
     * @brief Reset statistics
     */
    void reset_statistics();

    /**
     * @brief Get plugin execution statistics
     * @param plugin_id Plugin identifier
     * @return Plugin execution statistics
     */
    QJsonObject get_plugin_statistics(const QString& plugin_id) const;

    /**
     * @brief Enable task monitoring
     * @param enabled Whether to enable monitoring
     */
    void set_monitoring_enabled(bool enabled);

    /**
     * @brief Check if task monitoring is enabled
     * @return true if monitoring is enabled
     */
    bool is_monitoring_enabled() const;

signals:
    /**
     * @brief Emitted when task is submitted
     * @param task_id Task identifier
     * @param task_name Task name
     * @param plugin_id Plugin identifier
     */
    void task_submitted(const QString& task_id, const QString& task_name,
                        const QString& plugin_id);

    /**
     * @brief Emitted when task starts execution
     * @param task_id Task identifier
     */
    void task_started(const QString& task_id);

    /**
     * @brief Emitted when task completes
     * @param task_id Task identifier
     * @param success Whether task completed successfully
     * @param execution_time Task execution time
     */
    void task_completed(const QString& task_id, bool success,
                        qint64 execution_time);

    /**
     * @brief Emitted when task is cancelled
     * @param task_id Task identifier
     */
    void task_cancelled(const QString& task_id);

    /**
     * @brief Emitted when task times out
     * @param task_id Task identifier
     */
    void task_timeout(const QString& task_id);

    /**
     * @brief Emitted when thread pool configuration changes
     * @param config New configuration
     */
    void config_changed(const ThreadPoolConfig& config);

private slots:
    void on_monitoring_timer();
    void on_load_balancing_timer();
    void check_task_timeouts();

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::TaskPriority)
Q_DECLARE_METATYPE(qtplugin::TaskStatus)
Q_DECLARE_METATYPE(qtplugin::ThreadPoolConfig)
Q_DECLARE_METATYPE(qtplugin::TaskInfo)
Q_DECLARE_METATYPE(qtplugin::ThreadPoolStatistics)
