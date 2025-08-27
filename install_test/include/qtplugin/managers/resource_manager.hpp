/**
 * @file resource_manager.hpp
 * @brief Resource management system for plugins
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <any>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "../utils/error_handling.hpp"

namespace qtplugin {

/**
 * @brief Resource types for management
 */
enum class ResourceType {
    Thread,              ///< Thread resources
    Timer,               ///< Timer resources
    NetworkConnection,   ///< Network connection resources
    FileHandle,          ///< File handle resources
    DatabaseConnection,  ///< Database connection resources
    Memory,              ///< Memory resources
    Custom               ///< Custom resource types
};

/**
 * @brief Resource state
 */
enum class ResourceState {
    Available,  ///< Resource is available for use
    InUse,      ///< Resource is currently in use
    Reserved,   ///< Resource is reserved but not yet in use
    Cleanup,    ///< Resource is being cleaned up
    Error       ///< Resource is in error state
};

/**
 * @brief Resource priority levels
 */
enum class ResourcePriority {
    Low = 0,      ///< Low priority
    Normal = 1,   ///< Normal priority
    High = 2,     ///< High priority
    Critical = 3  ///< Critical priority
};

/**
 * @brief Resource usage statistics
 */
struct ResourceUsageStats {
    size_t total_created = 0;
    size_t total_destroyed = 0;
    size_t currently_active = 0;
    size_t peak_usage = 0;
    std::chrono::milliseconds average_lifetime{0};
    std::chrono::milliseconds total_usage_time{0};
    size_t allocation_failures = 0;

    double utilization_rate() const {
        return total_created > 0
                   ? static_cast<double>(currently_active) / total_created
                   : 0.0;
    }
};

/**
 * @brief Resource quota configuration
 */
struct ResourceQuota {
    size_t max_instances = 0;  ///< Maximum number of instances (0 = unlimited)
    size_t max_memory_bytes =
        0;  ///< Maximum memory usage in bytes (0 = unlimited)
    std::chrono::milliseconds max_lifetime{
        0};  ///< Maximum lifetime (0 = unlimited)
    ResourcePriority min_priority =
        ResourcePriority::Low;  ///< Minimum priority for allocation

    bool is_unlimited() const {
        return max_instances == 0 && max_memory_bytes == 0 &&
               max_lifetime.count() == 0;
    }
};

/**
 * @brief Resource handle for tracking and management
 */
class ResourceHandle {
public:
    ResourceHandle() = default;
    ResourceHandle(std::string id, ResourceType type, std::string plugin_id)
        : m_id(std::move(id)),
          m_type(type),
          m_plugin_id(std::move(plugin_id)),
          m_created_at(std::chrono::steady_clock::now()) {}

    const std::string& id() const { return m_id; }
    ResourceType type() const { return m_type; }
    const std::string& plugin_id() const { return m_plugin_id; }
    ResourceState state() const { return m_state; }
    ResourcePriority priority() const { return m_priority; }

    std::chrono::steady_clock::time_point created_at() const {
        return m_created_at;
    }
    std::chrono::steady_clock::time_point last_accessed() const {
        return m_last_accessed;
    }

    std::chrono::milliseconds age() const {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - m_created_at);
    }

    void set_state(ResourceState state) { m_state = state; }
    void set_priority(ResourcePriority priority) { m_priority = priority; }
    void update_access_time() {
        m_last_accessed = std::chrono::steady_clock::now();
    }

    // Metadata access
    void set_metadata(const std::string& key, const std::any& value) {
        m_metadata[key] = value;
    }
    std::optional<std::any> get_metadata(const std::string& key) const {
        auto it = m_metadata.find(key);
        return it != m_metadata.end() ? std::make_optional(it->second)
                                      : std::nullopt;
    }

    bool is_valid() const { return !m_id.empty(); }

private:
    std::string m_id;
    ResourceType m_type = ResourceType::Custom;
    std::string m_plugin_id;
    ResourceState m_state = ResourceState::Available;
    ResourcePriority m_priority = ResourcePriority::Normal;
    std::chrono::steady_clock::time_point m_created_at;
    std::chrono::steady_clock::time_point m_last_accessed;
    std::unordered_map<std::string, std::any> m_metadata;
};

/**
 * @brief Resource factory interface for creating resources
 */
template <typename T>
class IResourceFactory {
public:
    virtual ~IResourceFactory() = default;

    /**
     * @brief Create a new resource instance
     * @param handle Resource handle for tracking
     * @return Created resource or error
     */
    virtual qtplugin::expected<std::unique_ptr<T>, PluginError> create_resource(
        const ResourceHandle& handle) = 0;

    /**
     * @brief Validate resource before creation
     * @param handle Resource handle to validate
     * @return true if resource can be created
     */
    virtual bool can_create_resource(const ResourceHandle& handle) const = 0;

    /**
     * @brief Get estimated resource cost
     * @param handle Resource handle
     * @return Estimated memory usage in bytes
     */
    virtual size_t get_estimated_cost(const ResourceHandle& handle) const = 0;

