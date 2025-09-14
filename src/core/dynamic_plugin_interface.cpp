/**
 * @file dynamic_plugin_interface.cpp
 * @brief Implementation of dynamic plugin interface system
 * @version 3.2.0
 */

#include "../../include/qtplugin/core/dynamic_plugin_interface.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QString>
#include <algorithm>
#include <string_view>

Q_LOGGING_CATEGORY(dynamicPluginLog, "qtplugin.dynamic");

namespace qtplugin {

// === InterfaceCapability Implementation ===

QJsonObject InterfaceCapability::to_json() const {
    QJsonObject json;
    json["name"] = name;
    json["version"] = QString::fromStdString(version.to_string());
    json["metadata"] = metadata;
    json["required"] = required;
    return json;
}

qtplugin::expected<InterfaceCapability, PluginError>
InterfaceCapability::from_json(const QJsonObject& json) {
    InterfaceCapability capability;

    if (!json.contains("name") || !json["name"].isString()) {
        return qtplugin::unexpected(
            PluginError(PluginErrorCode::InvalidConfiguration,
                        "Missing or invalid capability name"));
    }
    capability.name = json["name"].toString();

    if (json.contains("version") && json["version"].isString()) {
        auto version_result =
            Version::parse(json["version"].toString().toStdString());
        if (!version_result) {
            return qtplugin::unexpected(
                PluginError{PluginErrorCode::InvalidConfiguration,
                            std::string_view("Invalid capability version")});
        }
        capability.version = version_result.value();
    }

    if (json.contains("metadata") && json["metadata"].isObject()) {
        capability.metadata = json["metadata"].toObject();
    }

    if (json.contains("required") && json["required"].isBool()) {
        capability.required = json["required"].toBool();
    }

    return capability;
}

// === InterfaceDescriptor Implementation ===

bool InterfaceDescriptor::is_compatible_with(
    const InterfaceDescriptor& other) const {
    // Check interface ID match
    if (interface_id != other.interface_id) {
        return false;
    }

    // Check version compatibility (semantic versioning)
    if (version.major() != other.version.major()) {
        return false;
    }

    // Minor version should be backward compatible
    if (version.minor() < other.version.minor()) {
        return false;
    }

    // Check required capabilities
    for (const auto& required_cap : other.capabilities) {
        if (!required_cap.required)
            continue;

        auto it = std::find_if(capabilities.begin(), capabilities.end(),
                               [&required_cap](const InterfaceCapability& cap) {
                                   return cap.name == required_cap.name &&
                                          cap.version >= required_cap.version;
                               });

        if (it == capabilities.end()) {
            return false;
        }
    }

    return true;
}

QJsonObject InterfaceDescriptor::to_json() const {
    QJsonObject json;
    json["interface_id"] = interface_id;
    json["version"] = QString::fromStdString(version.to_string());
    json["description"] = description;
    json["schema"] = schema;
    json["metadata"] = metadata;

    QJsonArray caps_array;
    for (const auto& cap : capabilities) {
        caps_array.append(cap.to_json());
    }
    json["capabilities"] = caps_array;

    return json;
}

qtplugin::expected<InterfaceDescriptor, PluginError>
InterfaceDescriptor::from_json(const QJsonObject& json) {
    InterfaceDescriptor descriptor;

    if (!json.contains("interface_id") || !json["interface_id"].isString()) {
        return make_error<InterfaceDescriptor>(
            PluginErrorCode::InvalidConfiguration,
            "Missing or invalid interface_id");
    }
    descriptor.interface_id = json["interface_id"].toString();

    if (json.contains("version") && json["version"].isString()) {
        auto version_result =
            Version::parse(json["version"].toString().toStdString());
        if (!version_result) {
            return make_error<InterfaceDescriptor>(
                PluginErrorCode::InvalidConfiguration,
                "Invalid interface version");
        }
        descriptor.version = version_result.value();
    }

    if (json.contains("description") && json["description"].isString()) {
        descriptor.description = json["description"].toString();
    }

    if (json.contains("schema") && json["schema"].isObject()) {
        descriptor.schema = json["schema"].toObject();
    }

    if (json.contains("metadata") && json["metadata"].isObject()) {
        descriptor.metadata = json["metadata"].toObject();
    }

    if (json.contains("capabilities") && json["capabilities"].isArray()) {
        QJsonArray caps_array = json["capabilities"].toArray();
        for (const auto& cap_value : caps_array) {
            if (cap_value.isObject()) {
                auto cap_result =
                    InterfaceCapability::from_json(cap_value.toObject());
                if (cap_result) {
                    descriptor.capabilities.push_back(cap_result.value());
                }
            }
        }
    }

    return descriptor;
}

// === PluginExecutionContext Implementation ===

QJsonObject PluginExecutionContext::to_json() const {
    QJsonObject json;
    json["type"] = static_cast<int>(type);
    json["interpreter_path"] = interpreter_path;
    json["environment"] = environment;
    json["security_policy"] = security_policy;
    json["timeout"] = static_cast<qint64>(timeout.count());
    return json;
}

// === InterfaceRegistry Implementation ===

InterfaceRegistry& InterfaceRegistry::instance() {
    static InterfaceRegistry instance;
    return instance;
}

qtplugin::expected<void, PluginError> InterfaceRegistry::register_interface(
    const InterfaceDescriptor& descriptor) {
    std::unique_lock lock(m_mutex);

    // Check if interface already exists
    auto it = m_interfaces.find(descriptor.interface_id);
    if (it != m_interfaces.end()) {
        // Check if this is a newer version
        if (descriptor.version <= it->second.version) {
            return make_error<void>(
                PluginErrorCode::AlreadyExists,
                "Interface already registered with same or newer version");
        }
    }

    m_interfaces[descriptor.interface_id] = descriptor;

    qCDebug(dynamicPluginLog)
        << "Registered interface:" << descriptor.interface_id
        << "version:" << descriptor.version.to_string().c_str();

    return make_success();
}

void InterfaceRegistry::unregister_interface(const QString& interface_id) {
    std::unique_lock lock(m_mutex);

    auto it = m_interfaces.find(interface_id);
    if (it != m_interfaces.end()) {
        qCDebug(dynamicPluginLog) << "Unregistered interface:" << interface_id;
        m_interfaces.erase(it);
    }
}

std::optional<InterfaceDescriptor> InterfaceRegistry::get_interface(
    const QString& interface_id) const {
    std::shared_lock lock(m_mutex);

    auto it = m_interfaces.find(interface_id);
    if (it != m_interfaces.end()) {
        return it->second;
    }

    return std::nullopt;
}

std::vector<InterfaceDescriptor> InterfaceRegistry::find_compatible_interfaces(
    const InterfaceDescriptor& requirements) const {
    std::shared_lock lock(m_mutex);
    std::vector<InterfaceDescriptor> compatible;

    for (const auto& [id, descriptor] : m_interfaces) {
        if (descriptor.is_compatible_with(requirements)) {
            compatible.push_back(descriptor);
        }
    }

    return compatible;
}

std::vector<InterfaceDescriptor> InterfaceRegistry::get_all_interfaces() const {
    std::shared_lock lock(m_mutex);
    std::vector<InterfaceDescriptor> interfaces;

    for (const auto& [id, descriptor] : m_interfaces) {
        interfaces.push_back(descriptor);
    }

    return interfaces;
}

// === PluginTypeUtils Implementation ===

QString PluginTypeUtils::plugin_type_to_string(PluginType type) {
    switch (type) {
        case PluginType::Native:
            return "native";
        case PluginType::Python:
            return "python";
        case PluginType::JavaScript:
            return "javascript";
        case PluginType::Lua:
            return "lua";
        case PluginType::Remote:
            return "remote";
        case PluginType::Composite:
            return "composite";
        default:
            return "unknown";
    }
}

std::optional<PluginType> PluginTypeUtils::string_to_plugin_type(
    const QString& str) {
    QString lower = str.toLower();

    if (lower == "native")
        return PluginType::Native;
    if (lower == "python")
        return PluginType::Python;
    if (lower == "javascript")
        return PluginType::JavaScript;
    if (lower == "lua")
        return PluginType::Lua;
    if (lower == "remote")
        return PluginType::Remote;
    if (lower == "composite")
        return PluginType::Composite;

    return std::nullopt;
}

bool PluginTypeUtils::supports_feature(PluginType type,
                                       const QString& feature) {
    QString lower_feature = feature.toLower();

    switch (type) {
        case PluginType::Native:
            return true;  // Native plugins support all features

        case PluginType::Python:
            return lower_feature != "direct_memory_access" &&
                   lower_feature != "native_threading";

        case PluginType::JavaScript:
            return lower_feature != "direct_memory_access" &&
                   lower_feature != "native_threading" &&
                   lower_feature != "file_system_access";

        case PluginType::Lua:
            return lower_feature != "direct_memory_access" &&
                   lower_feature != "native_threading";

        case PluginType::Remote:
            return lower_feature == "network_communication" ||
                   lower_feature == "async_operations";

        case PluginType::Composite:
            return true;  // Composite plugins inherit capabilities from
                          // components

        default:
            return false;
    }
}

PluginExecutionContext PluginTypeUtils::get_default_context(PluginType type) {
    PluginExecutionContext context;
    context.type = type;

    switch (type) {
        case PluginType::Python:
            context.interpreter_path = "python";
            context.timeout = std::chrono::seconds{60};
            break;

        case PluginType::JavaScript:
            context.interpreter_path = "node";
            context.timeout = std::chrono::seconds{30};
            break;

        case PluginType::Lua:
            context.interpreter_path = "lua";
            context.timeout = std::chrono::seconds{30};
            break;

        case PluginType::Remote:
            context.timeout = std::chrono::minutes{5};
            break;

        default:
            context.timeout = std::chrono::seconds{30};
            break;
    }

    return context;
}

}  // namespace qtplugin
