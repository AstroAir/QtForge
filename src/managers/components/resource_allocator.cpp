/**
 * @file resource_allocator.cpp
 * @brief Implementation of resource allocator
 * @version 3.0.0
 */

#include "../../../include/qtplugin/managers/components/resource_allocator.hpp"
#include <QDebug>
#include <QLoggingCategory>
#include <QUuid>
#include <algorithm>
#include <random>
#include "../../../include/qtplugin/managers/components/resource_pool.hpp"

Q_LOGGING_CATEGORY(resourceAllocatorLog, "qtplugin.resource.allocator")

namespace qtplugin {

ResourceAllocator::ResourceAllocator(QObject* parent) : QObject(parent) {
    initialize_default_policies();
    qCDebug(resourceAllocatorLog) << "Resource allocator initialized";
}

ResourceAllocator::~ResourceAllocator() {
    qCDebug(resourceAllocatorLog) << "Resource allocator destroyed";
}

qtplugin::expected<void, PluginError> ResourceAllocator::register_pool(
    std::shared_ptr<IComponentResourcePool> pool) {
    if (!pool) {
        return make_error<void>(PluginErrorCode::InvalidArgument,
                                "Pool cannot be null");
    }

    std::unique_lock lock(m_mutex);

    std::string pool_name = pool->name();
    ResourceType resource_type = pool->resource_type();

    // Check if pool already exists
    if (m_pools.find(pool_name) != m_pools.end()) {
        return make_error<void>(PluginErrorCode::InvalidArgument,
                                "Pool already registered: " + pool_name);
    }

    // Register pool
    m_pools[pool_name] = pool;
    m_pools_by_type[resource_type].push_back(pool_name);

    qCDebug(resourceAllocatorLog)
        << "Registered pool:" << QString::fromStdString(pool_name)
        << "for resource type" << static_cast<int>(resource_type);

    return make_success();
}

qtplugin::expected<void, PluginError> ResourceAllocator::unregister_pool(
    std::string_view pool_name) {
    std::unique_lock lock(m_mutex);

    std::string pool_name_str(pool_name);
    auto pool_it = m_pools.find(pool_name_str);
    if (pool_it == m_pools.end()) {
        return make_error<void>(PluginErrorCode::InvalidArgument,
                                "Pool not found: " + pool_name_str);
    }

    ResourceType resource_type = pool_it->second->resource_type();

    // Remove from pools by type
    auto& type_pools = m_pools_by_type[resource_type];
    type_pools.erase(
        std::remove(type_pools.begin(), type_pools.end(), pool_name_str),
        type_pools.end());

    // Remove from pools
    m_pools.erase(pool_it);

    qCDebug(resourceAllocatorLog)
        << "Unregistered pool:" << QString::fromStdString(pool_name_str);

    return make_success();
}

qtplugin::expected<AllocationRecord, PluginError>
ResourceAllocator::allocate_resource(
    ResourceType resource_type, std::string_view plugin_id,
    ResourcePriority priority,
    const std::unordered_map<std::string, std::string>& metadata) {
    std::unique_lock lock(m_mutex);

    // Get allocation policy
    auto policy_it = m_policies.find(resource_type);
    AllocationPolicy policy = (policy_it != m_policies.end())
                                  ? policy_it->second
                                  : AllocationPolicy{};

    // Check if allocation is allowed
    if (!check_allocation_limits(resource_type, plugin_id, policy)) {
        m_failed_allocations.fetch_add(1);
        emit allocation_failed(QString::fromStdString(std::string(plugin_id)),
                               static_cast<int>(resource_type),
                               "Allocation limits exceeded");
        return make_error<AllocationRecord>(
            PluginErrorCode::ResourceUnavailable, "Allocation limits exceeded");
    }

    // Check priority requirements
    if (priority < policy.min_priority) {
        m_failed_allocations.fetch_add(1);
        emit allocation_failed(QString::fromStdString(std::string(plugin_id)),
                               static_cast<int>(resource_type),
                               "Priority too low");
        return make_error<AllocationRecord>(
            PluginErrorCode::ResourceUnavailable, "Priority too low");
    }

    // Select pool using allocation strategy
    std::string selected_pool =
        select_pool(resource_type, plugin_id, priority, policy);
    if (selected_pool.empty()) {
        m_failed_allocations.fetch_add(1);
        emit allocation_failed(QString::fromStdString(std::string(plugin_id)),
                               static_cast<int>(resource_type),
                               "No available pools");
        return make_error<AllocationRecord>(
            PluginErrorCode::ResourceUnavailable, "No available pools");
    }

    // Create allocation record
    AllocationRecord record;
    record.allocation_id = generate_allocation_id();
    record.plugin_id = std::string(plugin_id);
    record.resource_type = resource_type;
    record.pool_name = selected_pool;
    record.priority = priority;
    record.allocated_at = std::chrono::system_clock::now();
    record.metadata = metadata;

    // Store allocation record
    m_active_allocations[record.allocation_id] = record;

    m_total_allocations.fetch_add(1);

    qCDebug(resourceAllocatorLog)
        << "Allocated resource" << static_cast<int>(resource_type)
        << "from pool" << QString::fromStdString(selected_pool) << "to plugin"
        << QString::fromStdString(std::string(plugin_id));

    emit resource_allocated(QString::fromStdString(record.allocation_id),
                            QString::fromStdString(std::string(plugin_id)),
                            static_cast<int>(resource_type));

    return record;
}

qtplugin::expected<void, PluginError> ResourceAllocator::deallocate_resource(
    const std::string& allocation_id) {
    std::unique_lock lock(m_mutex);

    auto it = m_active_allocations.find(allocation_id);
    if (it == m_active_allocations.end()) {
        return make_error<void>(PluginErrorCode::InvalidArgument,
                                "Allocation not found: " + allocation_id);
    }

    AllocationRecord record = it->second;
    m_active_allocations.erase(it);

    m_total_deallocations.fetch_add(1);

    qCDebug(resourceAllocatorLog)
        << "Deallocated resource" << QString::fromStdString(allocation_id)
        << "from plugin" << QString::fromStdString(record.plugin_id);

    emit resource_deallocated(QString::fromStdString(allocation_id),
                              QString::fromStdString(record.plugin_id));

    return make_success();
}

void ResourceAllocator::set_allocation_policy(ResourceType resource_type,
                                              const AllocationPolicy& policy) {
    std::unique_lock lock(m_mutex);
    m_policies[resource_type] = policy;

    qCDebug(resourceAllocatorLog)
        << "Updated allocation policy for resource type"
        << static_cast<int>(resource_type);
}

AllocationPolicy ResourceAllocator::get_allocation_policy(
    ResourceType resource_type) const {
    std::shared_lock lock(m_mutex);

    auto it = m_policies.find(resource_type);
    if (it != m_policies.end()) {
        return it->second;
    }

    return AllocationPolicy{};  // Default policy
}

ResourceUsageStats ResourceAllocator::get_allocation_statistics(
    std::optional<ResourceType> resource_type,
    std::string_view plugin_id) const {
    std::shared_lock lock(m_mutex);

    ResourceUsageStats stats;

    // Count allocations matching filters
    for (const auto& [allocation_id, record] : m_active_allocations) {
        bool matches = true;

        if (resource_type && record.resource_type != *resource_type) {
            matches = false;
        }

        if (!plugin_id.empty() && record.plugin_id != plugin_id) {
            matches = false;
        }

        if (matches) {
            stats.total_created++;
            // Note: allocation_size not available in ResourceUsageStats
            // stats.allocation_size += record.allocation_size;
        }
    }

    // Note: total_requests and failed_requests not available in
    // ResourceUsageStats Using available members instead
    stats.currently_active = stats.total_created - stats.total_destroyed;
    stats.allocation_failures = m_failed_allocations.load();

    return stats;
}

std::vector<AllocationRecord> ResourceAllocator::get_active_allocations(
    std::string_view plugin_id) const {
    std::shared_lock lock(m_mutex);

    std::vector<AllocationRecord> allocations;

    for (const auto& [allocation_id, record] : m_active_allocations) {
        if (plugin_id.empty() || record.plugin_id == plugin_id) {
            allocations.push_back(record);
        }
    }

    return allocations;
}

bool ResourceAllocator::can_allocate(ResourceType resource_type,
                                     std::string_view plugin_id,
                                     ResourcePriority priority) const {
    std::shared_lock lock(m_mutex);

    // Get allocation policy
    auto policy_it = m_policies.find(resource_type);
    AllocationPolicy policy = (policy_it != m_policies.end())
                                  ? policy_it->second
                                  : AllocationPolicy{};

    // Check allocation limits
    if (!check_allocation_limits(resource_type, plugin_id, policy)) {
        return false;
    }

    // Check priority requirements
    if (priority < policy.min_priority) {
        return false;
    }

    // Check if any pools are available
    auto pools_it = m_pools_by_type.find(resource_type);
    if (pools_it == m_pools_by_type.end() || pools_it->second.empty()) {
        return false;
    }

    // Check if at least one pool can allocate
    for (const auto& pool_name : pools_it->second) {
        auto pool_it = m_pools.find(pool_name);
        if (pool_it != m_pools.end() &&
            pool_it->second->can_allocate(plugin_id, priority)) {
            return true;
        }
    }

    return false;
}

std::vector<std::string> ResourceAllocator::get_available_pools(
    ResourceType resource_type) const {
    std::shared_lock lock(m_mutex);

    auto it = m_pools_by_type.find(resource_type);
    if (it != m_pools_by_type.end()) {
        return it->second;
    }

    return {};
}

size_t ResourceAllocator::optimize_allocations() {
    std::unique_lock lock(m_mutex);

    // This is a simplified optimization implementation
    // In a real system, you would implement sophisticated load balancing

    size_t optimizations = 0;

    // Example: Balance allocations across pools of the same type
    for (auto& [resource_type, pool_names] : m_pools_by_type) {
        if (pool_names.size() < 2) {
            continue;  // Need at least 2 pools to balance
        }

        // Count allocations per pool
        std::unordered_map<std::string, size_t> pool_allocations;
        for (const auto& pool_name : pool_names) {
            pool_allocations[pool_name] = 0;
        }

        for (const auto& [allocation_id, record] : m_active_allocations) {
            if (record.resource_type == resource_type) {
                pool_allocations[record.pool_name]++;
            }
        }

        // Simple optimization: log imbalances (in a real system, you'd
        // rebalance)
        auto [min_it, max_it] = std::minmax_element(
            pool_allocations.begin(), pool_allocations.end(),
            [](const auto& a, const auto& b) { return a.second < b.second; });

        if (max_it->second > min_it->second + 1) {
            qCDebug(resourceAllocatorLog)
                << "Load imbalance detected for resource type"
                << static_cast<int>(resource_type) << "- max:" << max_it->second
                << "min:" << min_it->second;
            optimizations++;
        }
    }

    if (optimizations > 0) {
        emit allocations_optimized(static_cast<int>(optimizations));
    }

    return optimizations;
}

std::string ResourceAllocator::generate_allocation_id() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
}

