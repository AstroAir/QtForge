/**
 * @file resource_lifecycle.hpp
 * @brief Resource lifecycle management and automatic cleanup
 * @version 3.0.0
 */

#pragma once

#include "resource_manager.hpp"
#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <functional>
#include <atomic>
#include <shared_mutex>

namespace qtplugin {

/**
 * @brief Resource lifecycle state tracking
 */
enum class LifecycleState {
    Created,        ///< Resource has been created
    Initialized,    ///< Resource has been initialized
    Active,         ///< Resource is actively being used
    Idle,           ///< Resource is idle but still valid
    Deprecated,     ///< Resource is deprecated but still functional
    Cleanup,        ///< Resource is being cleaned up
    Destroyed       ///< Resource has been destroyed
};

/**
 * @brief Resource lifecycle event
 */
struct LifecycleEvent {
    std::string resource_id;
    ResourceType resource_type;
    std::string plugin_id;
    LifecycleState old_state;
    LifecycleState new_state;
    std::chrono::system_clock::time_point timestamp;
    QJsonObject metadata;
    
    LifecycleEvent() = default;
    LifecycleEvent(std::string_view res_id, ResourceType type, std::string_view plugin,
                  LifecycleState old_st, LifecycleState new_st, const QJsonObject& meta = {})
        : resource_id(res_id), resource_type(type), plugin_id(plugin)
        , old_state(old_st), new_state(new_st)
        , timestamp(std::chrono::system_clock::now()), metadata(meta) {}
};

/**
 * @brief Resource cleanup policy
 */
struct CleanupPolicy {
    std::chrono::milliseconds max_idle_time{std::chrono::minutes(30)};
    std::chrono::milliseconds max_lifetime{std::chrono::hours(24)};
    size_t max_unused_resources = 10;
    bool cleanup_on_plugin_unload = true;
    bool cleanup_on_low_memory = true;
    ResourcePriority min_priority_to_keep = ResourcePriority::Low;
    
    bool should_cleanup_resource(const ResourceHandle& handle, LifecycleState state) const {
        auto age = handle.age();
        auto idle_time = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - handle.last_accessed());
        
        // Check maximum lifetime
        if (max_lifetime.count() > 0 && age >= max_lifetime) {
            return true;
        }
        
        // Check idle time for idle resources
        if (state == LifecycleState::Idle && max_idle_time.count() > 0 && idle_time >= max_idle_time) {
            return true;
        }
        
        // Check priority
        if (handle.priority() < min_priority_to_keep) {
            return true;
        }
        
        return false;
    }
};

/**
 * @brief Resource dependency tracking
 */
struct ResourceDependency {
    std::string dependent_id;      ///< Resource that depends on another
    std::string dependency_id;     ///< Resource that is depended upon
    std::string relationship_type; ///< Type of dependency (e.g., "parent", "shared", "weak")
    bool is_critical = true;       ///< Whether dependency is critical for operation
    
    ResourceDependency() = default;
    ResourceDependency(std::string_view dep, std::string_view dependency, 
                      std::string_view type, bool critical = true)
        : dependent_id(dep), dependency_id(dependency), relationship_type(type), is_critical(critical) {}
};

/**
 * @brief Resource lifecycle manager interface
 */
class IResourceLifecycleManager {
public:
    virtual ~IResourceLifecycleManager() = default;
    
    // === Lifecycle Tracking ===
    
    /**
     * @brief Register a resource for lifecycle tracking
     * @param handle Resource handle
     * @param initial_state Initial lifecycle state
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    register_resource(const ResourceHandle& handle, LifecycleState initial_state = LifecycleState::Created) = 0;
    
    /**
     * @brief Unregister a resource from lifecycle tracking
     * @param resource_id Resource ID
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    unregister_resource(const std::string& resource_id) = 0;
    
    /**
     * @brief Update resource lifecycle state
     * @param resource_id Resource ID
     * @param new_state New lifecycle state
     * @param metadata Additional metadata (optional)
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    update_state(const std::string& resource_id, LifecycleState new_state, 
                const QJsonObject& metadata = {}) = 0;
    
    /**
     * @brief Get current lifecycle state of a resource
     * @param resource_id Resource ID
     * @return Current lifecycle state or error
     */
    virtual qtplugin::expected<LifecycleState, PluginError>
    get_state(const std::string& resource_id) const = 0;
    
    // === Dependency Management ===
    
    /**
     * @brief Add a dependency relationship between resources
     * @param dependency Dependency relationship
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    add_dependency(const ResourceDependency& dependency) = 0;
    
    /**
     * @brief Remove a dependency relationship
     * @param dependent_id Dependent resource ID
     * @param dependency_id Dependency resource ID
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    remove_dependency(const std::string& dependent_id, const std::string& dependency_id) = 0;
    
    /**
     * @brief Get all dependencies for a resource
     * @param resource_id Resource ID
     * @return List of dependencies or error
     */
    virtual qtplugin::expected<std::vector<ResourceDependency>, PluginError>
    get_dependencies(const std::string& resource_id) const = 0;
    
