/**
 * @file plugin_capability_discovery.cpp
 * @brief Implementation of plugin capability discovery system
 * @version 3.0.0
 */

#include "qtplugin/core/plugin_capability_discovery.hpp"
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QMetaClassInfo>
#include <QMetaMethod>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMutex>
#include <QMutexLocker>
#include <algorithm>
#include <unordered_map>
#include "qtplugin/interfaces/data/data_processor_plugin_interface.hpp"
#include "qtplugin/interfaces/network_plugin_interface.hpp"
#include "qtplugin/interfaces/scripting_plugin_interface.hpp"
#include "qtplugin/interfaces/ui_plugin_interface.hpp"

Q_LOGGING_CATEGORY(capabilityDiscoveryLog, "qtplugin.capability.discovery")

namespace qtplugin {

// === PluginCapabilityInfo Implementation ===

QJsonObject PluginCapabilityInfo::to_json() const {
    QJsonObject json;
    json["name"] = name;
    json["description"] = description;
    json["capability_flag"] = static_cast<int>(capability_flag);
    json["required_methods"] = QJsonArray::fromStringList(required_methods);
    json["optional_methods"] = QJsonArray::fromStringList(optional_methods);
    json["required_properties"] =
        QJsonArray::fromStringList(required_properties);
    json["optional_properties"] =
        QJsonArray::fromStringList(optional_properties);
    json["metadata"] = metadata;
    return json;
}

PluginCapabilityInfo PluginCapabilityInfo::from_json(const QJsonObject& json) {
    PluginCapabilityInfo info;
    info.name = json["name"].toString();
    info.description = json["description"].toString();
    info.capability_flag =
        static_cast<PluginCapability>(json["capability_flag"].toInt());

    auto required_methods_array = json["required_methods"].toArray();
    for (const auto& value : required_methods_array) {
        info.required_methods.append(value.toString());
    }

    auto optional_methods_array = json["optional_methods"].toArray();
    for (const auto& value : optional_methods_array) {
        info.optional_methods.append(value.toString());
    }

    auto required_properties_array = json["required_properties"].toArray();
    for (const auto& value : required_properties_array) {
        info.required_properties.append(value.toString());
    }

    auto optional_properties_array = json["optional_properties"].toArray();
    for (const auto& value : optional_properties_array) {
        info.optional_properties.append(value.toString());
    }

    info.metadata = json["metadata"].toObject();
    return info;
}

// === PluginMethodInfo Implementation ===

QJsonObject PluginMethodInfo::to_json() const {
    QJsonObject json;
    json["name"] = name;
    json["signature"] = signature;
    json["return_type"] = return_type;
    json["parameter_types"] = QJsonArray::fromStringList(parameter_types);
    json["parameter_names"] = QJsonArray::fromStringList(parameter_names);
    json["is_invokable"] = is_invokable;
    json["is_slot"] = is_slot;
    json["is_signal"] = is_signal;
    json["access"] = static_cast<int>(access);
    json["annotations"] = annotations;
    return json;
}

// === PluginPropertyInfo Implementation ===

QJsonObject PluginPropertyInfo::to_json() const {
    QJsonObject json;
    json["name"] = name;
    json["type"] = type;
    json["default_value"] = QJsonValue::fromVariant(default_value);
    json["is_readable"] = is_readable;
    json["is_writable"] = is_writable;
    json["is_resettable"] = is_resettable;
    json["has_notify_signal"] = has_notify_signal;
    json["notify_signal"] = notify_signal;
    json["annotations"] = annotations;
    return json;
}

// === PluginInterfaceInfo Implementation ===

QJsonObject PluginInterfaceInfo::to_json() const {
    QJsonObject json;
    json["interface_id"] = interface_id;
    json["interface_name"] = interface_name;
    json["version"] = version;
    json["parent_interfaces"] = QJsonArray::fromStringList(parent_interfaces);

    QJsonArray methods_array;
    for (const auto& method : methods) {
        methods_array.append(method.to_json());
    }
    json["methods"] = methods_array;

    QJsonArray properties_array;
    for (const auto& property : properties) {
        properties_array.append(property.to_json());
    }
    json["properties"] = properties_array;

    json["metadata"] = metadata;
    return json;
}

// === PluginDiscoveryResult Implementation ===

QJsonObject PluginDiscoveryResult::to_json() const {
    QJsonObject json;
    json["plugin_id"] = plugin_id;
    json["plugin_name"] = plugin_name;
    json["capabilities"] = static_cast<int>(capabilities);

    QJsonArray capability_details_array;
    for (const auto& capability : capability_details) {
        capability_details_array.append(capability.to_json());
    }
    json["capability_details"] = capability_details_array;

    QJsonArray interfaces_array;
    for (const auto& interface : interfaces) {
        interfaces_array.append(interface.to_json());
    }
    json["interfaces"] = interfaces_array;

    QJsonArray methods_array;
    for (const auto& method : methods) {
        methods_array.append(method.to_json());
    }
    json["methods"] = methods_array;

    QJsonArray properties_array;
    for (const auto& property : properties) {
        properties_array.append(property.to_json());
    }
    json["properties"] = properties_array;

    json["metadata"] = metadata;
    return json;
}

// === PluginCapabilityMatcher Implementation ===

std::vector<std::shared_ptr<IPlugin>>
PluginCapabilityMatcher::match_by_capability(
    PluginCapability required_capability,
    const std::vector<std::shared_ptr<IPlugin>>& plugins) {
    std::vector<std::shared_ptr<IPlugin>> matches;

    for (const auto& plugin : plugins) {
        if (plugin && plugin->has_capability(required_capability)) {
            matches.push_back(plugin);
        }
    }

    return matches;
}

std::vector<std::shared_ptr<IPlugin>>
PluginCapabilityMatcher::match_by_capabilities(
    PluginCapabilities required_capabilities,
    const std::vector<std::shared_ptr<IPlugin>>& plugins, bool require_all) {
    std::vector<std::shared_ptr<IPlugin>> matches;

    for (const auto& plugin : plugins) {
        if (!plugin)
            continue;

        auto plugin_capabilities = plugin->capabilities();

        if (require_all) {
            // All required capabilities must be present
            if ((plugin_capabilities & required_capabilities) ==
                required_capabilities) {
                matches.push_back(plugin);
            }
        } else {
            // Any of the required capabilities must be present
            if ((plugin_capabilities & required_capabilities) != 0) {
                matches.push_back(plugin);
            }
        }
    }

    return matches;
}

std::vector<std::shared_ptr<IPlugin>>
PluginCapabilityMatcher::match_by_interface(
    const QString& interface_id,
    const std::vector<std::shared_ptr<IPlugin>>& plugins) {
    std::vector<std::shared_ptr<IPlugin>> matches;

    for (const auto& plugin : plugins) {
        if (!plugin)
            continue;

        // Check if plugin implements the interface using Qt's meta-object
        // system
        QObject* plugin_obj = dynamic_cast<QObject*>(plugin.get());
        if (plugin_obj) {
            const QMetaObject* meta = plugin_obj->metaObject();
            for (int i = 0; i < meta->classInfoCount(); ++i) {
                QMetaClassInfo classInfo = meta->classInfo(i);
                if (QString(classInfo.name()) == "IID" &&
                    QString(classInfo.value()).contains(interface_id)) {
                    matches.push_back(plugin);
                    break;
                }
            }
        }
    }

    return matches;
}

std::vector<std::shared_ptr<IPlugin>> PluginCapabilityMatcher::match_by_method(
    const QString& method_signature,
    const std::vector<std::shared_ptr<IPlugin>>& plugins) {
    std::vector<std::shared_ptr<IPlugin>> matches;

    for (const auto& plugin : plugins) {
        if (!plugin)
            continue;

        QObject* plugin_obj = dynamic_cast<QObject*>(plugin.get());
        if (plugin_obj) {
            const QMetaObject* meta = plugin_obj->metaObject();
            for (int i = 0; i < meta->methodCount(); ++i) {
                QMetaMethod method = meta->method(i);
                if (QString(method.methodSignature()) == method_signature) {
                    matches.push_back(plugin);
                    break;
                }
            }
        }
    }

    return matches;
}

int PluginCapabilityMatcher::score_compatibility(
    std::shared_ptr<IPlugin> plugin, const QJsonObject& requirements) {
    if (!plugin)
        return 0;

    int score = 0;
    int max_score = 0;

    // Score based on capabilities
    if (requirements.contains("capabilities")) {
        auto required_caps = static_cast<PluginCapabilities>(
            requirements["capabilities"].toInt());
        auto plugin_caps = plugin->capabilities();

        // Count matching capabilities
        for (int i = 0; i < 32; ++i) {
            PluginCapabilities cap_flag = 1 << i;
            if (required_caps & cap_flag) {
                max_score += 10;
                if (plugin_caps & cap_flag) {
                    score += 10;
                }
            }
        }
    }

    // Score based on interfaces
    if (requirements.contains("interfaces")) {
        auto interfaces_array = requirements["interfaces"].toArray();
        max_score += interfaces_array.size() * 20;

        for (const auto& interface_value : interfaces_array) {
            QString interface_id = interface_value.toString();
            auto matches = match_by_interface(interface_id, {plugin});
            if (!matches.empty()) {
                score += 20;
            }
        }
    }

    // Score based on methods
    if (requirements.contains("methods")) {
        auto methods_array = requirements["methods"].toArray();
        max_score += methods_array.size() * 5;

        for (const auto& method_value : methods_array) {
            QString method_signature = method_value.toString();
            auto matches = match_by_method(method_signature, {plugin});
            if (!matches.empty()) {
                score += 5;
            }
        }
    }

    // Return percentage score
    return max_score > 0 ? (score * 100) / max_score : 0;
}

// === PluginCapabilityDiscovery Private Implementation ===

class PluginCapabilityDiscovery::Private {
public:
    mutable QMutex mutex;
    std::unordered_map<QString, PluginCapabilityInfo> registered_capabilities;
    std::unordered_map<QString, PluginInterfaceInfo> registered_interfaces;

