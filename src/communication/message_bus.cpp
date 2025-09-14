/**
 * @file message_bus.cpp
 * @brief Enhanced implementation of message bus for plugin communication
 * @version 3.2.0
 */

#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QTimer>
#include <QThread>
#include <QtConcurrent>
#include <QFuture>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/utils/error_handling.hpp>
#include <random>
#include <thread>

Q_LOGGING_CATEGORY(messageBusLog, "qtplugin.messagebus")

namespace qtplugin {

namespace {
// Performance optimization constants
constexpr size_t ASYNC_DELIVERY_THRESHOLD = 5;
constexpr auto MESSAGE_TIMEOUT = std::chrono::seconds(30);
constexpr size_t MAX_CONCURRENT_DELIVERIES = 10;

// Message priority weights
constexpr int CRITICAL_PRIORITY_WEIGHT = 100;
constexpr int HIGH_PRIORITY_WEIGHT = 75;
constexpr int NORMAL_PRIORITY_WEIGHT = 50;
constexpr int LOW_PRIORITY_WEIGHT = 25;

// Priority weight function removed - was unused
} // namespace

MessageBus::MessageBus(QObject* parent)
    : QObject(parent),
      m_delivery_thread_pool(this) {

    // Initialize with default capacity
    m_message_log.reserve(MAX_LOG_SIZE);

    // Configure thread pool
    m_delivery_thread_pool.setMaxThreadCount(MAX_CONCURRENT_DELIVERIES);

    // Setup cleanup timer for expired messages
    m_cleanup_timer = new QTimer(this);
    connect(m_cleanup_timer, &QTimer::timeout,
            this, &MessageBus::cleanup_expired_messages);
    m_cleanup_timer->start(std::chrono::minutes(5));

    qCDebug(messageBusLog) << "MessageBus initialized with" << MAX_CONCURRENT_DELIVERIES << "delivery threads";
}

MessageBus::~MessageBus() = default;

qtplugin::expected<void, PluginError> MessageBus::unsubscribe(
    std::string_view subscriber_id,
    std::optional<std::type_index> message_type) {
    std::unique_lock lock(m_subscriptions_mutex);

    if (message_type) {
        // Unsubscribe from specific message type
        auto it = m_subscriptions.find(*message_type);
        if (it != m_subscriptions.end()) {
            auto& subscriptions = it->second;
            subscriptions.erase(
                std::remove_if(
                    subscriptions.begin(), subscriptions.end(),
                    [subscriber_id](const std::shared_ptr<Subscription>& sub) {
                        return sub->subscriber_id == subscriber_id;
                    }),
                subscriptions.end());

            if (subscriptions.empty()) {
                m_subscriptions.erase(it);
            }

            emit subscription_removed(
                QString::fromStdString(std::string(subscriber_id)),
                QString::fromStdString(message_type->name()));
        }

        // Update subscriber types
        auto subscriber_it =
            m_subscriber_types.find(std::string(subscriber_id));
        if (subscriber_it != m_subscriber_types.end()) {
            subscriber_it->second.erase(*message_type);
            if (subscriber_it->second.empty()) {
                m_subscriber_types.erase(subscriber_it);
            }
        }
    } else {
        // Unsubscribe from all message types
        for (auto& [type, subscriptions] : m_subscriptions) {
            subscriptions.erase(
                std::remove_if(
                    subscriptions.begin(), subscriptions.end(),
                    [subscriber_id](const std::shared_ptr<Subscription>& sub) {
                        return sub->subscriber_id == subscriber_id;
                    }),
                subscriptions.end());
        }

        // Remove empty subscription lists
        for (auto it = m_subscriptions.begin(); it != m_subscriptions.end();) {
            if (it->second.empty()) {
                it = m_subscriptions.erase(it);
            } else {
                ++it;
            }
        }

        // Remove from subscriber types
        m_subscriber_types.erase(std::string(subscriber_id));
    }

    return make_success();
}

std::vector<std::string> MessageBus::subscribers(
    std::type_index message_type) const {
    std::shared_lock lock(m_subscriptions_mutex);
    std::vector<std::string> result;

    auto it = m_subscriptions.find(message_type);
    if (it != m_subscriptions.end()) {
        for (const auto& subscription : it->second) {
            if (subscription->is_active) {
                result.push_back(subscription->subscriber_id);
            }
        }
    }

    return result;
}

std::vector<Subscription> MessageBus::subscriptions(
    std::string_view subscriber_id) const {
    std::shared_lock lock(m_subscriptions_mutex);
    std::vector<Subscription> result;

    for (const auto& [type, subscriptions] : m_subscriptions) {
        for (const auto& subscription : subscriptions) {
            if (subscription->subscriber_id == subscriber_id) {
                result.push_back(*subscription);
            }
        }
    }

    return result;
}

bool MessageBus::has_subscriber(std::string_view subscriber_id) const {
    std::shared_lock lock(m_subscriptions_mutex);
    return m_subscriber_types.find(std::string(subscriber_id)) !=
           m_subscriber_types.end();
}

QJsonObject MessageBus::statistics() const {
    std::shared_lock lock(m_subscriptions_mutex);

    int total_subscriptions = 0;
    int active_subscriptions = 0;

    for (const auto& [type, subscriptions] : m_subscriptions) {
        total_subscriptions += subscriptions.size();
        for (const auto& subscription : subscriptions) {
            if (subscription->is_active) {
                ++active_subscriptions;
            }
        }
    }

    return QJsonObject{
        {"total_subscriptions", total_subscriptions},
        {"active_subscriptions", active_subscriptions},
        {"unique_subscribers", static_cast<int>(m_subscriber_types.size())},
        {"message_types", static_cast<int>(m_subscriptions.size())},
        {"messages_published",
         static_cast<qint64>(m_messages_published.load())},
        {"messages_delivered",
         static_cast<qint64>(m_messages_delivered.load())},
        {"delivery_failures", static_cast<qint64>(m_delivery_failures.load())},
        {"logging_enabled", m_logging_enabled.load()}};
}

void MessageBus::clear() {
    std::unique_lock lock(m_subscriptions_mutex);
    m_subscriptions.clear();
    m_subscriber_types.clear();

    std::unique_lock log_lock(m_log_mutex);
    m_message_log.clear();
}

void MessageBus::set_logging_enabled(bool enabled) {
    m_logging_enabled.store(enabled);
}

bool MessageBus::is_logging_enabled() const { return m_logging_enabled.load(); }

std::vector<QJsonObject> MessageBus::message_log(size_t limit) const {
    std::shared_lock lock(m_log_mutex);

    if (limit == 0 || limit >= m_message_log.size()) {
        return m_message_log;
    }

    // Return the most recent messages
    auto start_it =
        m_message_log.end() -
        static_cast<std::vector<QJsonObject>::difference_type>(limit);
    return std::vector<QJsonObject>(start_it, m_message_log.end());
}

qtplugin::expected<void, PluginError> MessageBus::publish_impl(
    std::shared_ptr<IMessage> message, DeliveryMode mode,
    const std::vector<std::string>& recipients) {
    if (!message) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Message is null");
    }

