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

    // Bind all components
    qtforge_python::bind_core(core_module);
    qtforge_python::bind_utils(utils_module);

    // Convenience imports at module level
    // Import commonly used functions to the main module namespace
    m.attr("test_function") = core_module.attr("test_function");
    m.attr("get_version") = core_module.attr("get_version");
    m.attr("create_plugin_manager") = core_module.attr("create_plugin_manager");
    m.attr("load_plugin_demo") = core_module.attr("load_plugin_demo");

    // Import utils functions
    m.attr("utils_test") = utils_module.attr("utils_test");
    m.attr("create_version") = utils_module.attr("create_version");
    m.attr("parse_version") = utils_module.attr("parse_version");
    m.attr("create_error") = utils_module.attr("create_error");

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
