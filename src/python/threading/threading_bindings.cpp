/**
 * @file threading_bindings.cpp
 * @brief Threading system Python bindings
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/chrono.h>

#include <qtplugin/threading/plugin_thread_pool.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin::threading;

namespace qtforge_python {

void bind_threading(py::module& m) {
    // Thread priority enum
    py::enum_<ThreadPriority>(m, "ThreadPriority")
        .value("Idle", ThreadPriority::Idle)
        .value("Lowest", ThreadPriority::Lowest)
        .value("Low", ThreadPriority::Low)
        .value("Normal", ThreadPriority::Normal)
        .value("High", ThreadPriority::High)
        .value("Highest", ThreadPriority::Highest)
        .value("TimeCritical", ThreadPriority::TimeCritical)
        .export_values();

    // Task status enum
    py::enum_<TaskStatus>(m, "TaskStatus")
        .value("Pending", TaskStatus::Pending)
        .value("Running", TaskStatus::Running)
        .value("Completed", TaskStatus::Completed)
        .value("Failed", TaskStatus::Failed)
        .value("Cancelled", TaskStatus::Cancelled)
        .value("Timeout", TaskStatus::Timeout)
        .export_values();

    // Plugin task
    py::class_<PluginTask>(m, "PluginTask")
        .def(py::init<>())
        .def(py::init<const QString&, const QString&, const QString&>())
        .def_readwrite("task_id", &PluginTask::task_id)
        .def_readwrite("plugin_id", &PluginTask::plugin_id)
        .def_readwrite("method_name", &PluginTask::method_name)
        .def_readwrite("parameters", &PluginTask::parameters)
        .def_readwrite("priority", &PluginTask::priority)
        .def_readwrite("timeout_ms", &PluginTask::timeout_ms)
        .def_readwrite("retry_count", &PluginTask::retry_count)
        .def_readwrite("max_retries", &PluginTask::max_retries)
        .def("status", &PluginTask::status)
        .def("result", &PluginTask::result)
        .def("error_message", &PluginTask::error_message)
        .def("execution_time", &PluginTask::execution_time)
        .def("__repr__", [](const PluginTask& task) {
            return "<PluginTask id='" + task.task_id.toStdString() + 
                   "' plugin='" + task.plugin_id.toStdString() + "'>";
        });

    // Plugin thread pool interface
    py::class_<IPluginThreadPool, std::shared_ptr<IPluginThreadPool>>(
        m, "IPluginThreadPool")
        .def("submit_task", &IPluginThreadPool::submit_task)
        .def("cancel_task", &IPluginThreadPool::cancel_task)
        .def("get_task", &IPluginThreadPool::get_task)
        .def("wait_for_task", &IPluginThreadPool::wait_for_task)
        .def("get_pending_tasks", &IPluginThreadPool::get_pending_tasks)
        .def("get_running_tasks", &IPluginThreadPool::get_running_tasks)
        .def("get_completed_tasks", &IPluginThreadPool::get_completed_tasks)
        .def("clear_completed_tasks", &IPluginThreadPool::clear_completed_tasks)
        .def("set_max_threads", &IPluginThreadPool::set_max_threads)
        .def("get_max_threads", &IPluginThreadPool::get_max_threads)
        .def("get_active_threads", &IPluginThreadPool::get_active_threads)
        .def("shutdown", &IPluginThreadPool::shutdown)
        .def("is_shutdown", &IPluginThreadPool::is_shutdown)
        .def("__repr__", [](const IPluginThreadPool& pool) {
            return "<IPluginThreadPool max_threads=" + 
                   std::to_string(pool.get_max_threads()) + ">";
        });

    // Plugin thread pool implementation
    py::class_<PluginThreadPool, IPluginThreadPool, std::shared_ptr<PluginThreadPool>>(
        m, "PluginThreadPool")
        .def(py::init<int>(), py::arg("max_threads") = 4)
        .def_static("create", &PluginThreadPool::create, py::arg("max_threads") = 4)
        .def("__repr__", [](const PluginThreadPool& pool) {
            return "<PluginThreadPool max_threads=" + 
                   std::to_string(pool.get_max_threads()) + 
                   " active=" + std::to_string(pool.get_active_threads()) + ">";
        });

    // Plugin thread pool manager
    py::class_<PluginThreadPoolManager, std::shared_ptr<PluginThreadPoolManager>>(
        m, "PluginThreadPoolManager")
        .def(py::init<>())
        .def_static("create", &PluginThreadPoolManager::create)
        .def("create_pool", &PluginThreadPoolManager::create_pool)
        .def("get_pool", &PluginThreadPoolManager::get_pool)
        .def("remove_pool", &PluginThreadPoolManager::remove_pool)
        .def("get_default_pool", &PluginThreadPoolManager::get_default_pool)
        .def("set_default_pool", &PluginThreadPoolManager::set_default_pool)
        .def("list_pools", &PluginThreadPoolManager::list_pools)
        .def("shutdown_all", &PluginThreadPoolManager::shutdown_all)
        .def("get_global_stats", &PluginThreadPoolManager::get_global_stats)
        .def("__repr__", [](const PluginThreadPoolManager& manager) {
            auto pools = manager.list_pools();
            return "<PluginThreadPoolManager pools=" + std::to_string(pools.size()) + ">";
        });

    // Utility functions
    m.def(
        "create_thread_pool",
        [](int max_threads = 4) -> std::shared_ptr<PluginThreadPool> {
            return PluginThreadPool::create(max_threads);
        },
        py::arg("max_threads") = 4,
        "Create a new PluginThreadPool instance");

    m.def(
        "create_thread_pool_manager",
        []() -> std::shared_ptr<PluginThreadPoolManager> {
            return PluginThreadPoolManager::create();
        },
        "Create a new PluginThreadPoolManager instance");

    m.def(
        "create_plugin_task",
        [](const std::string& task_id, const std::string& plugin_id, 
           const std::string& method) -> PluginTask {
            return PluginTask(QString::fromStdString(task_id),
                            QString::fromStdString(plugin_id),
                            QString::fromStdString(method));
        },
        py::arg("task_id"), py::arg("plugin_id"), py::arg("method"),
        "Create a new PluginTask instance");

    // Helper functions for common threading patterns
    m.def(
        "execute_async",
        [](std::shared_ptr<PluginThreadPool> pool, const std::string& plugin_id,
           const std::string& method, const QJsonObject& params = QJsonObject{}) -> QString {
            PluginTask task(QString::number(QDateTime::currentMSecsSinceEpoch()),
                          QString::fromStdString(plugin_id),
                          QString::fromStdString(method));
            task.parameters = params;
            
            auto result = pool->submit_task(task);
            return result.value_or(QString{});
        },
        py::arg("pool"), py::arg("plugin_id"), py::arg("method"), py::arg("parameters") = QJsonObject{},
        "Execute a plugin method asynchronously");

    m.def(
        "execute_batch",
        [](std::shared_ptr<PluginThreadPool> pool, 
           const std::vector<std::tuple<std::string, std::string, QJsonObject>>& tasks) -> std::vector<QString> {
            std::vector<QString> task_ids;
            
            for (const auto& [plugin_id, method, params] : tasks) {
                PluginTask task(QString::number(QDateTime::currentMSecsSinceEpoch()) + 
                              QString::number(task_ids.size()),
                              QString::fromStdString(plugin_id),
                              QString::fromStdString(method));
                task.parameters = params;
                
                auto result = pool->submit_task(task);
                if (result) {
                    task_ids.push_back(result.value());
                }
            }
            
            return task_ids;
        },
        py::arg("pool"), py::arg("tasks"),
        "Execute multiple plugin methods in batch");

    m.def(
        "wait_for_all",
        [](std::shared_ptr<PluginThreadPool> pool, const std::vector<QString>& task_ids,
           int timeout_ms = 30000) -> bool {
            for (const auto& task_id : task_ids) {
                auto result = pool->wait_for_task(task_id, std::chrono::milliseconds(timeout_ms));
                if (!result) {
                    return false;
                }
            }
            return true;
        },
        py::arg("pool"), py::arg("task_ids"), py::arg("timeout_ms") = 30000,
        "Wait for all specified tasks to complete");

    m.def(
        "setup_threading_system",
        [](int default_threads = 4, int io_threads = 2, int compute_threads = 4) 
           -> std::shared_ptr<PluginThreadPoolManager> {
            auto manager = PluginThreadPoolManager::create();
            
            // Create default pool
            auto default_pool = manager->create_pool("default", default_threads);
            manager->set_default_pool("default");
            
            // Create specialized pools
            manager->create_pool("io", io_threads);
            manager->create_pool("compute", compute_threads);
            
            return manager;
        },
        py::arg("default_threads") = 4, py::arg("io_threads") = 2, py::arg("compute_threads") = 4,
        "Set up a complete threading system with specialized pools");
}

}  // namespace qtforge_python
