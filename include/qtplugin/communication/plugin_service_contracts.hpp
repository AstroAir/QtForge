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
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

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

// Bitwise operators for ServiceCapability enum
inline ServiceCapability operator|(ServiceCapability lhs,
                                   ServiceCapability rhs) {
    return static_cast<ServiceCapability>(static_cast<uint32_t>(lhs) |
                                          static_cast<uint32_t>(rhs));
}

inline ServiceCapability operator&(ServiceCapability lhs,
                                   ServiceCapability rhs) {
    return static_cast<ServiceCapability>(static_cast<uint32_t>(lhs) &
                                          static_cast<uint32_t>(rhs));
}

inline ServiceCapability operator^(ServiceCapability lhs,
                                   ServiceCapability rhs) {
    return static_cast<ServiceCapability>(static_cast<uint32_t>(lhs) ^
                                          static_cast<uint32_t>(rhs));
}

inline ServiceCapability operator~(ServiceCapability cap) {
    return static_cast<ServiceCapability>(~static_cast<uint32_t>(cap));
}

inline ServiceCapability& operator|=(ServiceCapability& lhs,
                                     ServiceCapability rhs) {
    lhs = lhs | rhs;
    return lhs;
}

inline ServiceCapability& operator&=(ServiceCapability& lhs,
                                     ServiceCapability rhs) {
    lhs = lhs & rhs;
    return lhs;
}

inline ServiceCapability& operator^=(ServiceCapability& lhs,
                                     ServiceCapability rhs) {
    lhs = lhs ^ rhs;
    return lhs;
}

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
                    const ServiceVersion& version = {});

    // Copy constructor and assignment operator
    ServiceContract(const ServiceContract& other);
    ServiceContract& operator=(const ServiceContract& other);

    // Move constructor and assignment operator
    ServiceContract(ServiceContract&& other) noexcept;
    ServiceContract& operator=(ServiceContract&& other) noexcept;

    // Destructor
    ~ServiceContract();

    // === Contract Definition ===

    ServiceContract& set_description(const QString& desc);
    ServiceContract& set_provider(const QString& provider);
    ServiceContract& add_method(const ServiceMethod& method);
    ServiceContract& set_capabilities(ServiceCapabilities caps);
    ServiceContract& add_dependency(const QString& service_name,
                                    const ServiceVersion& min_version = {});

    // === Contract Access ===

    const QString& service_name() const noexcept;
    const ServiceVersion& version() const noexcept;
    const QString& description() const noexcept;
    const QString& provider() const noexcept;
    ServiceCapabilities capabilities() const noexcept;

    const std::unordered_map<QString, ServiceMethod>& methods() const noexcept;
    const std::unordered_map<QString, ServiceVersion>& dependencies() const noexcept;

    bool has_method(const QString& method_name) const;
    const ServiceMethod* get_method(const QString& method_name) const;

    // === Validation ===

    qtplugin::expected<void, PluginError> validate() const;
    qtplugin::expected<void, PluginError> validate_method_call(
        const QString& method_name, const QJsonObject& parameters) const;

    // === Serialization ===

    QJsonObject to_json() const;
    static qtplugin::expected<ServiceContract, PluginError> from_json(
        const QJsonObject& json);

private:
    class Private;
    std::unique_ptr<Private> d;
};

/**
 * @brief Service contract registry for managing contracts
 */
class ServiceContractRegistry {
public:
    ServiceContractRegistry();
    ~ServiceContractRegistry();

    // Non-copyable and non-movable (singleton pattern)
    ServiceContractRegistry(const ServiceContractRegistry&) = delete;
    ServiceContractRegistry& operator=(const ServiceContractRegistry&) = delete;
    ServiceContractRegistry(ServiceContractRegistry&&) = delete;
    ServiceContractRegistry& operator=(ServiceContractRegistry&&) = delete;

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
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace qtplugin::contracts

// Qt metatype declarations
Q_DECLARE_METATYPE(qtplugin::contracts::ServiceVersion)
Q_DECLARE_METATYPE(qtplugin::contracts::ServiceCapability)
Q_DECLARE_METATYPE(qtplugin::contracts::ServiceParameter)
Q_DECLARE_METATYPE(qtplugin::contracts::ServiceMethod)
Q_DECLARE_METATYPE(qtplugin::contracts::ServiceContract)
