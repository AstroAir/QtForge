/**
 * @file resource_allocator.hpp
 * @brief Resource allocator interface and implementation
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
#include <functional>
#include <shared_mutex>
#include <atomic>

namespace qtplugin {

// Forward declarations
enum class ResourceType;
enum class ResourcePriority;
struct ResourceQuota;
struct ResourceUsageStats;
struct ResourceHandle;
class IComponentResourcePool;

/**
 * @brief Resource allocation strategy
 */
enum class AllocationStrategy {
    FirstFit,       ///< Allocate from first available pool
    BestFit,        ///< Allocate from pool with best size match
    WorstFit,       ///< Allocate from largest available pool
    RoundRobin,     ///< Rotate between available pools
    LoadBalanced,   ///< Balance load across pools
    Priority        ///< Allocate based on priority
};

/**
 * @brief Resource allocation policy
 */
struct AllocationPolicy {
    AllocationStrategy strategy = AllocationStrategy::FirstFit;
    ResourcePriority min_priority = ResourcePriority::Low;
    size_t max_allocations_per_plugin = 0; // 0 = unlimited
    std::chrono::milliseconds allocation_timeout{5000};
    bool allow_preemption = false;
    bool enable_load_balancing = true;
};

/**
 * @brief Resource allocation record
 */
struct AllocationRecord {
    std::string allocation_id;
    std::string plugin_id;
    ResourceType resource_type;
    std::string pool_name;
    ResourcePriority priority;
    std::chrono::system_clock::time_point allocated_at;
    size_t allocation_size = 0;
    std::unordered_map<std::string, std::string> metadata;
};

/**
 * @brief Interface for resource allocation management
 * 
 * The resource allocator handles allocation strategies, quota enforcement,
 * and resource distribution across multiple pools.
 */
class IResourceAllocator {
public:
    virtual ~IResourceAllocator() = default;
    
    /**
     * @brief Register resource pool
     * @param pool Resource pool to register
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError>
    register_pool(std::shared_ptr<IComponentResourcePool> pool) = 0;
    
    /**
     * @brief Unregister resource pool
     * @param pool_name Name of pool to unregister
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError>
    unregister_pool(std::string_view pool_name) = 0;
    
    /**
     * @brief Allocate resource using allocation strategy
     * @param resource_type Type of resource to allocate
     * @param plugin_id Plugin requesting resource
     * @param priority Resource priority
     * @param metadata Additional allocation metadata
     * @return Allocation record or error
     */
    virtual qtplugin::expected<AllocationRecord, PluginError>
    allocate_resource(ResourceType resource_type,
                     std::string_view plugin_id,
                     ResourcePriority priority = ResourcePriority::Normal,
                     const std::unordered_map<std::string, std::string>& metadata = {}) = 0;
    
    /**
     * @brief Deallocate resource
     * @param allocation_id Allocation identifier
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError>
    deallocate_resource(const std::string& allocation_id) = 0;
    
    /**
     * @brief Set allocation policy for resource type
     * @param resource_type Resource type
     * @param policy Allocation policy
     */
    virtual void set_allocation_policy(ResourceType resource_type, const AllocationPolicy& policy) = 0;
    
    /**
     * @brief Get allocation policy for resource type
     * @param resource_type Resource type
     * @return Current allocation policy
     */
    virtual AllocationPolicy get_allocation_policy(ResourceType resource_type) const = 0;
    
    /**
     * @brief Get allocation statistics
     * @param resource_type Optional resource type filter
     * @param plugin_id Optional plugin ID filter
     * @return Allocation statistics
     */
    virtual ResourceUsageStats get_allocation_statistics(std::optional<ResourceType> resource_type = std::nullopt,
                                                        std::string_view plugin_id = {}) const = 0;
    
    /**
     * @brief Get active allocations
     * @param plugin_id Optional plugin ID filter
     * @return List of active allocation records
     */
    virtual std::vector<AllocationRecord> get_active_allocations(std::string_view plugin_id = {}) const = 0;
    
    /**
     * @brief Check if allocation is possible
     * @param resource_type Resource type
     * @param plugin_id Plugin requesting resource
     * @param priority Resource priority
     * @return true if allocation is possible
     */
    virtual bool can_allocate(ResourceType resource_type,
                             std::string_view plugin_id,
                             ResourcePriority priority) const = 0;
    
