/**
 * @file plugin_service_discovery.cpp
 * @brief Implementation of plugin service discovery system
 * @version 3.0.0
 */

#include "qtplugin/communication/plugin_service_discovery.hpp"

#include <QDebug>
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include <QUuid>

#ifdef QTFORGE_HAS_NETWORK
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUdpSocket>
#endif

#include <algorithm>
#include <chrono>
#include <unordered_map>

Q_LOGGING_CATEGORY(serviceDiscoveryLog, "qtplugin.service_discovery")

namespace qtplugin {

#ifdef QTFORGE_HAS_NETWORK

// Private implementation class
class PluginServiceDiscovery::Private {
public:
    explicit Private(PluginServiceDiscovery* owner) : q(owner) {
        // Initialize timers
        heartbeat_timer = new QTimer(q);
        health_check_timer = new QTimer(q);
        network_discovery_timer = new QTimer(q);

        // Connect timer signals
        QObject::connect(heartbeat_timer, &QTimer::timeout, q,
                         &PluginServiceDiscovery::on_heartbeat_timer);
        QObject::connect(health_check_timer, &QTimer::timeout, q,
                         &PluginServiceDiscovery::on_health_check_timer);
        QObject::connect(network_discovery_timer, &QTimer::timeout, q,
                         &PluginServiceDiscovery::on_network_discovery_timer);

        // Configure timers
        heartbeat_timer->setInterval(30000);          // 30 seconds
        health_check_timer->setInterval(60000);       // 60 seconds
        network_discovery_timer->setInterval(10000);  // 10 seconds

        // Initialize network components
        network_manager = new QNetworkAccessManager(q);
        udp_socket = new QUdpSocket(q);

        QObject::connect(udp_socket, &QUdpSocket::readyRead, q,
                         &PluginServiceDiscovery::on_network_data_received);
    }

    ~Private() = default;

    PluginServiceDiscovery* q;

    // Configuration
    ServiceDiscoveryMode discovery_mode = ServiceDiscoveryMode::Local;
    bool auto_discovery_enabled = true;
    int discovery_port = 45678;
    QString multicast_group = "239.255.255.250";

    // Service registry
    mutable QMutex services_mutex;
    std::unordered_map<QString, ServiceRegistration> registered_services;
    std::unordered_map<QString, ServiceAvailability> service_availability;
    std::unordered_map<QString, std::chrono::system_clock::time_point>
        last_heartbeat;

    // Network components
    QNetworkAccessManager* network_manager;
    QUdpSocket* udp_socket;

    // Timers
    QTimer* heartbeat_timer;
    QTimer* health_check_timer;
    QTimer* network_discovery_timer;

    // Statistics
    ServiceDiscoveryResult discovery_stats;

