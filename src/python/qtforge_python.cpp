/**
 * @file qtforge_python.cpp
 * @brief Main Python binding module for QtForge
 * @version 3.0.0
 * @author QtForge Development Team
 *
 * This file contains the main pybind11 module definition for QtForge Python
 * bindings. It serves as the entry point and coordinates all submodule
 * bindings.
 */

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <string>

// Forward declarations for binding functions
namespace qtforge_python {
void bind_core(pybind11::module& m);
void bind_utils(pybind11::module& m);
// Temporarily disabled to focus on core bindings
// void bind_communication(pybind11::module& m);
// void bind_security(pybind11::module& m);
// void bind_managers(pybind11::module& m);
// void bind_orchestration(pybind11::module& m);
// void bind_monitoring(pybind11::module& m);
// void bind_transactions(pybind11::module& m);
// void bind_composition(pybind11::module& m);
// void bind_marketplace(pybind11::module& m);
// void bind_threading(pybind11::module& m);
}  // namespace qtforge_python

namespace py = pybind11;

/**
 * @brief Main QtForge Python module
 *
 * This module provides Python bindings for the QtForge plugin system,
 * enabling Python applications to use QtForge's plugin management,
 * communication, and security features.
 */
PYBIND11_MODULE(qtforge, m) {
    m.doc() =
        "QtForge Python Bindings - Modern C++ Plugin System for Qt "
        "Applications";

    // Module metadata
    m.attr("__version__") = std::to_string(QTPLUGIN_VERSION_MAJOR) + "." +
                            std::to_string(QTPLUGIN_VERSION_MINOR) + "." +
                            std::to_string(QTPLUGIN_VERSION_PATCH);
    m.attr("__version_major__") = QTPLUGIN_VERSION_MAJOR;
    m.attr("__version_minor__") = QTPLUGIN_VERSION_MINOR;
    m.attr("__version_patch__") = QTPLUGIN_VERSION_PATCH;

    // Create submodules
    auto core_module = m.def_submodule("core", "Core plugin system components");
    auto utils_module =
        m.def_submodule("utils", "Utility classes and functions");
    // Temporarily disabled to focus on core bindings
    // auto communication_module =
    //     m.def_submodule("communication", "Inter-plugin communication system");
    // auto security_module =
    //     m.def_submodule("security", "Security and validation components");
    // auto managers_module = m.def_submodule(
    //     "managers", "Configuration, logging, and resource management");
    // auto orchestration_module = m.def_submodule(
    //     "orchestration", "Plugin orchestration and workflow management");
    // auto monitoring_module = m.def_submodule(
    //     "monitoring", "Plugin monitoring, hot reload, and metrics collection");
    // auto transactions_module = m.def_submodule(
    //     "transactions", "Plugin transaction management and atomic operations");
    // auto composition_module = m.def_submodule(
    //     "composition", "Plugin composition and aggregation patterns");
    // auto marketplace_module = m.def_submodule(
    //     "marketplace", "Plugin marketplace and distribution system");
    // auto threading_module = m.def_submodule(
    //     "threading", "Plugin threading and concurrency management");

    // Bind all components
    qtforge_python::bind_core(core_module);
    qtforge_python::bind_utils(utils_module);
    // Temporarily disabled to focus on core bindings
    // qtforge_python::bind_communication(communication_module);
    // qtforge_python::bind_security(security_module);
    // qtforge_python::bind_managers(managers_module);
    // qtforge_python::bind_orchestration(orchestration_module);
    // qtforge_python::bind_monitoring(monitoring_module);
    // qtforge_python::bind_transactions(transactions_module);
    // qtforge_python::bind_composition(composition_module);
    // qtforge_python::bind_marketplace(marketplace_module);
    // qtforge_python::bind_threading(threading_module);

    // Convenience imports at module level
    // Import commonly used functions to the main module namespace
    // Temporarily disabled - only enable functions that actually exist
    m.attr("test_function") = core_module.attr("test_function");
    m.attr("get_version") = core_module.attr("get_version");
    // m.attr("create_plugin_manager") = core_module.attr("create_plugin_manager");
    // m.attr("load_plugin_demo") = core_module.attr("load_plugin_demo");

    // Import utils functions
    // m.attr("utils_test") = utils_module.attr("utils_test");
    // m.attr("create_version") = utils_module.attr("create_version");
    // m.attr("parse_version") = utils_module.attr("parse_version");
    // m.attr("create_error") = utils_module.attr("create_error");

    // Disabled imports for modules that are temporarily disabled
    // // Import communication functions
    // m.attr("create_message_bus") =
    //     communication_module.attr("create_message_bus");

    // // Import security functions
    // m.attr("SecurityManager") = security_module.attr("SecurityManager");

    // // Import manager functions
    // m.attr("create_configuration_manager") =
    //     managers_module.attr("create_configuration_manager");
    // m.attr("create_logging_manager") =
    //     managers_module.attr("create_logging_manager");
    // m.attr("create_resource_manager") =
    //     managers_module.attr("create_resource_manager");

    // // Import orchestration functions
    // m.attr("create_orchestrator") =
    //     orchestration_module.attr("create_orchestrator");
    // m.attr("create_workflow") = orchestration_module.attr("create_workflow");
    // m.attr("create_workflow_step") =
    //     orchestration_module.attr("create_workflow_step");

    // // Import monitoring functions
    // m.attr("create_hot_reload_manager") =
    //     monitoring_module.attr("create_hot_reload_manager");
    // m.attr("create_metrics_collector") =
    //     monitoring_module.attr("create_metrics_collector");
    // m.attr("setup_monitoring_system") =
    //     monitoring_module.attr("setup_monitoring_system");

    // // Import transaction functions
    // m.attr("create_transaction_manager") =
    //     transactions_module.attr("create_transaction_manager");
    // m.attr("create_transaction_operation") =
    //     transactions_module.attr("create_transaction_operation");
    // m.attr("create_transaction_context") =
    //     transactions_module.attr("create_transaction_context");
    // m.attr("execute_atomic_operation") =
    //     transactions_module.attr("execute_atomic_operation");

    // // Import composition functions
    // m.attr("create_composition_manager") =
    //     composition_module.attr("create_composition_manager");
    // m.attr("create_composition") =
    //     composition_module.attr("create_composition");
    // m.attr("create_composition_binding") =
    //     composition_module.attr("create_composition_binding");
    // m.attr("create_pipeline_composition") =
    //     composition_module.attr("create_pipeline_composition");
    // m.attr("create_facade_composition") =
    //     composition_module.attr("create_facade_composition");

    // // Import marketplace functions
    // m.attr("create_marketplace") =
    //     marketplace_module.attr("create_marketplace");
    // m.attr("create_search_filters") =
    //     marketplace_module.attr("create_search_filters");
    // m.attr("search_free_plugins") =
    //     marketplace_module.attr("search_free_plugins");
    // m.attr("get_top_rated_plugins") =
    //     marketplace_module.attr("get_top_rated_plugins");

    // // Import threading functions
    // m.attr("create_thread_pool") = threading_module.attr("create_thread_pool");
    // m.attr("create_thread_pool_manager") =
    //     threading_module.attr("create_thread_pool_manager");

    // Module-level utility functions
    m.def(
        "version",
        []() -> std::string {
            return std::to_string(QTPLUGIN_VERSION_MAJOR) + "." +
                   std::to_string(QTPLUGIN_VERSION_MINOR) + "." +
                   std::to_string(QTPLUGIN_VERSION_PATCH);
        },
        "Get QtForge version string");

    m.def(
        "version_info",
        []() -> py::tuple {
            return py::make_tuple(QTPLUGIN_VERSION_MAJOR,
                                  QTPLUGIN_VERSION_MINOR,
                                  QTPLUGIN_VERSION_PATCH);
        },
        "Get QtForge version as tuple (major, minor, patch)");
}
