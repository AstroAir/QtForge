/**
 * @file plugin_service_contracts.cpp
 * @brief Implementation of plugin service contracts
 * @version 3.1.0
 */

#include "qtplugin/communication/plugin_service_contracts.hpp"
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <algorithm>
#include <mutex>
#include <shared_mutex>

Q_LOGGING_CATEGORY(contractsLog, "qtplugin.contracts")

namespace qtplugin::contracts {

// === ServiceContract Implementation ===

qtplugin::expected<void, PluginError> ServiceContract::validate() const {
    // Validate service name
    if (m_service_name.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                "Service name cannot be empty");
    }

    // Validate service name format (should be like com.example.service)
    QRegularExpression name_pattern(
        R"(^[a-zA-Z][a-zA-Z0-9]*(\.[a-zA-Z][a-zA-Z0-9]*)*$)");
    if (!name_pattern.match(m_service_name).hasMatch()) {
        return make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Invalid service name format: " + m_service_name.toStdString());
    }

    // Validate methods
    if (m_methods.empty()) {
        return make_error<void>(
            PluginErrorCode::InvalidConfiguration,
            "Service contract must have at least one method");
    }

    for (const auto& [method_name, method] : m_methods) {
        if (method_name.isEmpty()) {
            return make_error<void>(PluginErrorCode::InvalidConfiguration,
                                    "Method name cannot be empty");
        }

        // Validate method parameters
        for (const auto& param : method.parameters) {
            if (param.name.isEmpty() || param.type.isEmpty()) {
                return make_error<void>(
                    PluginErrorCode::InvalidConfiguration,
                    "Parameter name and type cannot be empty in method: " +
                        method_name.toStdString());
            }
        }
    }

    return make_success();
}

qtplugin::expected<void, PluginError> ServiceContract::validate_method_call(
    const QString& method_name, const QJsonObject& parameters) const {
    // Check if method exists
    auto method_it = m_methods.find(method_name);
    if (method_it == m_methods.end()) {
        return make_error<void>(
            PluginErrorCode::CommandNotFound,
            "Method not found: " + method_name.toStdString());
    }

    const ServiceMethod& method = method_it->second;

    // Validate required parameters
    for (const auto& param : method.parameters) {
        if (param.required && !parameters.contains(param.name)) {
            return make_error<void>(
                PluginErrorCode::InvalidParameters,
                "Required parameter missing: " + param.name.toStdString());
        }

        // Validate parameter types if present
        if (parameters.contains(param.name)) {
            const QJsonValue& value = parameters[param.name];

            // Basic type validation
            if (param.type == "string" && !value.isString()) {
                return make_error<void>(PluginErrorCode::InvalidParameters,
                                        "Parameter " +
                                            param.name.toStdString() +
                                            " must be a string");
            } else if (param.type == "number" && !value.isDouble()) {
                return make_error<void>(PluginErrorCode::InvalidParameters,
                                        "Parameter " +
                                            param.name.toStdString() +
                                            " must be a number");
            } else if (param.type == "boolean" && !value.isBool()) {
                return make_error<void>(PluginErrorCode::InvalidParameters,
                                        "Parameter " +
                                            param.name.toStdString() +
                                            " must be a boolean");
            } else if (param.type == "array" && !value.isArray()) {
                return make_error<void>(PluginErrorCode::InvalidParameters,
                                        "Parameter " +
                                            param.name.toStdString() +
                                            " must be an array");
            } else if (param.type == "object" && !value.isObject()) {
                return make_error<void>(PluginErrorCode::InvalidParameters,
                                        "Parameter " +
                                            param.name.toStdString() +
                                            " must be an object");
            }

            // Validate against pattern if specified
            if (!param.validation_pattern.isEmpty() && value.isString()) {
                QRegularExpression pattern(param.validation_pattern);
                if (!pattern.match(value.toString()).hasMatch()) {
                    return make_error<void>(
                        PluginErrorCode::InvalidParameters,
                        "Parameter " + param.name.toStdString() +
                            " does not match validation pattern");
                }
            }
        }
    }

    return make_success();
}

