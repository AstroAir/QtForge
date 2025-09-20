/**
 * @file plugin_service_discovery.hpp
 * @brief Plugin service discovery system for automatic service registration and
 * discovery
 * @version 3.0.0
 */

#pragma once

#ifdef QTFORGE_HAS_NETWORK

#include <QJsonArray>
#include <QJsonObject>
#include <QMetaType>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <chrono>
#include <functional>
#include <memory>
#include <vector>

#include "../interfaces/core/plugin_interface.hpp"
#include "../utils/error_handling.hpp"

namespace qtplugin {

/**
 * @brief Service discovery modes
 */
enum class ServiceDiscoveryMode {
    Local,    ///< Local service discovery only
    Network,  ///< Network-based service discovery
    Hybrid,   ///< Both local and network discovery
    Custom    ///< Custom discovery mechanism
};

/**
 * @brief Service availability status
 */
enum class ServiceAvailability {
    Available,    ///< Service is available
    Unavailable,  ///< Service is unavailable
    Degraded,     ///< Service is available but degraded
    Maintenance,  ///< Service is under maintenance
    Unknown       ///< Service status is unknown
};

/**
 * @brief Service registration information
 */
struct ServiceRegistration {
    QString service_id;                ///< Service identifier
    QString plugin_id;                 ///< Plugin identifier
    QString service_name;              ///< Service name
    QString service_version;           ///< Service version
    QString description;               ///< Service description
    QStringList tags;                  ///< Service tags
    QStringList categories;            ///< Service categories
    QJsonObject endpoints;             ///< Service endpoints
    QJsonObject configuration;         ///< Service configuration
    ServiceAvailability availability;  ///< Service availability
    std::chrono::system_clock::time_point
        registration_time;  ///< Registration time
    std::chrono::system_clock::time_point
        last_heartbeat;    ///< Last heartbeat time
    QJsonObject metadata;  ///< Additional metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static ServiceRegistration from_json(const QJsonObject& json);
};

/**
 * @brief Service discovery query
 */
struct ServiceDiscoveryQuery {
    QString service_name;             ///< Service name filter
    QString service_version;          ///< Service version filter
    QStringList required_tags;        ///< Required tags
    QStringList required_categories;  ///< Required categories
    ServiceAvailability min_availability =
        ServiceAvailability::Available;   ///< Minimum availability
    QJsonObject capability_requirements;  ///< Capability requirements
    int max_results = 100;                ///< Maximum results
    bool include_unavailable = false;     ///< Include unavailable services
    QJsonObject custom_filters;           ///< Custom filters

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;

    /**
     * @brief Create from JSON object
     */
    static ServiceDiscoveryQuery from_json(const QJsonObject& json);
};

/**
 * @brief Service discovery result
 */
struct ServiceDiscoveryResult {
    std::vector<ServiceRegistration> services;    ///< Found services
    int total_found = 0;                          ///< Total services found
    std::chrono::milliseconds discovery_time{0};  ///< Discovery time
    QString discovery_source;                     ///< Discovery source
    QJsonObject metadata;                         ///< Result metadata

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Service health check information
 */
struct ServiceHealthCheck {
    QString service_id;                               ///< Service identifier
    QString health_check_url;                         ///< Health check URL
    std::chrono::milliseconds check_interval{60000};  ///< Check interval
    std::chrono::milliseconds timeout{5000};          ///< Health check timeout
    int max_failures = 3;       ///< Maximum failures before marking unavailable
    QJsonObject custom_checks;  ///< Custom health checks

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Service load balancing information
 */
struct ServiceLoadBalancing {
    QString service_name;             ///< Service name
    QString load_balancing_strategy;  ///< Load balancing strategy (round_robin,
                                      ///< least_connections, etc.)
    std::vector<QString> service_instances;  ///< Service instance IDs
    QJsonObject weights;                     ///< Instance weights
    QJsonObject configuration;               ///< Load balancing configuration

