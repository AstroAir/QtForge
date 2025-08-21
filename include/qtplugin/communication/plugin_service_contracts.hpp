/**
 * @file plugin_service_contracts.hpp
 * @brief Plugin service contracts for formal inter-plugin communication
 * @version 3.1.0
 * @author QtPlugin Development Team
 *
 * This file defines the service contract system that allows plugins to
 * formally declare and consume services from other plugins with type safety,
 * capability validation, and contract enforcement.
 */

#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QString>
#include <chrono>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include "../core/plugin_interface.hpp"
#include "../utils/concepts.hpp"
#include "../utils/error_handling.hpp"

namespace qtplugin::contracts {

/**
 * @brief Service contract version for compatibility checking
 */
struct ServiceVersion {
    uint32_t major{1};
    uint32_t minor{0};
    uint32_t patch{0};

    ServiceVersion() = default;
    ServiceVersion(uint32_t maj, uint32_t min, uint32_t pat)
        : major(maj), minor(min), patch(pat) {}

    bool is_compatible_with(const ServiceVersion& other) const noexcept {
        return major == other.major && minor >= other.minor;
    }

    std::string to_string() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." +
               std::to_string(patch);
    }
};

/**
 * @brief Service capability flags
 */
enum class ServiceCapability : uint32_t {
    None = 0x0000,
    Synchronous = 0x0001,    // Supports synchronous calls
    Asynchronous = 0x0002,   // Supports asynchronous calls
    Streaming = 0x0004,      // Supports streaming data
    Transactional = 0x0008,  // Supports transactions
    Cacheable = 0x0010,      // Results can be cached
    Idempotent = 0x0020,     // Operations are idempotent
    ThreadSafe = 0x0040,     // Thread-safe operations
    Stateful = 0x0080,       // Maintains state between calls
    Discoverable = 0x0100,   // Can be discovered automatically
    Versioned = 0x0200,      // Supports versioning
    Authenticated = 0x0400,  // Requires authentication
    Encrypted = 0x0800       // Supports encryption
};

using ServiceCapabilities = uint32_t;

/**
 * @brief Service method parameter definition
 */
struct ServiceParameter {
    QString name;
    QString type;  // JSON type or custom type name
    QString description;
    bool required{true};
    QJsonValue default_value;
    QString validation_pattern;  // Regex pattern for validation

    ServiceParameter() = default;
    ServiceParameter(const QString& n, const QString& t,
                     const QString& desc = "", bool req = true)
        : name(n), type(t), description(desc), required(req) {}
};

/**
 * @brief Service method definition
 */
struct ServiceMethod {
    QString name;
    QString description;
    std::vector<ServiceParameter> parameters;
    ServiceParameter return_type;
    ServiceCapabilities capabilities{
        static_cast<uint32_t>(ServiceCapability::Synchronous)};
    std::chrono::milliseconds timeout{std::chrono::milliseconds{30000}};
    QString example_usage;

    ServiceMethod() = default;
    ServiceMethod(const QString& n, const QString& desc = "")
        : name(n), description(desc) {}

    ServiceMethod& add_parameter(const ServiceParameter& param) {
        parameters.push_back(param);
        return *this;
    }

    ServiceMethod& set_return_type(const ServiceParameter& ret) {
        return_type = ret;
        return *this;
    }

    ServiceMethod& set_capabilities(ServiceCapabilities caps) {
        capabilities = caps;
        return *this;
    }

    ServiceMethod& set_timeout(std::chrono::milliseconds t) {
        timeout = t;
        return *this;
    }
};

/**
 * @brief Service contract definition
 */
class ServiceContract {
public:
    ServiceContract(const QString& service_name,
                    const ServiceVersion& version = {})
        : m_service_name(service_name), m_version(version) {}

    // === Contract Definition ===

    ServiceContract& set_description(const QString& desc) {
        m_description = desc;
        return *this;
    }

    ServiceContract& set_provider(const QString& provider) {
        m_provider = provider;
        return *this;
    }

    ServiceContract& add_method(const ServiceMethod& method) {
        m_methods[method.name] = method;
        return *this;
    }

    ServiceContract& set_capabilities(ServiceCapabilities caps) {
        m_capabilities = caps;
        return *this;
    }

