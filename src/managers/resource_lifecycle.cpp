/**
 * @file resource_lifecycle.cpp
 * @brief Implementation of resource lifecycle management
 * @version 3.0.0
 */

#include "qtplugin/managers/resource_lifecycle_impl.hpp"
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QLoggingCategory>
#include <algorithm>
#include <random>

Q_LOGGING_CATEGORY(lifecycleLog, "qtplugin.lifecycle")

namespace qtplugin {

// === Utility Functions ===

std::string lifecycle_state_to_string(LifecycleState state) {
    switch (state) {
        case LifecycleState::Created: return "created";
        case LifecycleState::Initialized: return "initialized";
        case LifecycleState::Active: return "active";
        case LifecycleState::Idle: return "idle";
        case LifecycleState::Deprecated: return "deprecated";
        case LifecycleState::Cleanup: return "cleanup";
        case LifecycleState::Destroyed: return "destroyed";
    }
    return "unknown";
}

std::optional<LifecycleState> string_to_lifecycle_state(std::string_view str) {
    if (str == "created") return LifecycleState::Created;
    if (str == "initialized") return LifecycleState::Initialized;
    if (str == "active") return LifecycleState::Active;
    if (str == "idle") return LifecycleState::Idle;
    if (str == "deprecated") return LifecycleState::Deprecated;
    if (str == "cleanup") return LifecycleState::Cleanup;
    if (str == "destroyed") return LifecycleState::Destroyed;
    return std::nullopt;
}

bool is_valid_state_transition(LifecycleState from, LifecycleState to) {
    // Define valid state transitions
    switch (from) {
        case LifecycleState::Created:
            return to == LifecycleState::Initialized || to == LifecycleState::Cleanup || to == LifecycleState::Destroyed;
        case LifecycleState::Initialized:
            return to == LifecycleState::Active || to == LifecycleState::Idle || to == LifecycleState::Cleanup || to == LifecycleState::Destroyed;
        case LifecycleState::Active:
            return to == LifecycleState::Idle || to == LifecycleState::Deprecated || to == LifecycleState::Cleanup || to == LifecycleState::Destroyed;
        case LifecycleState::Idle:
            return to == LifecycleState::Active || to == LifecycleState::Deprecated || to == LifecycleState::Cleanup || to == LifecycleState::Destroyed;
        case LifecycleState::Deprecated:
            return to == LifecycleState::Cleanup || to == LifecycleState::Destroyed;
        case LifecycleState::Cleanup:
            return to == LifecycleState::Destroyed;
        case LifecycleState::Destroyed:
            return false; // No transitions from destroyed state
    }
    return false;
}

std::vector<LifecycleState> get_valid_next_states(LifecycleState current) {
    std::vector<LifecycleState> valid_states;
    
    for (int i = 0; i <= static_cast<int>(LifecycleState::Destroyed); ++i) {
        LifecycleState state = static_cast<LifecycleState>(i);
        if (is_valid_state_transition(current, state)) {
            valid_states.push_back(state);
        }
    }
    
    return valid_states;
}

// === ResourceLifecycleManager Implementation ===

ResourceLifecycleManager::ResourceLifecycleManager(QObject* parent)
    : QObject(parent)
    , m_cleanup_timer(std::make_unique<QTimer>(this)) {
    
    // Set up automatic cleanup timer
    m_cleanup_timer->setSingleShot(false);
    m_cleanup_timer->setInterval(60000); // 1 minute default
    connect(m_cleanup_timer.get(), &QTimer::timeout, this, &ResourceLifecycleManager::perform_automatic_cleanup);
    m_cleanup_timer->start();
    
    qCDebug(lifecycleLog) << "Resource lifecycle manager initialized";
}

ResourceLifecycleManager::~ResourceLifecycleManager() {
    // Stop cleanup timer
    m_cleanup_timer->stop();
    
    // Clean up all tracked resources
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);
    m_tracked_resources.clear();
    
    qCDebug(lifecycleLog) << "Resource lifecycle manager destroyed";
}