    /**
     * @brief Get available pools for resource type
     * @param resource_type Resource type
     * @return List of available pool names
     */
    virtual std::vector<std::string> get_available_pools(ResourceType resource_type) const = 0;
    
    /**
     * @brief Optimize allocations across pools
     * @return Number of optimizations performed
     */
    virtual size_t optimize_allocations() = 0;
};

/**
 * @brief Resource allocator implementation
 * 
 * Manages resource allocation across multiple pools using configurable
 * strategies and policies.
 */
class ResourceAllocator : public QObject, public IResourceAllocator {
    Q_OBJECT
    
public:
    explicit ResourceAllocator(QObject* parent = nullptr);
    ~ResourceAllocator() override;
    
    // IResourceAllocator interface
    qtplugin::expected<void, PluginError>
    register_pool(std::shared_ptr<IComponentResourcePool> pool) override;
    
    qtplugin::expected<void, PluginError>
    unregister_pool(std::string_view pool_name) override;
    
    qtplugin::expected<AllocationRecord, PluginError>
    allocate_resource(ResourceType resource_type,
                     std::string_view plugin_id,
                     ResourcePriority priority = ResourcePriority::Normal,
                     const std::unordered_map<std::string, std::string>& metadata = {}) override;
    
    qtplugin::expected<void, PluginError>
    deallocate_resource(const std::string& allocation_id) override;
    
    void set_allocation_policy(ResourceType resource_type, const AllocationPolicy& policy) override;
    AllocationPolicy get_allocation_policy(ResourceType resource_type) const override;
    
    ResourceUsageStats get_allocation_statistics(std::optional<ResourceType> resource_type = std::nullopt,
                                                std::string_view plugin_id = {}) const override;
    
    std::vector<AllocationRecord> get_active_allocations(std::string_view plugin_id = {}) const override;
    
    bool can_allocate(ResourceType resource_type,
                     std::string_view plugin_id,
                     ResourcePriority priority) const override;
    
    std::vector<std::string> get_available_pools(ResourceType resource_type) const override;
    
    size_t optimize_allocations() override;

signals:
    /**
     * @brief Emitted when resource is allocated
     * @param allocation_id Allocation identifier
     * @param plugin_id Plugin that received allocation
     * @param resource_type Type of resource allocated
     */
    void resource_allocated(const QString& allocation_id, const QString& plugin_id, int resource_type);
    
    /**
     * @brief Emitted when resource is deallocated
     * @param allocation_id Allocation identifier
     * @param plugin_id Plugin that released allocation
     */
    void resource_deallocated(const QString& allocation_id, const QString& plugin_id);
    
    /**
     * @brief Emitted when allocation fails
     * @param plugin_id Plugin that failed to get allocation
     * @param resource_type Type of resource that failed
     * @param reason Failure reason
     */
    void allocation_failed(const QString& plugin_id, int resource_type, const QString& reason);
    
    /**
     * @brief Emitted when allocations are optimized
     * @param optimizations_count Number of optimizations performed
     */
    void allocations_optimized(int optimizations_count);

private:
    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, std::shared_ptr<IComponentResourcePool>> m_pools;
    std::unordered_map<ResourceType, std::vector<std::string>> m_pools_by_type;
    std::unordered_map<ResourceType, AllocationPolicy> m_policies;
    std::unordered_map<std::string, AllocationRecord> m_active_allocations;
    
    // Statistics
    mutable std::atomic<size_t> m_total_allocations{0};
    mutable std::atomic<size_t> m_total_deallocations{0};
    mutable std::atomic<size_t> m_failed_allocations{0};
    
    // Helper methods
    std::string generate_allocation_id() const;
    std::string select_pool(ResourceType resource_type, std::string_view plugin_id, 
                           ResourcePriority priority, const AllocationPolicy& policy) const;
    bool check_allocation_limits(ResourceType resource_type, std::string_view plugin_id, 
                                const AllocationPolicy& policy) const;
    void initialize_default_policies();
    size_t count_plugin_allocations(std::string_view plugin_id, ResourceType resource_type) const;
};

} // namespace qtplugin