    /**
     * @brief Get factory name
     * @return Factory name
     */
    virtual std::string name() const = 0;
};

/**
 * @brief Resource pool interface for managing resource instances
 */
template <typename T>
class IResourcePool {
public:
    virtual ~IResourcePool() = default;

    /**
     * @brief Acquire a resource from the pool
     * @param plugin_id Plugin requesting the resource
     * @param priority Resource priority
     * @return Resource handle and instance or error
     */
    virtual qtplugin::expected<std::pair<ResourceHandle, std::unique_ptr<T>>,
                               PluginError>
    acquire_resource(std::string_view plugin_id,
                     ResourcePriority priority = ResourcePriority::Normal) = 0;

    /**
     * @brief Release a resource back to the pool
     * @param handle Resource handle
     * @param resource Resource instance
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> release_resource(
        const ResourceHandle& handle, std::unique_ptr<T> resource) = 0;

    /**
     * @brief Get pool statistics
     * @return Usage statistics
     */
    virtual ResourceUsageStats get_statistics() const = 0;

    /**
     * @brief Set resource quota for the pool
     * @param quota Resource quota configuration
     */
    virtual void set_quota(const ResourceQuota& quota) = 0;

    /**
     * @brief Get current quota configuration
     * @return Current quota
     */
    virtual ResourceQuota get_quota() const = 0;

    /**
     * @brief Cleanup expired or unused resources
     * @return Number of resources cleaned up
     */
    virtual size_t cleanup_resources() = 0;

    /**
     * @brief Get pool name
     * @return Pool name
     */
    virtual std::string name() const = 0;
};

/**
 * @brief Resource manager interface
 *
 * Provides comprehensive resource management with pools, automatic cleanup,
 * usage monitoring, and resource lifecycle management.
 */
class IResourceManager {
public:
    virtual ~IResourceManager() = default;

    // === Resource Pool Management ===

    /**
     * @brief Register a resource factory
     * @param type Resource type
     * @param factory Resource factory
     * @return Success or error information
     */
    template <typename T>
    qtplugin::expected<void, PluginError> register_factory(
        ResourceType type, std::unique_ptr<IResourceFactory<T>> factory) {
        return register_factory_impl(
            type, std::type_index(typeid(T)),
            std::unique_ptr<void, std::function<void(void*)>>(
                factory.release(), [](void* ptr) {
                    delete static_cast<IResourceFactory<T>*>(ptr);
                }));
    }

    /**
     * @brief Create a resource pool
     * @param type Resource type
     * @param pool_name Pool name
     * @param quota Resource quota
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> create_pool(
        ResourceType type, std::string_view pool_name,
        const ResourceQuota& quota = {}) = 0;

    /**
     * @brief Remove a resource pool
     * @param pool_name Pool name
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> remove_pool(
        std::string_view pool_name) = 0;

    /**
     * @brief Get resource pool
     * @param pool_name Pool name
     * @return Resource pool or error
     */
    template <typename T>
    qtplugin::expected<IResourcePool<T>*, PluginError> get_pool(
        std::string_view pool_name) {
        auto result = get_pool_impl(pool_name, std::type_index(typeid(T)));
        if (!result) {
            return qtplugin::expected<IResourcePool<T>*, PluginError>(
                qtplugin::unexpected(result.error()));
        }
        return static_cast<IResourcePool<T>*>(result.value());
    }

    // === Resource Acquisition ===

    /**
     * @brief Acquire a resource
     * @param type Resource type
     * @param plugin_id Plugin requesting the resource
     * @param priority Resource priority
     * @return Resource handle and instance or error
     */
    template <typename T>
    qtplugin::expected<std::pair<ResourceHandle, std::unique_ptr<T>>,
                       PluginError>
    acquire_resource(ResourceType type, std::string_view plugin_id,
                     ResourcePriority priority = ResourcePriority::Normal) {
        return acquire_resource_impl<T>(type, plugin_id, priority);
    }

    /**
     * @brief Release a resource
     * @param handle Resource handle
     * @param resource Resource instance
     * @return Success or error information
     */
    template <typename T>
    qtplugin::expected<void, PluginError> release_resource(
        const ResourceHandle& handle, std::unique_ptr<T> resource) {
        return release_resource_impl(
            handle, std::unique_ptr<void, std::function<void(void*)>>(
                        resource.release(),
                        [](void* ptr) { delete static_cast<T*>(ptr); }));
    }

protected:
    virtual qtplugin::expected<void, PluginError> register_factory_impl(
        ResourceType type, std::type_index type_index,
        std::unique_ptr<void, std::function<void(void*)>> factory) = 0;

    virtual qtplugin::expected<void*, PluginError> get_pool_impl(
        std::string_view pool_name, std::type_index type_index) = 0;