qtplugin::expected<void, PluginError>
ResourceLifecycleManager::register_resource(const ResourceHandle& handle, LifecycleState initial_state) {
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);
    
    const std::string& resource_id = handle.id();
    
    // Check if resource is already tracked
    if (m_tracked_resources.find(resource_id) != m_tracked_resources.end()) {
        return make_error<void>(PluginErrorCode::AlreadyExists, 
                               "Resource already tracked: " + resource_id);
    }
    
    // Create tracker
    auto tracker = std::make_unique<ResourceLifecycleTracker>(handle);
    tracker->add_event(LifecycleState::Created, initial_state);
    
    m_tracked_resources[resource_id] = std::move(tracker);
    m_total_resources_tracked.fetch_add(1);
    
    // Notify subscribers
    LifecycleEvent event(resource_id, handle.type(), handle.plugin_id(), 
                        LifecycleState::Created, initial_state);
    notify_state_change(event);
    
    qCDebug(lifecycleLog) << "Registered resource:" << QString::fromStdString(resource_id)
                         << "initial state:" << QString::fromStdString(lifecycle_state_to_string(initial_state));
    
    return make_success();
}

qtplugin::expected<void, PluginError>
ResourceLifecycleManager::unregister_resource(const std::string& resource_id) {
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);
    
    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<void>(PluginErrorCode::NotFound, 
                               "Resource not found: " + resource_id);
    }
    
    // Update state to destroyed before removing
    auto& tracker = it->second;
    LifecycleState old_state = tracker->current_state;
    tracker->add_event(old_state, LifecycleState::Destroyed);
    
    // Notify subscribers
    LifecycleEvent event(resource_id, tracker->handle.type(), tracker->handle.plugin_id(), 
                        old_state, LifecycleState::Destroyed);
    notify_state_change(event);
    
    // Remove from tracking
    m_tracked_resources.erase(it);
    
    qCDebug(lifecycleLog) << "Unregistered resource:" << QString::fromStdString(resource_id);
    
    return make_success();
}

qtplugin::expected<void, PluginError>
ResourceLifecycleManager::update_state(const std::string& resource_id, LifecycleState new_state, 
                                      const QJsonObject& metadata) {
    std::unique_lock<std::shared_mutex> lock(m_resources_mutex);
    
    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<void>(PluginErrorCode::NotFound, 
                               "Resource not found: " + resource_id);
    }
    
    auto& tracker = it->second;
    LifecycleState old_state = tracker->current_state;
    
    // Validate state transition
    if (!is_state_transition_valid(old_state, new_state)) {
        return make_error<void>(PluginErrorCode::InvalidArgument, 
                               "Invalid state transition from " + lifecycle_state_to_string(old_state) +
                               " to " + lifecycle_state_to_string(new_state));
    }
    
    // Update state
    tracker->add_event(old_state, new_state, metadata);
    tracker->metadata = metadata;
    m_total_state_transitions.fetch_add(1);
    
    // Notify subscribers
    LifecycleEvent event(resource_id, tracker->handle.type(), tracker->handle.plugin_id(), 
                        old_state, new_state, metadata);
    notify_state_change(event);
    
    emit resource_state_changed(QString::fromStdString(resource_id), 
                               static_cast<int>(old_state), static_cast<int>(new_state));
    
    qCDebug(lifecycleLog) << "Updated resource state:" << QString::fromStdString(resource_id)
                         << "from" << QString::fromStdString(lifecycle_state_to_string(old_state))
                         << "to" << QString::fromStdString(lifecycle_state_to_string(new_state));
    
    return make_success();
}

qtplugin::expected<LifecycleState, PluginError>
ResourceLifecycleManager::get_state(const std::string& resource_id) const {
    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);
    
    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<LifecycleState>(PluginErrorCode::NotFound, 
                                         "Resource not found: " + resource_id);
    }
    
    return it->second->current_state;
}

qtplugin::expected<void, PluginError>
ResourceLifecycleManager::add_dependency(const ResourceDependency& dependency) {
    std::unique_lock<std::shared_mutex> lock(m_dependencies_mutex);
    
    // Add to dependencies map
    m_dependencies[dependency.dependent_id].push_back(dependency);
    
    // Add to dependents map
    ResourceDependency reverse_dep;
    reverse_dep.dependent_id = dependency.dependency_id;
    reverse_dep.dependency_id = dependency.dependent_id;
    reverse_dep.relationship_type = dependency.relationship_type;
    reverse_dep.is_critical = dependency.is_critical;
    
    m_dependents[dependency.dependency_id].push_back(reverse_dep);
    
    qCDebug(lifecycleLog) << "Added dependency:" << QString::fromStdString(dependency.dependent_id)
                         << "depends on" << QString::fromStdString(dependency.dependency_id);
    
    return make_success();
}