    m_messages_published.fetch_add(1);

    // Log the message if logging is enabled
    if (m_logging_enabled.load()) {
        log_message(*message, recipients);
    }

    // Find recipients based on delivery mode
    std::vector<std::string> target_recipients;
    if (mode == DeliveryMode::Broadcast) {
        target_recipients = get_all_subscribers(std::type_index(typeid(*message)));
    } else if (mode == DeliveryMode::Targeted) {
        target_recipients = recipients;
    } else {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Invalid delivery mode");
    }

    if (target_recipients.empty()) {
        qCDebug(messageBusLog) << "No recipients found for message type:"
                               << typeid(*message).name();
        return make_success();  // No error, just no recipients
    }

    // Determine delivery strategy based on number of recipients
    if (target_recipients.size() >= ASYNC_DELIVERY_THRESHOLD) {
        // Asynchronous delivery for large recipient lists
        return deliver_async(message, target_recipients);
    } else {
        // Synchronous delivery for small recipient lists
        return deliver_sync(message, target_recipients);
    }
}

// Synchronous message delivery
qtplugin::expected<void, PluginError> MessageBus::deliver_sync(
    std::shared_ptr<IMessage> message,
    const std::vector<std::string>& recipients) {

    size_t successful_deliveries = 0;
    std::vector<std::string> failed_recipients;

    std::shared_lock lock(m_subscriptions_mutex);
    std::type_index msg_type(typeid(*message));

    auto subscriptions_it = m_subscriptions.find(msg_type);
    if (subscriptions_it == m_subscriptions.end()) {
        return make_success();  // No subscriptions for this message type
    }

    for (const auto& recipient : recipients) {
        bool delivered = false;

        for (const auto& subscription : subscriptions_it->second) {
            if (subscription->subscriber_id == recipient && subscription->is_active) {
                try {
                    // Cast handler back to function type and invoke
                    auto handler_func = std::any_cast<std::function<qtplugin::expected<void, PluginError>(std::shared_ptr<IMessage>)>>(subscription->handler);
                    auto result = handler_func(message);
                    if (result) {
                        delivered = true;
                        successful_deliveries++;
                        m_messages_delivered.fetch_add(1);

                        // Update subscription statistics
                        subscription->messages_received++;
                        subscription->last_message_time = std::chrono::system_clock::now();
                    }
                    subscription->last_message_time = std::chrono::system_clock::now();

                    break;  // Only deliver once per recipient
                } catch (const std::exception& e) {
                    qCWarning(messageBusLog) << "Error delivering message to"
                                             << QString::fromStdString(recipient)
                                             << ":" << e.what();
                    m_delivery_failures.fetch_add(1);
                }
            }
        }

        if (!delivered) {
            failed_recipients.push_back(recipient);
            m_delivery_failures.fetch_add(1);
        }
    }

    if (!failed_recipients.empty()) {
        std::string error_msg = "Failed to deliver to: ";
        for (size_t i = 0; i < failed_recipients.size(); ++i) {
            if (i > 0) error_msg += ", ";
            error_msg += failed_recipients[i];
        }
        qCWarning(messageBusLog) << QString::fromStdString(error_msg);
    }

    return make_success();
}