    void initialize_default_capabilities();
    PluginMethodInfo analyze_method(const QMetaMethod& method);
    PluginPropertyInfo analyze_property(const QMetaProperty& property);
    std::vector<PluginInterfaceInfo> discover_interfaces(QObject* plugin_obj);
};

void PluginCapabilityDiscovery::Private::initialize_default_capabilities() {
    // Register default capability definitions
    PluginCapabilityInfo ui_capability;
    ui_capability.name = "UI";
    ui_capability.description = "User interface capabilities";
    ui_capability.capability_flag = PluginCapability::UI;
    ui_capability.required_methods << "create_widget"
                                   << "get_available_widgets";
    ui_capability.optional_methods << "create_action" << "create_menu"
                                   << "create_toolbar";
    registered_capabilities[ui_capability.name] = ui_capability;

    PluginCapabilityInfo service_capability;
    service_capability.name = "Service";
    service_capability.description = "Background service capabilities";
    service_capability.capability_flag = PluginCapability::Service;
    service_capability.required_methods << "start_service" << "stop_service";
    service_capability.optional_methods << "pause_service" << "resume_service";
    registered_capabilities[service_capability.name] = service_capability;

    PluginCapabilityInfo network_capability;
    network_capability.name = "Network";
    network_capability.description = "Network communication capabilities";
    network_capability.capability_flag = PluginCapability::Network;
    network_capability.required_methods << "execute_request"
                                        << "supported_protocols";
    service_capability.optional_methods << "establish_connection"
                                        << "configure_ssl";
    registered_capabilities[network_capability.name] = network_capability;

    PluginCapabilityInfo data_processing_capability;
    data_processing_capability.name = "DataProcessing";
    data_processing_capability.description = "Data processing capabilities";
    data_processing_capability.capability_flag =
        PluginCapability::DataProcessing;
    data_processing_capability.required_methods << "process_data"
                                                << "supported_operations";
    data_processing_capability.optional_methods << "process_batch"
                                                << "validate_data";
    registered_capabilities[data_processing_capability.name] =
        data_processing_capability;

    PluginCapabilityInfo scripting_capability;
    scripting_capability.name = "Scripting";
    scripting_capability.description = "Script execution capabilities";
    scripting_capability.capability_flag = PluginCapability::Scripting;
    scripting_capability.required_methods << "execute_script"
                                          << "supported_languages";
    scripting_capability.optional_methods << "validate_script"
                                          << "register_function";
    registered_capabilities[scripting_capability.name] = scripting_capability;
}

PluginMethodInfo PluginCapabilityDiscovery::Private::analyze_method(
    const QMetaMethod& method) {
    PluginMethodInfo info;
    info.name = QString(method.name());
    info.signature = QString(method.methodSignature());
    info.return_type = QString(method.typeName());
    info.is_invokable = (method.methodType() == QMetaMethod::Method);
    info.is_slot = (method.methodType() == QMetaMethod::Slot);
    info.is_signal = (method.methodType() == QMetaMethod::Signal);
    info.access = method.access();

    // Extract parameter information
    QList<QByteArray> param_types = method.parameterTypes();
    QList<QByteArray> param_names = method.parameterNames();

    for (const auto& type : param_types) {
        info.parameter_types.append(QString(type));
    }

    for (const auto& name : param_names) {
        info.parameter_names.append(QString(name));
    }

    return info;
}

PluginPropertyInfo PluginCapabilityDiscovery::Private::analyze_property(
    const QMetaProperty& property) {
    PluginPropertyInfo info;
    info.name = QString(property.name());
    info.type = QString(property.typeName());
    info.is_readable = property.isReadable();
    info.is_writable = property.isWritable();
    info.is_resettable = property.isResettable();
    info.has_notify_signal = property.hasNotifySignal();

    if (info.has_notify_signal) {
        QMetaMethod notify_method = property.notifySignal();
        info.notify_signal = QString(notify_method.name());
    }

    return info;
}

std::vector<PluginInterfaceInfo>
PluginCapabilityDiscovery::Private::discover_interfaces(QObject* plugin_obj) {
    std::vector<PluginInterfaceInfo> interfaces;

    if (!plugin_obj)
        return interfaces;

    const QMetaObject* meta = plugin_obj->metaObject();

    // Look for interface information in class info
    for (int i = 0; i < meta->classInfoCount(); ++i) {
        QMetaClassInfo classInfo = meta->classInfo(i);
        if (QString(classInfo.name()) == "IID") {
            PluginInterfaceInfo interface_info;
            interface_info.interface_id = QString(classInfo.value());
            interface_info.interface_name =
                interface_info.interface_id.split('/').first();

            // Extract version if present
            QStringList parts = interface_info.interface_id.split('/');
            if (parts.size() > 1) {
                interface_info.version = parts.last();
            }

            interfaces.push_back(interface_info);
        }
    }

    return interfaces;
}

// === PluginCapabilityDiscovery Implementation ===

PluginCapabilityDiscovery::PluginCapabilityDiscovery(QObject* parent)
    : QObject(parent), d(std::make_unique<Private>()) {
    d->initialize_default_capabilities();
    qCDebug(capabilityDiscoveryLog)
        << "Plugin capability discovery initialized";
}

PluginCapabilityDiscovery::~PluginCapabilityDiscovery() = default;

qtplugin::expected<PluginDiscoveryResult, PluginError>
PluginCapabilityDiscovery::discover_capabilities(
    std::shared_ptr<IPlugin> plugin) {
    if (!plugin) {
        return make_error<PluginDiscoveryResult>(
            PluginErrorCode::InvalidArgument, "Plugin is null");
    }

    PluginDiscoveryResult result;
    result.plugin_id = QString::fromStdString(plugin->id());
    result.plugin_name = QString::fromStdString(std::string(plugin->name()));
    result.capabilities = plugin->capabilities();

    // Analyze plugin as QObject for meta-object introspection
    QObject* plugin_obj = dynamic_cast<QObject*>(plugin.get());
    if (plugin_obj) {
        result.interfaces = d->discover_interfaces(plugin_obj);
        result.methods = get_plugin_methods(plugin);
        result.properties = get_plugin_properties(plugin);
    }

    // Match capabilities with registered definitions
    QMutexLocker locker(&d->mutex);
    for (const auto& [name, capability_info] : d->registered_capabilities) {
        if (plugin->has_capability(capability_info.capability_flag)) {
            result.capability_details.push_back(capability_info);
        }
    }

    // Add metadata
    result.metadata["discovery_time"] =
        QDateTime::currentDateTime().toString(Qt::ISODate);
    result.metadata["plugin_version"] = QString("%1.%2.%3")
                                            .arg(plugin->version().major)
                                            .arg(plugin->version().minor)
                                            .arg(plugin->version().patch);

    emit capability_discovered(result.plugin_id, result);

    qCDebug(capabilityDiscoveryLog)
        << "Discovered capabilities for plugin:" << result.plugin_id
        << "capabilities:" << result.capabilities
        << "interfaces:" << result.interfaces.size()
        << "methods:" << result.methods.size()
        << "properties:" << result.properties.size();

    return result;
}

std::vector<PluginDiscoveryResult>
PluginCapabilityDiscovery::discover_batch_capabilities(
    const std::vector<std::shared_ptr<IPlugin>>& plugins) {
    std::vector<PluginDiscoveryResult> results;
    results.reserve(plugins.size());

    for (const auto& plugin : plugins) {
        auto result = discover_capabilities(plugin);
        if (result) {
            results.push_back(result.value());
        } else {
            qCWarning(capabilityDiscoveryLog)
                << "Failed to discover capabilities for plugin:"
                << (plugin ? QString::fromStdString(plugin->id()) : "null")
                << "error:" << result.error().message.c_str();
        }
    }

    return results;
}

std::vector<PluginMethodInfo> PluginCapabilityDiscovery::get_plugin_methods(
    std::shared_ptr<IPlugin> plugin) {
    std::vector<PluginMethodInfo> methods;

    QObject* plugin_obj = dynamic_cast<QObject*>(plugin.get());
    if (!plugin_obj)
        return methods;

    const QMetaObject* meta = plugin_obj->metaObject();
    for (int i = 0; i < meta->methodCount(); ++i) {
        QMetaMethod method = meta->method(i);
        methods.push_back(d->analyze_method(method));
    }

    return methods;
}

std::vector<PluginPropertyInfo>
PluginCapabilityDiscovery::get_plugin_properties(
    std::shared_ptr<IPlugin> plugin) {
    std::vector<PluginPropertyInfo> properties;

    QObject* plugin_obj = dynamic_cast<QObject*>(plugin.get());
    if (!plugin_obj)
        return properties;

    const QMetaObject* meta = plugin_obj->metaObject();
    for (int i = 0; i < meta->propertyCount(); ++i) {
        QMetaProperty property = meta->property(i);
        properties.push_back(d->analyze_property(property));
    }

    return properties;
}

qtplugin::expected<void, PluginError>
PluginCapabilityDiscovery::register_capability(
    const PluginCapabilityInfo& capability_info) {
    QMutexLocker locker(&d->mutex);

    d->registered_capabilities[capability_info.name] = capability_info;

    emit capability_registration_changed(capability_info.name, true);

    qCDebug(capabilityDiscoveryLog)
        << "Registered capability:" << capability_info.name;

    return make_success();
}

qtplugin::expected<void, PluginError>
PluginCapabilityDiscovery::unregister_capability(
    const QString& capability_name) {
    QMutexLocker locker(&d->mutex);

    auto it = d->registered_capabilities.find(capability_name);
    if (it == d->registered_capabilities.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Capability not found: " + capability_name.toStdString());
    }

    d->registered_capabilities.erase(it);

    emit capability_registration_changed(capability_name, false);

    qCDebug(capabilityDiscoveryLog)
        << "Unregistered capability:" << capability_name;

    return make_success();
}

std::vector<PluginCapabilityInfo>
PluginCapabilityDiscovery::get_registered_capabilities() const {
    QMutexLocker locker(&d->mutex);

    std::vector<PluginCapabilityInfo> capabilities;
    capabilities.reserve(d->registered_capabilities.size());

    for (const auto& [name, capability_info] : d->registered_capabilities) {
        capabilities.push_back(capability_info);
    }

    return capabilities;
}

qtplugin::expected<PluginCapabilityInfo, PluginError>
PluginCapabilityDiscovery::get_capability_definition(
    const QString& capability_name) const {
    QMutexLocker locker(&d->mutex);

    auto it = d->registered_capabilities.find(capability_name);
    if (it == d->registered_capabilities.end()) {
        return make_error<PluginCapabilityInfo>(
            PluginErrorCode::NotFound,
            "Capability not found: " + capability_name.toStdString());
    }

    return it->second;
}

qtplugin::expected<QVariant, PluginError>
PluginCapabilityDiscovery::invoke_method(std::shared_ptr<IPlugin> plugin,
                                         const QString& method_name,
                                         const QVariantList& arguments) {
    if (!plugin) {
        return make_error<QVariant>(PluginErrorCode::InvalidArgument,
                                    "Plugin is null");
    }

    QObject* plugin_obj = dynamic_cast<QObject*>(plugin.get());
    if (!plugin_obj) {
        return make_error<QVariant>(PluginErrorCode::InvalidArgument,
                                    "Plugin is not a QObject");
    }

    // Find the method
    const QMetaObject* meta = plugin_obj->metaObject();
    int method_index = -1;

    for (int i = 0; i < meta->methodCount(); ++i) {
        QMetaMethod method = meta->method(i);
        if (QString(method.name()) == method_name) {
            method_index = i;
            break;
        }
    }

    if (method_index == -1) {
        return make_error<QVariant>(
            PluginErrorCode::CommandNotFound,
            "Method not found: " + method_name.toStdString());
    }

    QMetaMethod method = meta->method(method_index);

    // Prepare arguments for invocation
    QGenericReturnArgument return_arg;
    QVariant return_value;

    if (method.returnType() != QMetaType::Void) {
        return_value = QVariant(method.returnType());
        return_arg =
            QGenericReturnArgument(method.typeName(), return_value.data());
    }

    // Convert arguments
    QList<QGenericArgument> generic_args;
    for (int i = 0; i < arguments.size() && i < method.parameterCount(); ++i) {
        const QVariant& arg = arguments[i];
        generic_args.append(QGenericArgument(arg.typeName(), arg.constData()));
    }

    // Invoke the method
    bool success = false;
    if (generic_args.isEmpty()) {
        success = method.invoke(plugin_obj, return_arg);
    } else {
        // Handle up to 10 arguments (Qt limitation)
        success = method.invoke(plugin_obj, return_arg,
                                generic_args.value(0, QGenericArgument()),
                                generic_args.value(1, QGenericArgument()),
                                generic_args.value(2, QGenericArgument()),
                                generic_args.value(3, QGenericArgument()),
                                generic_args.value(4, QGenericArgument()),
                                generic_args.value(5, QGenericArgument()),
                                generic_args.value(6, QGenericArgument()),
                                generic_args.value(7, QGenericArgument()),
                                generic_args.value(8, QGenericArgument()),
                                generic_args.value(9, QGenericArgument()));
    }

    if (!success) {
        return make_error<QVariant>(
            PluginErrorCode::ExecutionFailed,
            "Failed to invoke method: " + method_name.toStdString());
    }

    return return_value;
}

}  // namespace qtplugin
