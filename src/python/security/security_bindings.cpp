/**
 * @file security_bindings.cpp
 * @brief Security system Python bindings (simplified version)
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>

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
        .value("Moderate", SecurityLevel::Moderate)
        .value("Strict", SecurityLevel::Strict)
        .value("Permissive", SecurityLevel::Permissive)
        .value("Maximum", SecurityLevel::Maximum)
        .export_values();

    // Security validation result
    py::class_<SecurityValidationResult>(m, "SecurityValidationResult")
        .def(py::init<>())
        .def_readwrite("is_valid", &SecurityValidationResult::is_valid)
        .def_readwrite("validated_level", &SecurityValidationResult::validated_level)
        .def_readwrite("warnings", &SecurityValidationResult::warnings)
        .def_readwrite("error_message", &SecurityValidationResult::error_message)
        .def("__repr__", [](const SecurityValidationResult& result) {
            return "<SecurityValidationResult valid=" +
                   std::string(result.is_valid ? "true" : "false") + ">";
        });

    // Security manager (simplified - only methods that actually exist)
    py::class_<SecurityManager>(m, "SecurityManager")
        .def(py::init<>())
        .def("validate_plugin", &SecurityManager::validate_plugin)
        .def("is_trusted", &SecurityManager::is_trusted)
        .def("add_trusted_plugin", &SecurityManager::add_trusted_plugin)
        .def("remove_trusted_plugin", &SecurityManager::remove_trusted_plugin)
        .def("set_security_level", &SecurityManager::set_security_level)
        .def("get_security_level", &SecurityManager::get_security_level)
        .def("security_level", &SecurityManager::security_level)
        .def("security_statistics", &SecurityManager::security_statistics)
        .def("get_validations_performed", &SecurityManager::get_validations_performed)
        .def("get_violations_detected", &SecurityManager::get_violations_detected)
        .def("load_trusted_plugins", &SecurityManager::load_trusted_plugins)
        .def("save_trusted_plugins", &SecurityManager::save_trusted_plugins)
        .def("__repr__", [](const SecurityManager& manager) {
            return "<SecurityManager>";
        });

}

}  // namespace qtforge_python