    /**
     * @brief Convert to JSON object
     */
    QJsonObject to_json() const;
};

/**
 * @brief Service discovery event callback
 */
using ServiceDiscoveryCallback =
    std::function<void(const ServiceRegistration&, bool /* added */)>;

/**
 * @brief Service health check callback
 */
using ServiceHealthCheckCallback =
    std::function<ServiceAvailability(const QString& service_id)>;

/**
 * @brief Plugin service discovery system
 *
 * This class provides automatic service discovery and registration for plugins,
 * including health monitoring, load balancing, and network discovery.
 */
class PluginServiceDiscovery : public QObject {
    Q_OBJECT

public:
    explicit PluginServiceDiscovery(QObject* parent = nullptr);
    ~PluginServiceDiscovery() override;

    // === Configuration ===

    /**
     * @brief Set discovery mode
     * @param mode Discovery mode
     */
    void set_discovery_mode(ServiceDiscoveryMode mode);

    /**
     * @brief Get discovery mode
     * @return Current discovery mode
     */
    ServiceDiscoveryMode get_discovery_mode() const;

    /**
     * @brief Set discovery configuration
     * @param config Discovery configuration
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> set_discovery_config(
        const QJsonObject& config);

    /**
     * @brief Get discovery configuration
     * @return Discovery configuration
     */
    QJsonObject get_discovery_config() const;

    // === Service Registration ===

    /**
     * @brief Register service
     * @param registration Service registration information
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> register_service(
        const ServiceRegistration& registration);

    /**
     * @brief Unregister service
     * @param service_id Service identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_service(
        const QString& service_id);

    /**
     * @brief Update service registration
     * @param service_id Service identifier
     * @param registration Updated registration information
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> update_service_registration(
        const QString& service_id, const ServiceRegistration& registration);

    /**
     * @brief Auto-register plugin services
     * @param plugin Plugin to register services for
     * @return Number of registered services
     */
    int auto_register_plugin_services(std::shared_ptr<IPlugin> plugin);

    /**
     * @brief Auto-unregister plugin services
     * @param plugin_id Plugin identifier
     * @return Number of unregistered services
     */
    int auto_unregister_plugin_services(const QString& plugin_id);

    // === Service Discovery ===

    /**
     * @brief Discover services
     * @param query Discovery query
     * @return Discovery result or error
     */
    qtplugin::expected<ServiceDiscoveryResult, PluginError> discover_services(
        const ServiceDiscoveryQuery& query);

    /**
     * @brief Discover services by name
     * @param service_name Service name
     * @param version Optional version filter
     * @return Vector of matching services
     */
    std::vector<ServiceRegistration> discover_services_by_name(
        const QString& service_name, const QString& version = QString());

    /**
     * @brief Discover services by tag
     * @param tag Service tag
     * @return Vector of matching services
     */
    std::vector<ServiceRegistration> discover_services_by_tag(
        const QString& tag);

    /**
     * @brief Discover services by category
     * @param category Service category
     * @return Vector of matching services
     */
    std::vector<ServiceRegistration> discover_services_by_category(
        const QString& category);

    /**
     * @brief Get all registered services
     * @param include_unavailable Include unavailable services
     * @return Vector of all services
     */
    std::vector<ServiceRegistration> get_all_services(
        bool include_unavailable = false) const;

    /**
     * @brief Get service registration
     * @param service_id Service identifier
     * @return Service registration or error
     */
    qtplugin::expected<ServiceRegistration, PluginError>
    get_service_registration(const QString& service_id) const;

    // === Service Health Monitoring ===

    /**
     * @brief Enable health monitoring for service
     * @param service_id Service identifier
     * @param health_check Health check configuration
     * @param callback Optional custom health check callback
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> enable_health_monitoring(
        const QString& service_id, const ServiceHealthCheck& health_check,
        ServiceHealthCheckCallback callback = nullptr);

    /**
     * @brief Disable health monitoring for service
     * @param service_id Service identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> disable_health_monitoring(
        const QString& service_id);

    /**
     * @brief Perform health check
     * @param service_id Service identifier
     * @return Service availability or error
     */
    qtplugin::expected<ServiceAvailability, PluginError> check_service_health(
        const QString& service_id);

