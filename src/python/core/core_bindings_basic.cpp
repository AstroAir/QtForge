/**
 * @file core_bindings_basic.cpp
 * @brief Comprehensive core bindings for QtForge Python interface
 * @version 3.2.0
 * @author QtForge Development Team
 *
 * This file contains comprehensive bindings for all core QtForge functionality
 * including plugin management, loading, registry, and advanced interfaces.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <string>
#include <filesystem>

// Include comprehensive core headers
#include <qtplugin/core/plugin_interface.hpp>
// Don't include the original plugin_manager.hpp to avoid conflicts
// #include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/core/plugin_loader.hpp>
#include <qtplugin/core/plugin_registry.hpp>
#include <qtplugin/core/plugin_dependency_resolver.hpp>
#include <qtplugin/core/plugin_lifecycle_manager.hpp>
#include <qtplugin/core/advanced_plugin_interface.hpp>
#include <qtplugin/core/dynamic_plugin_interface.hpp>
#include <qtplugin/utils/version.hpp>
#include <qtplugin/utils/error_handling.hpp>

namespace py = pybind11;
using namespace qtplugin;

// Include only the types we need, without PluginManager
#include "plugin_types_only.hpp"
#include "plugin_manager_simple.hpp"

namespace qtforge_python {

void bind_core_components(py::module& module) {
    // === Test and Utility Functions ===
    module.def("test_function", []() -> std::string {
        return "QtForge Python bindings are working!";
    }, "Test function to verify bindings work");

    module.def("get_version", []() -> std::string {
        return "3.2.0";
    }, "Get QtForge version");

    // === Plugin States ===
    py::enum_<PluginState>(module, "PluginState", "Plugin lifecycle states")
        .value("Unloaded", PluginState::Unloaded, "Plugin is not loaded")
        .value("Loading", PluginState::Loading, "Plugin is being loaded")
        .value("Loaded", PluginState::Loaded, "Plugin is loaded but not initialized")
        .value("Initializing", PluginState::Initializing, "Plugin is being initialized")
        .value("Running", PluginState::Running, "Plugin is running normally")
        .value("Paused", PluginState::Paused, "Plugin is paused")
        .value("Stopping", PluginState::Stopping, "Plugin is being stopped")
        .value("Stopped", PluginState::Stopped, "Plugin is stopped")
        .value("Error", PluginState::Error, "Plugin is in error state")
        .value("Reloading", PluginState::Reloading, "Plugin is being reloaded")
        .export_values();

    // === Plugin Capabilities ===
    py::enum_<PluginCapability>(module, "PluginCapability", "Plugin capability flags")
        .value("None", PluginCapability::None, "No special capabilities")
        .value("UI", PluginCapability::UI, "User interface capability")
        .value("Service", PluginCapability::Service, "Service capability")
        .value("Network", PluginCapability::Network, "Network capability")
        .value("DataProcessing", PluginCapability::DataProcessing, "Data processing capability")
        .value("Scripting", PluginCapability::Scripting, "Scripting capability")
        .value("FileSystem", PluginCapability::FileSystem, "File system capability")
        .value("Database", PluginCapability::Database, "Database capability")
        .value("AsyncInit", PluginCapability::AsyncInit, "Asynchronous initialization")
        .value("HotReload", PluginCapability::HotReload, "Hot reload capability")
        .value("Configuration", PluginCapability::Configuration, "Configuration capability")
        .value("Logging", PluginCapability::Logging, "Logging capability")
        .value("Security", PluginCapability::Security, "Security capability")
        .value("Threading", PluginCapability::Threading, "Threading capability")
        .value("Monitoring", PluginCapability::Monitoring, "Monitoring capability")
        .export_values();

    // === Plugin Priority ===
    py::enum_<PluginPriority>(module, "PluginPriority", "Plugin loading priority")
        .value("Lowest", PluginPriority::Lowest, "Lowest priority")
        .value("Low", PluginPriority::Low, "Low priority")
        .value("Normal", PluginPriority::Normal, "Normal priority")
        .value("High", PluginPriority::High, "High priority")
        .value("Highest", PluginPriority::Highest, "Highest priority")
        .export_values();

    // === Version ===
    py::class_<Version>(module, "Version", "Version information")
        .def(py::init<>(), "Create default version (0.0.0)")
        .def(py::init<int, int, int>(), "Create version with major, minor, patch",
             py::arg("major"), py::arg("minor"), py::arg("patch"))
        .def("major", &Version::major, "Get major version number")
        .def("minor", &Version::minor, "Get minor version number")
        .def("patch", &Version::patch, "Get patch version number")
        .def("to_string", [](const Version& v, bool include_build) { return v.to_string(include_build); },
             "Convert to string representation", py::arg("include_build") = true)
        .def("__str__", [](const Version& v) { return v.to_string(); })
        .def("__repr__", [](const Version& version) {
            return "<Version: " + version.to_string() + ">";
        })
        .def("__eq__", [](const Version& a, const Version& b) { return a == b; })
        .def("__ne__", [](const Version& a, const Version& b) { return a != b; })
        .def("__lt__", [](const Version& a, const Version& b) { return a < b; })
        .def("__le__", [](const Version& a, const Version& b) { return a <= b; })
        .def("__gt__", [](const Version& a, const Version& b) { return a > b; })
        .def("__ge__", [](const Version& a, const Version& b) { return a >= b; });

    // === Plugin Metadata ===
    py::class_<PluginMetadata>(module, "PluginMetadata", "Plugin metadata information")
        .def(py::init<>(), "Create empty metadata")
        .def_readwrite("name", &PluginMetadata::name, "Plugin name")
        .def_readwrite("version", &PluginMetadata::version, "Plugin version")
        .def_readwrite("description", &PluginMetadata::description, "Plugin description")
        .def_readwrite("author", &PluginMetadata::author, "Plugin author")
        .def_readwrite("license", &PluginMetadata::license, "Plugin license")
        .def_readwrite("dependencies", &PluginMetadata::dependencies, "Plugin dependencies")
        .def_readwrite("tags", &PluginMetadata::tags, "Plugin tags")
        .def("__repr__", [](const PluginMetadata& metadata) {
            return "<PluginMetadata: " + metadata.name + " v" +
                   metadata.version.to_string() + ">";
        });

    // === Plugin Load Options ===
    py::class_<PluginLoadOptions>(module, "PluginLoadOptions", "Plugin loading configuration")
        .def(py::init<>(), "Create default load options")
        .def_readwrite("validate_signature", &PluginLoadOptions::validate_signature,
                      "Whether to validate plugin signature")
        .def_readwrite("check_dependencies", &PluginLoadOptions::check_dependencies,
                      "Whether to check plugin dependencies")
        .def_readwrite("initialize_immediately", &PluginLoadOptions::initialize_immediately,
                      "Whether to initialize plugin immediately after loading")
        .def_readwrite("enable_hot_reload", &PluginLoadOptions::enable_hot_reload,
                      "Whether to enable hot reloading for this plugin");

    // === Plugin Info ===
    py::class_<PluginInfo>(module, "PluginInfo", "Plugin information structure")
        .def(py::init<>(), "Create empty plugin info")
        .def_readwrite("id", &PluginInfo::id, "Plugin identifier")
        .def_readwrite("metadata", &PluginInfo::metadata, "Plugin metadata")
        .def_readwrite("state", &PluginInfo::state, "Current plugin state")
        .def_readwrite("hot_reload_enabled", &PluginInfo::hot_reload_enabled,
                      "Whether hot reload is enabled")
        .def("to_json", &PluginInfo::to_json, "Convert to JSON representation")
        .def("__repr__", [](const PluginInfo& info) {
            return "<PluginInfo: " + info.id + " (" +
                   std::to_string(static_cast<int>(info.state)) + ")>";
        });

    // === Basic Plugin Interface ===
    py::class_<IPlugin, std::shared_ptr<IPlugin>>(module, "IPlugin", "Base plugin interface")
        .def("metadata", &IPlugin::metadata, "Get plugin metadata")
        .def("initialize", &IPlugin::initialize, "Initialize plugin")
        .def("state", &IPlugin::state, "Get current plugin state")
        .def("capabilities", &IPlugin::capabilities, "Get plugin capabilities")
        .def("priority", &IPlugin::priority, "Get plugin priority")
        .def("is_initialized", &IPlugin::is_initialized, "Check if plugin is initialized")
        .def("__repr__", [](const IPlugin& plugin) {
            auto meta = plugin.metadata();
            return "<IPlugin: " + meta.name + ">";
        });

    // === Plugin Manager ===
    py::class_<SimplePluginManager>(module, "PluginManager", "Simplified plugin management system")
        .def(py::init<>(), "Create a new plugin manager instance")
        .def("load_plugin", [](SimplePluginManager& self, const std::filesystem::path& file_path, const PluginLoadOptions& options) -> py::object {
            auto result = self.load_plugin(file_path, options);
            if (result) {
                return py::cast(result.value());
            } else {
                // Return error information as a dict
                py::dict error_dict;
                error_dict["success"] = false;
                error_dict["error_code"] = static_cast<int>(result.error().code);
                error_dict["error_message"] = result.error().message;
                error_dict["details"] = result.error().details;
                return error_dict;
            }
        }, "Load a plugin from file path",
           py::arg("file_path"), py::arg("options") = PluginLoadOptions{})
        .def("unload_plugin", [](SimplePluginManager& self, std::string_view plugin_id, bool force) -> py::object {
            auto result = self.unload_plugin(plugin_id, force);
            if (result) {
                return py::cast(true);
            } else {
                // Return error information as a dict
                py::dict error_dict;
                error_dict["success"] = false;
                error_dict["error_code"] = static_cast<int>(result.error().code);
                error_dict["error_message"] = result.error().message;
                error_dict["details"] = result.error().details;
                return error_dict;
            }
        }, "Unload a plugin",
           py::arg("plugin_id"), py::arg("force") = false)
        .def("loaded_plugins", &SimplePluginManager::loaded_plugins,
             "Get list of loaded plugin IDs")
        .def("search_paths", &SimplePluginManager::search_paths,
             "Get plugin search paths")
        .def("add_search_path", &SimplePluginManager::add_search_path,
             "Add plugin search path", py::arg("path"))
        .def("remove_search_path", &SimplePluginManager::remove_search_path,
             "Remove plugin search path", py::arg("path"))
        .def("get_plugin", &SimplePluginManager::get_plugin,
             "Get plugin by ID", py::arg("plugin_id"))
        .def("all_plugin_info", &SimplePluginManager::all_plugin_info,
             "Get information about all plugins")
        .def("discover_plugins", &SimplePluginManager::discover_plugins,
             "Discover plugins in directory",
             py::arg("directory"), py::arg("recursive") = false)
        .def("is_ready", &SimplePluginManager::is_ready,
             "Check if plugin manager is ready")
        .def("get_statistics", &SimplePluginManager::get_statistics,
             "Get plugin manager statistics")
        .def("__repr__", &SimplePluginManager::__repr__);

    // === Convenience Functions ===
    module.def("create_version", [](int major, int minor, int patch) -> Version {
        return Version(major, minor, patch);
    }, "Create a version object", py::arg("major"), py::arg("minor"), py::arg("patch"));

    module.def("create_metadata", [](const std::string& name, const std::string& description, const Version& version = Version(1, 0, 0)) -> PluginMetadata {
        PluginMetadata meta;
        meta.name = name;
        meta.description = description;
        meta.version = version;
        return meta;
    }, "Create basic plugin metadata", py::arg("name"), py::arg("description"), py::arg("version") = Version(1, 0, 0));

    module.def("create_plugin_manager", []() -> std::unique_ptr<SimplePluginManager> {
        return create_simple_plugin_manager();
    }, "Create a new plugin manager instance");

    // === Plugin System Status ===
    module.def("get_system_status", []() -> py::dict {
        py::dict status;
        status["version"] = "3.2.0";
        status["ready"] = true;
        status["api_level"] = "comprehensive";
        status["features"] = py::list();
        auto features = status["features"].cast<py::list>();
        features.append("plugin_manager");
        features.append("plugin_loading");
        features.append("plugin_discovery");
        features.append("plugin_metadata");
        features.append("version_management");
        return status;
    }, "Get comprehensive plugin system status");
}

} // namespace qtforge_python
