/**
 * @file qt_conversions.cpp
 * @brief Implementation of Qt type conversion utilities
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include "qt_conversions.hpp"
#include <pybind11/chrono.h>
#include <pybind11/stl.h>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <qtplugin/utils/error_handling.hpp>

namespace py = pybind11;

namespace qtforge_python {

void register_qt_conversions(pybind11::module& m) {
    // Register Qt basic types
    py::class_<QString>(m, "QString")
        .def(py::init<>())
        .def(py::init<const char*>())
        .def("__str__", [](const QString& s) { return s.toStdString(); })
        .def("__repr__",
             [](const QString& s) {
                 return "QString('" + s.toStdString() + "')";
             })
        .def("isEmpty", &QString::isEmpty)
        .def("length", &QString::length)
        .def("toStdString", &QString::toStdString);

    // Register QJsonObject
    py::class_<QJsonObject>(m, "QJsonObject")
        .def(py::init<>())
        .def("isEmpty", &QJsonObject::isEmpty)
        .def("size", &QJsonObject::size)
        .def("keys", &QJsonObject::keys)
        .def("contains", &QJsonObject::contains)
        .def("__contains__", &QJsonObject::contains)
        .def("__len__", &QJsonObject::size)
        .def("__repr__", [](const QJsonObject& obj) {
            return "QJsonObject(" +
                   QJsonDocument(obj)
                       .toJson(QJsonDocument::Compact)
                       .toStdString() +
                   ")";
        });

    // Register QJsonArray
    py::class_<QJsonArray>(m, "QJsonArray")
        .def(py::init<>())
        .def("isEmpty", &QJsonArray::isEmpty)
        .def("size", &QJsonArray::size)
        .def("__len__", &QJsonArray::size)
        .def("__repr__", [](const QJsonArray& arr) {
            return "QJsonArray(" +
                   QJsonDocument(arr)
                       .toJson(QJsonDocument::Compact)
                       .toStdString() +
                   ")";
        });

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

    // Register PluginErrorCode enum
    py::enum_<qtplugin::PluginErrorCode>(m, "PluginErrorCode")
        .value("None", qtplugin::PluginErrorCode::None)
        .value("FileNotFound", qtplugin::PluginErrorCode::FileNotFound)
        .value("InvalidFormat", qtplugin::PluginErrorCode::InvalidFormat)
        .value("LoadingFailed", qtplugin::PluginErrorCode::LoadingFailed)
        .value("InitializationFailed",
               qtplugin::PluginErrorCode::InitializationFailed)
        .value("DependencyMissing",
               qtplugin::PluginErrorCode::DependencyMissing)
        .value("SecurityViolation",
               qtplugin::PluginErrorCode::SecurityViolation)
        .value("InvalidState", qtplugin::PluginErrorCode::InvalidState)
        .value("TimeoutError", qtplugin::PluginErrorCode::TimeoutError)
        .value("UnknownError", qtplugin::PluginErrorCode::UnknownError)
        .export_values();

    // Register exception for PluginError
    py::register_exception<qtplugin::PluginError>(m, "PluginException");
}

// Utility functions for type conversion
std::string qstring_to_string(const QString& qstr) {
    return qstr.toStdString();
}

QString string_to_qstring(const std::string& str) {
    return QString::fromStdString(str);
}

py::dict qjsonobject_to_dict(const QJsonObject& obj) {
    py::dict result;
    for (auto it = obj.begin(); it != obj.end(); ++it) {
        const QString& key = it.key();
        const QJsonValue& value = it.value();

        if (value.isString()) {
            result[key.toStdString()] = value.toString().toStdString();
        } else if (value.isDouble()) {
            result[key.toStdString()] = value.toDouble();
        } else if (value.isBool()) {
            result[key.toStdString()] = value.toBool();
        } else if (value.isObject()) {
            result[key.toStdString()] = qjsonobject_to_dict(value.toObject());
        } else if (value.isArray()) {
            result[key.toStdString()] = qjsonarray_to_list(value.toArray());
        }
    }
    return result;
}

py::list qjsonarray_to_list(const QJsonArray& arr) {
    py::list result;
    for (const QJsonValue& value : arr) {
        if (value.isString()) {
            result.append(value.toString().toStdString());
        } else if (value.isDouble()) {
            result.append(value.toDouble());
        } else if (value.isBool()) {
            result.append(value.toBool());
        } else if (value.isObject()) {
            result.append(qjsonobject_to_dict(value.toObject()));
        } else if (value.isArray()) {
            result.append(qjsonarray_to_list(value.toArray()));
        }
    }
    return result;
}

QJsonObject dict_to_qjsonobject(const py::dict& dict) {
    QJsonObject result;
    for (auto item : dict) {
        std::string key = py::str(item.first);
        py::object value = item.second;

        if (py::isinstance<py::str>(value)) {
            result[QString::fromStdString(key)] =
                QString::fromStdString(py::str(value));
        } else if (py::isinstance<py::float_>(value)) {
            result[QString::fromStdString(key)] = py::float_(value);
        } else if (py::isinstance<py::int_>(value)) {
            result[QString::fromStdString(key)] = py::int_(value);
        } else if (py::isinstance<py::bool_>(value)) {
            result[QString::fromStdString(key)] = py::bool_(value);
        } else if (py::isinstance<py::dict>(value)) {
            result[QString::fromStdString(key)] =
                dict_to_qjsonobject(py::dict(value));
        } else if (py::isinstance<py::list>(value)) {
            result[QString::fromStdString(key)] =
                list_to_qjsonarray(py::list(value));
        }
    }
    return result;
}

QJsonArray list_to_qjsonarray(const py::list& list) {
    QJsonArray result;
    for (auto item : list) {
        if (py::isinstance<py::str>(item)) {
            result.append(QString::fromStdString(py::str(item)));
        } else if (py::isinstance<py::float_>(item)) {
            result.append(py::float_(item));
        } else if (py::isinstance<py::int_>(item)) {
            result.append(py::int_(item));
        } else if (py::isinstance<py::bool_>(item)) {
            result.append(py::bool_(item));
        } else if (py::isinstance<py::dict>(item)) {
            result.append(dict_to_qjsonobject(py::dict(item)));
        } else if (py::isinstance<py::list>(item)) {
            result.append(list_to_qjsonarray(py::list(item)));
        }
    }
    return result;
}

}  // namespace qtforge_python
