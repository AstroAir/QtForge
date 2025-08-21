/**
 * @file resource_pool.cpp
 * @brief Implementation of resource pool
 * @version 3.0.0
 */

#include "../../../include/qtplugin/managers/components/resource_pool.hpp"
#include <QDebug>
#include <QLoggingCategory>
#include <QUuid>
#include <algorithm>
#include <random>

Q_LOGGING_CATEGORY(resourcePoolLog, "qtplugin.resource.pool")

namespace qtplugin {

// ResourcePoolBase implementation
ResourcePoolBase::ResourcePoolBase(QObject* parent) : QObject(parent) {}

template <typename T>
ResourcePool<T>::ResourcePool(const std::string& name, ResourceType type,
                              QObject* parent)
    : ResourcePoolBase(parent), m_name(name), m_resource_type(type) {
    qCDebug(resourcePoolLog)
        << "Resource pool created:" << QString::fromStdString(name);
}

template <typename T>
ResourcePool<T>::~ResourcePool() {
    clear();
    qCDebug(resourcePoolLog)
        << "Resource pool destroyed:" << QString::fromStdString(m_name);
}

template <typename T>
std::string ResourcePool<T>::name() const {
    return m_name;
}

template <typename T>
ResourceType ResourcePool<T>::resource_type() const {
    return m_resource_type;
}

template <typename T>
void ResourcePool<T>::set_quota(const ResourceQuota& quota) {
    std::unique_lock lock(m_mutex);
    m_quota = quota;
    qCDebug(resourcePoolLog)
        << "Quota updated for pool" << QString::fromStdString(m_name);
}

template <typename T>
ResourceQuota ResourcePool<T>::get_quota() const {
    std::shared_lock lock(m_mutex);
    return m_quota;
}

template <typename T>
ResourceUsageStats ResourcePool<T>::get_statistics() const {
    std::shared_lock lock(m_mutex);

    ResourceUsageStats stats;
    stats.currently_active = m_active_resources.size();
    stats.total_created = m_total_acquisitions.load();
    stats.total_destroyed = m_total_releases.load();
    stats.peak_usage = std::max(stats.currently_active, stats.total_created);

    return stats;
}

template <typename T>
size_t ResourcePool<T>::cleanup_resources() {
    std::unique_lock lock(m_mutex);

    size_t cleaned_count = 0;
    auto now = std::chrono::system_clock::now();

    // Clean up expired available resources
    std::queue<std::unique_ptr<PooledResource<T>>> new_available;
    while (!m_available_resources.empty()) {
        auto resource = std::move(m_available_resources.front());
        m_available_resources.pop();

        if (!is_resource_expired(*resource)) {
            new_available.push(std::move(resource));
        } else {
            ++cleaned_count;
        }
    }
    m_available_resources = std::move(new_available);

    // Clean up expired active resources (if they're not in use)
    auto it = m_active_resources.begin();
    while (it != m_active_resources.end()) {
        if (!it->second->in_use && is_resource_expired(*it->second)) {
            it = m_active_resources.erase(it);
            ++cleaned_count;
        } else {
            ++it;
        }
    }

    m_total_cleanups.fetch_add(cleaned_count);

    if (cleaned_count > 0) {
        qCDebug(resourcePoolLog)
            << "Cleaned up" << cleaned_count << "resources from pool"
            << QString::fromStdString(m_name);
        emit resources_cleaned_up(static_cast<int>(cleaned_count));
    }

    return cleaned_count;
}

template <typename T>
void ResourcePool<T>::clear() {
    std::unique_lock lock(m_mutex);

    size_t total_cleared =
        m_active_resources.size() + m_available_resources.size();

    m_active_resources.clear();
    while (!m_available_resources.empty()) {
        m_available_resources.pop();
    }

    qCDebug(resourcePoolLog)
        << "Cleared" << total_cleared << "resources from pool"
        << QString::fromStdString(m_name);
}

template <typename T>
bool ResourcePool<T>::can_allocate(std::string_view plugin_id,
                                   ResourcePriority priority) const {
    std::shared_lock lock(m_mutex);

    // Check quota limits
    if (!check_quota_limits(plugin_id)) {
        return false;
    }

    // Check priority requirements
    if (priority < m_quota.min_priority) {
        return false;
    }

    return true;
}

template <typename T>
size_t ResourcePool<T>::available_count() const {
    std::shared_lock lock(m_mutex);
    return m_available_resources.size();
}

template <typename T>
size_t ResourcePool<T>::active_count() const {
    std::shared_lock lock(m_mutex);
    return m_active_resources.size();
}

template <typename T>
qtplugin::expected<std::pair<ResourceHandle, std::unique_ptr<T>>, PluginError>
ResourcePool<T>::acquire_resource(std::string_view plugin_id,
                                  ResourcePriority priority) {
    std::unique_lock lock(m_mutex);

    // Check if allocation is allowed
    if (!can_allocate(plugin_id, priority)) {
        emit quota_exceeded(QString::fromStdString(std::string(plugin_id)));
        return make_error<std::pair<ResourceHandle, std::unique_ptr<T>>>(
            PluginErrorCode::ResourceUnavailable,
            "Resource quota exceeded or priority too low");
    }

    std::unique_ptr<PooledResource<T>> pooled_resource;

    // Try to reuse an available resource first
    if (!m_available_resources.empty()) {
        pooled_resource = std::move(m_available_resources.front());
        m_available_resources.pop();

        // Update resource info
        pooled_resource->owner_plugin_id = std::string(plugin_id);
        pooled_resource->priority = priority;
        pooled_resource->last_used = std::chrono::system_clock::now();
        pooled_resource->in_use = true;
        pooled_resource->use_count++;
    } else {
        // Create new resource
        pooled_resource = create_new_resource(plugin_id, priority);
        if (!pooled_resource) {
            return make_error<std::pair<ResourceHandle, std::unique_ptr<T>>>(
                PluginErrorCode::ResourceUnavailable,
                "Failed to create new resource");
        }
    }

    // Generate handle and move resource out of pool
    std::string handle_str = generate_handle();
    ResourceHandle handle(handle_str, m_resource_type, std::string(plugin_id));

    auto resource = std::move(pooled_resource->resource);

    // Store in active resources
    m_active_resources[handle_str] = std::move(pooled_resource);

    m_total_acquisitions.fetch_add(1);

    qCDebug(resourcePoolLog)
        << "Resource acquired from pool" << QString::fromStdString(m_name)
        << "by plugin" << QString::fromStdString(std::string(plugin_id));

    emit resource_acquired(QString::fromStdString(handle_str),
                           QString::fromStdString(std::string(plugin_id)));

    return std::make_pair(std::move(handle), std::move(resource));
}

template <typename T>
qtplugin::expected<void, PluginError> ResourcePool<T>::release_resource(
    const ResourceHandle& handle, std::unique_ptr<T> resource) {
    std::unique_lock lock(m_mutex);

    auto it = m_active_resources.find(handle.id);
    if (it == m_active_resources.end()) {
        return make_error<void>(PluginErrorCode::InvalidArgument,
                                "Invalid resource handle");
    }

    auto& pooled_resource = it->second;

    // Update resource info
    pooled_resource->resource = std::move(resource);
    pooled_resource->last_used = std::chrono::system_clock::now();
    pooled_resource->in_use = false;

    // Move to available resources if pool has space
    if (m_quota.max_instances == 0 ||
        m_available_resources.size() < m_quota.max_instances) {
        m_available_resources.push(std::move(pooled_resource));
    }

    // Remove from active resources
    m_active_resources.erase(it);

    m_total_releases.fetch_add(1);

    qCDebug(resourcePoolLog)
        << "Resource released to pool" << QString::fromStdString(m_name)
        << "by plugin" << QString::fromStdString(handle.plugin_id());

    emit resource_released(QString::fromStdString(handle.id()),
                           QString::fromStdString(handle.plugin_id()));

    return make_success();
}

template <typename T>
void ResourcePool<T>::set_factory(std::function<std::unique_ptr<T>()> factory) {
    std::unique_lock lock(m_mutex);
    m_factory = std::move(factory);
}

template <typename T>
std::string ResourcePool<T>::generate_handle() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
}