qtplugin::expected<void, PluginError>
ResourceLifecycleManager::remove_dependency(const std::string& dependent_id, const std::string& dependency_id) {
    std::unique_lock<std::shared_mutex> lock(m_dependencies_mutex);
    
    // Remove from dependencies map
    auto deps_it = m_dependencies.find(dependent_id);
    if (deps_it != m_dependencies.end()) {
        auto& deps = deps_it->second;
        deps.erase(std::remove_if(deps.begin(), deps.end(),
                                 [&dependency_id](const ResourceDependency& dep) {
                                     return dep.dependency_id == dependency_id;
                                 }), deps.end());
        
        if (deps.empty()) {
            m_dependencies.erase(deps_it);
        }
    }
    
    // Remove from dependents map
    auto dependents_it = m_dependents.find(dependency_id);
    if (dependents_it != m_dependents.end()) {
        auto& dependents = dependents_it->second;
        dependents.erase(std::remove_if(dependents.begin(), dependents.end(),
                                       [&dependent_id](const ResourceDependency& dep) {
                                           return dep.dependency_id == dependent_id;
                                       }), dependents.end());
        
        if (dependents.empty()) {
            m_dependents.erase(dependents_it);
        }
    }
    
    qCDebug(lifecycleLog) << "Removed dependency:" << QString::fromStdString(dependent_id)
                         << "no longer depends on" << QString::fromStdString(dependency_id);
    
    return make_success();
}

qtplugin::expected<std::vector<ResourceDependency>, PluginError>
ResourceLifecycleManager::get_dependencies(const std::string& resource_id) const {
    std::shared_lock<std::shared_mutex> lock(m_dependencies_mutex);
    
    auto it = m_dependencies.find(resource_id);
    if (it == m_dependencies.end()) {
        return std::vector<ResourceDependency>{}; // Return empty vector if no dependencies
    }
    
    return it->second;
}

qtplugin::expected<std::vector<ResourceDependency>, PluginError>
ResourceLifecycleManager::get_dependents(const std::string& resource_id) const {
    std::shared_lock<std::shared_mutex> lock(m_dependencies_mutex);
    
    auto it = m_dependents.find(resource_id);
    if (it == m_dependents.end()) {
        return std::vector<ResourceDependency>{}; // Return empty vector if no dependents
    }
    
    return it->second;
}

void ResourceLifecycleManager::set_cleanup_policy(const CleanupPolicy& policy) {
    std::unique_lock<std::shared_mutex> lock(m_cleanup_mutex);
    m_cleanup_policy = policy;
    emit cleanup_policy_changed();
    
    qCDebug(lifecycleLog) << "Updated cleanup policy - max idle time:" 
                         << m_cleanup_policy.max_idle_time.count() << "ms";
}

CleanupPolicy ResourceLifecycleManager::get_cleanup_policy() const {
    std::shared_lock<std::shared_mutex> lock(m_cleanup_mutex);
    return m_cleanup_policy;
}

size_t ResourceLifecycleManager::perform_cleanup() {
    if (!m_automatic_cleanup_enabled.load()) {
        return 0;
    }
    
    auto candidates = get_cleanup_candidates(1000); // Get up to 1000 candidates
    if (candidates.empty()) {
        return 0;
    }
    
    // Order candidates for safe cleanup (dependencies first)
    auto ordered_candidates = get_cleanup_order(candidates);
    
    size_t cleaned = 0;
    for (const auto& resource_id : ordered_candidates) {
        if (can_cleanup_resource(resource_id)) {
            cleanup_resource_internal(resource_id);
            cleaned++;
        }
    }
    
    m_total_resources_cleaned.fetch_add(cleaned);
    
    if (cleaned > 0) {
        qCDebug(lifecycleLog) << "Automatic cleanup completed, cleaned" << cleaned << "resources";
    }
    
    return cleaned;
}

qtplugin::expected<void, PluginError>
ResourceLifecycleManager::force_cleanup(const std::string& resource_id, bool force_cleanup) {
    std::shared_lock<std::shared_mutex> resources_lock(m_resources_mutex);

    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                               "Resource not found: " + resource_id);
    }

    if (!force_cleanup && has_critical_dependents(resource_id)) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                               "Resource has critical dependents: " + resource_id);
    }

    resources_lock.unlock();

    cleanup_resource_internal(resource_id);

    qCDebug(lifecycleLog) << "Force cleanup completed for resource:" << QString::fromStdString(resource_id);

    return make_success();
}

size_t ResourceLifecycleManager::cleanup_plugin_resources(const std::string& plugin_id) {
    std::vector<std::string> plugin_resources;

    {
        std::shared_lock<std::shared_mutex> lock(m_resources_mutex);
        for (const auto& [resource_id, tracker] : m_tracked_resources) {
            if (tracker->handle.plugin_id() == plugin_id) {
                plugin_resources.push_back(resource_id);
            }
        }
    }

    size_t cleaned = 0;
    for (const auto& resource_id : plugin_resources) {
        cleanup_resource_internal(resource_id);
        cleaned++;
    }

    qCDebug(lifecycleLog) << "Cleaned up" << cleaned << "resources for plugin:"
                         << QString::fromStdString(plugin_id);

    return cleaned;
}

