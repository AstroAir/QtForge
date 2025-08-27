/**
 * @file security_bindings.cpp
 * @brief Security system Python bindings
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/security/security_manager.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_security(py::module& m) {
    // Security level enum
    py::enum_<SecurityLevel>(m, "SecurityLevel")
        .value("None", SecurityLevel::None)
        .value("Basic", SecurityLevel::Basic)
        .value("Standard", SecurityLevel::Standard)
        .value("High", SecurityLevel::High)
        .value("Maximum", SecurityLevel::Maximum)
        .export_values();

    // Validation result
    py::class_<ValidationResult>(m, "ValidationResult")
        .def(py::init<>())
        .def_readwrite("is_valid", &ValidationResult::is_valid)
        .def_readwrite("error_message", &ValidationResult::error_message)
        .def_readwrite("warnings", &ValidationResult::warnings)
        .def_readwrite("security_level", &ValidationResult::security_level)
        .def("__repr__", [](const ValidationResult& result) {
            return "<ValidationResult valid=" +
                   std::string(result.is_valid ? "true" : "false") + ">";
        });

    // Security manager
    py::class_<SecurityManager, std::shared_ptr<SecurityManager>>(
        m, "SecurityManager")
        .def(py::init<>())
        .def_static("create", &SecurityManager::create)
        .def("validate_plugin", &SecurityManager::validate_plugin)
        .def("set_security_level", &SecurityManager::set_security_level)
        .def("get_security_level", &SecurityManager::get_security_level)
        .def("is_plugin_trusted", &SecurityManager::is_plugin_trusted)
        .def("add_trusted_plugin", &SecurityManager::add_trusted_plugin)
        .def("remove_trusted_plugin", &SecurityManager::remove_trusted_plugin)
        .def("get_trusted_plugins", &SecurityManager::get_trusted_plugins)
        .def("clear_trusted_plugins", &SecurityManager::clear_trusted_plugins)
        .def("enable_signature_verification",
             &SecurityManager::enable_signature_verification)
        .def("is_signature_verification_enabled",
             &SecurityManager::is_signature_verification_enabled)
        .def("get_statistics", &SecurityManager::get_statistics)
        .def("__repr__", [](const SecurityManager& manager) {
            return "<SecurityManager level=" +
                   std::to_string(
                       static_cast<int>(manager.get_security_level())) +
                   ">";
        });

    // Utility functions
    m.def(
        "create_security_manager",
        []() -> std::shared_ptr<SecurityManager> {
            return SecurityManager::create();
        },
        "Create a new SecurityManager instance");
}

}  // namespace qtforge_python
