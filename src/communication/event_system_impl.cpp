/**
 * @file event_system_impl.cpp
 * @brief Event system adapter implementation
 * @version 3.1.0
 */

#include "event_system_impl.hpp"
#include <QLoggingCategory>
#include <QMutexLocker>
#include <QUuid>

Q_LOGGING_CATEGORY(eventSystemAdapterLog,
                   "qtforge.communication.eventsystem.adapter")

namespace qtplugin::communication {

/**
 * @brief Wrapper class to adapt IMessage to TypedEvent
 *
 * This wrapper allows IMessage objects to be published through TypedEventSystem
 * by wrapping them in a TypedEvent container.
 */
class MessageEventWrapper {
public:
    explicit MessageEventWrapper(std::shared_ptr<IMessage> message)
        : message_(std::move(message)) {}

    std::shared_ptr<IMessage> message() const { return message_; }

private:
    std::shared_ptr<IMessage> message_;
};

EventSystemImpl::EventSystemImpl(
    const CommunicationConfig::EventSystemConfig& config)
    : config_(config),
      typed_event_system_(std::make_unique<qtplugin::TypedEventSystem>()) {
    qCDebug(eventSystemAdapterLog) << "EventSystemImpl adapter created";
}

EventSystemImpl::~EventSystemImpl() {
    QMutexLocker lock(&subscriptions_mutex_);

    // Unsubscribe all tracked subscriptions
    for (const auto& [sub_id, typed_sub_id] : subscription_id_map_) {
        typed_event_system_->unsubscribe(typed_sub_id);
    }
    subscription_id_map_.clear();

    qCDebug(eventSystemAdapterLog) << "EventSystemImpl adapter destroyed";
}

Result<void> EventSystemImpl::publish_event_impl(
    std::shared_ptr<IMessage> event) {
    if (!event) {
        return qtplugin::unexpected(
            CommunicationError{CommunicationError::Type::InvalidMessage,
                               "Cannot publish null event", ""});
    }

    try {
        // Wrap the IMessage in a TypedEvent for publishing
        MessageEventWrapper wrapper(event);

        // Publish through TypedEventSystem
        auto result = typed_event_system_->publish(
            QString::fromStdString(std::string(event->sender())), wrapper);

        if (!result.has_value()) {
            // Convert TypedEventSystem error to CommunicationError
            const auto& error = result.error();
            return qtplugin::unexpected(CommunicationError{
                CommunicationError::Type::DeliveryFailed,
                "Failed to publish event: " + error.message, error.details});
        }

        qCDebug(eventSystemAdapterLog)
            << "Event published successfully:"
            << QString::fromStdString(std::string(event->type()));
        return Result<void>{};

    } catch (const std::exception& e) {
        qCWarning(eventSystemAdapterLog)
            << "Exception publishing event:" << e.what();
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::DeliveryFailed,
            "Exception during event publish: " + std::string(e.what()), ""});
    }
}

Result<std::shared_ptr<ISubscription>> EventSystemImpl::subscribe_event_impl(
    std::string_view subscriber_id, std::type_index event_type,
    std::function<void(const IMessage&)> handler,
    std::function<bool(const IMessage&)> filter) {
    if (subscriber_id.empty()) {
        return qtplugin::unexpected(
            CommunicationError{CommunicationError::Type::InvalidMessage,
                               "Subscriber ID cannot be empty", ""});
    }

    if (!handler) {
        return qtplugin::unexpected(
            CommunicationError{CommunicationError::Type::InvalidMessage,
                               "Handler cannot be null", ""});
    }

    try {
        // Create a wrapper handler that unwraps MessageEventWrapper
        auto wrapped_handler =
            [handler,
             filter](const qtplugin::TypedEvent<MessageEventWrapper>& event) {
                auto message = event.data().message();
                if (!message) {
                    return;
                }

                // Apply filter if provided
                if (filter && !filter(*message)) {
                    return;
                }

                // Invoke the original handler
                handler(*message);
            };

        // Subscribe through TypedEventSystem
        auto result = typed_event_system_->subscribe<MessageEventWrapper>(
            QString::fromStdString(std::string(subscriber_id)),
            wrapped_handler);

        if (!result.has_value()) {
            const auto& error = result.error();
            return qtplugin::unexpected(CommunicationError{
                CommunicationError::Type::DeliveryFailed,
                "Failed to subscribe: " + error.message, error.details});
        }

        // Create a subscription wrapper that implements ISubscription
        // We'll use a simple implementation that tracks the TypedEventSystem
        // subscription
        class SubscriptionWrapper : public ISubscription {
        public:
            SubscriptionWrapper(std::string id, std::string subscriber_id,
                                std::type_index message_type,
                                QString typed_sub_id,
                                qtplugin::TypedEventSystem* event_system)
                : id_(std::move(id)),
                  subscriber_id_(std::move(subscriber_id)),
                  message_type_(message_type),
                  typed_subscription_id_(std::move(typed_sub_id)),
                  event_system_(event_system),
                  active_(true) {}

            std::string id() const noexcept override { return id_; }
            std::string_view subscriber_id() const noexcept override {
                return subscriber_id_;
            }
            std::type_index message_type() const noexcept override {
                return message_type_;
            }
            bool is_active() const noexcept override { return active_.load(); }

            void cancel() override {
                if (active_.exchange(false) && event_system_) {
                    event_system_->unsubscribe(typed_subscription_id_);
                }
            }

            Result<void> deliver(const IMessage& message) override {
                if (!is_active()) {
                    return qtplugin::unexpected(CommunicationError{
                        CommunicationError::Type::DeliveryFailed,
                        "Subscription is not active",
                        "Subscription ID: " + id_});
                }

                // TypedEventSystem handles delivery internally
                // This method is not used for event system subscriptions
                return Result<void>{};
            }

        private:
            std::string id_;
            std::string subscriber_id_;
            std::type_index message_type_;
            QString typed_subscription_id_;
            qtplugin::TypedEventSystem* event_system_;
            std::atomic<bool> active_;
        };

        // Generate unique subscription ID
        std::string sub_id =
            QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
        QString typed_sub_id = result.value();

        // Track the subscription mapping
        {
            QMutexLocker lock(&subscriptions_mutex_);
            subscription_id_map_[sub_id] = typed_sub_id;
        }

        auto subscription = std::make_shared<SubscriptionWrapper>(
            sub_id, std::string(subscriber_id), event_type, typed_sub_id,
            typed_event_system_.get());

        qCDebug(eventSystemAdapterLog)
            << "Subscription created:"
            << QString::fromStdString(std::string(subscriber_id));

        return Result<std::shared_ptr<ISubscription>>{subscription};

    } catch (const std::exception& e) {
        qCWarning(eventSystemAdapterLog)
            << "Exception subscribing to event:" << e.what();
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::DeliveryFailed,
            "Exception during subscribe: " + std::string(e.what()), ""});
    }
}

}  // namespace qtplugin::communication
