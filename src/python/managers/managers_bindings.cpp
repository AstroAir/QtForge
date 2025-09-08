/**
 * @file managers_bindings.cpp
 * @brief Manager classes Python bindings (simplified version)
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>

// Include manager headers
#include "qtplugin/managers/configuration_manager.hpp"
#include "qtplugin/managers/logging_manager.hpp"
#include "qtplugin/managers/resource_manager.hpp"

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_managers(py::module& m) {
    // === Configuration Scope Enum ===
    py::enum_<ConfigurationScope>(m, "ConfigurationScope", "Configuration scope levels")
        .value("Global", ConfigurationScope::Global, "Global configuration")
        .value("Plugin", ConfigurationScope::Plugin, "Plugin-specific configuration")
        .value("User", ConfigurationScope::User, "User-specific configuration")
        .value("Session", ConfigurationScope::Session, "Session-specific configuration")
        .value("Runtime", ConfigurationScope::Runtime, "Runtime configuration")
        .export_values();

    // === Configuration Change Type Enum ===
    py::enum_<ConfigurationChangeType>(m, "ConfigurationChangeType", "Configuration change types")
        .value("Added", ConfigurationChangeType::Added, "Configuration added")
        .value("Modified", ConfigurationChangeType::Modified, "Configuration modified")
        .value("Removed", ConfigurationChangeType::Removed, "Configuration removed")
        .value("Reloaded", ConfigurationChangeType::Reloaded, "Configuration reloaded")
        .export_values();

    // === Log Level Enum ===
    py::enum_<LogLevel>(m, "LogLevel", "Logging levels")
        .value("Debug", LogLevel::Debug, "Debug level")
        .value("Info", LogLevel::Info, "Info level")
        .value("Warning", LogLevel::Warning, "Warning level")
        .value("Error", LogLevel::Error, "Error level")
        .value("Critical", LogLevel::Critical, "Critical level")
        .export_values();

    // === Resource State Enum ===
    py::enum_<ResourceState>(m, "ResourceState", "Resource states")
        .value("Available", ResourceState::Available, "Resource is available")
        .value("InUse", ResourceState::InUse, "Resource is in use")
        .value("Reserved", ResourceState::Reserved, "Resource is reserved")
        .value("Cleanup", ResourceState::Cleanup, "Resource is being cleaned up")
        .value("Error", ResourceState::Error, "Resource is in error state")
        .export_values();

    // === Utility Functions ===
    m.def("test_managers", []() -> std::string {
        return "Managers module working!";
    }, "Test function for managers module");

    m.def("get_available_manager_features", []() -> py::list {
        py::list features;
        features.append("configuration_scope");
        features.append("configuration_changes");
        features.append("logging_levels");
        features.append("resource_states");
        return features;
    }, "Get list of available manager features");

    m.def("validate_log_level", [](int level) -> bool {
        return level >= static_cast<int>(LogLevel::Debug) &&
               level <= static_cast<int>(LogLevel::Critical);
    }, "Validate log level value", py::arg("level"));

    m.def("validate_configuration_scope", [](int scope) -> bool {
        return scope >= static_cast<int>(ConfigurationScope::Global) &&
               scope <= static_cast<int>(ConfigurationScope::Runtime);
    }, "Validate configuration scope value", py::arg("scope"));
}

}  // namespace qtforge_python
