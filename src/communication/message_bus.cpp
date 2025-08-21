/**
 * @file message_bus.cpp
 * @brief Implementation of message bus for plugin communication
 * @version 3.0.0
 */

#include <qtplugin/communication/message_bus.hpp>
#include <QJsonDocument>
#include <QJsonArray>
#include <algorithm>
#include <future>

namespace qtplugin {

MessageBus::MessageBus(QObject* parent) : QObject(parent) {
    // Initialize with default capacity
    m_message_log.reserve(MAX_LOG_SIZE);
}

MessageBus::~MessageBus() = default;

qtplugin::expected<void, PluginError> MessageBus::unsubscribe(std::string_view subscriber_id,
                                                        std::optional<std::type_index> message_type) {
    std::unique_lock lock(m_subscriptions_mutex);
    
    if (message_type) {
        // Unsubscribe from specific message type
        auto it = m_subscriptions.find(*message_type);
        if (it != m_subscriptions.end()) {
            auto& subscriptions = it->second;
            subscriptions.erase(
                std::remove_if(subscriptions.begin(), subscriptions.end(),
                    [subscriber_id](const std::unique_ptr<Subscription>& sub) {
                        return sub->subscriber_id == subscriber_id;
                    }),
                subscriptions.end()
            );
            
            if (subscriptions.empty()) {
                m_subscriptions.erase(it);
            }
            
            emit subscription_removed(QString::fromStdString(std::string(subscriber_id)), 
                                    QString::fromStdString(message_type->name()));
        }
        
        // Update subscriber types
        auto subscriber_it = m_subscriber_types.find(std::string(subscriber_id));
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
                std::remove_if(subscriptions.begin(), subscriptions.end(),
                    [subscriber_id](const std::unique_ptr<Subscription>& sub) {
                        return sub->subscriber_id == subscriber_id;
                    }),
                subscriptions.end()
            );
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

std::vector<std::string> MessageBus::subscribers(std::type_index message_type) const {
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

std::vector<Subscription> MessageBus::subscriptions(std::string_view subscriber_id) const {
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
    return m_subscriber_types.find(std::string(subscriber_id)) != m_subscriber_types.end();
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
        {"messages_published", static_cast<qint64>(m_messages_published.load())},
        {"messages_delivered", static_cast<qint64>(m_messages_delivered.load())},
        {"delivery_failures", static_cast<qint64>(m_delivery_failures.load())},
        {"logging_enabled", m_logging_enabled.load()}
    };
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

bool MessageBus::is_logging_enabled() const {
    return m_logging_enabled.load();
}

std::vector<QJsonObject> MessageBus::message_log(size_t limit) const {
    std::shared_lock lock(m_log_mutex);
    
    if (limit == 0 || limit >= m_message_log.size()) {
        return m_message_log;
    }
    
    // Return the most recent messages
    auto start_it = m_message_log.end() - static_cast<std::vector<QJsonObject>::difference_type>(limit);
    return std::vector<QJsonObject>(start_it, m_message_log.end());
}

qtplugin::expected<void, PluginError> MessageBus::publish_impl(std::shared_ptr<IMessage> message,
                                                         DeliveryMode mode,
                                                         const std::vector<std::string>& recipients) {
    if (!message) {
        return make_error<void>(PluginErrorCode::InvalidParameters, "Message is null");
    }
    
    m_messages_published.fetch_add(1);
    
    // Log the message if logging is enabled
    if (m_logging_enabled.load()) {
        log_message(*message, recipients);
    }
    
    // Find recipients based on delivery mode
    std::vector<std::string> target_recipients;
    if (mode == DeliveryMode::Broadcast) {
        target_recipients = find_recipients(std::type_index(typeid(*message)), {});
    } else {
        target_recipients = recipients;
    }
    
    // Deliver the message
    auto delivery_result = deliver_message(*message, target_recipients);
    if (!delivery_result) {
        m_delivery_failures.fetch_add(1);
        return delivery_result;
    }
    
    // Emit signal
    emit message_published(QString::fromStdString(std::string(message->type())),
                          QString::fromStdString(std::string(message->sender())),
                          static_cast<int>(target_recipients.size()));
    
    return make_success();
}

std::future<qtplugin::expected<void, PluginError>> MessageBus::publish_async_impl(std::shared_ptr<IMessage> message,
                                                                            DeliveryMode mode,
                                                                            const std::vector<std::string>& recipients) {
    return std::async(std::launch::async, [this, message, mode, recipients]() {
        return publish_impl(message, mode, recipients);
    });
}

qtplugin::expected<void, PluginError> MessageBus::subscribe_impl(std::string_view subscriber_id,
                                                           std::type_index message_type,
                                                           std::any handler,
                                                           std::function<bool(const IMessage&)> filter) {
    std::unique_lock lock(m_subscriptions_mutex);
    
    // Create subscription
    auto subscription = std::make_unique<Subscription>(subscriber_id, message_type, std::move(handler));
    subscription->filter = std::move(filter);
    
    // Add to subscriptions map
    m_subscriptions[message_type].push_back(std::move(subscription));
    
    // Add to subscriber types
    m_subscriber_types[std::string(subscriber_id)].insert(message_type);
    
    emit subscription_added(QString::fromStdString(std::string(subscriber_id)),
                           QString::fromStdString(message_type.name()));
    
    return make_success();
}

void MessageBus::log_message(const IMessage& message, const std::vector<std::string>& recipients) {
    std::unique_lock lock(m_log_mutex);
    
    QJsonArray recipients_array;
    for (const auto& recipient : recipients) {
        recipients_array.append(QString::fromStdString(recipient));
    }
    
    QJsonObject log_entry = message.to_json();
    log_entry["recipients"] = recipients_array;
    log_entry["recipient_count"] = static_cast<int>(recipients.size());
    
    m_message_log.push_back(log_entry);
    
    // Maintain log size limit
    if (m_message_log.size() > MAX_LOG_SIZE) {
        m_message_log.erase(m_message_log.begin(), 
                           m_message_log.begin() + (m_message_log.size() - MAX_LOG_SIZE));
    }
}

qtplugin::expected<void, PluginError> MessageBus::deliver_message(const IMessage& message,
                                                           const std::vector<std::string>& recipients) {
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
        bool should_deliver = recipients.empty() || 
                             std::find(recipients.begin(), recipients.end(), subscription->subscriber_id) != recipients.end();
        
        if (!should_deliver) {
            continue;
        }
        
        // Apply filter if present
        if (subscription->filter && !subscription->filter(message)) {
            continue;
        }
        
        // Attempt delivery (this is a simplified version)
        // In a real implementation, you would cast the handler to the correct type
        // and invoke it with the properly typed message
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

std::vector<std::string> MessageBus::find_recipients(std::type_index message_type, 
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

} // namespace qtplugin

// MOC will be handled by CMake
