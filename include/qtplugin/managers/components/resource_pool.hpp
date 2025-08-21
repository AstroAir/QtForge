/**
 * @file resource_pool.hpp
 * @brief Resource pool interface and implementation
 * @version 3.0.0
 */

#pragma once

#include "../../utils/error_handling.hpp"
#include "../resource_manager.hpp"
#include <QObject>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <queue>
#include <shared_mutex>
#include <chrono>
#include <functional>

namespace qtplugin {

// Forward declarations
enum class ResourceType;
enum class ResourcePriority;
struct ResourceQuota;
struct ResourceUsageStats;
struct ResourceHandle;

/**
 * @brief Resource pool entry
 */
template<typename T>
struct PooledResource {
    std::unique_ptr<T> resource;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point last_used;
    std::string owner_plugin_id;
    ResourcePriority priority;
    bool in_use = false;
    size_t use_count = 0;
};

/**
 * @brief Interface for component resource pooling and management
 *
 * The resource pool handles resource allocation, deallocation, reuse,
 * and lifecycle management for specific resource types.
 */
class IComponentResourcePool {
public:
    virtual ~IComponentResourcePool() = default;

    /**
     * @brief Get pool name
     * @return Pool name
     */
    virtual std::string name() const = 0;

    /**
     * @brief Get resource type handled by this pool
     * @return Resource type
     */
    virtual ResourceType resource_type() const = 0;

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
     * @brief Get pool statistics
     * @return Usage statistics
     */
    virtual ResourceUsageStats get_statistics() const = 0;

    /**
     * @brief Cleanup expired or unused resources
     * @return Number of resources cleaned up
     */
    virtual size_t cleanup_resources() = 0;

    /**
     * @brief Clear all resources from pool
     */
    virtual void clear() = 0;

    /**
     * @brief Check if pool can allocate resource
     * @param plugin_id Plugin requesting resource
     * @param priority Resource priority
     * @return true if allocation is possible
     */
    virtual bool can_allocate(std::string_view plugin_id, ResourcePriority priority) const = 0;

    /**
     * @brief Get number of available resources
     * @return Number of available resources
     */
    virtual size_t available_count() const = 0;

    /**
     * @brief Get number of active resources
     * @return Number of active resources
     */
    virtual size_t active_count() const = 0;
};

/**
 * @brief Template interface for typed resource pools
 */
template<typename T>
class ITypedComponentResourcePool : public IComponentResourcePool {
public:
    /**
     * @brief Acquire resource from pool
     * @param plugin_id Plugin requesting resource
     * @param priority Resource priority
     * @return Resource handle and instance or error
     */
    virtual qtplugin::expected<std::pair<ResourceHandle, std::unique_ptr<T>>, PluginError>
    acquire_resource(std::string_view plugin_id, ResourcePriority priority = ResourcePriority::Normal) = 0;

    /**
     * @brief Release resource back to pool
     * @param handle Resource handle
     * @param resource Resource instance
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError>
    release_resource(const ResourceHandle& handle, std::unique_ptr<T> resource) = 0;

    /**
     * @brief Set resource factory for creating new instances
     * @param factory Factory function
     */
    virtual void set_factory(std::function<std::unique_ptr<T>()> factory) = 0;
};

/**
 * @brief Base class for resource pools with Qt functionality
 *
 * Non-template base class that provides Qt signals/slots functionality
 * for resource pools. Template classes cannot use Q_OBJECT directly.
 */
class ResourcePoolBase : public QObject {
    Q_OBJECT

public:
    explicit ResourcePoolBase(QObject* parent = nullptr);
    virtual ~ResourcePoolBase() = default;

signals:
    /**
     * @brief Emitted when resource is acquired
     * @param handle Resource handle
     * @param plugin_id Plugin that acquired the resource
     */
    void resource_acquired(const QString& handle, const QString& plugin_id);

    /**
     * @brief Emitted when resource is released
     * @param handle Resource handle
     * @param plugin_id Plugin that released the resource
     */
    void resource_released(const QString& handle, const QString& plugin_id);

    /**
     * @brief Emitted when quota is exceeded
     * @param plugin_id Plugin that triggered quota exceeded
     */
    void quota_exceeded(const QString& plugin_id);

    /**
     * @brief Emitted when resources are cleaned up
     * @param count Number of resources cleaned up
     */
    void resources_cleaned_up(int count);
};

/**
 * @brief Generic resource pool implementation
 *
 * Provides resource pooling with configurable quotas, automatic cleanup,
 * and resource reuse strategies.
 */
template<typename T>
class ResourcePool : public ResourcePoolBase, public ITypedComponentResourcePool<T> {
public:
    explicit ResourcePool(const std::string& name, ResourceType type, QObject* parent = nullptr);
    ~ResourcePool() override;

    // IComponentResourcePool interface
    std::string name() const override;
    ResourceType resource_type() const override;
    void set_quota(const ResourceQuota& quota) override;
    ResourceQuota get_quota() const override;
    ResourceUsageStats get_statistics() const override;
    size_t cleanup_resources() override;
    void clear() override;
    bool can_allocate(std::string_view plugin_id, ResourcePriority priority) const override;
    size_t available_count() const override;
    size_t active_count() const override;

    // ITypedComponentResourcePool interface
    qtplugin::expected<std::pair<ResourceHandle, std::unique_ptr<T>>, PluginError>
    acquire_resource(std::string_view plugin_id, ResourcePriority priority = ResourcePriority::Normal) override;

    qtplugin::expected<void, PluginError>
    release_resource(const ResourceHandle& handle, std::unique_ptr<T> resource) override;

    void set_factory(std::function<std::unique_ptr<T>()> factory) override;

private:
    std::string m_name;
    ResourceType m_resource_type;
    ResourceQuota m_quota;
    std::function<std::unique_ptr<T>()> m_factory;
    
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, std::unique_ptr<PooledResource<T>>> m_active_resources;
    std::queue<std::unique_ptr<PooledResource<T>>> m_available_resources;
    
    // Statistics
    mutable std::atomic<size_t> m_total_acquisitions{0};
    mutable std::atomic<size_t> m_total_releases{0};
    mutable std::atomic<size_t> m_total_cleanups{0};
    
    // Helper methods
    std::string generate_handle() const;
    std::unique_ptr<PooledResource<T>> create_new_resource(std::string_view plugin_id, ResourcePriority priority);
    std::unique_ptr<PooledResource<T>> try_reuse_resource(std::string_view plugin_id, ResourcePriority priority);
    bool is_resource_expired(const PooledResource<T>& resource) const;
    bool check_quota_limits(std::string_view plugin_id) const;
    size_t calculate_memory_usage() const;
};

} // namespace qtplugin
