/**
 * @file property_system_bindings.cpp
 * @brief Python bindings for PluginPropertySystem
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/core/plugin_property_system.hpp>
#include <qtplugin/interfaces/core/plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>

#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

// Helper function to convert QVariant to Python object
py::object qvariant_to_python(const QVariant& variant) {
    switch (variant.type()) {
        case QVariant::Invalid:
            return py::none();
        case QVariant::Bool:
            return py::cast(variant.toBool());
        case QVariant::Int:
            return py::cast(variant.toInt());
        case QVariant::UInt:
            return py::cast(variant.toUInt());
        case QVariant::LongLong:
            return py::cast(variant.toLongLong());
        case QVariant::ULongLong:
            return py::cast(variant.toULongLong());
        case QVariant::Double:
            return py::cast(variant.toDouble());
        case QVariant::String:
            return py::cast(variant.toString().toStdString());
        case QVariant::StringList: {
            py::list result;
            for (const auto& str : variant.toStringList()) {
                result.append(str.toStdString());
            }
            return result;
        }
        case QVariant::List: {
            py::list result;
            for (const auto& item : variant.toList()) {
                result.append(qvariant_to_python(item));
            }
            return result;
        }
        case QVariant::Map: {
            py::dict result;
            auto map = variant.toMap();
            for (auto it = map.begin(); it != map.end(); ++it) {
                result[it.key().toStdString().c_str()] =
                    qvariant_to_python(it.value());
            }
            return result;
        }
        default:
            return py::cast(variant.toString().toStdString());
    }
}

// Helper function to convert Python object to QVariant
QVariant python_to_qvariant(const py::object& obj) {
    if (obj.is_none()) {
        return QVariant();
    } else if (py::isinstance<py::bool_>(obj)) {
        return QVariant(obj.cast<bool>());
    } else if (py::isinstance<py::int_>(obj)) {
        return QVariant(obj.cast<int>());
    } else if (py::isinstance<py::float_>(obj)) {
        return QVariant(obj.cast<double>());
    } else if (py::isinstance<py::str>(obj)) {
        return QVariant(QString::fromStdString(obj.cast<std::string>()));
    } else if (py::isinstance<py::list>(obj)) {
        QVariantList list;
        for (auto item : obj.cast<py::list>()) {
            list.append(
                python_to_qvariant(py::reinterpret_borrow<py::object>(item)));
        }
        return QVariant(list);
    } else if (py::isinstance<py::dict>(obj)) {
        QVariantMap map;
        for (auto item : obj.cast<py::dict>()) {
            std::string key = py::str(item.first);
            map[QString::fromStdString(key)] = python_to_qvariant(
                py::reinterpret_borrow<py::object>(item.second));
        }
        return QVariant(map);
    }
    return QVariant();
}

void bind_property_system(py::module& module) {
    // === PropertyBindingType Enum ===
    py::enum_<PropertyBindingType>(
        module, "PropertyBindingType",
        "Property binding types for plugin properties")
        .value("OneWay", PropertyBindingType::OneWay,
               "One-way binding (source -> target)")
        .value("TwoWay", PropertyBindingType::TwoWay,
               "Two-way binding (bidirectional)")
        .value("OneTime", PropertyBindingType::OneTime,
               "One-time binding (set once)")
        .export_values();

    // === PropertyValidationType Enum ===
    py::enum_<PropertyValidationType>(module, "PropertyValidationType",
                                      "Property validation types")
        .value("None_", PropertyValidationType::None, "No validation")
        .value("Range", PropertyValidationType::Range,
               "Range validation (min/max)")
        .value("Enum", PropertyValidationType::Enum, "Enumeration validation")
        .value("Regex", PropertyValidationType::Regex,
               "Regular expression validation")
        .value("Custom", PropertyValidationType::Custom,
               "Custom validation function")
        .export_values();

    // === PropertyNotificationMode Enum ===
    py::enum_<PropertyNotificationMode>(module, "PropertyNotificationMode",
                                        "Property change notification modes")
        .value("Immediate", PropertyNotificationMode::Immediate,
               "Immediate notification")
        .value("Debounced", PropertyNotificationMode::Debounced,
               "Debounced notification (delay after last change)")
        .value("Throttled", PropertyNotificationMode::Throttled,
               "Throttled notification (maximum frequency)")
        .value("Batched", PropertyNotificationMode::Batched,
               "Batched notification (collect multiple changes)")
        .export_values();

    // === PropertyMetadata Struct ===
    py::class_<PropertyMetadata>(module, "PropertyMetadata",
                                 "Metadata for plugin properties")
        .def(py::init<>())
        .def_readwrite("name", &PropertyMetadata::name)
        .def_readwrite("display_name", &PropertyMetadata::display_name)
        .def_readwrite("description", &PropertyMetadata::description)
        .def_readwrite("category", &PropertyMetadata::category)
        .def_property(
            "default_value",
            [](const PropertyMetadata& self) {
                return qvariant_to_python(self.default_value);
            },
            [](PropertyMetadata& self, const py::object& value) {
                self.default_value = python_to_qvariant(value);
            })
        .def_property(
            "minimum_value",
            [](const PropertyMetadata& self) {
                return qvariant_to_python(self.minimum_value);
            },
            [](PropertyMetadata& self, const py::object& value) {
                self.minimum_value = python_to_qvariant(value);
            })
        .def_property(
            "maximum_value",
            [](const PropertyMetadata& self) {
                return qvariant_to_python(self.maximum_value);
            },
            [](PropertyMetadata& self, const py::object& value) {
                self.maximum_value = python_to_qvariant(value);
            })
        .def_readwrite("validation_type", &PropertyMetadata::validation_type)
        .def_readwrite("is_required", &PropertyMetadata::is_required)
        .def_readwrite("is_readonly", &PropertyMetadata::is_readonly)
        .def_readwrite("is_specialized", &PropertyMetadata::is_specialized)
        .def_readwrite("units", &PropertyMetadata::units)
        .def("to_json",
             [](const PropertyMetadata& self) -> py::dict {
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
            [](const py::dict& json_dict) -> PropertyMetadata {
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
                return PropertyMetadata::from_json(json);
            })
        .def("__repr__", [](const PropertyMetadata& self) {
            return "<PropertyMetadata: " + self.name.toStdString() + ">";
        });

    // === PropertyBinding Struct ===
    py::class_<PropertyBinding>(module, "PropertyBinding",
                                "Property binding information")
        .def(py::init<>())
        .def_readwrite("binding_id", &PropertyBinding::binding_id)
        .def_readwrite("source_plugin_id", &PropertyBinding::source_plugin_id)
        .def_readwrite("source_property", &PropertyBinding::source_property)
        .def_readwrite("target_plugin_id", &PropertyBinding::target_plugin_id)
        .def_readwrite("target_property", &PropertyBinding::target_property)
        .def_readwrite("binding_type", &PropertyBinding::binding_type)
        .def_readwrite("is_active", &PropertyBinding::is_active)
        .def("__repr__", [](const PropertyBinding& self) {
            return "<PropertyBinding: " + self.binding_id.toStdString() + ">";
        });

    // === PropertyChangeEvent Struct ===
    py::class_<PropertyChangeEvent>(module, "PropertyChangeEvent",
                                    "Property change event information")
        .def(py::init<>())
        .def_readwrite("plugin_id", &PropertyChangeEvent::plugin_id)
        .def_readwrite("property_name", &PropertyChangeEvent::property_name)
        .def_property(
            "old_value",
            [](const PropertyChangeEvent& self) {
                return qvariant_to_python(self.old_value);
            },
            [](PropertyChangeEvent& self, const py::object& value) {
                self.old_value = python_to_qvariant(value);
            })
        .def_property(
            "new_value",
            [](const PropertyChangeEvent& self) {
                return qvariant_to_python(self.new_value);
            },
            [](PropertyChangeEvent& self, const py::object& value) {
                self.new_value = python_to_qvariant(value);
            })
        .def_readwrite("source", &PropertyChangeEvent::source)
        .def("__repr__", [](const PropertyChangeEvent& self) {
            return "<PropertyChangeEvent: " + self.plugin_id.toStdString() +
                   "." + self.property_name.toStdString() + ">";
        });

    // === PluginPropertySystem Class ===
    py::class_<PluginPropertySystem, QObject,
               std::shared_ptr<PluginPropertySystem>>(
        module, "PluginPropertySystem",
        "Plugin property management system with binding and validation")
        .def(py::init<QObject*>(), py::arg("parent") = nullptr)

        // Plugin registration
        .def(
            "register_plugin",
            [](PluginPropertySystem& self,
               std::shared_ptr<IPlugin> plugin) -> py::object {
                auto result = self.register_plugin(plugin);
                if (result) {
                    return py::cast(true);
                } else {
                    py::dict error_dict;
                    error_dict["success"] = false;
                    error_dict["error_code"] =
                        static_cast<int>(result.error().code);
                    error_dict["error_message"] = result.error().message;
                    return error_dict;
                }
            },
            "Register plugin for property management", py::arg("plugin"))

        .def(
            "unregister_plugin",
            [](PluginPropertySystem& self,
               const std::string& plugin_id) -> py::object {
                auto result =
                    self.unregister_plugin(QString::fromStdString(plugin_id));
                if (result) {
                    return py::cast(true);
                } else {
                    py::dict error_dict;
                    error_dict["success"] = false;
                    error_dict["error_code"] =
                        static_cast<int>(result.error().code);
                    error_dict["error_message"] = result.error().message;
                    return error_dict;
                }
            },
            "Unregister plugin from property management", py::arg("plugin_id"))

        .def(
            "is_plugin_registered",
            [](const PluginPropertySystem& self, const std::string& plugin_id) {
                return self.is_plugin_registered(
                    QString::fromStdString(plugin_id));
            },
            "Check if plugin is registered", py::arg("plugin_id"))

        // Property value access
        .def(
            "get_property_value",
            [](const PluginPropertySystem& self, const std::string& plugin_id,
               const std::string& property_name) -> py::object {
                auto result = self.get_property_value(
                    QString::fromStdString(plugin_id),
                    QString::fromStdString(property_name));
                if (result) {
                    return qvariant_to_python(result.value());
                } else {
                    py::dict error_dict;
                    error_dict["success"] = false;
                    error_dict["error_code"] =
                        static_cast<int>(result.error().code);
                    error_dict["error_message"] = result.error().message;
                    return error_dict;
                }
            },
            "Get property value", py::arg("plugin_id"),
            py::arg("property_name"))

        .def(
            "set_property_value",
            [](PluginPropertySystem& self, const std::string& plugin_id,
               const std::string& property_name, const py::object& value,
               const std::string& source) -> py::object {
                auto result = self.set_property_value(
                    QString::fromStdString(plugin_id),
                    QString::fromStdString(property_name),
                    python_to_qvariant(value), QString::fromStdString(source));
                if (result) {
                    return py::cast(true);
                } else {
                    py::dict error_dict;
                    error_dict["success"] = false;
                    error_dict["error_code"] =
                        static_cast<int>(result.error().code);
                    error_dict["error_message"] = result.error().message;
                    return error_dict;
                }
            },
            "Set property value", py::arg("plugin_id"),
            py::arg("property_name"), py::arg("value"),
            py::arg("source") = "user")

        // Property binding
        .def(
            "create_property_binding",
            [](PluginPropertySystem& self, const std::string& source_plugin_id,
               const std::string& source_property,
               const std::string& target_plugin_id,
               const std::string& target_property,
               PropertyBindingType binding_type) -> py::object {
                auto result = self.create_property_binding(
                    QString::fromStdString(source_plugin_id),
                    QString::fromStdString(source_property),
                    QString::fromStdString(target_plugin_id),
                    QString::fromStdString(target_property), binding_type);
                if (result) {
                    return py::cast(result.value().toStdString());
                }
                py::dict error_dict;
                error_dict["success"] = false;
                error_dict["error_code"] =
                    static_cast<int>(result.error().code);
                error_dict["error_message"] = result.error().message;
                return error_dict;
            },
            "Create property binding", py::arg("source_plugin_id"),
            py::arg("source_property"), py::arg("target_plugin_id"),
            py::arg("target_property"),
            py::arg("binding_type") = PropertyBindingType::OneWay)

        .def(
            "remove_property_binding",
            [](PluginPropertySystem& self,
               const std::string& binding_id) -> py::object {
                auto result = self.remove_property_binding(
                    QString::fromStdString(binding_id));
                if (result) {
                    return py::cast(true);
                }
                py::dict error_dict;
                error_dict["success"] = false;
                error_dict["error_code"] =
                    static_cast<int>(result.error().code);
                error_dict["error_message"] = result.error().message;
                return error_dict;
            },
            "Remove property binding", py::arg("binding_id"))

        .def("__repr__", [](const PluginPropertySystem&) {
            return "<PluginPropertySystem>";
        });
}

}  // namespace qtforge_python
