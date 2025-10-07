/**
 * @file message_router.hpp
 * @brief Message router implementation for communication system
 * @version 3.0.0
 */

#pragma once

#include <memory>
#include <vector>
#include <qtplugin/communication/interfaces.hpp>

namespace qtplugin::communication {

/**
 * @brief Default implementation of IMessageRouter
 */
class MessageRouter : public IMessageRouter {
public:
    explicit MessageRouter(std::shared_ptr<ISubscriptionManager> subscription_manager);
    ~MessageRouter() override = default;

    Result<std::vector<std::shared_ptr<ISubscription>>> find_subscribers(
        const IMessage& message, DeliveryMode mode,
        const std::vector<std::string>& recipients = {}) const override;

    Result<void> deliver_message(
        const IMessage& message,
        const std::vector<std::shared_ptr<ISubscription>>& subscriptions) override;

private:
    std::shared_ptr<ISubscriptionManager> subscription_manager_;
};

}  // namespace qtplugin::communication