    // Helper methods
    QString generate_service_id() {
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    bool is_service_healthy(const QString& service_id) {
        auto it = last_heartbeat.find(service_id);
        if (it == last_heartbeat.end()) {
            return false;
        }

        auto now = std::chrono::system_clock::now();
        auto elapsed =
            std::chrono::duration_cast<std::chrono::seconds>(now - it->second);

        return elapsed.count() <
               120;  // Consider healthy if heartbeat within 2 minutes
    }

    void update_service_availability(const QString& service_id,
                                     ServiceAvailability availability) {
        auto old_availability = service_availability[service_id];
        service_availability[service_id] = availability;

        if (old_availability != availability) {
            emit q->service_availability_changed(service_id, availability);
        }
    }

    void send_network_announcement(const ServiceRegistration& registration) {
        if (discovery_mode == ServiceDiscoveryMode::Local) {
            return;
        }

        QJsonObject announcement;
        announcement["type"] = "service_announcement";
        announcement["service_id"] = registration.service_id;
        announcement["service_name"] = registration.service_name;
        announcement["service_version"] = registration.service_version;
        announcement["endpoints"] = registration.endpoints;
        announcement["metadata"] = registration.metadata;
        announcement["timestamp"] =
            QDateTime::currentDateTime().toString(Qt::ISODate);

        QJsonDocument doc(announcement);
        QByteArray data = doc.toJson(QJsonDocument::Compact);

        udp_socket->writeDatagram(data, QHostAddress(multicast_group),
                                  discovery_port);
    }

    void process_network_announcement(const QJsonObject& announcement) {
        if (announcement["type"].toString() != "service_announcement") {
            return;
        }

        ServiceRegistration registration;
        registration.service_id = announcement["service_id"].toString();
        registration.service_name = announcement["service_name"].toString();
        registration.service_version =
            announcement["service_version"].toString();
        registration.endpoints = announcement["endpoints"].toObject();
        registration.metadata = announcement["metadata"].toObject();

        // Check if this is a new service
        bool is_new = registered_services.find(registration.service_id) ==
                      registered_services.end();

        registered_services[registration.service_id] = registration;
        last_heartbeat[registration.service_id] =
            std::chrono::system_clock::now();
        update_service_availability(registration.service_id,
                                    ServiceAvailability::Available);

        if (is_new) {
            emit q->network_service_discovered(registration);
        }
    }
};

// PluginServiceDiscovery implementation
PluginServiceDiscovery::PluginServiceDiscovery(QObject* parent)
    : QObject(parent), d(std::make_unique<Private>(this)) {
    qCDebug(serviceDiscoveryLog) << "PluginServiceDiscovery created";
}

PluginServiceDiscovery::~PluginServiceDiscovery() {
    qCDebug(serviceDiscoveryLog) << "PluginServiceDiscovery destroyed";
}

void PluginServiceDiscovery::set_discovery_mode(ServiceDiscoveryMode mode) {
    d->discovery_mode = mode;

    if (mode == ServiceDiscoveryMode::Network ||
        mode == ServiceDiscoveryMode::Hybrid) {
        d->heartbeat_timer->start();
        d->health_check_timer->start();
        d->network_discovery_timer->start();

        // Bind UDP socket for network discovery
        if (!d->udp_socket->bind(QHostAddress::AnyIPv4, d->discovery_port,
                                 QUdpSocket::ShareAddress)) {
            qCWarning(serviceDiscoveryLog)
                << "Failed to bind UDP socket for service discovery";
        }
    } else {
        d->heartbeat_timer->stop();
        d->health_check_timer->stop();
        d->network_discovery_timer->stop();
        d->udp_socket->close();
    }

    qCDebug(serviceDiscoveryLog)
        << "Discovery mode set to:" << static_cast<int>(mode);
}

ServiceDiscoveryMode PluginServiceDiscovery::get_discovery_mode() const {
    return d->discovery_mode;
}

qtplugin::expected<void, PluginError> PluginServiceDiscovery::register_service(
    const ServiceRegistration& registration) {
    if (registration.service_name.isEmpty()) {
        return qtplugin::unexpected(PluginError(
            PluginErrorCode::InvalidArgument, "Service name is required"));
    }

    QMutexLocker locker(&d->services_mutex);

    QString service_id = registration.service_id;
    if (service_id.isEmpty()) {
        service_id = d->generate_service_id();
    }

    ServiceRegistration reg = registration;
    reg.service_id = service_id;
    reg.registration_time = std::chrono::system_clock::now();

    d->registered_services[service_id] = reg;
    d->last_heartbeat[service_id] = std::chrono::system_clock::now();
    d->update_service_availability(service_id, ServiceAvailability::Available);

    locker.unlock();

    // Send network announcement if in network mode
    d->send_network_announcement(reg);

    qCDebug(serviceDiscoveryLog) << "Registered service:" << service_id
                                 << "name:" << registration.service_name;

    emit service_registered(reg);

    return {};
}

qtplugin::expected<void, PluginError>
PluginServiceDiscovery::unregister_service(const QString& service_id) {
    if (service_id.isEmpty()) {
        return qtplugin::unexpected(PluginError(
            PluginErrorCode::InvalidArgument, "Service ID is required"));
    }

    QMutexLocker locker(&d->services_mutex);

    auto it = d->registered_services.find(service_id);
    if (it == d->registered_services.end()) {
        return qtplugin::unexpected(
            PluginError(PluginErrorCode::NotFound, "Service not found"));
    }

    d->registered_services.erase(it);
    d->service_availability.erase(service_id);
    d->last_heartbeat.erase(service_id);

    qCDebug(serviceDiscoveryLog) << "Unregistered service:" << service_id;

    emit service_unregistered(service_id);

    return {};
}

qtplugin::expected<ServiceDiscoveryResult, PluginError>
PluginServiceDiscovery::discover_services(const ServiceDiscoveryQuery& query) {
    std::vector<ServiceRegistration> results;

    QMutexLocker locker(&d->services_mutex);

    for (const auto& [service_id, registration] : d->registered_services) {
        // Apply filters
        if (!query.service_name.isEmpty() &&
            registration.service_name != query.service_name) {
            continue;
        }

        if (!query.service_version.isEmpty() &&
            registration.service_version != query.service_version) {
            continue;
        }

        // Check availability
        auto availability_it = d->service_availability.find(service_id);
        if (availability_it != d->service_availability.end() &&
            availability_it->second != ServiceAvailability::Available) {
            continue;
        }

        results.push_back(registration);

        if (query.max_results > 0 &&
            results.size() >= static_cast<size_t>(query.max_results)) {
            break;
        }
    }

    ServiceDiscoveryResult result;
    result.services = results;
    result.total_found = results.size();
    result.discovery_time =
        std::chrono::milliseconds(10);  // Placeholder timing
    result.discovery_source = "local";

    return result;
}

qtplugin::expected<ServiceRegistration, PluginError>
PluginServiceDiscovery::get_service_registration(
    const QString& service_id) const {
    if (service_id.isEmpty()) {
        return qtplugin::unexpected(PluginError(
            PluginErrorCode::InvalidArgument, "Service ID is required"));
    }

    QMutexLocker locker(&d->services_mutex);

    auto it = d->registered_services.find(service_id);
    if (it == d->registered_services.end()) {
        return qtplugin::unexpected(
            PluginError(PluginErrorCode::NotFound, "Service not found"));
    }

    return it->second;
}

// Additional methods would be implemented here based on header declarations

// Private slots implementation
void PluginServiceDiscovery::on_heartbeat_timer() {
    QMutexLocker locker(&d->services_mutex);

    // Send heartbeat for all registered services
    for (const auto& [service_id, registration] : d->registered_services) {
        d->send_network_announcement(registration);
    }
}

void PluginServiceDiscovery::on_health_check_timer() {
    QMutexLocker locker(&d->services_mutex);

    std::vector<QString> unhealthy_services;

    for (const auto& [service_id, registration] : d->registered_services) {
        if (!d->is_service_healthy(service_id)) {
            unhealthy_services.push_back(service_id);
        }
    }

    locker.unlock();

    // Update availability for unhealthy services
    for (const QString& service_id : unhealthy_services) {
        d->update_service_availability(service_id,
                                       ServiceAvailability::Unavailable);
    }
}

void PluginServiceDiscovery::on_network_discovery_timer() {
    if (!d->auto_discovery_enabled) {
        return;
    }

    // Send discovery query
    QJsonObject query;
    query["type"] = "discovery_query";
    query["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(query);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    d->udp_socket->writeDatagram(data, QHostAddress(d->multicast_group),
                                 d->discovery_port);
}

void PluginServiceDiscovery::on_network_data_received() {
    while (d->udp_socket->hasPendingDatagrams()) {
        QByteArray data;
        data.resize(d->udp_socket->pendingDatagramSize());

        QHostAddress sender;
        quint16 sender_port;

        d->udp_socket->readDatagram(data.data(), data.size(), &sender,
                                    &sender_port);

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);

        if (error.error != QJsonParseError::NoError) {
            qCWarning(serviceDiscoveryLog)
                << "Failed to parse network data:" << error.errorString();
            continue;
        }

        QJsonObject message = doc.object();
        QString type = message["type"].toString();

        if (type == "service_announcement") {
            d->process_network_announcement(message);
        } else if (type == "discovery_query") {
            // Respond with our services
            QMutexLocker locker(&d->services_mutex);
            for (const auto& [service_id, registration] :
                 d->registered_services) {
                d->send_network_announcement(registration);
            }
        }
    }
}

#else  // !QTFORGE_HAS_NETWORK

// Stub implementation when network support is disabled
PluginServiceDiscovery::PluginServiceDiscovery(QObject* parent)
    : QObject(parent) {
    qCWarning(serviceDiscoveryLog)
        << "Network support disabled - service discovery limited to local mode";
}

PluginServiceDiscovery::~PluginServiceDiscovery() = default;

void PluginServiceDiscovery::set_discovery_mode(ServiceDiscoveryMode) {
    qCWarning(serviceDiscoveryLog)
        << "Network discovery not available - network support disabled";
}

ServiceDiscoveryMode PluginServiceDiscovery::discovery_mode() const {
    return ServiceDiscoveryMode::Local;
}

qtplugin::expected<QString, PluginError>
PluginServiceDiscovery::register_service(const ServiceRegistration&) {
    return qtplugin::unexpected(PluginError(
        PluginErrorCode::NotSupported,
        "Service discovery not available - network support disabled"));
}

qtplugin::expected<void, PluginError>
PluginServiceDiscovery::unregister_service(const QString&) {
    return qtplugin::unexpected(PluginError(
        PluginErrorCode::NotSupported,
        "Service discovery not available - network support disabled"));
}

std::vector<ServiceRegistration> PluginServiceDiscovery::discover_services(
    const ServiceDiscoveryQuery&) {
    return {};
}

qtplugin::expected<ServiceRegistration, PluginError>
PluginServiceDiscovery::get_service(const QString&) {
    return qtplugin::unexpected(PluginError(
        PluginErrorCode::NotSupported,
        "Service discovery not available - network support disabled"));
}

#endif  // QTFORGE_HAS_NETWORK

}  // namespace qtplugin