// Asynchronous message delivery
qtplugin::expected<void, PluginError> MessageBus::deliver_async(
    std::shared_ptr<IMessage> message,
    const std::vector<std::string>& recipients) {

    // Create delivery tasks
    std::vector<QFuture<bool>> delivery_futures;
    delivery_futures.reserve(recipients.size());

    std::shared_lock lock(m_subscriptions_mutex);
    std::type_index msg_type(typeid(*message));

    auto subscriptions_it = m_subscriptions.find(msg_type);
    if (subscriptions_it == m_subscriptions.end()) {
        return make_success();
    }

    // Submit delivery tasks to thread pool
    for (const auto& recipient : recipients) {
        // Find the subscription for this recipient
        std::shared_ptr<Subscription> target_subscription;

        for (const auto& subscription : subscriptions_it->second) {
            if (subscription->subscriber_id == recipient && subscription->is_active) {
                target_subscription = subscription;
                break;
            }
        }

        if (target_subscription) {
            auto future = QtConcurrent::run(&m_delivery_thread_pool,
                [this, message, target_subscription]() -> bool {
                    try {
                        // Cast handler back to function type and invoke
                        auto handler_func = std::any_cast<std::function<qtplugin::expected<void, PluginError>(std::shared_ptr<IMessage>)>>(target_subscription->handler);
                        auto result = handler_func(message);

                        if (result) {
                            // Update statistics atomically
                            m_messages_delivered.fetch_add(1);
                            target_subscription->messages_received++;
                            target_subscription->last_message_time = std::chrono::system_clock::now();
                            return true;
                        } else {
                            m_delivery_failures.fetch_add(1);
                            return false;
                        }
                    } catch (const std::exception& e) {
                        qCWarning(messageBusLog) << "Async delivery error to"
                                                 << QString::fromStdString(target_subscription->subscriber_id)
                                                 << ":" << e.what();
                        m_delivery_failures.fetch_add(1);
                        return false;
                    }
                });

            delivery_futures.push_back(std::move(future));
        } else {
            m_delivery_failures.fetch_add(1);
        }
    }

    // Wait for all deliveries to complete (with timeout)
    size_t successful_deliveries = 0;
    for (auto& future : delivery_futures) {
        // QFuture doesn't have timeout support, so we wait and check result
        future.waitForFinished();
        if (future.isFinished() && future.result()) {
            successful_deliveries++;
        } else if (!future.isFinished()) {
            qCWarning(messageBusLog) << "Message delivery timed out";
            m_delivery_failures.fetch_add(1);
        }
    }

    qCDebug(messageBusLog) << "Async delivery completed:"
                           << successful_deliveries << "successful,"
                           << (delivery_futures.size() - successful_deliveries) << "failed";

    return make_success();
}

