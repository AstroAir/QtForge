/**
 * @file security_bindings.cpp
 * @brief Security system Python bindings
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

void bind_security(py::module& m) {
    // Security level enum
    py::enum_<SecurityLevel>(m, "SecurityLevel")
        .value("None", SecurityLevel::None)
        .value("Basic", SecurityLevel::Basic)
        .value("Standard", SecurityLevel::Standard)
        .value("High", SecurityLevel::High)
        .value("Maximum", SecurityLevel::Maximum)
        .export_values();

    // Plugin permission enum
    py::enum_<PluginPermission>(m, "PluginPermission")
        .value("FileSystemRead", PluginPermission::FileSystemRead)
        .value("FileSystemWrite", PluginPermission::FileSystemWrite)
        .value("NetworkAccess", PluginPermission::NetworkAccess)
        .value("RegistryAccess", PluginPermission::RegistryAccess)
        .value("ProcessCreation", PluginPermission::ProcessCreation)
        .value("SystemInfo", PluginPermission::SystemInfo)
        .value("HardwareAccess", PluginPermission::HardwareAccess)
        .value("DatabaseAccess", PluginPermission::DatabaseAccess)
        .export_values();

    // Trust level enum (if available)
    py::enum_<TrustLevel>(m, "TrustLevel")
        .value("Untrusted", TrustLevel::Untrusted)
        .value("Limited", TrustLevel::Limited)
        .value("Trusted", TrustLevel::Trusted)
        .value("FullyTrusted", TrustLevel::FullyTrusted)
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

    // Security validator
    py::class_<SecurityValidator, std::shared_ptr<SecurityValidator>>(
        m, "SecurityValidator")
        .def(py::init<>())
        .def("validate_file_integrity", &SecurityValidator::validate_file_integrity)
        .def("validate_plugin_metadata", &SecurityValidator::validate_plugin_metadata)
        .def("validate_plugin_dependencies", &SecurityValidator::validate_plugin_dependencies)
        .def("validate_plugin_permissions", &SecurityValidator::validate_plugin_permissions)
        .def("set_validation_rules", &SecurityValidator::set_validation_rules)
        .def("get_validation_rules", &SecurityValidator::get_validation_rules)
        .def("__repr__", [](const SecurityValidator& validator) {
            return "<SecurityValidator>";
        });

    // Signature verifier
    py::class_<SignatureVerifier, std::shared_ptr<SignatureVerifier>>(
        m, "SignatureVerifier")
        .def(py::init<>())
        .def("verify_plugin_signature", &SignatureVerifier::verify_plugin_signature)
        .def("add_trusted_certificate", &SignatureVerifier::add_trusted_certificate)
        .def("remove_trusted_certificate", &SignatureVerifier::remove_trusted_certificate)
        .def("get_trusted_certificates", &SignatureVerifier::get_trusted_certificates)
        .def("clear_trusted_certificates", &SignatureVerifier::clear_trusted_certificates)
        .def("set_signature_algorithm", &SignatureVerifier::set_signature_algorithm)
        .def("get_signature_algorithm", &SignatureVerifier::get_signature_algorithm)
        .def("__repr__", [](const SignatureVerifier& verifier) {
            return "<SignatureVerifier>";
        });

    // Permission manager
    py::class_<PermissionManager, std::shared_ptr<PermissionManager>>(
        m, "PermissionManager")
        .def(py::init<>())
        .def("grant_permission", &PermissionManager::grant_permission)
        .def("revoke_permission", &PermissionManager::revoke_permission)
        .def("has_permission", &PermissionManager::has_permission)
        .def("get_plugin_permissions", &PermissionManager::get_plugin_permissions)
        .def("set_plugin_permissions", &PermissionManager::set_plugin_permissions)
        .def("get_all_permissions", &PermissionManager::get_all_permissions)
        .def("clear_plugin_permissions", &PermissionManager::clear_plugin_permissions)
        .def("set_default_permissions", &PermissionManager::set_default_permissions)
        .def("get_default_permissions", &PermissionManager::get_default_permissions)
        .def("__repr__", [](const PermissionManager& manager) {
            return "<PermissionManager>";
        });

    // Security policy engine
    py::class_<SecurityPolicyEngine, std::shared_ptr<SecurityPolicyEngine>>(
        m, "SecurityPolicyEngine")
        .def(py::init<>())
        .def("evaluate_policy", &SecurityPolicyEngine::evaluate_policy)
        .def("add_policy_rule", &SecurityPolicyEngine::add_policy_rule)
        .def("remove_policy_rule", &SecurityPolicyEngine::remove_policy_rule)
        .def("get_policy_rules", &SecurityPolicyEngine::get_policy_rules)
        .def("clear_policy_rules", &SecurityPolicyEngine::clear_policy_rules)
        .def("set_policy_mode", &SecurityPolicyEngine::set_policy_mode)
        .def("get_policy_mode", &SecurityPolicyEngine::get_policy_mode)
        .def("validate_policy_syntax", &SecurityPolicyEngine::validate_policy_syntax)
        .def("__repr__", [](const SecurityPolicyEngine& engine) {
            return "<SecurityPolicyEngine>";
        });

    // Utility functions
    m.def(
        "create_security_manager",
        []() -> std::shared_ptr<SecurityManager> {
            return SecurityManager::create();
        },
        "Create a new SecurityManager instance");

    m.def(
        "create_security_validator",
        []() -> std::shared_ptr<SecurityValidator> {
            return std::make_shared<SecurityValidator>();
        },
        "Create a new SecurityValidator instance");

    m.def(
        "create_signature_verifier",
        []() -> std::shared_ptr<SignatureVerifier> {
            return std::make_shared<SignatureVerifier>();
        },
        "Create a new SignatureVerifier instance");

    m.def(
        "create_permission_manager",
        []() -> std::shared_ptr<PermissionManager> {
            return std::make_shared<PermissionManager>();
        },
        "Create a new PermissionManager instance");

    m.def(
        "create_security_policy_engine",
        []() -> std::shared_ptr<SecurityPolicyEngine> {
            return std::make_shared<SecurityPolicyEngine>();
        },
        "Create a new SecurityPolicyEngine instance");
}

}  // namespace qtforge_python