std::string ResourceAllocator::select_pool(
    ResourceType resource_type, std::string_view plugin_id,
    ResourcePriority priority, const AllocationPolicy& policy) const {
    auto pools_it = m_pools_by_type.find(resource_type);
    if (pools_it == m_pools_by_type.end() || pools_it->second.empty()) {
        return {};
    }

    const auto& pool_names = pools_it->second;

    // Filter available pools
    std::vector<std::string> available_pools;
    for (const auto& pool_name : pool_names) {
        auto pool_it = m_pools.find(pool_name);
        if (pool_it != m_pools.end() &&
            pool_it->second->can_allocate(plugin_id, priority)) {
            available_pools.push_back(pool_name);
        }
    }

    if (available_pools.empty()) {
        return {};
    }

    // Select pool based on strategy
    switch (policy.strategy) {
        case AllocationStrategy::FirstFit:
            return available_pools.front();

        case AllocationStrategy::RoundRobin: {
            // Simple round-robin based on hash of plugin_id
            std::hash<std::string> hasher;
            size_t index =
                hasher(std::string(plugin_id)) % available_pools.size();
            return available_pools[index];
        }

        case AllocationStrategy::LoadBalanced: {
            // Select pool with least active resources
            std::string best_pool;
            size_t min_active = SIZE_MAX;

            for (const auto& pool_name : available_pools) {
                auto pool_it = m_pools.find(pool_name);
                if (pool_it != m_pools.end()) {
                    size_t active = pool_it->second->active_count();
                    if (active < min_active) {
                        min_active = active;
                        best_pool = pool_name;
                    }
                }
            }

            return best_pool;
        }

        case AllocationStrategy::BestFit:
        case AllocationStrategy::WorstFit:
        case AllocationStrategy::Priority:
        default:
            // Default to first fit
            return available_pools.front();
    }
}