    ServiceContract& add_dependency(const QString& service_name,
                                    const ServiceVersion& min_version = {}) {
        m_dependencies[service_name] = min_version;
        return *this;
    }

    // === Contract Access ===

    const QString& service_name() const noexcept { return m_service_name; }
    const ServiceVersion& version() const noexcept { return m_version; }
    const QString& description() const noexcept { return m_description; }
    const QString& provider() const noexcept { return m_provider; }
    ServiceCapabilities capabilities() const noexcept { return m_capabilities; }

    const std::unordered_map<QString, ServiceMethod>& methods() const noexcept {
        return m_methods;
    }

    const std::unordered_map<QString, ServiceVersion>& dependencies()
        const noexcept {
        return m_dependencies;
    }

    bool has_method(const QString& method_name) const {
        return m_methods.find(method_name) != m_methods.end();
    }

    const ServiceMethod* get_method(const QString& method_name) const {
        auto it = m_methods.find(method_name);
        return it != m_methods.end() ? &it->second : nullptr;
    }

    // === Validation ===

    qtplugin::expected<void, PluginError> validate() const;
    qtplugin::expected<void, PluginError> validate_method_call(
        const QString& method_name, const QJsonObject& parameters) const;

    // === Serialization ===

    QJsonObject to_json() const;
    static qtplugin::expected<ServiceContract, PluginError> from_json(
        const QJsonObject& json);

private:
    QString m_service_name;
    ServiceVersion m_version;
    QString m_description;
    QString m_provider;
    ServiceCapabilities m_capabilities{
        static_cast<uint32_t>(ServiceCapability::Synchronous)};
    std::unordered_map<QString, ServiceMethod> m_methods;
    std::unordered_map<QString, ServiceVersion> m_dependencies;
};

/**
 * @brief Service contract registry for managing contracts
 */
class ServiceContractRegistry {
public:
    static ServiceContractRegistry& instance();

    // === Contract Management ===

    qtplugin::expected<void, PluginError> register_contract(
        const QString& plugin_id, const ServiceContract& contract);

    qtplugin::expected<void, PluginError> unregister_contract(
        const QString& plugin_id, const QString& service_name);

    qtplugin::expected<ServiceContract, PluginError> get_contract(
        const QString& service_name,
        const ServiceVersion& min_version = {}) const;

    std::vector<ServiceContract> find_contracts_by_capability(
        ServiceCapability capability) const;

    std::vector<QString> list_services() const;
    std::vector<QString> list_providers() const;

    // === Contract Validation ===

    qtplugin::expected<void, PluginError> validate_dependencies(
        const ServiceContract& contract) const;

    qtplugin::expected<void, PluginError> validate_compatibility(
        const QString& service_name,
        const ServiceVersion& required_version) const;

    // === Contract Discovery ===

    std::vector<ServiceContract> discover_services_for_plugin(
        const QString& plugin_id) const;

    qtplugin::expected<QString, PluginError> find_provider(
        const QString& service_name,
        const ServiceVersion& min_version = {}) const;

private:
    ServiceContractRegistry() = default;

    struct ContractInfo {
        QString plugin_id;
        ServiceContract contract;
        std::chrono::system_clock::time_point registered_at;

        ContractInfo() = default;
        ContractInfo(const QString& id, const ServiceContract& c)
            : plugin_id(id),
              contract(c),
              registered_at(std::chrono::system_clock::now()) {}
    };

    mutable std::shared_mutex m_mutex;
    std::unordered_map<QString, std::vector<ContractInfo>>
        m_contracts;  // service_name -> contracts
    std::unordered_map<QString, std::vector<QString>>
        m_plugin_services;  // plugin_id -> service_names
};

}  // namespace qtplugin::contracts

// Qt metatype declarations
Q_DECLARE_METATYPE(qtplugin::contracts::ServiceVersion)
Q_DECLARE_METATYPE(qtplugin::contracts::ServiceCapability)
Q_DECLARE_METATYPE(qtplugin::contracts::ServiceParameter)
Q_DECLARE_METATYPE(qtplugin::contracts::ServiceMethod)
Q_DECLARE_METATYPE(qtplugin::contracts::ServiceContract)
