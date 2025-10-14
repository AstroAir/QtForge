/**
 * @file typed_event_system.cpp
 * @brief Implementation of typed event system for plugin communication
 * @version 3.0.0
 */

#include "qtplugin/communication/typed_event_system.hpp"

#include <QDebug>
#include <QLoggingCategory>
#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QTimer>
#include <QUuid>
#include <QWaitCondition>

#include <algorithm>
#include <chrono>
#include <queue>
#include <unordered_map>
#include <unordered_set>

Q_LOGGING_CATEGORY(typedEventLog, "qtplugin.typed_event")

namespace qtplugin {

// Private implementation class
class TypedEventSystem::Private {
public:
    explicit Private(TypedEventSystem* owner) : q(owner) {
        // Initialize timers
        queued_timer = new QTimer(q);
        deferred_timer = new QTimer(q);
        batched_timer = new QTimer(q);

        // Connect timer signals
        QObject::connect(queued_timer, &QTimer::timeout, q,
                         &TypedEventSystem::process_queued_events);
        QObject::connect(deferred_timer, &QTimer::timeout, q,
                         &TypedEventSystem::process_deferred_events);
        QObject::connect(batched_timer, &QTimer::timeout, q,
                         &TypedEventSystem::process_batched_events);

        // Configure timers
        queued_timer->setInterval(10);  // Process queued events every 10ms
        deferred_timer->setInterval(
            100);                        // Process deferred events every 100ms
        batched_timer->setInterval(50);  // Process batched events every 50ms

        queued_timer->start();
        deferred_timer->start();
        batched_timer->start();
    }

    ~Private() = default;

    struct EventSubscription {
        QString subscription_id;
        QString subscriber_id;
        QString event_type;
        std::function<void(const IEvent&)> handler;
        std::function<bool(const IEvent&)> filter;
        EventPriority min_priority;
        bool enabled = true;
        std::chrono::system_clock::time_point created_at;
    };

    struct PendingEvent {
        std::unique_ptr<IEvent> event;
        EventDeliveryMode delivery_mode;
        EventRoutingMode routing_mode;
        std::vector<QString> recipients;
        std::chrono::system_clock::time_point created_at;
    };

    TypedEventSystem* q;

    // Event storage
    mutable QMutex events_mutex;
    std::queue<PendingEvent> queued_events;
    std::queue<PendingEvent> deferred_events;
    std::vector<PendingEvent> batched_events;

    // Subscription management
    mutable QMutex subscriptions_mutex;
    std::unordered_map<QString, EventSubscription> subscriptions;
    std::unordered_map<QString, std::vector<QString>>
        type_index;  // event_type -> subscription_ids
    std::unordered_map<QString, std::vector<QString>>
        subscriber_index;  // subscriber_id -> subscription_ids

    // Statistics
    mutable QMutex stats_mutex;
    EventStatistics statistics;

    // Event history
    bool history_enabled = false;
    size_t max_history_size = 1000;
    std::vector<QJsonObject> event_history;

    // Timers
    QTimer* queued_timer;
    QTimer* deferred_timer;
    QTimer* batched_timer;

    // Helper methods
    QString generate_subscription_id() {
        return QUuid::createUuid().toString(QUuid::WithoutBraces);
    }

    QString get_event_id(const IEvent& event) {
        // Try to extract event_id from JSON representation
        QJsonObject json = event.to_json();
        if (json.contains("event_id")) {
            return json["event_id"].toString();
        }
        // Fallback to generating a new ID
        return QUuid::createUuid().toString();
    }

    std::vector<QString> find_matching_subscriptions(const IEvent& event) {
        std::vector<QString> matching_subscriptions;

        QString event_type = event.event_type();
        auto type_it = type_index.find(event_type);
        if (type_it != type_index.end()) {
            for (const QString& subscription_id : type_it->second) {
                auto sub_it = subscriptions.find(subscription_id);
                if (sub_it != subscriptions.end() && sub_it->second.enabled) {
                    const auto& subscription = sub_it->second;

                    // Check priority
                    if (event.priority() < subscription.min_priority) {
                        continue;
                    }

                    // Check filter
                    if (subscription.filter && !subscription.filter(event)) {
                        continue;
                    }

                    matching_subscriptions.push_back(subscription_id);
                }
            }
        }

        return matching_subscriptions;
    }

