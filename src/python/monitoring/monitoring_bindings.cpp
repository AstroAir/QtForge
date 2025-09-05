/**
 * @file monitoring_bindings.cpp
 * @brief Monitoring system Python bindings
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/monitoring/plugin_hot_reload_manager.hpp>
#include <qtplugin/monitoring/plugin_metrics_collector.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_monitoring(py::module& m) {
    // Hot reload manager interface
    py::class_<IPluginHotReloadManager,
               std::shared_ptr<IPluginHotReloadManager>>(
        m, "IPluginHotReloadManager")
        .def("enable_hot_reload", &IPluginHotReloadManager::enable_hot_reload)
        .def("disable_hot_reload", &IPluginHotReloadManager::disable_hot_reload)
        .def("is_hot_reload_enabled",
             &IPluginHotReloadManager::is_hot_reload_enabled)
        .def("set_reload_callback",
             &IPluginHotReloadManager::set_reload_callback)
        .def("get_hot_reload_plugins",
             &IPluginHotReloadManager::get_hot_reload_plugins)
        .def("clear", &IPluginHotReloadManager::clear)
        .def("set_global_hot_reload_enabled",
             &IPluginHotReloadManager::set_global_hot_reload_enabled)
        .def("is_global_hot_reload_enabled",
             &IPluginHotReloadManager::is_global_hot_reload_enabled)
        .def("__repr__", [](const IPluginHotReloadManager& manager) {
            return "<IPluginHotReloadManager>";
        });

    // Hot reload manager implementation
    py::class_<PluginHotReloadManager, IPluginHotReloadManager,
               std::shared_ptr<PluginHotReloadManager>>(
        m, "PluginHotReloadManager")
        .def(py::init<QObject*>(), py::arg("parent") = nullptr)
        .def("__repr__", [](const PluginHotReloadManager& manager) {
            auto plugins = manager.get_hot_reload_plugins();
            return "<PluginHotReloadManager plugins=" +
                   std::to_string(plugins.size()) + ">";
        });

    // Metrics collector interface
    py::class_<IPluginMetricsCollector,
               std::shared_ptr<IPluginMetricsCollector>>(
        m, "IPluginMetricsCollector")
        .def("start_monitoring", &IPluginMetricsCollector::start_monitoring)
        .def("stop_monitoring", &IPluginMetricsCollector::stop_monitoring)
        .def("is_monitoring_active",
             &IPluginMetricsCollector::is_monitoring_active)
        .def("update_plugin_metrics",
             &IPluginMetricsCollector::update_plugin_metrics)
        .def("get_plugin_metrics", &IPluginMetricsCollector::get_plugin_metrics)
        .def("get_system_metrics", &IPluginMetricsCollector::get_system_metrics)
        .def("update_all_metrics", &IPluginMetricsCollector::update_all_metrics)
        .def("clear_metrics", &IPluginMetricsCollector::clear_metrics)
        .def("get_monitoring_interval",
             &IPluginMetricsCollector::get_monitoring_interval)
        .def("__repr__", [](const IPluginMetricsCollector& collector) {
            return "<IPluginMetricsCollector active=" +
                   std::string(collector.is_monitoring_active() ? "true"
                                                                : "false") +
                   ">";
        });

    // Metrics collector implementation
    py::class_<PluginMetricsCollector, IPluginMetricsCollector,
               std::shared_ptr<PluginMetricsCollector>>(
        m, "PluginMetricsCollector")
        .def(py::init<QObject*>(), py::arg("parent") = nullptr)
        .def("__repr__", [](const PluginMetricsCollector& collector) {
            return "<PluginMetricsCollector active=" +
                   std::string(collector.is_monitoring_active() ? "true"
                                                                : "false") +
                   ">";
        });

    // Utility functions
    m.def(
        "create_hot_reload_manager",
        []() -> std::shared_ptr<PluginHotReloadManager> {
            return std::make_shared<PluginHotReloadManager>();
        },
        "Create a new PluginHotReloadManager instance");

    m.def(
        "create_metrics_collector",
        []() -> std::shared_ptr<PluginMetricsCollector> {
            return std::make_shared<PluginMetricsCollector>();
        },
        "Create a new PluginMetricsCollector instance");

    // Helper functions for common monitoring tasks
    m.def(
        "enable_plugin_monitoring",
        [](std::shared_ptr<PluginHotReloadManager> hot_reload,
           std::shared_ptr<PluginMetricsCollector> metrics,
           const std::string& plugin_id, const std::string& file_path) -> bool {
            try {
                // Enable hot reload
                auto result =
                    hot_reload->enable_hot_reload(plugin_id, file_path);
                if (!result) {
                    return false;
                }

                // Start metrics collection if not already active
                if (!metrics->is_monitoring_active()) {
                    metrics->start_monitoring(std::chrono::milliseconds(5000));
                }

                return true;
            } catch (...) {
                return false;
            }
        },
        py::arg("hot_reload"), py::arg("metrics"), py::arg("plugin_id"),
        py::arg("file_path"),
        "Enable comprehensive monitoring (hot reload + metrics) for a plugin");

    m.def(
        "disable_plugin_monitoring",
        [](std::shared_ptr<PluginHotReloadManager> hot_reload,
           const std::string& plugin_id) {
            hot_reload->disable_hot_reload(plugin_id);
        },
        py::arg("hot_reload"), py::arg("plugin_id"),
        "Disable monitoring for a plugin");

    // Monitoring configuration helpers
    m.def(
        "setup_monitoring_system",
        [](std::chrono::milliseconds metrics_interval =
               std::chrono::milliseconds(5000)) -> py::tuple {
            auto hot_reload = std::make_shared<PluginHotReloadManager>();
            auto metrics = std::make_shared<PluginMetricsCollector>();

            // Configure metrics collection
            metrics->start_monitoring(metrics_interval);

            return py::make_tuple(hot_reload, metrics);
        },
        py::arg("metrics_interval") = std::chrono::milliseconds(5000),
        "Set up a complete monitoring system with hot reload and metrics "
        "collection");
}

}  // namespace qtforge_python