QJsonObject ServiceContract::to_json() const {
    QJsonObject json;
    json["service_name"] = m_service_name;
    json["version"] = QJsonObject{{"major", static_cast<int>(m_version.major)},
                                  {"minor", static_cast<int>(m_version.minor)},
                                  {"patch", static_cast<int>(m_version.patch)}};
    json["description"] = m_description;
    json["provider"] = m_provider;
    json["capabilities"] = static_cast<int>(m_capabilities);

    // Serialize methods
    QJsonObject methods_json;
    for (const auto& [name, method] : m_methods) {
        QJsonObject method_json;
        method_json["name"] = method.name;
        method_json["description"] = method.description;
        method_json["capabilities"] = static_cast<int>(method.capabilities);
        method_json["timeout"] = static_cast<int>(method.timeout.count());
        method_json["example_usage"] = method.example_usage;

        // Serialize parameters
        QJsonArray params_json;
        for (const auto& param : method.parameters) {
            QJsonObject param_json;
            param_json["name"] = param.name;
            param_json["type"] = param.type;
            param_json["description"] = param.description;
            param_json["required"] = param.required;
            param_json["default_value"] = param.default_value;
            param_json["validation_pattern"] = param.validation_pattern;
            params_json.append(param_json);
        }
        method_json["parameters"] = params_json;

        // Serialize return type
        QJsonObject return_json;
        return_json["name"] = method.return_type.name;
        return_json["type"] = method.return_type.type;
        return_json["description"] = method.return_type.description;
        method_json["return_type"] = return_json;

        methods_json[name] = method_json;
    }
    json["methods"] = methods_json;

    // Serialize dependencies
    QJsonObject deps_json;
    for (const auto& [service_name, version] : m_dependencies) {
        deps_json[service_name] =
            QJsonObject{{"major", static_cast<int>(version.major)},
                        {"minor", static_cast<int>(version.minor)},
                        {"patch", static_cast<int>(version.patch)}};
    }
    json["dependencies"] = deps_json;

    return json;
}

qtplugin::expected<ServiceContract, PluginError> ServiceContract::from_json(
    const QJsonObject& json) {
    // Validate required fields
    if (!json.contains("service_name") || !json["service_name"].isString()) {
        return make_error<ServiceContract>(
            PluginErrorCode::InvalidConfiguration,
            "Missing or invalid service_name");
    }

    QString service_name = json["service_name"].toString();

    // Parse version
    ServiceVersion version;
    if (json.contains("version") && json["version"].isObject()) {
        QJsonObject version_json = json["version"].toObject();
        version.major = version_json.value("major").toInt(1);
        version.minor = version_json.value("minor").toInt(0);
        version.patch = version_json.value("patch").toInt(0);
    }

    ServiceContract contract(service_name, version);

    // Parse optional fields
    if (json.contains("description")) {
        contract.set_description(json["description"].toString());
    }

    if (json.contains("provider")) {
        contract.set_provider(json["provider"].toString());
    }

    if (json.contains("capabilities")) {
        contract.set_capabilities(json["capabilities"].toInt());
    }

    // Parse methods
    if (json.contains("methods") && json["methods"].isObject()) {
        QJsonObject methods_json = json["methods"].toObject();
        for (auto it = methods_json.begin(); it != methods_json.end(); ++it) {
            const QString& method_name = it.key();
            const QJsonObject& method_json = it.value().toObject();

            ServiceMethod method(method_name);
            method.description = method_json.value("description").toString();
            method.capabilities = method_json.value("capabilities").toInt();
            method.timeout = std::chrono::milliseconds(
                method_json.value("timeout").toInt(30000));
            method.example_usage =
                method_json.value("example_usage").toString();

            // Parse parameters
            if (method_json.contains("parameters") &&
                method_json["parameters"].isArray()) {
                QJsonArray params_json = method_json["parameters"].toArray();
                for (const auto& param_value : params_json) {
                    QJsonObject param_json = param_value.toObject();
                    ServiceParameter param;
                    param.name = param_json["name"].toString();
                    param.type = param_json["type"].toString();
                    param.description = param_json["description"].toString();
                    param.required = param_json.value("required").toBool(true);
                    param.default_value = param_json["default_value"];
                    param.validation_pattern =
                        param_json["validation_pattern"].toString();
                    method.parameters.push_back(param);
                }
            }

            // Parse return type
            if (method_json.contains("return_type") &&
                method_json["return_type"].isObject()) {
                QJsonObject return_json = method_json["return_type"].toObject();
                method.return_type.name = return_json["name"].toString();
                method.return_type.type = return_json["type"].toString();
                method.return_type.description =
                    return_json["description"].toString();
            }

            contract.add_method(method);
        }
    }

    // Parse dependencies
    if (json.contains("dependencies") && json["dependencies"].isObject()) {
        QJsonObject deps_json = json["dependencies"].toObject();
        for (auto it = deps_json.begin(); it != deps_json.end(); ++it) {
            const QString& dep_service = it.key();
            QJsonObject version_json = it.value().toObject();
            ServiceVersion dep_version;
            dep_version.major = version_json.value("major").toInt(1);
            dep_version.minor = version_json.value("minor").toInt(0);
            dep_version.patch = version_json.value("patch").toInt(0);
            contract.add_dependency(dep_service, dep_version);
        }
    }

    // Validate the contract
    auto validation_result = contract.validate();
    if (!validation_result) {
        return qtplugin::unexpected<PluginError>(validation_result.error());
    }

    return contract;
}