    EventDeliveryResult deliver_to_subscriptions(
        const IEvent& event, const std::vector<QString>& subscription_ids) {
        EventDeliveryResult result;
        result.event_id = get_event_id(event);
        result.success = true;

        auto start_time = std::chrono::steady_clock::now();

        for (const QString& subscription_id : subscription_ids) {
            auto sub_it = subscriptions.find(subscription_id);
            if (sub_it != subscriptions.end()) {
                try {
                    sub_it->second.handler(event);
                    result.delivered_count++;
                    result.delivered_to.push_back(sub_it->second.subscriber_id);

                    emit q->event_delivered(result.event_id,
                                            sub_it->second.subscriber_id, true);
                } catch (const std::exception& e) {
                    result.failed_count++;
                    result.failed_to.push_back(sub_it->second.subscriber_id);
                    result.error_message = QString::fromStdString(e.what());

                    emit q->event_delivered(
                        result.event_id, sub_it->second.subscriber_id, false);

                    qCWarning(typedEventLog)
                        << "Event delivery failed:" << e.what();
                }
            }
        }

        auto end_time = std::chrono::steady_clock::now();
        result.delivery_time =
            std::chrono::duration_cast<std::chrono::milliseconds>(end_time -
                                                                  start_time);

        if (result.failed_count > 0) {
            result.success = false;
        }

        return result;
    }

    void record_event_in_history(const IEvent& event) {
        if (!history_enabled)
            return;

        QJsonObject event_json = event.to_json();
        event_json["recorded_at"] =
            QDateTime::currentDateTime().toString(Qt::ISODate);

        event_history.push_back(event_json);

        // Maintain history size limit
        if (event_history.size() > max_history_size) {
            event_history.erase(event_history.begin(),
                                event_history.begin() +
                                    (event_history.size() - max_history_size));
        }
    }

