/**
 * @file subscription_manager.cpp
 * @brief Subscription manager implementation
 * @version 3.0.0
 */

#include "subscription_manager.hpp"
#include <QLoggingCategory>
#include <QMutexLocker>
#include <QReadLocker>
#include <QUuid>
#include <QWriteLocker>
#include <algorithm>

Q_LOGGING_CATEGORY(subscriptionLog, "qtforge.communication.subscription")

namespace qtplugin::communication {

// === Subscription Implementation ===

Subscription::Subscription(std::string id, std::string subscriber_id,
                           std::type_index message_type,
                           ISubscriptionManager::MessageHandler handler,
                           ISubscriptionManager::MessageFilter filter)
    : id_(std::move(id)),
      subscriber_id_(std::move(subscriber_id)),
      message_type_(message_type),
      handler_(std::move(handler)),
      filter_(std::move(filter)) {}

std::string Subscription::id() const noexcept { return id_; }

std::string_view Subscription::subscriber_id() const noexcept {
    return subscriber_id_;
}

std::type_index Subscription::message_type() const noexcept {
    return message_type_;
}

bool Subscription::is_active() const noexcept { return active_.load(); }

void Subscription::cancel() { active_.store(false); }

Result<void> Subscription::deliver(const IMessage& message) {
    if (!is_active()) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::DeliveryFailed,
            "Subscription is not active", "Subscription ID: " + id_});
    }

    // Check filter before acquiring lock for better performance
    if (filter_ && !filter_(message)) {
        // Message filtered out - this is not an error, just skip delivery
        return Result<void>{};
    }

    QMutexLocker lock(&handler_mutex_);
    if (!handler_) {
        return qtplugin::unexpected(
            CommunicationError{CommunicationError::Type::DeliveryFailed,
                               "No handler registered for subscription",
                               "Subscription ID: " + id_});
    }

    try {
        handler_(message);
        return Result<void>{};
    } catch (const std::exception& e) {
        qCWarning(subscriptionLog)
            << "Exception in message handler:" << e.what();
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::DeliveryFailed,
            "Handler threw exception: " + std::string(e.what()),
            "Subscription ID: " + id_});
    } catch (...) {
        qCWarning(subscriptionLog) << "Unknown exception in message handler";
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::DeliveryFailed,
            "Handler threw unknown exception", "Subscription ID: " + id_});
    }
}

void Subscription::handle_message(const IMessage& message) {
    // Delegate to deliver() method for consistency
    // This method is kept for backward compatibility
    auto result = deliver(message);
    if (!result.has_value()) {
        qCWarning(subscriptionLog)
            << "Message delivery failed:" << result.error().message.c_str();
    }
}

bool Subscription::matches_filter(const IMessage& message) const {
    return !filter_ || filter_(message);
}

// === SubscriptionManager Implementation ===

SubscriptionManager::SubscriptionManager(
    const CommunicationConfig::MessageBusConfig& config, QObject* parent)
    : QObject(parent), config_(config) {
    qCDebug(subscriptionLog)
        << "SubscriptionManager created with max queue size:"
        << config_.max_queue_size;
}

SubscriptionManager::~SubscriptionManager() {
    QWriteLocker lock(&subscriptions_lock_);
    subscriptions_by_id_.clear();
    subscriptions_by_subscriber_.clear();
    subscriptions_by_type_.clear();
    qCDebug(subscriptionLog) << "SubscriptionManager destroyed";
}

Result<std::shared_ptr<ISubscription>> SubscriptionManager::subscribe(
    std::string_view subscriber_id, std::type_index message_type,
    MessageHandler handler, MessageFilter filter) {
    if (!validate_subscription_request(subscriber_id, message_type, handler)) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::InvalidHandler,
            "Invalid subscription request",
            "Subscriber ID, message type, or handler is invalid"});
    }

    std::string subscription_id = generate_subscription_id();
    auto subscription = std::make_shared<Subscription>(
        subscription_id, std::string(subscriber_id), message_type,
        std::move(handler), std::move(filter));

    {
        QWriteLocker lock(&subscriptions_lock_);
        add_subscription_to_indices(subscription);
    }

    qCDebug(subscriptionLog)
        << "Created subscription:" << QString::fromStdString(subscription_id)
        << "for subscriber:"
        << QString::fromStdString(std::string(subscriber_id));

    return Result<std::shared_ptr<ISubscription>>(
        std::static_pointer_cast<ISubscription>(subscription));
}

