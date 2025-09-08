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
#include <qtplugin/security/components/security_validator.hpp>
#include <qtplugin/security/components/signature_verifier.hpp>
#include <qtplugin/security/components/permission_manager.hpp>
#include <qtplugin/security/components/security_policy_engine.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_security(py::module& module) {
    // Security level enum (this one actually exists)
    py::enum_<SecurityLevel>(module, "SecurityLevel", "Security levels for plugin validation")
        .value("None", SecurityLevel::None, "No security validation")
        .value("Basic", SecurityLevel::Basic, "Basic file and metadata validation")
        .value("Standard", SecurityLevel::Standard, "Standard security checks including signatures")
        .value("Moderate", SecurityLevel::Moderate, "Alias for Standard (backward compatibility)")
        .value("Strict", SecurityLevel::Strict, "Strict validation with sandboxing")
        .value("Permissive", SecurityLevel::Permissive, "Alias for Basic (backward compatibility)")
        .value("Maximum", SecurityLevel::Maximum, "Maximum security with full isolation")
        .export_values();

    // Note: TrustLevel and ValidationStatus enums are not defined in the current headers
    // They would need to be defined in the security headers first before binding them here

    // Security validation result
    py::class_<SecurityValidationResult>(module, "SecurityValidationResult", "Security validation result")
        .def(py::init<>(), "Create empty security validation result")
        .def_readwrite("is_valid", &SecurityValidationResult::is_valid, "Whether validation passed")
        .def_readwrite("validated_level", &SecurityValidationResult::validated_level, "Validated security level")
        .def_readwrite("warnings", &SecurityValidationResult::warnings, "Validation warnings")
        .def_readwrite("errors", &SecurityValidationResult::errors, "Validation errors")
        .def_readwrite("details", &SecurityValidationResult::details, "Additional validation details")
        .def("passed", &SecurityValidationResult::passed, "Check if validation passed")
        .def("has_warnings", &SecurityValidationResult::has_warnings, "Check if there are warnings")
        .def("has_errors", &SecurityValidationResult::has_errors, "Check if there are errors")
        .def("__bool__", [](const SecurityValidationResult& result) {
            return result.is_valid;
        }, "Boolean conversion based on validation result")
        .def("__repr__", [](const SecurityValidationResult& result) {
            return "SecurityValidationResult(valid=" +
                   std::string(result.is_valid ? "true" : "false") +
                   ", warnings=" + std::to_string(result.warnings.size()) +
                   ", errors=" + std::to_string(result.errors.size()) + ")";
        });

    // Security manager implementation
    py::class_<SecurityManager>(module, "SecurityManager", "Default security manager implementation")
        .def(py::init<>(), "Create security manager")
        .def("validate_plugin", &SecurityManager::validate_plugin, "Validate plugin security",
             py::arg("file_path"), py::arg("required_level"))
        .def("is_trusted", &SecurityManager::is_trusted, "Check if plugin is trusted", py::arg("plugin_id"))
        .def("add_trusted_plugin", &SecurityManager::add_trusted_plugin, "Add plugin to trusted list",
             py::arg("plugin_id"), py::arg("trust_level"))
        .def("remove_trusted_plugin", &SecurityManager::remove_trusted_plugin, "Remove plugin from trusted list", py::arg("plugin_id"))
        .def("set_security_level", &SecurityManager::set_security_level, "Set security level", py::arg("level"))
        .def("get_security_level", &SecurityManager::get_security_level, "Get current security level")
        .def("security_level", &SecurityManager::security_level, "Get current security level")
        .def("security_statistics", &SecurityManager::security_statistics, "Get security statistics")
        .def("get_validations_performed", &SecurityManager::get_validations_performed, "Get number of validations performed")
        .def("get_violations_detected", &SecurityManager::get_violations_detected, "Get number of violations detected")
        .def("validate_metadata", &SecurityManager::validate_metadata, "Validate plugin metadata", py::arg("file_path"))
        .def("validate_signature", &SecurityManager::validate_signature, "Validate plugin signature", py::arg("file_path"))
        .def("is_safe_file_path", &SecurityManager::is_safe_file_path, "Check if file path is safe", py::arg("file_path"))
        .def("load_trusted_plugins", [](SecurityManager& manager, const std::filesystem::path& file_path) -> py::object {
            auto result = manager.load_trusted_plugins(file_path);
            if (result.has_value()) {
                return py::none();
            } else {
                throw py::value_error("Failed to load trusted plugins: " + result.error().message);
            }
        }, "Load trusted plugins list from file", py::arg("file_path"))
        .def("save_trusted_plugins", [](SecurityManager& manager, const std::filesystem::path& file_path) -> py::object {
            auto result = manager.save_trusted_plugins(file_path);
            if (result.has_value()) {
                return py::none();
            } else {
                throw py::value_error("Failed to save trusted plugins: " + result.error().message);
            }
        }, "Save trusted plugins list to file", py::arg("file_path"))
        .def("set_signature_verification_enabled", &SecurityManager::set_signature_verification_enabled,
             "Enable or disable signature verification", py::arg("enabled"))
        .def("is_signature_verification_enabled", &SecurityManager::is_signature_verification_enabled,
             "Check if signature verification is enabled")
        .def("__repr__", [](const SecurityManager& manager) {
            return "SecurityManager(level=" +
                   std::to_string(static_cast<int>(manager.security_level())) +
                   ", validations=" + std::to_string(manager.get_validations_performed()) + ")";
        });

    // === Factory Functions ===
    module.def("create_security_manager", []() -> std::unique_ptr<SecurityManager> {
        return std::make_unique<SecurityManager>();
    }, "Create a new security manager instance");

    // === Utility Functions ===
    module.def("test_security", []() -> std::string {
        return "Security module working!";
    }, "Test function for security module");

    module.def("get_available_security_features", []() -> py::list {
        py::list features;
        features.append("security_manager");
        features.append("security_validation");
        features.append("trust_levels");
        features.append("signature_verification");
        return features;
    }, "Get list of available security features");

    module.def("validate_security_level", [](int level) -> bool {
        return level >= static_cast<int>(SecurityLevel::None) &&
               level <= static_cast<int>(SecurityLevel::Maximum);
    }, "Validate security level value", py::arg("level"));

}

}  // namespace qtforge_python
