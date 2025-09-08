/**
 * @file qtforge_python.cpp
 * @brief Comprehensive Python bindings for QtForge - Complete Plugin System Integration
 * @version 3.2.0
 * @author QtForge Development Team
 *
 * This file implements comprehensive Python bindings for QtForge:
 * - Complete core plugin system functionality
 * - All major modules with conditional compilation support
 * - Comprehensive error handling and type conversions
 * - Full coverage of QtForge features for Python users
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/functional.h>

// Forward declarations for all binding functions
namespace qtforge_python {
void bind_core_components(pybind11::module& module);
void bind_utils(pybind11::module& module);

// Conditionally available modules
#ifdef QTFORGE_PYTHON_ENABLE_SECURITY_MODULE
void bind_security(pybind11::module& module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_MANAGERS_MODULE
void bind_managers(pybind11::module& module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_MONITORING_MODULE
void bind_monitoring(pybind11::module& module);
#endif

// Conditionally available modules
#ifdef QTFORGE_PYTHON_ENABLE_COMMUNICATION_MODULE
void bind_communication(pybind11::module& module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE
void bind_orchestration(pybind11::module& module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_THREADING_MODULE
void bind_threading(pybind11::module& module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_TRANSACTIONS_MODULE
void bind_transactions(pybind11::module& module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_COMPOSITION_MODULE
void bind_composition(pybind11::module& module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_MARKETPLACE_MODULE
void bind_marketplace(pybind11::module& module);
#endif
}  // namespace qtforge_python

PYBIND11_MODULE(qtforge, module) {
    module.doc() = "QtForge Python Bindings - Complete Plugin System Integration";
    module.attr("__version__") = "3.2.0";
    module.attr("__author__") = "QtForge Team";

    // Add module-level information
    module.def("get_version", []() -> std::string {
        return "3.2.0";
    }, "Get QtForge version");

    module.def("get_version_info", []() -> pybind11::tuple {
        return pybind11::make_tuple(3, 2, 0);
    }, "Get QtForge version as tuple (major, minor, patch)");

    module.def("test_function", []() -> std::string {
        return "QtForge test function called successfully";
    }, "Test function for threading and basic functionality tests");

    module.def("get_build_info", []() -> pybind11::dict {
        pybind11::dict info;
        info["version"] = "3.2.0";
        info["build_type"] = "Progressive";
        info["python_version"] = PY_VERSION;

        // Module availability
        info["moduleInfo"] = pybind11::dict();
        info["moduleInfo"]["core"] = true;
        info["moduleInfo"]["utils"] = true;

#ifdef QTFORGE_PYTHON_ENABLE_SECURITY_MODULE
        info["moduleInfo"]["security"] = true;
#else
        info["moduleInfo"]["security"] = false;
#endif

#ifdef QTFORGE_PYTHON_ENABLE_MANAGERS_MODULE
        info["moduleInfo"]["managers"] = true;
#else
        info["moduleInfo"]["managers"] = false;
#endif

#ifdef QTFORGE_PYTHON_ENABLE_COMMUNICATION_MODULE
        info["moduleInfo"]["communication"] = true;
#else
        info["moduleInfo"]["communication"] = false;
#endif

#ifdef QTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE
        info["moduleInfo"]["orchestration"] = true;
#else
        info["moduleInfo"]["orchestration"] = false;
#endif

#ifdef QTFORGE_PYTHON_ENABLE_MONITORING_MODULE
        info["moduleInfo"]["monitoring"] = true;
#else
        info["moduleInfo"]["monitoring"] = false;
#endif

#ifdef QTFORGE_PYTHON_ENABLE_THREADING_MODULE
        info["moduleInfo"]["threading"] = true;
#else
        info["moduleInfo"]["threading"] = false;
#endif

#ifdef QTFORGE_PYTHON_ENABLE_TRANSACTIONS_MODULE
        info["moduleInfo"]["transactions"] = true;
#else
        info["moduleInfo"]["transactions"] = false;
#endif

#ifdef QTFORGE_PYTHON_ENABLE_COMPOSITION_MODULE
        info["moduleInfo"]["composition"] = true;
#else
        info["moduleInfo"]["composition"] = false;
#endif

#ifdef QTFORGE_PYTHON_ENABLE_MARKETPLACE_MODULE
        info["moduleInfo"]["marketplace"] = true;
#else
        info["moduleInfo"]["marketplace"] = false;
#endif

        return info;
    }, "Get build and module information");

    // Create submodules for all QtForge functionality
    auto core_module = module.def_submodule("core", "Core plugin system components");
    auto utils_module = module.def_submodule("utils", "Utility classes and functions");

    // Bind working core modules
    qtforge_python::bind_core_components(core_module);
    qtforge_python::bind_utils(utils_module);

    // Conditionally enabled modules
#ifdef QTFORGE_PYTHON_ENABLE_SECURITY_MODULE
    auto security_module = module.def_submodule("security", "Security and validation components");
    qtforge_python::bind_security(security_module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_MANAGERS_MODULE
    auto managers_module = module.def_submodule("managers", "Configuration, logging, and resource management");
    qtforge_python::bind_managers(managers_module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_MONITORING_MODULE
    auto monitoring_module = module.def_submodule("monitoring", "Plugin monitoring, hot reload, and metrics collection");
    qtforge_python::bind_monitoring(monitoring_module);
#endif

    // Conditionally available modules
#ifdef QTFORGE_PYTHON_ENABLE_COMMUNICATION_MODULE
    auto communication_module = module.def_submodule("communication", "Inter-plugin communication system");
    qtforge_python::bind_communication(communication_module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE
    auto orchestration_module = module.def_submodule("orchestration", "Plugin orchestration and workflow management");
    qtforge_python::bind_orchestration(orchestration_module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_THREADING_MODULE
    auto threading_module = module.def_submodule("threading", "Plugin threading and concurrency management");
    qtforge_python::bind_threading(threading_module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_TRANSACTIONS_MODULE
    auto transactions_module = module.def_submodule("transactions", "Plugin transaction management and atomic operations");
    qtforge_python::bind_transactions(transactions_module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_COMPOSITION_MODULE
    auto composition_module = module.def_submodule("composition", "Plugin composition and aggregation patterns");
    qtforge_python::bind_composition(composition_module);
#endif

#ifdef QTFORGE_PYTHON_ENABLE_MARKETPLACE_MODULE
    auto marketplace_module = module.def_submodule("marketplace", "Plugin marketplace and distribution system");
    qtforge_python::bind_marketplace(marketplace_module);
#endif

    // Add convenience functions for common operations
    module.def("test_connection", []() -> std::string {
        return "Hello from QtForge! Complete plugin system ready.";
    }, "Test function to verify bindings work");

    module.def("list_available_modules", []() -> pybind11::list {
        pybind11::list availableModules;
        availableModules.append("core");
        availableModules.append("utils");
        // Temporarily disabled: security, managers, monitoring

#ifdef QTFORGE_PYTHON_ENABLE_COMMUNICATION_MODULE
        availableModules.append("communication");
#endif
#ifdef QTFORGE_PYTHON_ENABLE_ORCHESTRATION_MODULE
        availableModules.append("orchestration");
#endif
#ifdef QTFORGE_PYTHON_ENABLE_THREADING_MODULE
        availableModules.append("threading");
#endif
#ifdef QTFORGE_PYTHON_ENABLE_TRANSACTIONS_MODULE
        availableModules.append("transactions");
#endif
#ifdef QTFORGE_PYTHON_ENABLE_COMPOSITION_MODULE
        availableModules.append("composition");
#endif
#ifdef QTFORGE_PYTHON_ENABLE_MARKETPLACE_MODULE
        availableModules.append("marketplace");
#endif

        return availableModules;
    }, "List all available QtForge modules");

    // Add comprehensive system information
    module.def("get_system_info", []() -> pybind11::dict {
        pybind11::dict info;
        info["qtforge_version"] = "3.2.0";
        info["python_version"] = PY_VERSION;
        info["pybind11_version"] = std::to_string(PYBIND11_VERSION_MAJOR) + "." +
                                   std::to_string(PYBIND11_VERSION_MINOR) + "." +
                                   std::to_string(PYBIND11_VERSION_PATCH);
        info["build_timestamp"] = __DATE__ " " __TIME__;

        // Feature flags
        info["features"] = pybind11::dict();
        info["features"]["core"] = true;
        info["features"]["utils"] = true;
        info["features"]["plugin_manager"] = true;
        info["features"]["qt_conversions"] = true;
        info["features"]["error_handling"] = true;
        // Temporarily disabled: security, managers, monitoring

        return info;
    }, "Get comprehensive system information");

    module.def("create_plugin_manager", []() -> pybind11::object {
        // Import the core module and create plugin manager there
        auto core_module = pybind11::module_::import("qtforge.core");
        return core_module.attr("create_plugin_manager")();
    }, "Create a new PluginManager instance (convenience function)");

    // Add convenience functions that are expected by examples
    module.def("create_version", [](int major, int minor, int patch) -> pybind11::object {
        // Import the core module and create version there
        auto core_module = pybind11::module_::import("qtforge.core");
        return core_module.attr("create_version")(major, minor, patch);
    }, "Create a version object", pybind11::arg("major"), pybind11::arg("minor"), pybind11::arg("patch"));

    module.def("create_metadata", [](const std::string& name, const std::string& description) -> pybind11::object {
        // Import the core module and create metadata there
        auto core_module = pybind11::module_::import("qtforge.core");
        return core_module.attr("create_metadata")(name, description);
    }, "Create basic plugin metadata", pybind11::arg("name"), pybind11::arg("description"));

    module.def("get_help", []() -> std::string {
        return R"(
QtForge Python Bindings v3.2.0

Available modules:
- qtforge.core: Core plugin system (PluginManager, PluginLoader, etc.)
- qtforge.utils: Utility functions and classes
- qtforge.security: Security and validation components
- qtforge.managers: Configuration, logging, and resource management
- qtforge.communication: Inter-plugin communication system
- qtforge.orchestration: Plugin orchestration and workflow management
- qtforge.monitoring: Plugin monitoring, hot reload, and metrics
- qtforge.threading: Plugin threading and concurrency management
- qtforge.transactions: Plugin transaction management
- qtforge.composition: Plugin composition and aggregation patterns
- qtforge.marketplace: Plugin marketplace and distribution system

Quick start:
    import qtforge
    print(qtforge.test_connection())
    print(qtforge.list_available_modules())

    # Create a plugin manager
    from qtforge.core import PluginManager
    manager = PluginManager()
        )";
    }, "Get help information for QtForge Python bindings");
}