Result<void> SubscriptionManager::unsubscribe(
    const std::string& subscription_id) {
    QWriteLocker lock(&subscriptions_lock_);

    auto it = subscriptions_by_id_.find(subscription_id);
    if (it == subscriptions_by_id_.end()) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::SystemError, "Subscription not found",
            "Subscription ID: " + subscription_id});
    }

    it->second->cancel();
    remove_subscription_from_indices(subscription_id);

    qCDebug(subscriptionLog)
        << "Removed subscription:" << QString::fromStdString(subscription_id);

    return {};
}

Result<void> SubscriptionManager::unsubscribe_all(
    std::string_view subscriber_id) {
    QWriteLocker lock(&subscriptions_lock_);

    auto it = subscriptions_by_subscriber_.find(std::string(subscriber_id));
    if (it == subscriptions_by_subscriber_.end()) {
        return {};  // No subscriptions for this subscriber
    }

    std::vector<std::string> subscription_ids;
    for (const auto& subscription : it->second) {
        if (subscription) {
            subscription->cancel();
            subscription_ids.push_back(subscription->id());
        }
    }

    for (const auto& id : subscription_ids) {
        remove_subscription_from_indices(id);
    }

    qCDebug(subscriptionLog)
        << "Removed all subscriptions for subscriber:"
        << QString::fromStdString(std::string(subscriber_id))
        << "Count:" << subscription_ids.size();

    return {};
}

std::vector<std::shared_ptr<ISubscription>>
SubscriptionManager::get_subscriptions(std::string_view subscriber_id) const {
    QReadLocker lock(&subscriptions_lock_);

    if (subscriber_id.empty()) {
        // Return all subscriptions
        std::vector<std::shared_ptr<ISubscription>> result;
        result.reserve(subscriptions_by_id_.size());
        for (const auto& [id, subscription] : subscriptions_by_id_) {
            if (subscription && subscription->is_active()) {
                result.push_back(subscription);
            }
        }
        return result;
    } else {
        // Return subscriptions for specific subscriber
        auto it = subscriptions_by_subscriber_.find(std::string(subscriber_id));
        if (it != subscriptions_by_subscriber_.end()) {
            std::vector<std::shared_ptr<ISubscription>> result;
            for (const auto& subscription : it->second) {
                if (subscription && subscription->is_active()) {
                    result.push_back(subscription);
                }
            }
            return result;
        }
    }

    return {};
}

std::vector<std::shared_ptr<ISubscription>>
SubscriptionManager::find_subscriptions_for_message(
    const IMessage& message) const {
    QReadLocker lock(&subscriptions_lock_);

    auto it = subscriptions_by_type_.find(std::type_index(typeid(message)));
    if (it == subscriptions_by_type_.end()) {
        return {};
    }

    std::vector<std::shared_ptr<ISubscription>> result;
    for (const auto& subscription : it->second) {
        if (subscription && subscription->is_active() &&
            subscription->matches_filter(message)) {
            result.push_back(subscription);
        }
    }

    return result;
}

std::vector<std::shared_ptr<ISubscription>>
SubscriptionManager::find_subscriptions_for_type(
    std::type_index message_type) const {
    QReadLocker lock(&subscriptions_lock_);

    auto it = subscriptions_by_type_.find(message_type);
    if (it != subscriptions_by_type_.end()) {
        std::vector<std::shared_ptr<ISubscription>> result;
        for (const auto& subscription : it->second) {
            if (subscription && subscription->is_active()) {
                result.push_back(subscription);
            }
        }
        return result;
    }

    return {};
}

std::vector<std::shared_ptr<ISubscription>>
SubscriptionManager::find_subscriptions_for_subscriber(
    std::string_view subscriber_id) const {
    return get_subscriptions(subscriber_id);
}