std::string ResourceLifecycleManager::subscribe_to_lifecycle_events(
    std::function<void(const LifecycleEvent&)> callback,
    const std::string& resource_filter, std::optional<LifecycleState> state_filter) {

    std::unique_lock<std::shared_mutex> lock(m_subscriptions_mutex);

    std::string subscription_id = generate_subscription_id();

    auto subscription = std::make_unique<LifecycleEventSubscription>();
    subscription->id = subscription_id;
    subscription->callback = std::move(callback);
    subscription->resource_filter = resource_filter;
    subscription->state_filter = state_filter;

    m_event_subscriptions[subscription_id] = std::move(subscription);

    qCDebug(lifecycleLog) << "Created lifecycle event subscription:" << QString::fromStdString(subscription_id);

    return subscription_id;
}

qtplugin::expected<void, PluginError>
ResourceLifecycleManager::unsubscribe_from_lifecycle_events(const std::string& subscription_id) {
    std::unique_lock<std::shared_mutex> lock(m_subscriptions_mutex);

    auto it = m_event_subscriptions.find(subscription_id);
    if (it == m_event_subscriptions.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                               "Event subscription not found: " + subscription_id);
    }

    m_event_subscriptions.erase(it);

    qCDebug(lifecycleLog) << "Removed lifecycle event subscription:" << QString::fromStdString(subscription_id);

    return make_success();
}

QJsonObject ResourceLifecycleManager::get_lifecycle_statistics() const {
    QJsonObject stats;

    stats["total_resources_tracked"] = static_cast<qint64>(m_total_resources_tracked.load());
    stats["total_resources_cleaned"] = static_cast<qint64>(m_total_resources_cleaned.load());
    stats["total_state_transitions"] = static_cast<qint64>(m_total_state_transitions.load());
    stats["automatic_cleanup_enabled"] = m_automatic_cleanup_enabled.load();

    // Current resource counts by state
    QJsonObject state_counts;
    {
        std::shared_lock<std::shared_mutex> lock(m_resources_mutex);
        stats["currently_tracked"] = static_cast<qint64>(m_tracked_resources.size());

        std::unordered_map<LifecycleState, size_t> counts;
        for (const auto& [resource_id, tracker] : m_tracked_resources) {
            counts[tracker->current_state]++;
        }

        for (const auto& [state, count] : counts) {
            state_counts[QString::fromStdString(lifecycle_state_to_string(state))] = static_cast<qint64>(count);
        }
    }
    stats["state_counts"] = state_counts;

    // Dependency statistics
    {
        std::shared_lock<std::shared_mutex> lock(m_dependencies_mutex);
        stats["total_dependencies"] = static_cast<qint64>(m_dependencies.size());
        stats["total_dependents"] = static_cast<qint64>(m_dependents.size());
    }

    // Subscription statistics
    {
        std::shared_lock<std::shared_mutex> lock(m_subscriptions_mutex);
        stats["event_subscriptions"] = static_cast<qint64>(m_event_subscriptions.size());
    }

    return stats;
}

qtplugin::expected<std::vector<LifecycleEvent>, PluginError>
ResourceLifecycleManager::get_resource_history(const std::string& resource_id, size_t max_events) const {
    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return make_error<std::vector<LifecycleEvent>>(PluginErrorCode::NotFound,
                                                       "Resource not found: " + resource_id);
    }

    const auto& history = it->second->history;
    std::vector<LifecycleEvent> result;

    size_t count = std::min(max_events, history.size());
    result.reserve(count);

    // Return most recent events
    auto start_it = history.end() - static_cast<std::ptrdiff_t>(count);
    result.assign(start_it, history.end());

    return result;
}

std::vector<std::string> ResourceLifecycleManager::get_resources_in_state(LifecycleState state) const {
    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    std::vector<std::string> resources;
    for (const auto& [resource_id, tracker] : m_tracked_resources) {
        if (tracker->current_state == state) {
            resources.push_back(resource_id);
        }
    }

    return resources;
}