    template <typename T>
    qtplugin::expected<std::pair<ResourceHandle, std::unique_ptr<T>>,
                       PluginError>
    acquire_resource_impl(ResourceType type, std::string_view plugin_id,
                          ResourcePriority priority) {
        // Implementation will be in concrete class
        return qtplugin::make_error<
            std::pair<ResourceHandle, std::unique_ptr<T>>>(
            qtplugin::PluginErrorCode::NotImplemented,
            "acquire_resource_impl not implemented");
    }

    virtual qtplugin::expected<void, PluginError> release_resource_impl(
        const ResourceHandle& handle,
        std::unique_ptr<void, std::function<void(void*)>> resource) = 0;

public:
    // === Resource Monitoring ===

    /**
     * @brief Get resource usage statistics
     * @param type Resource type (optional)
     * @param plugin_id Plugin ID (optional)
     * @return Usage statistics
     */
    virtual ResourceUsageStats get_usage_statistics(
        std::optional<ResourceType> type = std::nullopt,
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Get all active resource handles
     * @param plugin_id Plugin ID filter (optional)
     * @return List of active resource handles
     */
    virtual std::vector<ResourceHandle> get_active_resources(
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Set resource quota for plugin
     * @param plugin_id Plugin ID
     * @param type Resource type
     * @param quota Resource quota
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> set_plugin_quota(
        std::string_view plugin_id, ResourceType type,
        const ResourceQuota& quota) = 0;

    /**
     * @brief Get resource quota for plugin
     * @param plugin_id Plugin ID
     * @param type Resource type
     * @return Resource quota or error
     */
    virtual qtplugin::expected<ResourceQuota, PluginError> get_plugin_quota(
        std::string_view plugin_id, ResourceType type) const = 0;

    // === Resource Lifecycle ===

    /**
     * @brief Force cleanup of resources for plugin
     * @param plugin_id Plugin ID
     * @param type Resource type (optional)
     * @return Number of resources cleaned up
     */
    virtual size_t cleanup_plugin_resources(
        std::string_view plugin_id,
        std::optional<ResourceType> type = std::nullopt) = 0;

    /**
     * @brief Force cleanup of expired resources
     * @param max_age Maximum age for resources
     * @return Number of resources cleaned up
     */
    virtual size_t cleanup_expired_resources(
        std::chrono::milliseconds max_age) = 0;

    /**
     * @brief Set automatic cleanup interval
     * @param interval Cleanup interval (0 to disable)
     */
    virtual void set_cleanup_interval(std::chrono::milliseconds interval) = 0;

    /**
     * @brief Get automatic cleanup interval
     * @return Current cleanup interval
     */
    virtual std::chrono::milliseconds get_cleanup_interval() const = 0;

    // === Resource Events ===

    /**
     * @brief Subscribe to resource events
     * @param callback Event callback function
     * @param type Resource type filter (optional)
     * @param plugin_id Plugin ID filter (optional)
     * @return Subscription ID for unsubscribing
     */
    virtual std::string subscribe_to_events(
        std::function<void(const ResourceHandle&, ResourceState, ResourceState)>
            callback,
        std::optional<ResourceType> type = std::nullopt,
        std::string_view plugin_id = {}) = 0;

    /**
     * @brief Unsubscribe from resource events
     * @param subscription_id Subscription ID
     * @return Success or error information
     */
    virtual qtplugin::expected<void, PluginError> unsubscribe_from_events(
        const std::string& subscription_id) = 0;

    // === Utility Functions ===

    /**
     * @brief Get resource manager statistics
     * @return Statistics as JSON object
     */
    virtual QJsonObject get_statistics() const = 0;

    /**
     * @brief Get list of available resource pools
     * @return List of pool names
     */
    virtual std::vector<std::string> get_pool_names() const = 0;

    /**
     * @brief Check if resource type is supported
     * @param type Resource type
     * @return true if supported
     */
    virtual bool is_resource_type_supported(ResourceType type) const = 0;

    /**
     * @brief Get total memory usage
     * @param plugin_id Plugin ID filter (optional)
     * @return Total memory usage in bytes
     */
    virtual size_t get_total_memory_usage(
        std::string_view plugin_id = {}) const = 0;

    /**
     * @brief Enable or disable resource tracking
     * @param enabled If true, enable detailed resource tracking
     */
    virtual void set_tracking_enabled(bool enabled) = 0;

    /**
     * @brief Check if resource tracking is enabled
     * @return true if tracking is enabled
     */
    virtual bool is_tracking_enabled() const = 0;
};

// === Utility Functions ===

/**
 * @brief Convert resource type to string
 * @param type Resource type
 * @return String representation
 */
std::string resource_type_to_string(ResourceType type);

/**
 * @brief Convert string to resource type
 * @param str String representation
 * @return Resource type or nullopt if invalid
 */
std::optional<ResourceType> string_to_resource_type(std::string_view str);

/**
 * @brief Convert resource state to string
 * @param state Resource state
 * @return String representation
 */
std::string resource_state_to_string(ResourceState state);

/**
 * @brief Convert resource priority to string
 * @param priority Resource priority
 * @return String representation
 */
std::string resource_priority_to_string(ResourcePriority priority);

}  // namespace qtplugin