template <typename T>
std::unique_ptr<PooledResource<T>> ResourcePool<T>::create_new_resource(
    std::string_view plugin_id, ResourcePriority priority) {
    if (!m_factory) {
        return nullptr;
    }

    try {
        auto resource = m_factory();
        if (!resource) {
            return nullptr;
        }

        auto pooled = std::make_unique<PooledResource<T>>();
        pooled->resource = std::move(resource);
        pooled->created_at = std::chrono::system_clock::now();
        pooled->last_used = pooled->created_at;
        pooled->owner_plugin_id = std::string(plugin_id);
        pooled->priority = priority;
        pooled->in_use = true;
        pooled->use_count = 1;

        return pooled;
    } catch (const std::exception& e) {
        qCWarning(resourcePoolLog) << "Failed to create resource:" << e.what();
        return nullptr;
    }
}

template <typename T>
std::unique_ptr<PooledResource<T>> ResourcePool<T>::try_reuse_resource(
    std::string_view plugin_id, ResourcePriority priority) {
    Q_UNUSED(plugin_id);
    Q_UNUSED(priority);

    if (m_available_resources.empty()) {
        return nullptr;
    }

    auto resource = std::move(m_available_resources.front());
    m_available_resources.pop();

    return resource;
}

template <typename T>
bool ResourcePool<T>::is_resource_expired(
    const PooledResource<T>& resource) const {
    if (m_quota.max_lifetime.count() == 0) {
        return false;  // No expiration
    }

    auto now = std::chrono::system_clock::now();
    auto age = now - resource.created_at;

    return age > m_quota.max_lifetime;
}

template <typename T>
bool ResourcePool<T>::check_quota_limits(std::string_view plugin_id) const {
    Q_UNUSED(plugin_id);

    // Check instance limit
    if (m_quota.max_instances > 0) {
        size_t total_instances =
            m_active_resources.size() + m_available_resources.size();
        if (total_instances >= m_quota.max_instances) {
            return false;
        }
    }

    // Check memory limit
    if (m_quota.max_memory_bytes > 0) {
        if (calculate_memory_usage() >= m_quota.max_memory_bytes) {
            return false;
        }
    }

    return true;
}

template <typename T>
size_t ResourcePool<T>::calculate_memory_usage() const {
    // This is a simplified calculation
    // In a real implementation, you would calculate actual memory usage
    size_t total_resources =
        m_active_resources.size() + m_available_resources.size();
    return total_resources * sizeof(T);  // Rough estimate
}

}  // namespace qtplugin