    /**
     * @brief Get all dependents for a resource
     * @param resource_id Resource ID
     * @return List of dependents or error
     */
    virtual qtplugin::expected<std::vector<ResourceDependency>, PluginError>
    get_dependents(const std::string& resource_id) const = 0;
    
    // === Cleanup Management ===
    
    /**
     * @brief Set cleanup policy
     * @param policy Cleanup policy configuration
     */
    virtual void set_cleanup_policy(const CleanupPolicy& policy) = 0;
    
    /**
     * @brief Get current cleanup policy
     * @return Current cleanup policy
     */
    virtual CleanupPolicy get_cleanup_policy() const = 0;
    
    /**
     * @brief Perform automatic cleanup based on policy
     * @return Number of resources cleaned up
     */
    virtual size_t perform_cleanup() = 0;
    
    /**
     * @brief Force cleanup of specific resource
     * @param resource_id Resource ID
     * @param force_cleanup Whether to ignore dependencies
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    force_cleanup(const std::string& resource_id, bool force_cleanup = false) = 0;
    
    /**
     * @brief Cleanup all resources for a plugin
     * @param plugin_id Plugin ID
     * @return Number of resources cleaned up
     */
    virtual size_t cleanup_plugin_resources(const std::string& plugin_id) = 0;
    
    // === Event Management ===
    
    /**
     * @brief Subscribe to lifecycle events
     * @param callback Event callback function
     * @param resource_filter Resource ID filter (optional)
     * @param state_filter State filter (optional)
     * @return Subscription ID for unsubscribing
     */
    virtual std::string subscribe_to_lifecycle_events(
        std::function<void(const LifecycleEvent&)> callback,
        const std::string& resource_filter = {},
        std::optional<LifecycleState> state_filter = std::nullopt) = 0;
    
    /**
     * @brief Unsubscribe from lifecycle events
     * @param subscription_id Subscription ID
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError>
    unsubscribe_from_lifecycle_events(const std::string& subscription_id) = 0;
    
    // === Statistics and Monitoring ===
    
    /**
     * @brief Get lifecycle statistics
     * @return Statistics as JSON object
     */
    virtual QJsonObject get_lifecycle_statistics() const = 0;
    
    /**
     * @brief Get resource lifecycle history
     * @param resource_id Resource ID
     * @param max_events Maximum number of events to return
     * @return List of lifecycle events or error
     */
    virtual qtplugin::expected<std::vector<LifecycleEvent>, PluginError>
    get_resource_history(const std::string& resource_id, size_t max_events = 100) const = 0;
    
    /**
     * @brief Get all resources in specific state
     * @param state Lifecycle state
     * @return List of resource IDs in the specified state
     */
    virtual std::vector<std::string> get_resources_in_state(LifecycleState state) const = 0;
    
    /**
     * @brief Check if resource can be safely cleaned up
     * @param resource_id Resource ID
     * @return true if resource can be safely cleaned up
     */
    virtual bool can_cleanup_resource(const std::string& resource_id) const = 0;
    
    /**
     * @brief Get cleanup candidates
     * @param max_candidates Maximum number of candidates to return
     * @return List of resource IDs that are candidates for cleanup
     */
    virtual std::vector<std::string> get_cleanup_candidates(size_t max_candidates = 100) const = 0;
    
    /**
     * @brief Enable or disable automatic cleanup
     * @param enabled Whether to enable automatic cleanup
     */
    virtual void set_automatic_cleanup_enabled(bool enabled) = 0;
    
    /**
     * @brief Check if automatic cleanup is enabled
     * @return true if automatic cleanup is enabled
     */
    virtual bool is_automatic_cleanup_enabled() const = 0;
};

// === Utility Functions ===

/**
 * @brief Convert lifecycle state to string
 * @param state Lifecycle state
 * @return String representation
 */
std::string lifecycle_state_to_string(LifecycleState state);

/**
 * @brief Convert string to lifecycle state
 * @param str String representation
 * @return Lifecycle state or nullopt if invalid
 */
std::optional<LifecycleState> string_to_lifecycle_state(std::string_view str);

/**
 * @brief Check if state transition is valid
 * @param from Source state
 * @param to Target state
 * @return true if transition is valid
 */
bool is_valid_state_transition(LifecycleState from, LifecycleState to);

/**
 * @brief Get next valid states from current state
 * @param current Current state
 * @return List of valid next states
 */
std::vector<LifecycleState> get_valid_next_states(LifecycleState current);

} // namespace qtplugin
