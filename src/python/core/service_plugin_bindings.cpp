/**
 * @file service_plugin_bindings.cpp
 * @brief Python bindings for IServicePlugin interface
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/interfaces/core/plugin_interface.hpp>
#include <qtplugin/interfaces/core/service_plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_service_plugin(py::module& module) {
    // === ServiceExecutionMode Enum ===
    py::enum_<ServiceExecutionMode>(
        module, "ServiceExecutionMode",
        "Service execution modes for plugin services")
        .value("MainThread", ServiceExecutionMode::MainThread,
               "Execute in main thread")
        .value("WorkerThread", ServiceExecutionMode::WorkerThread,
               "Execute in dedicated worker thread")
        .value("ThreadPool", ServiceExecutionMode::ThreadPool,
               "Execute using thread pool")
        .value("Async", ServiceExecutionMode::Async, "Asynchronous execution")
        .value("Custom", ServiceExecutionMode::Custom, "Custom execution mode")
        .export_values();

    // === ServiceState Enum ===
    py::enum_<ServiceState>(module, "ServiceState",
                            "Service states for plugin services")
        .value("Stopped", ServiceState::Stopped, "Service is stopped")
        .value("Starting", ServiceState::Starting, "Service is starting")
        .value("Running", ServiceState::Running, "Service is running")
        .value("Pausing", ServiceState::Pausing, "Service is pausing")
        .value("Paused", ServiceState::Paused, "Service is paused")
        .value("Resuming", ServiceState::Resuming, "Service is resuming")
        .value("Stopping", ServiceState::Stopping, "Service is stopping")
        .value("Error", ServiceState::Error, "Service is in error state")
        .value("Restarting", ServiceState::Restarting, "Service is restarting")
        .export_values();

    // === IServicePlugin Interface ===
    py::class_<IServicePlugin, IPlugin, std::shared_ptr<IServicePlugin>>(
        module, "IServicePlugin",
        "Service plugin interface for background services and task processing")

        // Service lifecycle methods
        .def(
            "start_service",
            [](IServicePlugin& self) -> py::object {
                auto result = self.start_service();
                if (result) {
                    return py::cast(true);
                } else {
                    py::dict error_dict;
                    error_dict["success"] = false;
                    error_dict["error_code"] =
                        static_cast<int>(result.error().code);
                    error_dict["error_message"] = result.error().message;
                    error_dict["details"] = result.error().details;
                    return error_dict;
                }
            },
            "Start the service")

        .def(
            "stop_service",
            [](IServicePlugin& self) -> py::object {
                auto result = self.stop_service();
                if (result) {
                    return py::cast(true);
                } else {
                    py::dict error_dict;
                    error_dict["success"] = false;
                    error_dict["error_code"] =
                        static_cast<int>(result.error().code);
                    error_dict["error_message"] = result.error().message;
                    error_dict["details"] = result.error().details;
                    return error_dict;
                }
            },
            "Stop the service")

        .def(
            "pause_service",
            [](IServicePlugin& self) -> py::object {
                auto result = self.pause_service();
                if (result) {
                    return py::cast(true);
                } else {
                    py::dict error_dict;
                    error_dict["success"] = false;
                    error_dict["error_code"] =
                        static_cast<int>(result.error().code);
                    error_dict["error_message"] = result.error().message;
                    error_dict["details"] = result.error().details;
                    return error_dict;
                }
            },
            "Pause the service")

        .def(
            "resume_service",
            [](IServicePlugin& self) -> py::object {
                auto result = self.resume_service();
                if (result) {
                    return py::cast(true);
                } else {
                    py::dict error_dict;
                    error_dict["success"] = false;
                    error_dict["error_code"] =
                        static_cast<int>(result.error().code);
                    error_dict["error_message"] = result.error().message;
                    error_dict["details"] = result.error().details;
                    return error_dict;
                }
            },
            "Resume the service")

        .def(
            "restart_service",
            [](IServicePlugin& self) -> py::object {
                auto result = self.restart_service();
                if (result) {
                    return py::cast(true);
                } else {
                    py::dict error_dict;
                    error_dict["success"] = false;
                    error_dict["error_code"] =
                        static_cast<int>(result.error().code);
                    error_dict["error_message"] = result.error().message;
                    error_dict["details"] = result.error().details;
                    return error_dict;
                }
            },
            "Restart the service")

        // Service state and info methods
        .def("service_state", &IServicePlugin::service_state,
             "Get current service state")

        .def("execution_mode", &IServicePlugin::execution_mode,
             "Get service execution mode")

        .def("is_service_running", &IServicePlugin::is_service_running,
             "Check if service is running")

        .def(
            "service_uptime",
            [](const IServicePlugin& self) -> int64_t {
                return self.service_uptime().count();
            },
            "Get service uptime in milliseconds")

        .def(
            "service_metrics",
            [](const IServicePlugin& self) -> py::dict {
                auto metrics = self.service_metrics();
                py::dict result;

                // Convert QJsonObject to Python dict
                for (auto it = metrics.begin(); it != metrics.end(); ++it) {
                    const auto& key = it.key();
                    const auto& value = it.value();

                    if (value.isBool()) {
                        result[key.toStdString().c_str()] = value.toBool();
                    } else if (value.isDouble()) {
                        result[key.toStdString().c_str()] = value.toDouble();
                    } else if (value.isString()) {
                        result[key.toStdString().c_str()] =
                            value.toString().toStdString();
                    } else if (value.isNull()) {
                        result[key.toStdString().c_str()] = py::none();
                    } else {
                        // For complex types, convert to string
                        result[key.toStdString().c_str()] =
                            value.toVariant().toString().toStdString();
                    }
                }

                return result;
            },
            "Get service metrics as dictionary")

        .def(
            "configure_service",
            [](IServicePlugin& self, const py::dict& config) -> py::object {
                // Convert Python dict to QJsonObject
                QJsonObject json_config;
                for (auto item : config) {
                    std::string key = py::str(item.first);
                    auto value = item.second;

                    if (py::isinstance<py::bool_>(value)) {
                        json_config[QString::fromStdString(key)] =
                            value.cast<bool>();
                    } else if (py::isinstance<py::int_>(value)) {
                        json_config[QString::fromStdString(key)] =
                            value.cast<int>();
                    } else if (py::isinstance<py::float_>(value)) {
                        json_config[QString::fromStdString(key)] =
                            value.cast<double>();
                    } else if (py::isinstance<py::str>(value)) {
                        json_config[QString::fromStdString(key)] =
                            QString::fromStdString(value.cast<std::string>());
                    } else if (value.is_none()) {
                        json_config[QString::fromStdString(key)] =
                            QJsonValue::Null;
                    }
                }

                auto result = self.configure_service(json_config);
                if (result) {
                    return py::cast(true);
                } else {
                    py::dict error_dict;
                    error_dict["success"] = false;
                    error_dict["error_code"] =
                        static_cast<int>(result.error().code);
                    error_dict["error_message"] = result.error().message;
                    error_dict["details"] = result.error().details;
                    return error_dict;
                }
            },
            "Configure the service with settings dictionary", py::arg("config"))

        .def(
            "service_configuration",
            [](const IServicePlugin& self) -> py::dict {
                auto config = self.service_configuration();
                py::dict result;

                // Convert QJsonObject to Python dict
                for (auto it = config.begin(); it != config.end(); ++it) {
                    const auto& key = it.key();
                    const auto& value = it.value();

                    if (value.isBool()) {
                        result[key.toStdString().c_str()] = value.toBool();
                    } else if (value.isDouble()) {
                        result[key.toStdString().c_str()] = value.toDouble();
                    } else if (value.isString()) {
                        result[key.toStdString().c_str()] =
                            value.toString().toStdString();
                    } else if (value.isNull()) {
                        result[key.toStdString().c_str()] = py::none();
                    } else {
                        result[key.toStdString().c_str()] =
                            value.toVariant().toString().toStdString();
                    }
                }

                return result;
            },
            "Get current service configuration as dictionary")

        .def("__repr__", [](const IServicePlugin& self) {
            auto meta = self.metadata();
            auto state = self.service_state();
            std::string state_str;
            switch (state) {
                case ServiceState::Stopped:
                    state_str = "Stopped";
                    break;
                case ServiceState::Starting:
                    state_str = "Starting";
                    break;
                case ServiceState::Running:
                    state_str = "Running";
                    break;
                case ServiceState::Pausing:
                    state_str = "Pausing";
                    break;
                case ServiceState::Paused:
                    state_str = "Paused";
                    break;
                case ServiceState::Resuming:
                    state_str = "Resuming";
                    break;
                case ServiceState::Stopping:
                    state_str = "Stopping";
                    break;
                case ServiceState::Error:
                    state_str = "Error";
                    break;
                case ServiceState::Restarting:
                    state_str = "Restarting";
                    break;
                default:
                    state_str = "Unknown";
                    break;
            }
            return "<IServicePlugin: " + meta.name + " [" + state_str + "]>";
        });
}

}  // namespace qtforge_python
