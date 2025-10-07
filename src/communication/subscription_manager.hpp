/**
 * @file subscription_manager.hpp
 * @brief Subscription manager implementation following Single Responsibility Principle
 * @version 3.0.0
 */

#pragma once

#include <qtplugin/communication/interfaces.hpp>
#include <qtplugin/communication/factory.hpp>
#include <QObject>
#include <QMutex>
#include <QReadWriteLock>
#include <unordered_map>
#include <vector>
#include <memory>
#include <atomic>

namespace qtplugin::communication {

/**
 * @brief Subscription implementation
 */
class Subscription : public ISubscription {
public:
    Subscription(
        std::string id,
        std::string subscriber_id,
        std::type_index message_type,
        ISubscriptionManager::MessageHandler handler,
        ISubscriptionManager::MessageFilter filter = nullptr
    );

    // ISubscription interface
    std::string id() const noexcept override;
    std::string_view subscriber_id() const noexcept override;
    std::type_index message_type() const noexcept override;
    bool is_active() const noexcept override;
    void cancel() override;

    // Internal interface
    void handle_message(const IMessage& message);
    bool matches_filter(const IMessage& message) const;

private:
    std::string id_;
    std::string subscriber_id_;
    std::type_index message_type_;
    ISubscriptionManager::MessageHandler handler_;
    ISubscriptionManager::MessageFilter filter_;
    std::atomic<bool> active_{true};
    mutable QMutex handler_mutex_;
};

/**
 * @brief Subscription manager implementation
 * 
 * Responsibilities (Single Responsibility Principle):
 * - Managing message subscriptions and unsubscriptions
 * - Maintaining subscriber registry
 * - Providing subscription lookup and filtering
 * - Handling subscription lifecycle
 */
class SubscriptionManager : public QObject, public ISubscriptionManager {
    Q_OBJECT

public:
    explicit SubscriptionManager(const CommunicationConfig::MessageBusConfig& config, QObject* parent = nullptr);
    ~SubscriptionManager() override;

    // ISubscriptionManager interface
    Result<std::shared_ptr<ISubscription>> subscribe(
        std::string_view subscriber_id,
        std::type_index message_type,
        MessageHandler handler,
        MessageFilter filter = nullptr
    ) override;
    
    Result<void> unsubscribe(const std::string& subscription_id) override;
    Result<void> unsubscribe_all(std::string_view subscriber_id) override;
    
    std::vector<std::shared_ptr<ISubscription>> get_subscriptions(
        std::string_view subscriber_id = {}
    ) const override;

    // Additional functionality for message routing
    std::vector<std::shared_ptr<ISubscription>> find_subscriptions_for_message(
        const IMessage& message
    ) const;
    
    std::vector<std::shared_ptr<ISubscription>> find_subscriptions_for_type(
        std::type_index message_type
    ) const;
    
    std::vector<std::shared_ptr<ISubscription>> find_subscriptions_for_subscriber(
        std::string_view subscriber_id
    ) const;

    // Statistics and monitoring
    size_t get_total_subscriptions() const;
    size_t get_active_subscriptions() const;
    size_t get_subscriber_count() const;
    std::vector<std::string> get_subscriber_ids() const;

signals:
    void subscription_added(const QString& subscription_id, const QString& subscriber_id, const QString& message_type);
    void subscription_removed(const QString& subscription_id, const QString& subscriber_id, const QString& message_type);
    void subscriber_added(const QString& subscriber_id);
    void subscriber_removed(const QString& subscriber_id);

private:
    // Configuration
    CommunicationConfig::MessageBusConfig config_;
    
    // Subscription storage
    mutable QReadWriteLock subscriptions_lock_;
    std::unordered_map<std::string, std::shared_ptr<Subscription>> subscriptions_by_id_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<Subscription>>> subscriptions_by_subscriber_;
    std::unordered_map<std::type_index, std::vector<std::shared_ptr<Subscription>>> subscriptions_by_type_;
    
    // ID generation
    std::atomic<uint64_t> next_subscription_id_{1};
    
    // Internal methods
    std::string generate_subscription_id();
    void add_subscription_to_indices(std::shared_ptr<Subscription> subscription);
    void remove_subscription_from_indices(const std::string& subscription_id);
    void cleanup_empty_entries();
    
    // Validation
    bool validate_subscription_request(
        std::string_view subscriber_id,
        std::type_index message_type,
        const MessageHandler& handler
    ) const;
};

} // namespace qtplugin::communication