bool ResourceLifecycleManager::can_cleanup_resource(const std::string& resource_id) const {
    std::shared_lock<std::shared_mutex> resources_lock(m_resources_mutex);

    auto it = m_tracked_resources.find(resource_id);
    if (it == m_tracked_resources.end()) {
        return false;
    }

    const auto& tracker = it->second;

    // Check if resource is in a cleanable state
    if (tracker->current_state == LifecycleState::Cleanup ||
        tracker->current_state == LifecycleState::Destroyed) {
        return false;
    }

    // Check cleanup policy
    std::shared_lock<std::shared_mutex> cleanup_lock(m_cleanup_mutex);
    if (!m_cleanup_policy.should_cleanup_resource(tracker->handle, tracker->current_state)) {
        return false;
    }

    cleanup_lock.unlock();
    resources_lock.unlock();

    // Check for critical dependents
    return !has_critical_dependents(resource_id);
}

std::vector<std::string> ResourceLifecycleManager::get_cleanup_candidates(size_t max_candidates) const {
    std::shared_lock<std::shared_mutex> lock(m_resources_mutex);

    std::vector<std::string> candidates;
    candidates.reserve(std::min(max_candidates, m_tracked_resources.size()));

    for (const auto& [resource_id, tracker] : m_tracked_resources) {
        if (candidates.size() >= max_candidates) {
            break;
        }

        if (can_cleanup_resource(resource_id)) {
            candidates.push_back(resource_id);
        }
    }

    return candidates;
}

void ResourceLifecycleManager::set_automatic_cleanup_enabled(bool enabled) {
    m_automatic_cleanup_enabled.store(enabled);

    if (enabled) {
        m_cleanup_timer->start();
    } else {
        m_cleanup_timer->stop();
    }

    qCDebug(lifecycleLog) << "Automatic cleanup" << (enabled ? "enabled" : "disabled");
}

bool ResourceLifecycleManager::is_automatic_cleanup_enabled() const {
    return m_automatic_cleanup_enabled.load();
}

void ResourceLifecycleManager::perform_automatic_cleanup() {
    perform_cleanup();
}

// Helper methods implementation
void ResourceLifecycleManager::notify_state_change(const LifecycleEvent& event) {
    std::shared_lock<std::shared_mutex> lock(m_subscriptions_mutex);

    for (const auto& [sub_id, subscription] : m_event_subscriptions) {
        if (subscription && subscription->matches(event)) {
            try {
                subscription->callback(event);
            } catch (const std::exception& e) {
                qCWarning(lifecycleLog) << "Exception in lifecycle event callback:" << e.what();
            }
        }
    }
}

std::string ResourceLifecycleManager::generate_subscription_id() const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::string id;
    id.reserve(16);
    for (int i = 0; i < 16; ++i) {
        id += "0123456789abcdef"[dis(gen)];
    }
    return id;
}

bool ResourceLifecycleManager::has_critical_dependents(const std::string& resource_id) const {
    std::shared_lock<std::shared_mutex> lock(m_dependencies_mutex);

    auto it = m_dependents.find(resource_id);
    if (it == m_dependents.end()) {
        return false;
    }

    for (const auto& dependent : it->second) {
        if (dependent.is_critical) {
            return true;
        }
    }

    return false;
}

std::vector<std::string> ResourceLifecycleManager::get_cleanup_order(const std::vector<std::string>& candidates) const {
    // Simple implementation - in practice, this would use topological sorting
    // to ensure dependencies are cleaned up in the correct order
    std::vector<std::string> ordered = candidates;

    // Sort by dependency count (resources with fewer dependents first)
    std::sort(ordered.begin(), ordered.end(), [this](const std::string& a, const std::string& b) {
        auto deps_a = get_dependents(a);
        auto deps_b = get_dependents(b);

        size_t count_a = deps_a ? deps_a.value().size() : 0;
        size_t count_b = deps_b ? deps_b.value().size() : 0;

        return count_a < count_b;
    });

    return ordered;
}

void ResourceLifecycleManager::cleanup_resource_internal(const std::string& resource_id) {
    emit resource_cleanup_started(QString::fromStdString(resource_id));

    // Update state to cleanup
    update_state(resource_id, LifecycleState::Cleanup);

    // Remove from tracking (this will set state to destroyed)
    unregister_resource(resource_id);

    emit resource_cleanup_completed(QString::fromStdString(resource_id));
}

bool ResourceLifecycleManager::is_state_transition_valid(LifecycleState from, LifecycleState to) const {
    return is_valid_state_transition(from, to);
}

// Factory function
std::unique_ptr<IResourceLifecycleManager> create_resource_lifecycle_manager(QObject* parent) {
    return std::make_unique<ResourceLifecycleManager>(parent);
}

} // namespace qtplugin
