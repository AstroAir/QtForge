#include "../../include/qtplugin/interfaces/core/advanced_plugin_interface.hpp"
#include <QDateTime>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include "../../include/qtplugin/utils/error_handling.hpp"

namespace qtplugin {

// AdvancedPluginBase implementation

qtplugin::expected<void, PluginError> AdvancedPluginBase::apply_configuration(
    const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Invalid configuration provided");
    }

    m_configuration = config;
    return make_success();
}

QJsonObject AdvancedPluginBase::get_configuration() const {
    return m_configuration;
}

bool AdvancedPluginBase::validate_configuration(
    const QJsonObject& config) const {
    Q_UNUSED(config)
    // Default implementation accepts any configuration
    // Derived classes should override this for specific validation
    return true;
}

qtplugin::expected<QVariant, PluginError> AdvancedPluginBase::handle_event(
    const QString& event_type, const QVariant& event_data) {
    Q_UNUSED(event_data)

    // Default implementation returns error for unsupported events
    return make_error<QVariant>(
        PluginErrorCode::CommandNotFound,
        "Event type not supported: " + event_type.toStdString());
}

QStringList AdvancedPluginBase::supported_event_types() const {
    // Default implementation supports no events
    // Derived classes should override this
    return QStringList();
}

qtplugin::expected<void, PluginError> AdvancedPluginBase::pre_initialize() {
    // Default implementation does nothing
    return make_success();
}

qtplugin::expected<void, PluginError> AdvancedPluginBase::post_initialize() {
    // Default implementation does nothing
    return make_success();
}

qtplugin::expected<void, PluginError> AdvancedPluginBase::pre_shutdown() {
    // Default implementation does nothing
    return make_success();
}

qtplugin::expected<void, PluginError> AdvancedPluginBase::post_shutdown() {
    // Default implementation does nothing
    return make_success();
}

QJsonObject AdvancedPluginBase::get_resource_usage() const {
    QJsonObject usage;
    usage["memory_usage"] = 0;  // Would need actual implementation
    usage["cpu_usage"] = 0.0;
    usage["thread_count"] = 1;
    usage["file_handles"] = 0;
    usage["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    return usage;
}

qtplugin::expected<void, PluginError> AdvancedPluginBase::set_resource_limits(
    const QJsonObject& limits) {
    m_resource_limits = limits;
    // Default implementation just stores the limits
    // Derived classes should implement actual enforcement
    return make_success();
}

qtplugin::expected<QVariant, PluginError> AdvancedPluginBase::send_message(
    const QString& target_plugin_id, const QVariant& message) {
    Q_UNUSED(target_plugin_id)
    Q_UNUSED(message)

    // Default implementation returns error - requires plugin manager
    // integration
    return make_error<QVariant>(
        PluginErrorCode::NotImplemented,
        "Message sending not implemented in base class");
}

qtplugin::expected<QVariant, PluginError> AdvancedPluginBase::receive_message(
    const QString& sender_plugin_id, const QVariant& message) {
    Q_UNUSED(sender_plugin_id)
    Q_UNUSED(message)

    // Default implementation returns error
    return make_error<QVariant>(PluginErrorCode::NotImplemented,
                                "Message receiving not implemented");
}

qtplugin::expected<QJsonObject, PluginError> AdvancedPluginBase::save_state() {
    QJsonObject state;
    state["configuration"] = m_configuration;
    state["capabilities"] = QJsonArray::fromStringList(m_capabilities);
    state["provided_services"] =
        QJsonArray::fromStringList(m_provided_services);
    state["resource_limits"] = m_resource_limits;
    state["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    return state;
}

qtplugin::expected<void, PluginError> AdvancedPluginBase::restore_state(
    const QJsonObject& state) {
    if (state.contains("configuration")) {
        m_configuration = state["configuration"].toObject();
    }

    if (state.contains("capabilities")) {
        QJsonArray caps = state["capabilities"].toArray();
        m_capabilities.clear();
        for (const auto& cap : caps) {
            m_capabilities.append(cap.toString());
        }
    }

    if (state.contains("provided_services")) {
        QJsonArray services = state["provided_services"].toArray();
        m_provided_services.clear();
        for (const auto& service : services) {
            m_provided_services.append(service.toString());
        }
    }

    if (state.contains("resource_limits")) {
        m_resource_limits = state["resource_limits"].toObject();
    }

    return make_success();
}

QStringList AdvancedPluginBase::get_capabilities() const {
    return m_capabilities;
}

bool AdvancedPluginBase::has_capability(const QString& capability) const {
    return m_capabilities.contains(capability);
}

qtplugin::expected<void, PluginError> AdvancedPluginBase::register_service(
    const QString& service_name, const QVariant& service_interface) {
    Q_UNUSED(service_interface)

    if (service_name.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Service name cannot be empty");
    }

    if (!m_provided_services.contains(service_name)) {
        m_provided_services.append(service_name);
    }

    return make_success();
}

qtplugin::expected<void, PluginError> AdvancedPluginBase::unregister_service(
    const QString& service_name) {
    if (service_name.isEmpty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Service name cannot be empty");
    }

    m_provided_services.removeAll(service_name);
    return make_success();
}

QStringList AdvancedPluginBase::get_provided_services() const {
    return m_provided_services;
}

QJsonObject AdvancedPluginBase::get_performance_metrics() const {
    if (m_performance_metrics.isEmpty()) {
        // Initialize with default metrics
        QJsonObject metrics;
        metrics["initialization_time"] = 0;
        metrics["total_commands_executed"] = 0;
        metrics["average_command_time"] = 0.0;
        metrics["error_count"] = 0;
        metrics["uptime_seconds"] = 0;
        metrics["last_activity"] =
            QDateTime::currentDateTime().toString(Qt::ISODate);

        // Cast away const to update metrics
        const_cast<AdvancedPluginBase*>(this)->m_performance_metrics = metrics;
    }

    return m_performance_metrics;
}

void AdvancedPluginBase::reset_metrics() {
    m_performance_metrics = QJsonObject();
}

qtplugin::expected<bool, PluginError> AdvancedPluginBase::validate_integrity()
    const {
    // Default implementation performs basic validation

    // Check if plugin has valid metadata
    auto meta = metadata();
    if (meta.name.empty() || meta.version.to_string().empty()) {
        return make_error<bool>(PluginErrorCode::InitializationFailed,
                                "Plugin metadata is incomplete");
    }

    // Check if plugin is in a valid state
    if (!is_initialized()) {
        return make_error<bool>(PluginErrorCode::InitializationFailed,
                                "Plugin is not properly initialized");
    }

    return true;
}

QJsonObject AdvancedPluginBase::get_health_status() const {
    QJsonObject health;
    health["status"] = is_initialized() ? "healthy" : "unhealthy";
    health["initialized"] = is_initialized();
    health["error_count"] = m_performance_metrics.value("error_count").toInt(0);
    health["last_check"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    // Add resource usage to health status
    QJsonObject resources = get_resource_usage();
    health["resource_usage"] = resources;

    // Determine overall health score (0-100)
    int health_score = 100;
    if (!is_initialized()) {
        health_score -= 50;
    }

    int error_count = m_performance_metrics.value("error_count").toInt(0);
    if (error_count > 0) {
        health_score -= std::min(30, error_count * 5);
    }

    health["health_score"] = health_score;
    health["health_level"] = health_score >= 80   ? "good"
                             : health_score >= 50 ? "warning"
                                                  : "critical";

    return health;
}

}  // namespace qtplugin