// === ServiceContractRegistry Implementation ===

ServiceContractRegistry& ServiceContractRegistry::instance() {
    static ServiceContractRegistry registry;
    return registry;
}

qtplugin::expected<void, PluginError>
ServiceContractRegistry::register_contract(const QString& plugin_id,
                                           const ServiceContract& contract) {
    // Validate the contract first
    auto validation_result = contract.validate();
    if (!validation_result) {
        return validation_result;
    }

    std::unique_lock lock(m_mutex);

    const QString& service_name = contract.service_name();

    // Check for existing contract with same version
    auto& contracts = m_contracts[service_name];
    for (const auto& info : contracts) {
        if (info.contract.version().major == contract.version().major &&
            info.contract.version().minor == contract.version().minor &&
            info.contract.version().patch == contract.version().patch) {
            return make_error<void>(PluginErrorCode::DuplicatePlugin,
                                    "Service contract already registered: " +
                                        service_name.toStdString());
        }
    }

    // Add the contract
    ContractInfo info;
    info.plugin_id = plugin_id;
    info.contract = contract;
    info.registered_at = std::chrono::system_clock::now();

    contracts.push_back(info);
    m_plugin_services[plugin_id].push_back(service_name);

    qCDebug(contractsLog) << "Registered service contract:" << service_name
                          << "version"
                          << QString::fromStdString(
                                 contract.version().to_string())
                          << "for plugin" << plugin_id;

    return make_success();
}

qtplugin::expected<void, PluginError>
ServiceContractRegistry::unregister_contract(const QString& plugin_id,
                                             const QString& service_name) {
    std::unique_lock lock(m_mutex);

    auto contracts_it = m_contracts.find(service_name);
    if (contracts_it == m_contracts.end()) {
        return make_error<void>(
            PluginErrorCode::PluginNotFound,
            "Service not found: " + service_name.toStdString());
    }

    auto& contracts = contracts_it->second;
    auto contract_it = std::find_if(contracts.begin(), contracts.end(),
                                    [&plugin_id](const ContractInfo& info) {
                                        return info.plugin_id == plugin_id;
                                    });

    if (contract_it == contracts.end()) {
        return make_error<void>(
            PluginErrorCode::PluginNotFound,
            "Service not provided by plugin: " + service_name.toStdString());
    }

    contracts.erase(contract_it);

    // Remove from plugin services
    auto plugin_it = m_plugin_services.find(plugin_id);
    if (plugin_it != m_plugin_services.end()) {
        auto& services = plugin_it->second;
        services.erase(
            std::remove(services.begin(), services.end(), service_name),
            services.end());
        if (services.empty()) {
            m_plugin_services.erase(plugin_it);
        }
    }

    // Remove service entry if no contracts left
    if (contracts.empty()) {
        m_contracts.erase(contracts_it);
    }

    qCDebug(contractsLog) << "Unregistered service contract:" << service_name
                          << "for plugin" << plugin_id;

    return make_success();
}

