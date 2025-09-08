/**
 * @file qt_conversions.cpp
 * @brief Implementation of Qt type conversion utilities
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include "qt_conversions.hpp"
#include <pybind11/chrono.h>
#include <pybind11/stl.h>
#include <qtplugin/utils/error_handling.hpp>

namespace py = pybind11;

namespace qtforge_python {

void register_qt_conversions(pybind11::module& m) {
    // Register PluginError for error handling
    py::class_<qtplugin::PluginError>(m, "PluginError")
        .def(py::init<qtplugin::PluginErrorCode, const std::string&>())
        .def_readonly("code", &qtplugin::PluginError::code)
        .def_readonly("message", &qtplugin::PluginError::message)
        .def("__str__",
             [](const qtplugin::PluginError& err) {
                 return "PluginError(" +
                        std::to_string(static_cast<int>(err.code)) + ", '" +
                        err.message + "')";
             })
        .def("__repr__", [](const qtplugin::PluginError& err) {
            return "PluginError(" + std::to_string(static_cast<int>(err.code)) +
                   ", '" + err.message + "')";
        });

    // Register PluginErrorCode enum (basic version)
    py::enum_<qtplugin::PluginErrorCode>(m, "PluginErrorCode")
        .value("Success", qtplugin::PluginErrorCode::Success)
        .value("LoadFailed", qtplugin::PluginErrorCode::LoadFailed)
        .value("InitializationFailed", qtplugin::PluginErrorCode::InitializationFailed)
        .value("UnknownError", qtplugin::PluginErrorCode::UnknownError)
        .export_values();

    // Register exception for PluginError
    py::register_exception<qtplugin::PluginError>(m, "PluginException");
}

}  // namespace qtforge_python