bool ResourceAllocator::check_allocation_limits(
    ResourceType resource_type, std::string_view plugin_id,
    const AllocationPolicy& policy) const {
    // Check per-plugin allocation limit
    if (policy.max_allocations_per_plugin > 0) {
        size_t plugin_allocations =
            count_plugin_allocations(plugin_id, resource_type);
        if (plugin_allocations >= policy.max_allocations_per_plugin) {
            return false;
        }
    }

    return true;
}

void ResourceAllocator::initialize_default_policies() {
    // Set default policies for each resource type
    AllocationPolicy default_policy;
    default_policy.strategy = AllocationStrategy::LoadBalanced;
    default_policy.min_priority = ResourcePriority::Low;
    default_policy.max_allocations_per_plugin = 10;
    default_policy.allocation_timeout = std::chrono::milliseconds(5000);
    default_policy.allow_preemption = false;
    default_policy.enable_load_balancing = true;

    // Apply to all resource types
    for (int i = 0; i < static_cast<int>(ResourceType::Custom); ++i) {
        m_policies[static_cast<ResourceType>(i)] = default_policy;
    }
}

size_t ResourceAllocator::count_plugin_allocations(
    std::string_view plugin_id, ResourceType resource_type) const {
    size_t count = 0;

    for (const auto& [allocation_id, record] : m_active_allocations) {
        if (record.plugin_id == plugin_id &&
            record.resource_type == resource_type) {
            count++;
        }
    }

    return count;
}

}  // namespace qtplugin
