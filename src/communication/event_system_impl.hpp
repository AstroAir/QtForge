/**
 * @file event_system_impl.hpp
 * @brief Event system adapter that bridges IEventSystem to TypedEventSystem
 * @version 3.1.0
 *
 * This adapter provides a functional implementation of IEventSystem by
 * delegating to the fully-featured TypedEventSystem. This replaces the previous
 * stub implementation that was non-functional.
 */

#pragma once

#include <QObject>
#include <functional>
#include <memory>
#include <qtplugin/communication/factory.hpp>
#include <qtplugin/communication/interfaces.hpp>
#include <qtplugin/communication/message_types.hpp>
#include <qtplugin/communication/typed_event_system.hpp>
#include <unordered_map>
#include <vector>

namespace qtplugin::communication {

/**
 * @brief Adapter that implements IEventSystem using TypedEventSystem
 *
 * This adapter bridges the factory-based IEventSystem interface to the
 * fully-functional TypedEventSystem implementation. It wraps IMessage
 * events into TypedEvent format for processing.
 *
 * Thread-safe: All operations are thread-safe through TypedEventSystem.
 */
class EventSystemImpl : public IEventSystem {
public:
    explicit EventSystemImpl(
        const CommunicationConfig::EventSystemConfig& config);
    ~EventSystemImpl() override;

protected:
    Result<void> publish_event_impl(std::shared_ptr<IMessage> event) override;

    Result<std::shared_ptr<ISubscription>> subscribe_event_impl(
        std::string_view subscriber_id, std::type_index event_type,
        std::function<void(const IMessage&)> handler,
        std::function<bool(const IMessage&)> filter) override;

private:
    CommunicationConfig::EventSystemConfig config_;
    std::unique_ptr<qtplugin::TypedEventSystem> typed_event_system_;

    // Track subscriptions for cleanup
    mutable QMutex subscriptions_mutex_;
    std::unordered_map<std::string, QString> subscription_id_map_;
};

}  // namespace qtplugin::communication