    void update_statistics(const EventDeliveryResult& result) {
        QMutexLocker locker(&stats_mutex);

        statistics.total_events_published++;
        statistics.total_events_delivered += result.delivered_count;
        statistics.total_events_failed += result.failed_count;

        // Update average delivery time
        if (statistics.total_events_published > 0) {
            auto total_time = statistics.average_delivery_time.count() *
                                  (statistics.total_events_published - 1) +
                              result.delivery_time.count();
            statistics.average_delivery_time = std::chrono::milliseconds(
                total_time / statistics.total_events_published);
        }
    }
};

// TypedEventSystem implementation
TypedEventSystem::TypedEventSystem(QObject* parent)
    : QObject(parent), d(std::make_unique<Private>(this)) {
    qCDebug(typedEventLog) << "TypedEventSystem created";
}

TypedEventSystem::~TypedEventSystem() {
    qCDebug(typedEventLog) << "TypedEventSystem destroyed";
}

qtplugin::expected<EventDeliveryResult, PluginError>
TypedEventSystem::publish_event(std::unique_ptr<IEvent> event,
                                EventDeliveryMode delivery_mode,
                                EventRoutingMode routing_mode,
                                const std::vector<QString>& recipients) {
    if (!event) {
        return qtplugin::unexpected(
            PluginError(PluginErrorCode::InvalidArgument, "Event is null"));
    }

    qCDebug(typedEventLog) << "Publishing event:" << event->event_type()
                           << "from" << event->source();

    emit event_published(event->event_type(), event->source(),
                         d->get_event_id(*event));

    // Record in history
    d->record_event_in_history(*event);

    EventDeliveryResult result;

    switch (delivery_mode) {
        case EventDeliveryMode::Immediate: {
            QMutexLocker locker(&d->subscriptions_mutex);
            auto matching_subscriptions =
                d->find_matching_subscriptions(*event);
            locker.unlock();

            result =
                d->deliver_to_subscriptions(*event, matching_subscriptions);
            break;
        }

        case EventDeliveryMode::Queued: {
            QMutexLocker locker(&d->events_mutex);
            Private::PendingEvent pending;
            pending.event = std::move(event);
            pending.delivery_mode = delivery_mode;
            pending.routing_mode = routing_mode;
            pending.recipients = recipients;
            pending.created_at = std::chrono::system_clock::now();

            d->queued_events.push(std::move(pending));

            result.success = true;
            result.event_id = d->get_event_id(*event);
            break;
        }

        case EventDeliveryMode::Deferred: {
            QMutexLocker locker(&d->events_mutex);
            Private::PendingEvent pending;
            pending.event = std::move(event);
            pending.delivery_mode = delivery_mode;
            pending.routing_mode = routing_mode;
            pending.recipients = recipients;
            pending.created_at = std::chrono::system_clock::now();

            d->deferred_events.push(std::move(pending));

            result.success = true;
            result.event_id = d->get_event_id(*event);
            break;
        }

        case EventDeliveryMode::Batched: {
            QMutexLocker locker(&d->events_mutex);
            Private::PendingEvent pending;
            pending.event = std::move(event);
            pending.delivery_mode = delivery_mode;
            pending.routing_mode = routing_mode;
            pending.recipients = recipients;
            pending.created_at = std::chrono::system_clock::now();

            d->batched_events.push_back(std::move(pending));

            result.success = true;
            result.event_id = d->get_event_id(*event);
            break;
        }
    }

    d->update_statistics(result);
    return result;
}

std::future<qtplugin::expected<EventDeliveryResult, PluginError>>
TypedEventSystem::publish_event_async(std::unique_ptr<IEvent> event,
                                      EventDeliveryMode delivery_mode,
                                      EventRoutingMode routing_mode,
                                      const std::vector<QString>& recipients) {
    return std::async(std::launch::async,
                      [this, event = std::move(event), delivery_mode,
                       routing_mode, recipients]() mutable {
                          return publish_event(std::move(event), delivery_mode,
                                               routing_mode, recipients);
                      });
}

std::vector<qtplugin::expected<EventDeliveryResult, PluginError>>
TypedEventSystem::publish_batch(std::vector<std::unique_ptr<IEvent>> events,
                                EventDeliveryMode delivery_mode) {
    std::vector<qtplugin::expected<EventDeliveryResult, PluginError>> results;
    results.reserve(events.size());

    for (auto& event : events) {
        results.push_back(publish_event(std::move(event), delivery_mode));
    }

    return results;
}

qtplugin::expected<QString, PluginError> TypedEventSystem::subscribe(
    const QString& subscriber_id, const QString& event_type,
    std::function<void(const IEvent&)> handler,
    std::function<bool(const IEvent&)> filter, EventPriority min_priority) {
    if (subscriber_id.isEmpty() || event_type.isEmpty() || !handler) {
        return qtplugin::unexpected(
            PluginError(PluginErrorCode::InvalidArgument,
                        "Invalid subscription parameters"));
    }

    QMutexLocker locker(&d->subscriptions_mutex);

    QString subscription_id = d->generate_subscription_id();

    Private::EventSubscription subscription;
    subscription.subscription_id = subscription_id;
    subscription.subscriber_id = subscriber_id;
    subscription.event_type = event_type;
    subscription.handler = std::move(handler);
    subscription.filter = std::move(filter);
    subscription.min_priority = min_priority;
    subscription.created_at = std::chrono::system_clock::now();

    d->subscriptions[subscription_id] = std::move(subscription);
    d->type_index[event_type].push_back(subscription_id);
    d->subscriber_index[subscriber_id].push_back(subscription_id);

    qCDebug(typedEventLog) << "Created subscription:" << subscription_id
                           << "for" << subscriber_id << "to" << event_type;

    emit subscription_created(subscription_id, subscriber_id, event_type);

    return subscription_id;
}

qtplugin::expected<void, PluginError> TypedEventSystem::unsubscribe(
    const QString& subscription_id) {
    if (subscription_id.isEmpty()) {
        return qtplugin::unexpected(PluginError(
            PluginErrorCode::InvalidArgument, "Subscription ID is empty"));
    }

    QMutexLocker locker(&d->subscriptions_mutex);

    auto sub_it = d->subscriptions.find(subscription_id);
    if (sub_it == d->subscriptions.end()) {
        return qtplugin::unexpected(
            PluginError(PluginErrorCode::NotFound, "Subscription not found"));
    }

    const auto& subscription = sub_it->second;
    QString event_type = subscription.event_type;
    QString subscriber_id = subscription.subscriber_id;

    // Remove from type index
    auto type_it = d->type_index.find(event_type);
    if (type_it != d->type_index.end()) {
        auto& subscription_ids = type_it->second;
        subscription_ids.erase(
            std::remove(subscription_ids.begin(), subscription_ids.end(),
                        subscription_id),
            subscription_ids.end());

        if (subscription_ids.empty()) {
            d->type_index.erase(type_it);
        }
    }

    // Remove from subscriber index
    auto subscriber_it = d->subscriber_index.find(subscriber_id);
    if (subscriber_it != d->subscriber_index.end()) {
        auto& subscription_ids = subscriber_it->second;
        subscription_ids.erase(
            std::remove(subscription_ids.begin(), subscription_ids.end(),
                        subscription_id),
            subscription_ids.end());

        if (subscription_ids.empty()) {
            d->subscriber_index.erase(subscriber_it);
        }
    }

    d->subscriptions.erase(sub_it);

    qCDebug(typedEventLog) << "Removed subscription:" << subscription_id;

    emit subscription_removed(subscription_id);

    return {};
}

int TypedEventSystem::unsubscribe_all(const QString& subscriber_id) {
    if (subscriber_id.isEmpty()) {
        return 0;
    }

    QMutexLocker locker(&d->subscriptions_mutex);

    auto subscriber_it = d->subscriber_index.find(subscriber_id);
    if (subscriber_it == d->subscriber_index.end()) {
        return 0;
    }

    auto subscription_ids = subscriber_it->second;  // Copy the list
    locker.unlock();

    int removed_count = 0;
    for (const QString& subscription_id : subscription_ids) {
        auto result = unsubscribe(subscription_id);
        if (result.has_value()) {
            removed_count++;
        }
    }

    return removed_count;
}

std::vector<EventSubscription> TypedEventSystem::get_subscriptions(
    const QString& subscriber_id) const {
    std::vector<EventSubscription> result;

    QMutexLocker locker(&d->subscriptions_mutex);

    auto subscriber_it = d->subscriber_index.find(subscriber_id);
    if (subscriber_it != d->subscriber_index.end()) {
        for (const QString& subscription_id : subscriber_it->second) {
            auto sub_it = d->subscriptions.find(subscription_id);
            if (sub_it != d->subscriptions.end()) {
                EventSubscription info;
                info.subscription_id = subscription_id;
                info.subscriber_id = sub_it->second.subscriber_id;
                info.event_type = sub_it->second.event_type;
                info.min_priority = sub_it->second.min_priority;
                info.is_active = sub_it->second.enabled;
                info.created_time = sub_it->second.created_at;

                result.push_back(info);
            }
        }
    }

    return result;
}

qtplugin::expected<void, PluginError>
TypedEventSystem::set_subscription_enabled(const QString& subscription_id,
                                           bool enabled) {
    if (subscription_id.isEmpty()) {
        return qtplugin::unexpected(PluginError(
            PluginErrorCode::InvalidArgument, "Subscription ID is empty"));
    }

    QMutexLocker locker(&d->subscriptions_mutex);

    auto sub_it = d->subscriptions.find(subscription_id);
    if (sub_it == d->subscriptions.end()) {
        return qtplugin::unexpected(
            PluginError(PluginErrorCode::NotFound, "Subscription not found"));
    }

    sub_it->second.enabled = enabled;

    qCDebug(typedEventLog) << "Subscription" << subscription_id
                           << (enabled ? "enabled" : "disabled");

    return {};
}

size_t TypedEventSystem::get_pending_events_count() const {
    QMutexLocker locker(&d->events_mutex);
    return d->queued_events.size() + d->deferred_events.size() +
           d->batched_events.size();
}

size_t TypedEventSystem::process_pending_events(size_t max_events) {
    size_t processed = 0;

    // Process queued events first
    QMutexLocker locker(&d->events_mutex);
    while (!d->queued_events.empty() && processed < max_events) {
        auto pending = std::move(d->queued_events.front());
        d->queued_events.pop();
        locker.unlock();

        QMutexLocker sub_locker(&d->subscriptions_mutex);
        auto matching_subscriptions =
            d->find_matching_subscriptions(*pending.event);
        sub_locker.unlock();

        d->deliver_to_subscriptions(*pending.event, matching_subscriptions);
        processed++;

        locker.relock();
    }

    return processed;
}

size_t TypedEventSystem::clear_pending_events(const QString& event_type) {
    QMutexLocker locker(&d->events_mutex);

    size_t cleared = 0;

    // Clear queued events
    std::queue<Private::PendingEvent> filtered_queue;
    while (!d->queued_events.empty()) {
        auto& pending = d->queued_events.front();
        if (event_type.isEmpty() || pending.event->event_type() == event_type) {
            cleared++;
        } else {
            filtered_queue.push(std::move(pending));
        }
        d->queued_events.pop();
    }
    d->queued_events = std::move(filtered_queue);

    // Clear deferred events
    std::queue<Private::PendingEvent> filtered_deferred;
    while (!d->deferred_events.empty()) {
        auto& pending = d->deferred_events.front();
        if (event_type.isEmpty() || pending.event->event_type() == event_type) {
            cleared++;
        } else {
            filtered_deferred.push(std::move(pending));
        }
        d->deferred_events.pop();
    }
    d->deferred_events = std::move(filtered_deferred);

    // Clear batched events
    auto it = d->batched_events.begin();
    while (it != d->batched_events.end()) {
        if (event_type.isEmpty() || it->event->event_type() == event_type) {
            it = d->batched_events.erase(it);
            cleared++;
        } else {
            ++it;
        }
    }

    return cleared;
}

EventStatistics TypedEventSystem::get_statistics() const {
    QMutexLocker locker(&d->stats_mutex);
    return d->statistics;
}

void TypedEventSystem::reset_statistics() {
    QMutexLocker locker(&d->stats_mutex);
    d->statistics = EventStatistics{};
}

void TypedEventSystem::set_event_history_enabled(bool enabled,
                                                 size_t max_history_size) {
    d->history_enabled = enabled;
    d->max_history_size = max_history_size;

    if (!enabled) {
        d->event_history.clear();
    }
}

std::vector<QJsonObject> TypedEventSystem::get_event_history(
    const QString& event_type, size_t max_events) const {
    std::vector<QJsonObject> result;

    for (const auto& event_json : d->event_history) {
        if (event_type.isEmpty() ||
            event_json["event_type"].toString() == event_type) {
            result.push_back(event_json);
            if (result.size() >= max_events) {
                break;
            }
        }
    }

    return result;
}

// Private slots implementation
void TypedEventSystem::process_queued_events() {
    process_pending_events(10);  // Process up to 10 events per timer tick
}

void TypedEventSystem::process_deferred_events() {
    QMutexLocker locker(&d->events_mutex);

    auto now = std::chrono::system_clock::now();

    while (!d->deferred_events.empty()) {
        auto& pending = d->deferred_events.front();

        // Check if enough time has passed (e.g., 100ms delay)
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - pending.created_at);

        if (elapsed.count() < 100) {
            break;  // Not ready yet
        }

        auto event = std::move(pending.event);
        d->deferred_events.pop();
        locker.unlock();

        QMutexLocker sub_locker(&d->subscriptions_mutex);
        auto matching_subscriptions = d->find_matching_subscriptions(*event);
        sub_locker.unlock();

        d->deliver_to_subscriptions(*event, matching_subscriptions);

        locker.relock();
    }
}

void TypedEventSystem::process_batched_events() {
    QMutexLocker locker(&d->events_mutex);

    if (d->batched_events.empty()) {
        return;
    }

    // Process all batched events at once
    std::vector<Private::PendingEvent> events_to_process;
    events_to_process.swap(d->batched_events);
    locker.unlock();

    for (auto& pending : events_to_process) {
        QMutexLocker sub_locker(&d->subscriptions_mutex);
        auto matching_subscriptions =
            d->find_matching_subscriptions(*pending.event);
        sub_locker.unlock();

        d->deliver_to_subscriptions(*pending.event, matching_subscriptions);
    }
}

}  // namespace qtplugin
