/**
 * @file request_response_plugin.cpp
 * @brief Request-Response communication pattern example plugin implementation
 * @version 1.0.0
 */

#include "request_response_plugin.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QRandomGenerator>

namespace qtplugin::examples {

RequestResponsePlugin::RequestResponsePlugin(QObject* parent)
    : QObject(parent)
    , m_stats_start_time(QDateTime::currentDateTime())
{
    qDebug() << "RequestResponsePlugin: Constructed";
}

RequestResponsePlugin::~RequestResponsePlugin() {
    if (m_state.load() != qtplugin::PluginState::Unloaded) {
        shutdown();
    }
    qDebug() << "RequestResponsePlugin: Destroyed";
}

bool RequestResponsePlugin::initialize(const QJsonObject& config) {
    QMutexLocker locker(&m_requests_mutex);
    
    if (m_state.load() != qtplugin::PluginState::Unloaded) {
        qWarning() << "RequestResponsePlugin: Already initialized";
        return false;
    }
    
    qDebug() << "RequestResponsePlugin: Initializing...";
    m_state.store(qtplugin::PluginState::Loading);
    
    try {
        // Apply configuration
        if (config.contains("default_timeout_ms")) {
            m_default_timeout_ms = config["default_timeout_ms"].toInt();
        }
        if (config.contains("max_pending_requests")) {
            m_max_pending_requests = config["max_pending_requests"].toInt();
        }
        if (config.contains("max_retry_count")) {
            m_max_retry_count = config["max_retry_count"].toInt();
        }
        if (config.contains("enable_request_queuing")) {
            m_enable_request_queuing = config["enable_request_queuing"].toBool();
        }
        
        // Initialize message bus
        m_message_bus = std::make_unique<qtplugin::MessageBus>();
        
        // Setup cleanup timer
        m_cleanup_timer = std::make_unique<QTimer>();
        m_cleanup_timer->setInterval(5000); // 5 seconds
        connect(m_cleanup_timer.get(), &QTimer::timeout, this, &RequestResponsePlugin::cleanup_expired_requests);
        m_cleanup_timer->start();
        
        // Setup queue processor
        if (m_enable_request_queuing) {
            m_queue_processor = std::make_unique<QTimer>();
            m_queue_processor->setInterval(100); // 100ms
            connect(m_queue_processor.get(), &QTimer::timeout, this, &RequestResponsePlugin::process_request_queue);
            m_queue_processor->start();
        }
        
        // Reset statistics
        m_stats = RequestStats{};
        m_stats_start_time = QDateTime::currentDateTime();
        
        m_state.store(qtplugin::PluginState::Initialized);
        qDebug() << "RequestResponsePlugin: Initialized successfully!";
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "RequestResponsePlugin: Initialization failed:" << e.what();
        m_state.store(qtplugin::PluginState::Error);
        return false;
    }
}

void RequestResponsePlugin::shutdown() {
    QMutexLocker locker(&m_requests_mutex);
    
    if (m_state.load() == qtplugin::PluginState::Unloaded) {
        return;
    }
    
    qDebug() << "RequestResponsePlugin: Shutting down...";
    
    // Stop timers
    if (m_cleanup_timer) {
        m_cleanup_timer->stop();
        m_cleanup_timer.reset();
    }
    if (m_queue_processor) {
        m_queue_processor->stop();
        m_queue_processor.reset();
    }
    
    // Cancel all pending requests
    for (const auto& [id, request] : m_pending_requests) {
        qDebug() << "RequestResponsePlugin: Cancelling pending request:" << id;
    }
    m_pending_requests.clear();
    
    // Cleanup message bus
    m_message_bus.reset();
    
    m_state.store(qtplugin::PluginState::Unloaded);
    qDebug() << "RequestResponsePlugin: Shutdown complete.";
}

QJsonObject RequestResponsePlugin::metadata() const {
    QMutexLocker locker(&m_requests_mutex);
    
    return QJsonObject{
        {"name", name()},
        {"version", version()},
        {"description", description()},
        {"state", static_cast<int>(state())},
        {"configuration", QJsonObject{
            {"default_timeout_ms", m_default_timeout_ms},
            {"max_pending_requests", m_max_pending_requests},
            {"max_retry_count", m_max_retry_count},
            {"enable_request_queuing", m_enable_request_queuing}
        }},
        {"statistics", QJsonObject{
            {"total_requests", m_stats.total_requests.load()},
            {"successful_responses", m_stats.successful_responses.load()},
            {"failed_responses", m_stats.failed_responses.load()},
            {"timeout_responses", m_stats.timeout_responses.load()},
            {"pending_requests", static_cast<int>(m_pending_requests.size())},
            {"average_response_time", m_stats.average_response_time.load()},
            {"uptime_seconds", m_stats_start_time.secsTo(QDateTime::currentDateTime())}
        }},
        {"commands", QJsonArray{"send_request", "get_statistics", "clear_statistics", "list_pending", "cancel_request"}}
    };
}

QJsonObject RequestResponsePlugin::execute_command(const QString& command, const QJsonObject& params) {
    if (m_state.load() != qtplugin::PluginState::Initialized) {
        return QJsonObject{{"error", "Plugin not initialized"}};
    }
    
    if (command == "send_request") {
        return execute_send_request_command(params);
    } else if (command == "get_statistics") {
        return execute_get_statistics_command(params);
    } else if (command == "clear_statistics") {
        return execute_clear_statistics_command(params);
    } else if (command == "list_pending") {
        return execute_list_pending_command(params);
    } else if (command == "cancel_request") {
        return execute_cancel_request_command(params);
    }
    
    return QJsonObject{{"error", "Unknown command: " + command}};
}

QString RequestResponsePlugin::send_request(const QJsonObject& request, const QString& target, bool async, int priority) {
    QString request_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    
    PendingRequest pending_request;
    pending_request.id = request_id;
    pending_request.request = request;
    pending_request.timestamp = QDateTime::currentDateTime();
    pending_request.timeout = pending_request.timestamp.addMSecs(m_default_timeout_ms);
    pending_request.sender = name();
    pending_request.priority = priority;
    pending_request.retry_count = 0;
    pending_request.is_async = async;
    
    add_pending_request(pending_request);
    
    // Simulate sending request via message bus
    qDebug() << "RequestResponsePlugin: Sending request" << request_id << "to" << target;
    
    // For demonstration, simulate some requests with responses
    if (QRandomGenerator::global()->bounded(100) < 80) { // 80% success rate
        QTimer::singleShot(QRandomGenerator::global()->bounded(1000, 5000), [this, request_id]() {
            QJsonObject response{
                {"status", "success"},
                {"data", "Sample response data"},
                {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
            };
            handle_response(request_id, response);
        });
    } else {
        // Simulate timeout
        QTimer::singleShot(m_default_timeout_ms + 1000, [this, request_id]() {
            on_request_timeout(request_id);
        });
    }
    
    m_stats.total_requests.fetch_add(1);
    return request_id;
}

void RequestResponsePlugin::handle_response(const QString& request_id, const QJsonObject& response) {
    QMutexLocker locker(&m_requests_mutex);
    
    auto* pending = find_pending_request(request_id);
    if (!pending) {
        qWarning() << "RequestResponsePlugin: Received response for unknown request:" << request_id;
        return;
    }
    
    double response_time = pending->timestamp.msecsTo(QDateTime::currentDateTime());
    bool success = response["status"].toString() == "success";
    
    update_statistics(request_id, success, response_time);
    
    qDebug() << "RequestResponsePlugin: Received response for request" << request_id 
             << "in" << response_time << "ms, success:" << success;
    
    remove_pending_request(request_id);
}

void RequestResponsePlugin::add_pending_request(const PendingRequest& request) {
    QMutexLocker locker(&m_requests_mutex);
    
    if (m_pending_requests.size() >= static_cast<size_t>(m_max_pending_requests)) {
        qWarning() << "RequestResponsePlugin: Maximum pending requests reached, dropping oldest";
        // Remove oldest request
        auto oldest = std::min_element(m_pending_requests.begin(), m_pending_requests.end(),
            [](const auto& a, const auto& b) {
                return a.second.timestamp < b.second.timestamp;
            });
        if (oldest != m_pending_requests.end()) {
            m_pending_requests.erase(oldest);
        }
    }
    
    m_pending_requests[request.id] = request;
    m_stats.pending_requests.store(static_cast<int>(m_pending_requests.size()));
}

void RequestResponsePlugin::remove_pending_request(const QString& request_id) {
    m_pending_requests.erase(request_id);
    m_stats.pending_requests.store(static_cast<int>(m_pending_requests.size()));
}

RequestResponsePlugin::PendingRequest* RequestResponsePlugin::find_pending_request(const QString& request_id) {
    auto it = m_pending_requests.find(request_id);
    return (it != m_pending_requests.end()) ? &it->second : nullptr;
}

void RequestResponsePlugin::update_statistics(const QString& request_id, bool success, double response_time) {
    Q_UNUSED(request_id)
    
    if (success) {
        m_stats.successful_responses.fetch_add(1);
    } else {
        m_stats.failed_responses.fetch_add(1);
    }
    
    // Update average response time (simple moving average)
    double current_avg = m_stats.average_response_time.load();
    int total_responses = m_stats.successful_responses.load() + m_stats.failed_responses.load();
    double new_avg = ((current_avg * (total_responses - 1)) + response_time) / total_responses;
    m_stats.average_response_time.store(new_avg);
}

QJsonObject RequestResponsePlugin::execute_send_request_command(const QJsonObject& params) {
    QString target = params["target"].toString();
    QJsonObject request = params["request"].toObject();
    bool async = params["async"].toBool(true);
    int priority = params["priority"].toInt(0);
    
    if (target.isEmpty() || request.isEmpty()) {
        return QJsonObject{{"error", "Target and request are required"}};
    }
    
    QString request_id = send_request(request, target, async, priority);
    
    return QJsonObject{
        {"success", true},
        {"request_id", request_id},
        {"target", target},
        {"async", async},
        {"priority", priority},
        {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };
}

QJsonObject RequestResponsePlugin::execute_get_statistics_command(const QJsonObject& params) {
    Q_UNUSED(params)
    
    QMutexLocker locker(&m_requests_mutex);
    
    return QJsonObject{
        {"statistics", QJsonObject{
            {"total_requests", m_stats.total_requests.load()},
            {"successful_responses", m_stats.successful_responses.load()},
            {"failed_responses", m_stats.failed_responses.load()},
            {"timeout_responses", m_stats.timeout_responses.load()},
            {"pending_requests", static_cast<int>(m_pending_requests.size())},
            {"average_response_time_ms", m_stats.average_response_time.load()},
            {"uptime_seconds", m_stats_start_time.secsTo(QDateTime::currentDateTime())},
            {"success_rate", m_stats.total_requests.load() > 0 ? 
                (static_cast<double>(m_stats.successful_responses.load()) / m_stats.total_requests.load()) * 100.0 : 0.0}
        }},
        {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };
}

QJsonObject RequestResponsePlugin::execute_clear_statistics_command(const QJsonObject& params) {
    Q_UNUSED(params)
    
    m_stats = RequestStats{};
    m_stats_start_time = QDateTime::currentDateTime();
    
    return QJsonObject{
        {"success", true},
        {"message", "Statistics cleared"},
        {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };
}

QJsonObject RequestResponsePlugin::execute_list_pending_command(const QJsonObject& params) {
    Q_UNUSED(params)
    
    QMutexLocker locker(&m_requests_mutex);
    
    QJsonArray pending_array;
    for (const auto& [id, request] : m_pending_requests) {
        pending_array.append(QJsonObject{
            {"id", id},
            {"timestamp", request.timestamp.toString(Qt::ISODate)},
            {"timeout", request.timeout.toString(Qt::ISODate)},
            {"priority", request.priority},
            {"retry_count", request.retry_count},
            {"is_async", request.is_async},
            {"age_ms", request.timestamp.msecsTo(QDateTime::currentDateTime())}
        });
    }
    
    return QJsonObject{
        {"pending_requests", pending_array},
        {"count", static_cast<int>(m_pending_requests.size())},
        {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
    };
}

QJsonObject RequestResponsePlugin::execute_cancel_request_command(const QJsonObject& params) {
    QString request_id = params["request_id"].toString();
    
    if (request_id.isEmpty()) {
        return QJsonObject{{"error", "Request ID is required"}};
    }
    
    QMutexLocker locker(&m_requests_mutex);
    
    if (m_pending_requests.find(request_id) != m_pending_requests.end()) {
        remove_pending_request(request_id);
        return QJsonObject{
            {"success", true},
            {"message", "Request cancelled"},
            {"request_id", request_id},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    } else {
        return QJsonObject{{"error", "Request not found: " + request_id}};
    }
}

void RequestResponsePlugin::on_request_received(const QString& request_id, const QJsonObject& request) {
    qDebug() << "RequestResponsePlugin: Received request:" << request_id;
    handle_request(request_id, request);
}

void RequestResponsePlugin::on_response_received(const QString& request_id, const QJsonObject& response) {
    qDebug() << "RequestResponsePlugin: Received response:" << request_id;
    handle_response(request_id, response);
}

void RequestResponsePlugin::on_request_timeout(const QString& request_id) {
    QMutexLocker locker(&m_requests_mutex);
    
    qWarning() << "RequestResponsePlugin: Request timeout:" << request_id;
    
    if (find_pending_request(request_id)) {
        m_stats.timeout_responses.fetch_add(1);
        remove_pending_request(request_id);
    }
}

void RequestResponsePlugin::cleanup_expired_requests() {
    QMutexLocker locker(&m_requests_mutex);
    
    QDateTime now = QDateTime::currentDateTime();
    auto it = m_pending_requests.begin();
    
    while (it != m_pending_requests.end()) {
        if (now > it->second.timeout) {
            qDebug() << "RequestResponsePlugin: Cleaning up expired request:" << it->first;
            m_stats.timeout_responses.fetch_add(1);
            it = m_pending_requests.erase(it);
        } else {
            ++it;
        }
    }
    
    m_stats.pending_requests.store(static_cast<int>(m_pending_requests.size()));
}

void RequestResponsePlugin::process_request_queue() {
    // Implementation for request queue processing
    // This would handle prioritization and batching
    qDebug() << "RequestResponsePlugin: Processing request queue...";
}

void RequestResponsePlugin::handle_request(const QString& request_id, const QJsonObject& request) {
    // Implementation for handling incoming requests
    Q_UNUSED(request_id)
    Q_UNUSED(request)
    qDebug() << "RequestResponsePlugin: Handling request...";
}

void RequestResponsePlugin::send_response(const QString& request_id, const QJsonObject& response, bool success) {
    Q_UNUSED(request_id)
    Q_UNUSED(response)
    Q_UNUSED(success)
    qDebug() << "RequestResponsePlugin: Sending response...";
}

} // namespace qtplugin::examples
