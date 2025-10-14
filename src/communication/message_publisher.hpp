/**
 * @file message_publisher.hpp
 * @brief Message publisher implementation following Single Responsibility
 * Principle
 * @version 3.0.0
 */

#pragma once

#include <QMutex>
#include <QObject>
#include <QTimer>
#include <atomic>
#include <qtplugin/communication/factory.hpp>
#include <qtplugin/communication/interfaces.hpp>
#include <queue>
#include <thread>

namespace qtplugin::communication {

/**
 * @brief Message publisher implementation
 *
 * Responsibilities (Single Responsibility Principle):
 * - Publishing messages to the message routing system
 * - Managing message queues and delivery
 * - Handling asynchronous message publishing
 * - Providing delivery guarantees and error handling
 */
class MessagePublisher : public QObject, public IMessagePublisher {
    Q_OBJECT

public:
    explicit MessagePublisher(
        const CommunicationConfig::MessageBusConfig& config,
        QObject* parent = nullptr);
    ~MessagePublisher() override;

    // IMessagePublisher interface
    Result<void> publish(
        std::shared_ptr<IMessage> message,
        DeliveryMode mode = DeliveryMode::Broadcast,
        const std::vector<std::string>& recipients = {}) override;

    std::future<Result<void>> publish_async(
        std::shared_ptr<IMessage> message,
        DeliveryMode mode = DeliveryMode::Broadcast,
        const std::vector<std::string>& recipients = {}) override;

    // Configuration and control
    void set_router(std::shared_ptr<IMessageRouter> router);
    void set_statistics(std::shared_ptr<IStatistics> statistics);

    void start();
    void stop();
    bool is_running() const;

signals:
    void message_published(const QString& message_type, const QString& sender,
                           int recipient_count);
    void publish_failed(const QString& message_type, const QString& error);

private slots:
    void process_message_queue();

private:
    struct PendingMessage {
        std::shared_ptr<IMessage> message;
        DeliveryMode mode;
        std::vector<std::string> recipients;
        std::promise<Result<void>> promise;
        std::chrono::system_clock::time_point timestamp;
    };

    // Configuration
    CommunicationConfig::MessageBusConfig config_;

    // Dependencies (Dependency Inversion Principle)
    std::shared_ptr<IMessageRouter> router_;
    std::shared_ptr<IStatistics> statistics_;

    // Message queue management
    mutable QMutex queue_mutex_;
    std::queue<PendingMessage> message_queue_;
    QTimer* process_timer_;

    // State management
    std::atomic<bool> running_{false};
    std::atomic<size_t> pending_count_{0};

    // Internal methods
    Result<void> publish_immediate(std::shared_ptr<IMessage> message,
                                   DeliveryMode mode,
                                   const std::vector<std::string>& recipients);

    void enqueue_message(std::shared_ptr<IMessage> message, DeliveryMode mode,
                         const std::vector<std::string>& recipients,
                         std::promise<Result<void>>&& promise);

    bool validate_message(const IMessage& message) const;
    void update_statistics(const IMessage& message, bool success,
                           size_t recipient_count);
    void emit_signals(const IMessage& message, bool success,
                      size_t recipient_count, const std::string& error = {});
};

}  // namespace qtplugin::communication