    /**
     * @brief Get service health status
     * @param service_id Service identifier
     * @return Health status information
     */
    qtplugin::expected<QJsonObject, PluginError> get_service_health_status(
        const QString& service_id) const;

    // === Load Balancing ===

    /**
     * @brief Configure load balancing for service
     * @param load_balancing Load balancing configuration
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> configure_load_balancing(
        const ServiceLoadBalancing& load_balancing);

    /**
     * @brief Get next service instance for load balancing
     * @param service_name Service name
     * @return Service instance ID or error
     */
    qtplugin::expected<QString, PluginError> get_next_service_instance(
        const QString& service_name);

    /**
     * @brief Update service instance weight
     * @param service_name Service name
     * @param instance_id Instance identifier
     * @param weight New weight
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> update_instance_weight(
        const QString& service_name, const QString& instance_id, double weight);

    // === Event Handling ===

    /**
     * @brief Register service discovery callback
     * @param callback Discovery callback
     * @return Callback ID for unregistration
     */
    QString register_discovery_callback(ServiceDiscoveryCallback callback);

    /**
     * @brief Unregister service discovery callback
     * @param callback_id Callback identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> unregister_discovery_callback(
        const QString& callback_id);

    // === Network Discovery ===

    /**
     * @brief Enable network discovery
     * @param multicast_address Multicast address for discovery
     * @param port Discovery port
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> enable_network_discovery(
        const QString& multicast_address = "239.255.255.250", int port = 1900);

    /**
     * @brief Disable network discovery
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> disable_network_discovery();

    /**
     * @brief Announce service on network
     * @param service_id Service identifier
     * @return Success or error
     */
    qtplugin::expected<void, PluginError> announce_service(
        const QString& service_id);

    /**
     * @brief Query network for services
     * @param query Discovery query
     * @return Discovery result or error
     */
    qtplugin::expected<ServiceDiscoveryResult, PluginError>
    query_network_services(const ServiceDiscoveryQuery& query);

    // === Statistics and Monitoring ===

    /**
     * @brief Get discovery statistics
     * @return Discovery statistics
     */
    QJsonObject get_discovery_statistics() const;

    /**
     * @brief Reset discovery statistics
     */
    void reset_statistics();

    /**
     * @brief Get service usage statistics
     * @param service_id Service identifier
     * @return Usage statistics or error
     */
    qtplugin::expected<QJsonObject, PluginError> get_service_usage_statistics(
        const QString& service_id) const;

signals:
    /**
     * @brief Emitted when service is registered
     * @param registration Service registration
     */
    void service_registered(const ServiceRegistration& registration);

    /**
     * @brief Emitted when service is unregistered
     * @param service_id Service identifier
     */
    void service_unregistered(const QString& service_id);

    /**
     * @brief Emitted when service availability changes
     * @param service_id Service identifier
     * @param availability New availability status
     */
    void service_availability_changed(const QString& service_id,
                                      ServiceAvailability availability);

    /**
     * @brief Emitted when network service is discovered
     * @param registration Service registration
     */
    void network_service_discovered(const ServiceRegistration& registration);

    /**
     * @brief Emitted when network service is lost
     * @param service_id Service identifier
     */
    void network_service_lost(const QString& service_id);

private slots:
    void on_heartbeat_timer();
    void on_health_check_timer();
    void on_network_discovery_timer();
    void on_network_data_received();

private:
    class Private;
    std::unique_ptr<Private> d;
};

}  // namespace qtplugin

// Register meta types for Qt's meta-object system
Q_DECLARE_METATYPE(qtplugin::ServiceDiscoveryMode)
Q_DECLARE_METATYPE(qtplugin::ServiceAvailability)
Q_DECLARE_METATYPE(qtplugin::ServiceRegistration)
Q_DECLARE_METATYPE(qtplugin::ServiceDiscoveryQuery)
Q_DECLARE_METATYPE(qtplugin::ServiceDiscoveryResult)
Q_DECLARE_METATYPE(qtplugin::ServiceHealthCheck)
Q_DECLARE_METATYPE(qtplugin::ServiceLoadBalancing)

#endif  // QTFORGE_HAS_NETWORK
