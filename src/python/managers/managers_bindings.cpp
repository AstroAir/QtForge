/**
 * @file managers_bindings.cpp
 * @brief Manager classes Python bindings
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>

#include <qtplugin/managers/configuration_manager.hpp>
#include <qtplugin/managers/logging_manager.hpp>
#include <qtplugin/managers/resource_manager.hpp>
#include <qtplugin/managers/plugin_version_manager.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_managers(py::module& m) {
    // Configuration scope enum
    py::enum_<ConfigurationScope>(m, "ConfigurationScope")
        .value("Global", ConfigurationScope::Global)
        .value("Plugin", ConfigurationScope::Plugin)
        .value("User", ConfigurationScope::User)
        .value("Session", ConfigurationScope::Session)
        .value("Runtime", ConfigurationScope::Runtime)
        .export_values();

    // Configuration change type enum
    py::enum_<ConfigurationChangeType>(m, "ConfigurationChangeType")
        .value("Added", ConfigurationChangeType::Added)
        .value("Modified", ConfigurationChangeType::Modified)
        .value("Removed", ConfigurationChangeType::Removed)
        .value("Reloaded", ConfigurationChangeType::Reloaded)
        .export_values();

    // Configuration validation result
    py::class_<ConfigurationValidationResult>(m, "ConfigurationValidationResult")
        .def(py::init<>())
        .def_readwrite("is_valid", &ConfigurationValidationResult::is_valid)
        .def_readwrite("errors", &ConfigurationValidationResult::errors)
        .def_readwrite("warnings", &ConfigurationValidationResult::warnings)
        .def("__bool__", [](const ConfigurationValidationResult& result) {
            return result.is_valid;
        })
        .def("__repr__", [](const ConfigurationValidationResult& result) {
            return "<ConfigurationValidationResult valid=" +
                   std::string(result.is_valid ? "true" : "false") + ">";
        });

    // Configuration change event
    py::class_<ConfigurationChangeEvent>(m, "ConfigurationChangeEvent")
        .def(py::init<ConfigurationChangeType, std::string_view, const QJsonValue&,
                      const QJsonValue&, ConfigurationScope, std::string_view>(),
             py::arg("type"), py::arg("key"), py::arg("old_value"),
             py::arg("new_value"), py::arg("scope"), py::arg("plugin_id") = "")
        .def_readwrite("type", &ConfigurationChangeEvent::type)
        .def_readwrite("key", &ConfigurationChangeEvent::key)
        .def_readwrite("old_value", &ConfigurationChangeEvent::old_value)
        .def_readwrite("new_value", &ConfigurationChangeEvent::new_value)
        .def_readwrite("scope", &ConfigurationChangeEvent::scope)
        .def_readwrite("plugin_id", &ConfigurationChangeEvent::plugin_id)
        .def_readwrite("timestamp", &ConfigurationChangeEvent::timestamp)
        .def("__repr__", [](const ConfigurationChangeEvent& event) {
            return "<ConfigurationChangeEvent key='" + event.key + "'>";
        });

    // Configuration schema
    py::class_<ConfigurationSchema>(m, "ConfigurationSchema")
        .def(py::init<>())
        .def(py::init<const QJsonObject&, bool>(),
             py::arg("schema"), py::arg("strict") = false)
        .def_readwrite("schema", &ConfigurationSchema::schema)
        .def_readwrite("strict_mode", &ConfigurationSchema::strict_mode)
        .def("__repr__", [](const ConfigurationSchema& schema) {
            return "<ConfigurationSchema strict=" +
                   std::string(schema.strict_mode ? "true" : "false") + ">";
        });

    // Configuration manager interface
    py::class_<IConfigurationManager, std::shared_ptr<IConfigurationManager>>(
        m, "IConfigurationManager")
        .def("get_value", &IConfigurationManager::get_value,
             py::arg("key"), py::arg("scope") = ConfigurationScope::Global,
             py::arg("plugin_id") = "")
        .def("get_value_or_default", &IConfigurationManager::get_value_or_default,
             py::arg("key"), py::arg("default_value"),
             py::arg("scope") = ConfigurationScope::Global, py::arg("plugin_id") = "")
        .def("set_value", &IConfigurationManager::set_value,
             py::arg("key"), py::arg("value"),
             py::arg("scope") = ConfigurationScope::Global, py::arg("plugin_id") = "")
        .def("remove_key", &IConfigurationManager::remove_key,
             py::arg("key"), py::arg("scope") = ConfigurationScope::Global,
             py::arg("plugin_id") = "")
        .def("has_key", &IConfigurationManager::has_key,
             py::arg("key"), py::arg("scope") = ConfigurationScope::Global,
             py::arg("plugin_id") = "")
        .def("get_configuration", &IConfigurationManager::get_configuration,
             py::arg("scope") = ConfigurationScope::Global, py::arg("plugin_id") = "")
        .def("set_configuration", &IConfigurationManager::set_configuration,
             py::arg("configuration"), py::arg("scope") = ConfigurationScope::Global,
             py::arg("plugin_id") = "", py::arg("merge") = true);

    // Configuration manager implementation
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

    // Plugin version manager interface
    py::class_<IPluginVersionManager, std::shared_ptr<IPluginVersionManager>>(
        m, "IPluginVersionManager")
        .def("install_version", &IPluginVersionManager::install_version,
             py::arg("plugin_id"), py::arg("version"), py::arg("file_path"),
             py::arg("replace_existing") = false)
        .def("uninstall_version", &IPluginVersionManager::uninstall_version,
             py::arg("plugin_id"), py::arg("version"), py::arg("force") = false)
        .def("get_installed_versions", &IPluginVersionManager::get_installed_versions)
        .def("get_active_version", &IPluginVersionManager::get_active_version)
        .def("set_active_version", &IPluginVersionManager::set_active_version)
        .def("get_version_info", &IPluginVersionManager::get_version_info)
        .def("is_version_compatible", &IPluginVersionManager::is_version_compatible)
        .def("migrate_version", &IPluginVersionManager::migrate_version,
             py::arg("plugin_id"), py::arg("from_version"), py::arg("to_version"))
        .def("rollback_version", &IPluginVersionManager::rollback_version)
        .def("cleanup_old_versions", &IPluginVersionManager::cleanup_old_versions,
             py::arg("plugin_id"), py::arg("keep_count") = 3)
        .def("get_migration_path", &IPluginVersionManager::get_migration_path)
        .def("validate_version_integrity", &IPluginVersionManager::validate_version_integrity)
        .def("export_version", &IPluginVersionManager::export_version)
        .def("import_version", &IPluginVersionManager::import_version,
             py::arg("archive_path"), py::arg("verify_signature") = true);

    // Plugin version manager implementation
    py::class_<PluginVersionManager, IPluginVersionManager,
               std::shared_ptr<PluginVersionManager>>(m, "PluginVersionManager")
        .def(py::init<std::shared_ptr<IPluginRegistry>,
                      std::shared_ptr<IConfigurationManager>,
                      std::shared_ptr<ILoggingManager>, QObject*>(),
             py::arg("registry"), py::arg("config_manager"),
             py::arg("logger"), py::arg("parent") = nullptr)
        .def("__repr__", [](const PluginVersionManager& manager) {
            return "<PluginVersionManager>";
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

    m.def(
        "create_plugin_version_manager",
        [](std::shared_ptr<IPluginRegistry> registry,
           std::shared_ptr<IConfigurationManager> config_manager,
           std::shared_ptr<ILoggingManager> logger) -> std::shared_ptr<PluginVersionManager> {
            return std::make_shared<PluginVersionManager>(registry, config_manager, logger);
        },
        py::arg("registry"), py::arg("config_manager"), py::arg("logger"),
        "Create a new PluginVersionManager instance");
}

}  // namespace qtforge_python
