/**
 * @file resource_lifecycle_impl.hpp
 * @brief Concrete implementation of resource lifecycle management
 * @version 3.0.0
 */

#pragma once

#include <QObject>
#include <QTimer>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include "resource_lifecycle.hpp"

namespace qtplugin {

/**
 * @brief Resource lifecycle tracker
 */
struct ResourceLifecycleTracker {
    ResourceHandle handle;
    LifecycleState current_state = LifecycleState::Created;
    std::chrono::system_clock::time_point state_changed_at;
    std::deque<LifecycleEvent> history;
    QJsonObject metadata;

    ResourceLifecycleTracker() = default;
    explicit ResourceLifecycleTracker(const ResourceHandle& h)
        : handle(h), state_changed_at(std::chrono::system_clock::now()) {}

    void add_event(LifecycleState old_state, LifecycleState new_state,
                   const QJsonObject& meta = {}) {
        LifecycleEvent event(handle.id(), handle.type(), handle.plugin_id(),
                             old_state, new_state, meta);
        history.push_back(event);

        // Keep only recent history (last 100 events)
        if (history.size() > 100) {
            history.pop_front();
        }

        current_state = new_state;
        state_changed_at = std::chrono::system_clock::now();
    }

    std::chrono::milliseconds time_in_current_state() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now() - state_changed_at);
    }
};

/**
 * @brief Lifecycle event subscription
 */
struct LifecycleEventSubscription {
    std::string id;
    std::function<void(const LifecycleEvent&)> callback;
    std::string resource_filter;
    std::optional<LifecycleState> state_filter;

    bool matches(const LifecycleEvent& event) const {
        if (!resource_filter.empty() && event.resource_id != resource_filter) {
            return false;
        }

        if (state_filter && event.new_state != state_filter.value()) {
            return false;
        }

        return true;
    }
};

/**
 * @brief Default resource lifecycle manager implementation
 */
class ResourceLifecycleManager : public QObject,
                                 public IResourceLifecycleManager {
    Q_OBJECT

public:
    explicit ResourceLifecycleManager(QObject* parent = nullptr);
    ~ResourceLifecycleManager() override;

    // IResourceLifecycleManager implementation
    qtplugin::expected<void, PluginError> register_resource(
        const ResourceHandle& handle,
        LifecycleState initial_state = LifecycleState::Created) override;

    qtplugin::expected<void, PluginError> unregister_resource(
        const std::string& resource_id) override;

    qtplugin::expected<void, PluginError> update_state(
        const std::string& resource_id, LifecycleState new_state,
        const QJsonObject& metadata = {}) override;

    qtplugin::expected<LifecycleState, PluginError> get_state(
        const std::string& resource_id) const override;

    qtplugin::expected<void, PluginError> add_dependency(
        const ResourceDependency& dependency) override;

    qtplugin::expected<void, PluginError> remove_dependency(
        const std::string& dependent_id,
        const std::string& dependency_id) override;

    qtplugin::expected<std::vector<ResourceDependency>, PluginError>
    get_dependencies(const std::string& resource_id) const override;

    qtplugin::expected<std::vector<ResourceDependency>, PluginError>
    get_dependents(const std::string& resource_id) const override;

    void set_cleanup_policy(const CleanupPolicy& policy) override;
    CleanupPolicy get_cleanup_policy() const override;
    size_t perform_cleanup() override;

    qtplugin::expected<void, PluginError> force_cleanup(
        const std::string& resource_id, bool force_cleanup = false) override;

    size_t cleanup_plugin_resources(const std::string& plugin_id) override;

    std::string subscribe_to_lifecycle_events(
        std::function<void(const LifecycleEvent&)> callback,
        const std::string& resource_filter = {},
        std::optional<LifecycleState> state_filter = std::nullopt) override;

    qtplugin::expected<void, PluginError> unsubscribe_from_lifecycle_events(
        const std::string& subscription_id) override;

    QJsonObject get_lifecycle_statistics() const override;

    qtplugin::expected<std::vector<LifecycleEvent>, PluginError>
    get_resource_history(const std::string& resource_id,
                         size_t max_events = 100) const override;

    std::vector<std::string> get_resources_in_state(
        LifecycleState state) const override;
    bool can_cleanup_resource(const std::string& resource_id) const override;
    std::vector<std::string> get_cleanup_candidates(
        size_t max_candidates = 100) const override;
    void set_automatic_cleanup_enabled(bool enabled) override;
    bool is_automatic_cleanup_enabled() const override;

signals:
    void resource_state_changed(const QString& resource_id, int old_state,
                                int new_state);
    void resource_cleanup_started(const QString& resource_id);
    void resource_cleanup_completed(const QString& resource_id);
    void cleanup_policy_changed();

private slots:
    void perform_automatic_cleanup();

private:
    // Resource tracking
    std::unordered_map<std::string, std::unique_ptr<ResourceLifecycleTracker>>
        m_tracked_resources;
    mutable std::shared_mutex m_resources_mutex;

    // Dependency tracking
    std::unordered_map<std::string, std::vector<ResourceDependency>>
        m_dependencies;
    std::unordered_map<std::string, std::vector<ResourceDependency>>
        m_dependents;
    mutable std::shared_mutex m_dependencies_mutex;

    // Event subscriptions
    std::unordered_map<std::string, std::unique_ptr<LifecycleEventSubscription>>
        m_event_subscriptions;
    mutable std::shared_mutex m_subscriptions_mutex;

    // Cleanup management
    CleanupPolicy m_cleanup_policy;
    std::unique_ptr<QTimer> m_cleanup_timer;
    std::atomic<bool> m_automatic_cleanup_enabled{true};
    mutable std::shared_mutex m_cleanup_mutex;

    // Statistics
    std::atomic<size_t> m_total_resources_tracked{0};
    std::atomic<size_t> m_total_resources_cleaned{0};
    std::atomic<size_t> m_total_state_transitions{0};

    // Helper methods
    void notify_state_change(const LifecycleEvent& event);
    std::string generate_subscription_id() const;
    bool has_critical_dependents(const std::string& resource_id) const;
    std::vector<std::string> get_cleanup_order(
        const std::vector<std::string>& candidates) const;
    void cleanup_resource_internal(const std::string& resource_id);
    bool is_state_transition_valid(LifecycleState from,
                                   LifecycleState to) const;
};

/**
 * @brief Create a default resource lifecycle manager instance
 * @param parent Parent QObject
 * @return Unique pointer to resource lifecycle manager
 */
std::unique_ptr<IResourceLifecycleManager> create_resource_lifecycle_manager(
    QObject* parent = nullptr);

}  // namespace qtplugin
