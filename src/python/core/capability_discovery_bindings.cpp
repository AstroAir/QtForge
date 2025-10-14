/**
 * @file capability_discovery_bindings.cpp
 * @brief Python bindings for PluginCapabilityDiscovery
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/core/plugin_capability_discovery.hpp>
#include <qtplugin/interfaces/core/plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>

#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

// Helper to convert QVariant to Python (reuse from property_system_bindings)
py::object qvariant_to_python(const QVariant& variant);

void bind_capability_discovery(py::module& module) {
    // === PluginCapabilityInfo Struct ===
    py::class_<PluginCapabilityInfo>(module, "PluginCapabilityInfo",
                                     "Plugin capability information")
        .def(py::init<>())
        .def_readwrite("name", &PluginCapabilityInfo::name)
        .def_readwrite("description", &PluginCapabilityInfo::description)
        .def_readwrite("capability_flag",
                       &PluginCapabilityInfo::capability_flag)
        .def("to_json",
             [](const PluginCapabilityInfo& self) -> py::dict {
                 auto json = self.to_json();
                 py::dict result;
                 for (auto it = json.begin(); it != json.end(); ++it) {
                     const auto& key = it.key();
                     const auto& value = it.value();
                     if (value.isBool()) {
                         result[key.toStdString().c_str()] = value.toBool();
                     } else if (value.isDouble()) {
                         result[key.toStdString().c_str()] = value.toDouble();
                     } else if (value.isString()) {
                         result[key.toStdString().c_str()] =
                             value.toString().toStdString();
                     } else if (value.isNull()) {
                         result[key.toStdString().c_str()] = py::none();
                     }
                 }
                 return result;
             })
        .def_static(
            "from_json",
            [](const py::dict& json_dict) -> PluginCapabilityInfo {
                QJsonObject json;
                for (auto item : json_dict) {
                    std::string key = py::str(item.first);
                    auto value = item.second;
                    if (py::isinstance<py::bool_>(value)) {
                        json[QString::fromStdString(key)] = value.cast<bool>();
                    } else if (py::isinstance<py::int_>(value)) {
                        json[QString::fromStdString(key)] = value.cast<int>();
                    } else if (py::isinstance<py::float_>(value)) {
                        json[QString::fromStdString(key)] =
                            value.cast<double>();
                    } else if (py::isinstance<py::str>(value)) {
                        json[QString::fromStdString(key)] =
                            QString::fromStdString(value.cast<std::string>());
                    }
                }
                return PluginCapabilityInfo::from_json(json);
            })
        .def("__repr__", [](const PluginCapabilityInfo& self) {
            return "<PluginCapabilityInfo: " + self.name.toStdString() + ">";
        });

    // === PluginMethodInfo Struct ===
    py::class_<PluginMethodInfo>(module, "PluginMethodInfo",
                                 "Plugin method information")
        .def(py::init<>())
        .def_readwrite("name", &PluginMethodInfo::name)
        .def_readwrite("signature", &PluginMethodInfo::signature)
        .def_readwrite("return_type", &PluginMethodInfo::return_type)
        .def_readwrite("is_invokable", &PluginMethodInfo::is_invokable)
        .def_readwrite("is_slot", &PluginMethodInfo::is_slot)
        .def_readwrite("is_signal", &PluginMethodInfo::is_signal)
        .def("to_json",
             [](const PluginMethodInfo& self) -> py::dict {
                 auto json = self.to_json();
                 py::dict result;
                 for (auto it = json.begin(); it != json.end(); ++it) {
                     const auto& key = it.key();
                     const auto& value = it.value();
                     if (value.isBool()) {
                         result[key.toStdString().c_str()] = value.toBool();
                     } else if (value.isDouble()) {
                         result[key.toStdString().c_str()] = value.toDouble();
                     } else if (value.isString()) {
                         result[key.toStdString().c_str()] =
                             value.toString().toStdString();
                     } else if (value.isNull()) {
                         result[key.toStdString().c_str()] = py::none();
                     }
                 }
                 return result;
             })
        .def("__repr__", [](const PluginMethodInfo& self) {
            return "<PluginMethodInfo: " + self.name.toStdString() + ">";
        });

    // === PluginPropertyInfo Struct ===
    py::class_<PluginPropertyInfo>(module, "PluginPropertyInfo",
                                   "Plugin property information")
        .def(py::init<>())
        .def_readwrite("name", &PluginPropertyInfo::name)
        .def_readwrite("type", &PluginPropertyInfo::type)
        .def_readwrite("is_readable", &PluginPropertyInfo::is_readable)
        .def_readwrite("is_writable", &PluginPropertyInfo::is_writable)
        .def_readwrite("is_resettable", &PluginPropertyInfo::is_resettable)
        .def_readwrite("has_notify_signal",
                       &PluginPropertyInfo::has_notify_signal)
        .def_readwrite("notify_signal", &PluginPropertyInfo::notify_signal)
        .def("to_json",
             [](const PluginPropertyInfo& self) -> py::dict {
                 auto json = self.to_json();
                 py::dict result;
                 for (auto it = json.begin(); it != json.end(); ++it) {
                     const auto& key = it.key();
                     const auto& value = it.value();
                     if (value.isBool()) {
                         result[key.toStdString().c_str()] = value.toBool();
                     } else if (value.isDouble()) {
                         result[key.toStdString().c_str()] = value.toDouble();
                     } else if (value.isString()) {
                         result[key.toStdString().c_str()] =
                             value.toString().toStdString();
                     } else if (value.isNull()) {
                         result[key.toStdString().c_str()] = py::none();
                     }
                 }
                 return result;
             })
        .def("__repr__", [](const PluginPropertyInfo& self) {
            return "<PluginPropertyInfo: " + self.name.toStdString() + ">";
        });

    // === PluginInterfaceInfo Struct ===
    py::class_<PluginInterfaceInfo>(module, "PluginInterfaceInfo",
                                    "Plugin interface information")
        .def(py::init<>())
        .def_readwrite("interface_id", &PluginInterfaceInfo::interface_id)
        .def_readwrite("interface_name", &PluginInterfaceInfo::interface_name)
        .def_readwrite("version", &PluginInterfaceInfo::version)
        .def("to_json",
             [](const PluginInterfaceInfo& self) -> py::dict {
                 auto json = self.to_json();
                 py::dict result;
                 for (auto it = json.begin(); it != json.end(); ++it) {
                     const auto& key = it.key();
                     const auto& value = it.value();
                     if (value.isBool()) {
                         result[key.toStdString().c_str()] = value.toBool();
                     } else if (value.isDouble()) {
                         result[key.toStdString().c_str()] = value.toDouble();
                     } else if (value.isString()) {
                         result[key.toStdString().c_str()] =
                             value.toString().toStdString();
                     } else if (value.isNull()) {
                         result[key.toStdString().c_str()] = py::none();
                     }
                 }
                 return result;
             })
        .def("__repr__", [](const PluginInterfaceInfo& self) {
            return "<PluginInterfaceInfo: " + self.interface_id.toStdString() +
                   ">";
        });

    // === PluginCapabilityDiscovery Class ===
    py::class_<PluginCapabilityDiscovery, QObject,
               std::shared_ptr<PluginCapabilityDiscovery>>(
        module, "PluginCapabilityDiscovery",
        "Plugin capability discovery and introspection system")
        .def(py::init<QObject*>(), py::arg("parent") = nullptr)

        // Plugin analysis
        .def(
            "discover_capabilities",
            [](PluginCapabilityDiscovery& self,
               std::shared_ptr<IPlugin> plugin) -> py::object {
                auto result = self.discover_capabilities(plugin);
                if (result) {
                    // Convert PluginDiscoveryResult to dict
                    py::dict discovery_dict;
                    discovery_dict["plugin_id"] =
                        result.value().plugin_id.toStdString();
                    // Add more fields as needed
                    return discovery_dict;
                }
                py::dict error_dict;
                error_dict["success"] = false;
                error_dict["error_code"] =
                    static_cast<int>(result.error().code);
                error_dict["error_message"] = result.error().message;
                return error_dict;
            },
            "Discover plugin capabilities", py::arg("plugin"))

        .def(
            "get_plugin_methods",
            [](PluginCapabilityDiscovery& self,
               std::shared_ptr<IPlugin> plugin) -> py::list {
                auto methods = self.get_plugin_methods(plugin);
                py::list result;
                for (const auto& method : methods) {
                    result.append(method);
                }
                return result;
            },
            "Get plugin methods", py::arg("plugin"))

        .def(
            "get_plugin_properties",
            [](PluginCapabilityDiscovery& self,
               std::shared_ptr<IPlugin> plugin) -> py::list {
                auto properties = self.get_plugin_properties(plugin);
                py::list result;
                for (const auto& prop : properties) {
                    result.append(prop);
                }
                return result;
            },
            "Get plugin properties", py::arg("plugin"))

        .def(
            "validate_interface",
            [](PluginCapabilityDiscovery& self, std::shared_ptr<IPlugin> plugin,
               const std::string& interface_id) -> py::object {
                auto result = self.validate_interface(
                    plugin, QString::fromStdString(interface_id));
                if (result) {
                    return py::cast(result.value());
                }
                py::dict error_dict;
                error_dict["success"] = false;
                error_dict["error_code"] =
                    static_cast<int>(result.error().code);
                error_dict["error_message"] = result.error().message;
                return error_dict;
            },
            "Validate plugin against interface requirements", py::arg("plugin"),
            py::arg("interface_id"))

        .def("__repr__", [](const PluginCapabilityDiscovery&) {
            return "<PluginCapabilityDiscovery>";
        });
}

}  // namespace qtforge_python