size_t SubscriptionManager::get_total_subscriptions() const {
    QReadLocker lock(&subscriptions_lock_);
    return subscriptions_by_id_.size();
}

size_t SubscriptionManager::get_active_subscriptions() const {
    QReadLocker lock(&subscriptions_lock_);
    return std::count_if(subscriptions_by_id_.begin(),
                         subscriptions_by_id_.end(), [](const auto& pair) {
                             return pair.second && pair.second->is_active();
                         });
}

size_t SubscriptionManager::get_subscriber_count() const {
    QReadLocker lock(&subscriptions_lock_);
    return subscriptions_by_subscriber_.size();
}

std::vector<std::string> SubscriptionManager::get_subscriber_ids() const {
    QReadLocker lock(&subscriptions_lock_);
    std::vector<std::string> result;
    result.reserve(subscriptions_by_subscriber_.size());
    for (const auto& [subscriber_id, subscriptions] :
         subscriptions_by_subscriber_) {
        if (!subscriptions.empty()) {
            result.push_back(subscriber_id);
        }
    }
    return result;
}

// Private methods

std::string SubscriptionManager::generate_subscription_id() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
}

void SubscriptionManager::add_subscription_to_indices(
    std::shared_ptr<Subscription> subscription) {
    const std::string& id = subscription->id();
    std::string subscriber_id = std::string(subscription->subscriber_id());
    std::type_index message_type = subscription->message_type();

    // Add to ID index
    subscriptions_by_id_[id] = subscription;

    // Add to subscriber index
    subscriptions_by_subscriber_[subscriber_id].push_back(subscription);

    // Add to type index
    subscriptions_by_type_[message_type].push_back(subscription);
}

void SubscriptionManager::remove_subscription_from_indices(
    const std::string& subscription_id) {
    auto it = subscriptions_by_id_.find(subscription_id);
    if (it == subscriptions_by_id_.end()) {
        return;
    }

    auto subscription = it->second;
    std::string subscriber_id = std::string(subscription->subscriber_id());
    std::type_index message_type = subscription->message_type();

    // Remove from ID index
    subscriptions_by_id_.erase(it);

    // Remove from subscriber index
    auto subscriber_it = subscriptions_by_subscriber_.find(subscriber_id);
    if (subscriber_it != subscriptions_by_subscriber_.end()) {
        auto& subscriptions = subscriber_it->second;
        subscriptions.erase(std::remove(subscriptions.begin(),
                                        subscriptions.end(), subscription),
                            subscriptions.end());

        if (subscriptions.empty()) {
            subscriptions_by_subscriber_.erase(subscriber_it);
        }
    }

    // Remove from type index
    auto type_it = subscriptions_by_type_.find(message_type);
    if (type_it != subscriptions_by_type_.end()) {
        auto& subscriptions = type_it->second;
        subscriptions.erase(std::remove(subscriptions.begin(),
                                        subscriptions.end(), subscription),
                            subscriptions.end());

        if (subscriptions.empty()) {
            subscriptions_by_type_.erase(type_it);
        }
    }
}

void SubscriptionManager::cleanup_empty_entries() {
    // Remove empty entries from subscriber index
    for (auto it = subscriptions_by_subscriber_.begin();
         it != subscriptions_by_subscriber_.end();) {
        if (it->second.empty()) {
            it = subscriptions_by_subscriber_.erase(it);
        } else {
            ++it;
        }
    }

    // Remove empty entries from type index
    for (auto it = subscriptions_by_type_.begin();
         it != subscriptions_by_type_.end();) {
        if (it->second.empty()) {
            it = subscriptions_by_type_.erase(it);
        } else {
            ++it;
        }
    }
}

bool SubscriptionManager::validate_subscription_request(
    std::string_view subscriber_id, std::type_index message_type,
    const MessageHandler& handler) const {
    return !subscriber_id.empty() &&
           message_type != std::type_index(typeid(void)) && handler != nullptr;
}

}  // namespace qtplugin::communication
