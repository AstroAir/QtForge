/**
 * @file message_publisher.cpp
 * @brief Implementation of message publisher
 * @version 3.0.0
 */

#include "message_publisher.hpp"
#include <QLoggingCategory>
#include <QMutexLocker>

Q_LOGGING_CATEGORY(messagePublisherLog, "qtforge.communication.publisher")

namespace qtplugin::communication {

MessagePublisher::MessagePublisher(
    const CommunicationConfig::MessageBusConfig& config, QObject* parent)
    : QObject(parent), config_(config) {
    process_timer_ = new QTimer(this);
    process_timer_->setSingleShot(false);
    process_timer_->setInterval(10);  // Process queue every 10ms

    connect(process_timer_, &QTimer::timeout, this,
            &MessagePublisher::process_message_queue);

    qCDebug(messagePublisherLog)
        << "MessagePublisher created with max queue size:"
        << config_.max_queue_size;
}

MessagePublisher::~MessagePublisher() { stop(); }

Result<void> MessagePublisher::publish(
    std::shared_ptr<IMessage> message, DeliveryMode mode,
    const std::vector<std::string>& recipients) {
    if (!message) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::InvalidMessage, "Null message provided",
            "Message pointer is null"});
    }

    if (!validate_message(*message)) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::InvalidMessage, "Invalid message format",
            "Message validation failed"});
    }

    if (!running_) {
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::SystemError, "Publisher not running",
            "MessagePublisher must be started before publishing"});
    }

    return publish_immediate(std::move(message), mode, recipients);
}

std::future<Result<void>> MessagePublisher::publish_async(
    std::shared_ptr<IMessage> message, DeliveryMode mode,
    const std::vector<std::string>& recipients) {
    std::promise<Result<void>> promise;
    auto future = promise.get_future();

    if (!message) {
        promise.set_value(qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::InvalidMessage, "Null message provided",
            "Message pointer is null"}));
        return future;
    }

    if (!validate_message(*message)) {
        promise.set_value(qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::InvalidMessage, "Invalid message format",
            "Message validation failed"}));
        return future;
    }

    if (!running_) {
        promise.set_value(qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::SystemError, "Publisher not running",
            "MessagePublisher must be started before publishing"}));
        return future;
    }

    enqueue_message(std::move(message), mode, recipients, std::move(promise));
    return future;
}

void MessagePublisher::set_router(std::shared_ptr<IMessageRouter> router) {
    router_ = std::move(router);
}

void MessagePublisher::set_statistics(std::shared_ptr<IStatistics> statistics) {
    statistics_ = std::move(statistics);
}

void MessagePublisher::start() {
    if (running_.exchange(true)) {
        return;  // Already running
    }

    if (!router_) {
        qCWarning(messagePublisherLog) << "Starting publisher without router - "
                                          "messages will not be delivered";
    }

    process_timer_->start();
    qCDebug(messagePublisherLog) << "MessagePublisher started";
}

void MessagePublisher::stop() {
    if (!running_.exchange(false)) {
        return;  // Already stopped
    }

    process_timer_->stop();

    // Process remaining messages in queue
    process_message_queue();

    qCDebug(messagePublisherLog) << "MessagePublisher stopped";
}

bool MessagePublisher::is_running() const { return running_.load(); }

void MessagePublisher::process_message_queue() {
    QMutexLocker locker(&queue_mutex_);

    while (!message_queue_.empty()) {
        auto& pending = message_queue_.front();

        // Check for timeout
        auto now = std::chrono::system_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - pending.timestamp);

        if (elapsed > config_.delivery_timeout) {
            pending.promise.set_value(qtplugin::unexpected(CommunicationError{
                CommunicationError::Type::TimeoutExpired,
                "Message delivery timeout",
                "Message was not delivered within timeout period"}));
            message_queue_.pop();
            pending_count_--;
            continue;
        }

        // Try to publish the message
        auto result = publish_immediate(pending.message, pending.mode,
                                        pending.recipients);
        pending.promise.set_value(result);

        message_queue_.pop();
        pending_count_--;
    }
}

Result<void> MessagePublisher::publish_immediate(
    std::shared_ptr<IMessage> message, DeliveryMode mode,
    const std::vector<std::string>& recipients) {
    if (!router_) {
        return qtplugin::unexpected(
            CommunicationError{CommunicationError::Type::SystemError,
                               "No router available", "MessageRouter not set"});
    }

    // Find subscribers
    auto subscribers_result =
        router_->find_subscribers(*message, mode, recipients);
    if (!subscribers_result.has_value()) {
        auto error = subscribers_result.error();
        update_statistics(*message, false, 0);
        emit_signals(*message, false, 0, error.message);
        return qtplugin::unexpected(error);
    }

    auto subscribers = subscribers_result.value();

    if (subscribers.empty()) {
        update_statistics(*message, false, 0);
        emit_signals(*message, false, 0, "No subscribers found");
        return qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::NoSubscribers, "No subscribers found",
            "No active subscriptions for message type"});
    }

    // Deliver message
    auto delivery_result = router_->deliver_message(*message, subscribers);
    bool success = delivery_result.has_value();

    update_statistics(*message, success, subscribers.size());
    emit_signals(*message, success, subscribers.size(),
                 success ? "" : delivery_result.error().message);

    return delivery_result;
}

void MessagePublisher::enqueue_message(
    std::shared_ptr<IMessage> message, DeliveryMode mode,
    const std::vector<std::string>& recipients,
    std::promise<Result<void>>&& promise) {
    QMutexLocker locker(&queue_mutex_);

    if (message_queue_.size() >= config_.max_queue_size) {
        promise.set_value(qtplugin::unexpected(CommunicationError{
            CommunicationError::Type::SystemError, "Message queue full",
            "Cannot enqueue message - queue at maximum capacity"}));
        return;
    }

    message_queue_.emplace(PendingMessage{std::move(message), mode, recipients,
                                          std::move(promise),
                                          std::chrono::system_clock::now()});

    pending_count_++;
}

bool MessagePublisher::validate_message(const IMessage& message) const {
    // Basic validation
    if (message.type().empty()) {
        return false;
    }

    if (message.sender().empty()) {
        return false;
    }

    if (message.id().empty()) {
        return false;
    }

    return true;
}

void MessagePublisher::update_statistics(const IMessage& message, bool success,
                                         size_t recipient_count) {
    if (statistics_) {
        // Statistics update would be implemented here
        // This is a placeholder for the actual statistics interface
    }
}

void MessagePublisher::emit_signals(const IMessage& message, bool success,
                                    size_t recipient_count,
                                    const std::string& error) {
    if (success) {
        emit message_published(
            QString::fromStdString(std::string(message.type())),
            QString::fromStdString(std::string(message.sender())),
            static_cast<int>(recipient_count));
    } else {
        emit publish_failed(QString::fromStdString(std::string(message.type())),
                            QString::fromStdString(error));
    }
}

}  // namespace qtplugin::communication

// Note: moc file will be generated by build system
