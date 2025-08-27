/**
 * @file managers_bindings.cpp
 * @brief Manager classes Python bindings
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/managers/configuration_manager.hpp>
#include <qtplugin/managers/logging_manager.hpp>
#include <qtplugin/managers/resource_manager.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_managers(py::module& m) {
    // Configuration manager
    py::class_<IConfigurationManager, std::shared_ptr<IConfigurationManager>>(
        m, "IConfigurationManager")
        .def("get_value", &IConfigurationManager::get_value)
        .def("set_value", &IConfigurationManager::set_value)
        .def("has_value", &IConfigurationManager::has_value)
        .def("remove_value", &IConfigurationManager::remove_value)
        .def("get_section", &IConfigurationManager::get_section)
        .def("set_section", &IConfigurationManager::set_section)
        .def("has_section", &IConfigurationManager::has_section)
        .def("remove_section", &IConfigurationManager::remove_section)
        .def("list_sections", &IConfigurationManager::list_sections)
        .def("list_keys", &IConfigurationManager::list_keys)
        .def("clear", &IConfigurationManager::clear)
        .def("save", &IConfigurationManager::save)
        .def("load", &IConfigurationManager::load)
        .def("is_modified", &IConfigurationManager::is_modified);

    py::class_<ConfigurationManager, IConfigurationManager,
               std::shared_ptr<ConfigurationManager>>(m, "ConfigurationManager")
        .def(py::init<>())
        .def_static("create", &ConfigurationManager::create)
        .def("__repr__", [](const ConfigurationManager& manager) {
            return "<ConfigurationManager>";
        });

    // Logging level enum
    py::enum_<LogLevel>(m, "LogLevel")
        .value("Debug", LogLevel::Debug)
        .value("Info", LogLevel::Info)
        .value("Warning", LogLevel::Warning)
        .value("Error", LogLevel::Error)
        .value("Critical", LogLevel::Critical)
        .export_values();

    // Logging manager
    py::class_<ILoggingManager, std::shared_ptr<ILoggingManager>>(
        m, "ILoggingManager")
        .def("log", &ILoggingManager::log)
        .def("debug", &ILoggingManager::debug)
        .def("info", &ILoggingManager::info)
        .def("warning", &ILoggingManager::warning)
        .def("error", &ILoggingManager::error)
        .def("critical", &ILoggingManager::critical)
        .def("set_log_level", &ILoggingManager::set_log_level)
        .def("get_log_level", &ILoggingManager::get_log_level)
        .def("enable_file_logging", &ILoggingManager::enable_file_logging)
        .def("disable_file_logging", &ILoggingManager::disable_file_logging)
        .def("is_file_logging_enabled",
             &ILoggingManager::is_file_logging_enabled)
        .def("get_log_file_path", &ILoggingManager::get_log_file_path)
        .def("flush", &ILoggingManager::flush);

    py::class_<LoggingManager, ILoggingManager,
               std::shared_ptr<LoggingManager>>(m, "LoggingManager")
        .def(py::init<>())
        .def_static("create", &LoggingManager::create)
        .def("__repr__",
             [](const LoggingManager& manager) { return "<LoggingManager>"; });

    // Resource manager
    py::class_<IResourceManager, std::shared_ptr<IResourceManager>>(
        m, "IResourceManager")
        .def("allocate_resource", &IResourceManager::allocate_resource)
        .def("deallocate_resource", &IResourceManager::deallocate_resource)
        .def("get_resource", &IResourceManager::get_resource)
        .def("has_resource", &IResourceManager::has_resource)
        .def("list_resources", &IResourceManager::list_resources)
        .def("get_resource_info", &IResourceManager::get_resource_info)
        .def("get_statistics", &IResourceManager::get_statistics)
        .def("cleanup", &IResourceManager::cleanup);

    py::class_<ResourceManager, IResourceManager,
               std::shared_ptr<ResourceManager>>(m, "ResourceManager")
        .def(py::init<>())
        .def_static("create", &ResourceManager::create)
        .def("__repr__", [](const ResourceManager& manager) {
            return "<ResourceManager>";
        });

    // Utility functions
    m.def(
        "create_configuration_manager",
        []() -> std::shared_ptr<ConfigurationManager> {
            return ConfigurationManager::create();
        },
        "Create a new ConfigurationManager instance");

    m.def(
        "create_logging_manager",
        []() -> std::shared_ptr<LoggingManager> {
            return LoggingManager::create();
        },
        "Create a new LoggingManager instance");

    m.def(
        "create_resource_manager",
        []() -> std::shared_ptr<ResourceManager> {
            return ResourceManager::create();
        },
        "Create a new ResourceManager instance");
}

}  // namespace qtforge_python