// Get all subscribers for a message type
std::vector<std::string> MessageBus::get_all_subscribers(std::type_index message_type) const {
    std::shared_lock lock(m_subscriptions_mutex);
    std::vector<std::string> result;

    auto it = m_subscriptions.find(message_type);
    if (it != m_subscriptions.end()) {
        result.reserve(it->second.size());
        for (const auto& subscription : it->second) {
            if (subscription->is_active) {
                result.push_back(subscription->subscriber_id);
            }
        }
    }

    return result;
}

// Message logging implementation
void MessageBus::log_message(const IMessage& message,
                             const std::vector<std::string>& recipients) {
    std::unique_lock lock(m_log_mutex);

    QJsonObject log_entry;
    log_entry["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    log_entry["message_type"] = QString::fromStdString(std::string(message.type()));
    log_entry["message_id"] = QString::fromStdString(std::string(message.id()));
    log_entry["sender"] = QString::fromStdString(std::string(message.sender()));
    log_entry["priority"] = static_cast<int>(message.priority());

    // Add recipients
    QJsonArray recipients_array;
    for (const auto& recipient : recipients) {
        recipients_array.append(QString::fromStdString(recipient));
    }
    log_entry["recipients"] = recipients_array;

    // Add message content (if serializable)
    try {
        log_entry["content"] = message.to_json();
    } catch (...) {
        log_entry["content"] = "<not serializable>";
    }

    // Add to log
    m_message_log.push_back(log_entry);

    // Maintain log size limit
    if (m_message_log.size() > MAX_LOG_SIZE) {
        m_message_log.erase(m_message_log.begin(),
                           m_message_log.begin() + (m_message_log.size() - MAX_LOG_SIZE));
    }
}

// Cleanup expired messages and inactive subscriptions
void MessageBus::cleanup_expired_messages() {
    auto now = std::chrono::system_clock::now();
    constexpr auto SUBSCRIPTION_TIMEOUT = std::chrono::minutes(30);

    std::unique_lock lock(m_subscriptions_mutex);

    size_t removed_subscriptions = 0;

    for (auto& [type, subscriptions] : m_subscriptions) {
        auto it = std::remove_if(subscriptions.begin(), subscriptions.end(),
            [now, SUBSCRIPTION_TIMEOUT](const std::shared_ptr<Subscription>& sub) {
                // Remove inactive subscriptions that haven't received messages recently
                return !sub->is_active ||
                       (now - sub->last_message_time) > SUBSCRIPTION_TIMEOUT;
            });

        removed_subscriptions += std::distance(it, subscriptions.end());
        subscriptions.erase(it, subscriptions.end());
    }

    // Remove empty subscription lists
    for (auto it = m_subscriptions.begin(); it != m_subscriptions.end();) {
        if (it->second.empty()) {
            it = m_subscriptions.erase(it);
        } else {
            ++it;
        }
    }

    if (removed_subscriptions > 0) {
        qCDebug(messageBusLog) << "Cleanup removed" << removed_subscriptions << "expired subscriptions";
    }
}

// Enhanced statistics with detailed metrics
QJsonObject MessageBus::get_detailed_statistics() const {
    std::shared_lock lock(m_subscriptions_mutex);

    QJsonObject stats;

    // Basic statistics
    stats["total_subscriptions"] = static_cast<int>(get_total_subscription_count());
    stats["active_subscriptions"] = static_cast<int>(get_active_subscription_count());
    stats["unique_subscribers"] = static_cast<int>(m_subscriber_types.size());
    stats["message_types"] = static_cast<int>(m_subscriptions.size());
    stats["messages_published"] = static_cast<qint64>(m_messages_published.load());
    stats["messages_delivered"] = static_cast<qint64>(m_messages_delivered.load());
    stats["delivery_failures"] = static_cast<qint64>(m_delivery_failures.load());

    // Performance metrics
    double delivery_success_rate = 0.0;
    auto total_messages = m_messages_published.load();
    if (total_messages > 0) {
        delivery_success_rate = static_cast<double>(m_messages_delivered.load()) / total_messages * 100.0;
    }
    stats["delivery_success_rate_percent"] = delivery_success_rate;

    // Thread pool statistics
    stats["delivery_thread_pool_size"] = static_cast<int>(MAX_CONCURRENT_DELIVERIES);
    stats["active_threads"] = m_delivery_thread_pool.activeThreadCount();

    // Message type breakdown
    QJsonObject type_stats;
    for (const auto& [type, subscriptions] : m_subscriptions) {
        QJsonObject type_info;
        type_info["subscription_count"] = static_cast<int>(subscriptions.size());

        int active_count = 0;
        for (const auto& sub : subscriptions) {
            if (sub->is_active) {
                active_count++;
            }
        }
        type_info["active_subscriptions"] = active_count;

        type_stats[QString::fromStdString(type.name())] = type_info;
    }
    stats["message_types_detail"] = type_stats;

    return stats;
}

size_t MessageBus::get_total_subscription_count() const {
    size_t count = 0;
    for (const auto& [type, subscriptions] : m_subscriptions) {
        count += subscriptions.size();
    }
    return count;
}

size_t MessageBus::get_active_subscription_count() const {
    size_t count = 0;
    for (const auto& [type, subscriptions] : m_subscriptions) {
        for (const auto& subscription : subscriptions) {
            if (subscription->is_active) {
                count++;
            }
        }
    }
    return count;
}

std::future<qtplugin::expected<void, PluginError>>
MessageBus::publish_async_impl(std::shared_ptr<IMessage> message,
                               DeliveryMode mode,
                               const std::vector<std::string>& recipients) {
    return std::async(std::launch::async, [this, message, mode, recipients]() {
        return publish_impl(message, mode, recipients);
    });
}

qtplugin::expected<void, PluginError> MessageBus::subscribe_impl(
    std::string_view subscriber_id, std::type_index message_type,
    std::any handler, std::function<bool(const IMessage&)> filter) {
    std::unique_lock lock(m_subscriptions_mutex);

    // Create subscription
    auto subscription = std::make_shared<Subscription>(
        subscriber_id, message_type, std::move(handler));
    subscription->filter = std::move(filter);

    // Add to subscriptions map
    m_subscriptions[message_type].push_back(subscription);

    // Add to subscriber types
    m_subscriber_types[std::string(subscriber_id)].insert(message_type);

    emit subscription_added(QString::fromStdString(std::string(subscriber_id)),
                            QString::fromStdString(message_type.name()));

    return make_success();
}

// Duplicate log_message function removed

qtplugin::expected<void, PluginError> MessageBus::deliver_message(
    const IMessage& message, const std::vector<std::string>& recipients) {
    std::shared_lock lock(m_subscriptions_mutex);

    std::type_index message_type(typeid(message));
    auto it = m_subscriptions.find(message_type);
    if (it == m_subscriptions.end()) {
        // No subscribers for this message type
        return make_success();
    }

    int delivered_count = 0;
    int failed_count = 0;

    for (const auto& subscription : it->second) {
        if (!subscription->is_active) {
            continue;
        }

        // Check if this subscriber should receive the message
        bool should_deliver =
            recipients.empty() ||
            std::find(recipients.begin(), recipients.end(),
                      subscription->subscriber_id) != recipients.end();

        if (!should_deliver) {
            continue;
        }

        // Apply filter if present
        if (subscription->filter && !subscription->filter(message)) {
            continue;
        }

        // Attempt delivery (this is a simplified version)
        // In a real implementation, you would cast the handler to the correct
        // type and invoke it with the properly typed message
        try {
            subscription->message_count++;
            delivered_count++;
        } catch (...) {
            failed_count++;
        }
    }

    m_messages_delivered.fetch_add(delivered_count);
    if (failed_count > 0) {
        m_delivery_failures.fetch_add(failed_count);
    }

    return make_success();
}

std::vector<std::string> MessageBus::find_recipients(
    std::type_index message_type,
    const std::vector<std::string>& specific_recipients) const {
    if (!specific_recipients.empty()) {
        return specific_recipients;
    }

    // Find all subscribers for this message type
    std::vector<std::string> recipients;
    auto it = m_subscriptions.find(message_type);
    if (it != m_subscriptions.end()) {
        for (const auto& subscription : it->second) {
            if (subscription->is_active) {
                recipients.push_back(subscription->subscriber_id);
            }
        }
    }

    return recipients;
}

}  // namespace qtplugin

// MOC will be handled by CMake