qtplugin::expected<ServiceContract, PluginError>
ServiceContractRegistry::get_contract(const QString& service_name,
                                      const ServiceVersion& min_version) const {
    std::shared_lock lock(m_mutex);

    auto contracts_it = m_contracts.find(service_name);
    if (contracts_it == m_contracts.end()) {
        return make_error<ServiceContract>(
            PluginErrorCode::PluginNotFound,
            "Service not found: " + service_name.toStdString());
    }

    const auto& contracts = contracts_it->second;

    // Find the best matching version
    const ContractInfo* best_match = nullptr;
    for (const auto& info : contracts) {
        if (info.contract.version().is_compatible_with(min_version)) {
            if (!best_match || info.contract.version().minor >
                                   best_match->contract.version().minor) {
                best_match = &info;
            }
        }
    }

    if (!best_match) {
        return make_error<ServiceContract>(
            PluginErrorCode::IncompatibleVersion,
            "No compatible version found for service: " +
                service_name.toStdString());
    }

    return best_match->contract;
}

std::vector<ServiceContract>
ServiceContractRegistry::find_contracts_by_capability(
    ServiceCapability capability) const {
    std::shared_lock lock(m_mutex);
    std::vector<ServiceContract> result;

    uint32_t capability_flag = static_cast<uint32_t>(capability);

    for (const auto& [service_name, contracts] : m_contracts) {
        for (const auto& info : contracts) {
            if (info.contract.capabilities() & capability_flag) {
                result.push_back(info.contract);
            }
        }
    }

    return result;
}

std::vector<QString> ServiceContractRegistry::list_services() const {
    std::shared_lock lock(m_mutex);
    std::vector<QString> services;
    services.reserve(m_contracts.size());

    for (const auto& [service_name, _] : m_contracts) {
        services.push_back(service_name);
    }

    return services;
}

std::vector<QString> ServiceContractRegistry::list_providers() const {
    std::shared_lock lock(m_mutex);
    std::vector<QString> providers;

    for (const auto& [plugin_id, _] : m_plugin_services) {
        providers.push_back(plugin_id);
    }

    return providers;
}

qtplugin::expected<void, PluginError>
ServiceContractRegistry::validate_dependencies(
    const ServiceContract& contract) const {
    std::shared_lock lock(m_mutex);

    for (const auto& [dep_service, min_version] : contract.dependencies()) {
        auto dep_result = get_contract(dep_service, min_version);
        if (!dep_result) {
            return make_error<void>(
                PluginErrorCode::DependencyMissing,
                "Dependency not satisfied: " + dep_service.toStdString());
        }
    }

    return make_success();
}

qtplugin::expected<void, PluginError>
ServiceContractRegistry::validate_compatibility(
    const QString& service_name, const ServiceVersion& required_version) const {
    auto contract_result = get_contract(service_name, required_version);
    if (!contract_result) {
        return qtplugin::unexpected<PluginError>(contract_result.error());
    }

    return make_success();
}

std::vector<ServiceContract>
ServiceContractRegistry::discover_services_for_plugin(
    const QString& plugin_id) const {
    std::shared_lock lock(m_mutex);
    std::vector<ServiceContract> result;

    auto plugin_it = m_plugin_services.find(plugin_id);
    if (plugin_it != m_plugin_services.end()) {
        for (const QString& service_name : plugin_it->second) {
            auto contracts_it = m_contracts.find(service_name);
            if (contracts_it != m_contracts.end()) {
                for (const auto& info : contracts_it->second) {
                    if (info.plugin_id == plugin_id) {
                        result.push_back(info.contract);
                    }
                }
            }
        }
    }

    return result;
}

qtplugin::expected<QString, PluginError> ServiceContractRegistry::find_provider(
    const QString& service_name, const ServiceVersion& min_version) const {
    auto contract_result = get_contract(service_name, min_version);
    if (!contract_result) {
        return qtplugin::unexpected<PluginError>(contract_result.error());
    }

    std::shared_lock lock(m_mutex);
    auto contracts_it = m_contracts.find(service_name);
    if (contracts_it != m_contracts.end()) {
        for (const auto& info : contracts_it->second) {
            if (info.contract.version().is_compatible_with(min_version)) {
                return info.plugin_id;
            }
        }
    }

    return make_error<QString>(
        PluginErrorCode::PluginNotFound,
        "No provider found for service: " + service_name.toStdString());
}

}  // namespace qtplugin::contracts
