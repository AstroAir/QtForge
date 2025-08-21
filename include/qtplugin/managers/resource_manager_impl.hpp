/**
 * @file resource_manager_impl.hpp
 * @brief Concrete implementation of resource management system
 * @version 3.0.0
 */

#pragma once

#include <QMutex>
#include <QObject>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <condition_variable>
#include <deque>
#include <queue>
#include <thread>
#include "resource_manager.hpp"

namespace qtplugin {

// === Built-in Resource Factories ===

/**
 * @brief Thread resource factory
 */
class ThreadResourceFactory : public IResourceFactory<QThread> {
public:
    qtplugin::expected<std::unique_ptr<QThread>, PluginError> create_resource(
        const ResourceHandle& handle) override;

    bool can_create_resource(const ResourceHandle& handle) const override;
    size_t get_estimated_cost(const ResourceHandle& handle) const override;
    std::string name() const override { return "thread"; }
};

/**
 * @brief Timer resource factory
 */
class TimerResourceFactory : public IResourceFactory<QTimer> {
public:
    qtplugin::expected<std::unique_ptr<QTimer>, PluginError> create_resource(
        const ResourceHandle& handle) override;

    bool can_create_resource(const ResourceHandle& handle) const override;
    size_t get_estimated_cost(const ResourceHandle& handle) const override;
    std::string name() const override { return "timer"; }
};

/**
 * @brief Memory resource for tracking memory allocations
 */
class MemoryResource {
public:
    explicit MemoryResource(size_t size)
        : m_size(size), m_data(std::make_unique<char[]>(size)) {}
    ~MemoryResource() = default;

    size_t size() const { return m_size; }
    void* data() { return m_data.get(); }
    const void* data() const { return m_data.get(); }

private:
    size_t m_size;
    std::unique_ptr<char[]> m_data;
};

/**
 * @brief Memory resource factory
 */
class MemoryResourceFactory : public IResourceFactory<MemoryResource> {
public:
    qtplugin::expected<std::unique_ptr<MemoryResource>, PluginError>
    create_resource(const ResourceHandle& handle) override;

    bool can_create_resource(const ResourceHandle& handle) const override;
    size_t get_estimated_cost(const ResourceHandle& handle) const override;
    std::string name() const override { return "memory"; }
};

// === Built-in Resource Pools ===

/**
 * @brief Generic resource pool implementation
 */
template <typename T>
class BasicResourcePool : public IResourcePool<T> {
public:
    explicit BasicResourcePool(std::string name,
                               std::unique_ptr<IResourceFactory<T>> factory,
                               const ResourceQuota& quota = {})
        : m_name(std::move(name)),
          m_factory(std::move(factory)),
          m_quota(quota) {}

    ~BasicResourcePool() override { cleanup_all_resources(); }

    qtplugin::expected<std::pair<ResourceHandle, std::unique_ptr<T>>,
                       PluginError>
    acquire_resource(
        std::string_view plugin_id,
        ResourcePriority priority = ResourcePriority::Normal) override {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        // Check quota
        if (m_quota.max_instances > 0 &&
            m_active_resources.size() >= m_quota.max_instances) {
            // Try to find a resource to reuse or wait
            if (auto reused = try_reuse_resource(plugin_id, priority)) {
                return std::move(*reused);
            }
            return make_error<std::pair<ResourceHandle, std::unique_ptr<T>>>(
                PluginErrorCode::ResourceUnavailable,
                "Resource quota exceeded");
        }

        // Check priority
        if (priority < m_quota.min_priority) {
            return make_error<std::pair<ResourceHandle, std::unique_ptr<T>>>(
                PluginErrorCode::ResourceUnavailable, "Priority too low");
        }

        // Create new resource
        std::string resource_id = generate_resource_id();
        ResourceHandle handle(resource_id, get_resource_type(),
                              std::string(plugin_id));
        handle.set_priority(priority);
        handle.set_state(ResourceState::Reserved);

        if (!m_factory->can_create_resource(handle)) {
            m_stats.allocation_failures++;
            return make_error<std::pair<ResourceHandle, std::unique_ptr<T>>>(
                PluginErrorCode::ResourceUnavailable,
                "Factory cannot create resource");
        }

        auto resource_result = m_factory->create_resource(handle);
        if (!resource_result) {
            m_stats.allocation_failures++;
            return qtplugin::expected<
                std::pair<ResourceHandle, std::unique_ptr<T>>, PluginError>(
                qtplugin::unexpected(resource_result.error()));
        }

        handle.set_state(ResourceState::InUse);
        handle.update_access_time();

        m_active_resources[resource_id] = handle;
        m_stats.total_created++;
        m_stats.currently_active++;
        if (m_stats.currently_active > m_stats.peak_usage) {
            m_stats.peak_usage = m_stats.currently_active;
        }

        return std::make_pair(std::move(handle),
                              std::move(resource_result.value()));
    }

    qtplugin::expected<void, PluginError> release_resource(
        const ResourceHandle& handle, std::unique_ptr<T> resource) override {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        auto it = m_active_resources.find(handle.id());
        if (it == m_active_resources.end()) {
            return make_error<void>(PluginErrorCode::NotFound,
                                    "Resource handle not found");
        }

        // Update statistics
        auto lifetime = handle.age();
        m_stats.total_usage_time += lifetime;
        m_stats.average_lifetime = std::chrono::milliseconds(
            m_stats.total_usage_time.count() /
            std::max(size_t(1), m_stats.total_created));

        // Check if resource can be reused
        if (can_reuse_resource(handle, resource.get())) {
            // Add to available pool
            it->second.set_state(ResourceState::Available);
            m_available_resources.push({it->second, std::move(resource)});
        } else {
            // Destroy resource
            m_active_resources.erase(it);
            m_stats.currently_active--;
            m_stats.total_destroyed++;
        }

        return make_success();
    }

    ResourceUsageStats get_statistics() const override {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_stats;
    }

    void set_quota(const ResourceQuota& quota) override {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_quota = quota;
    }

    ResourceQuota get_quota() const override {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return m_quota;
    }

    size_t cleanup_resources() override {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        size_t cleaned = 0;
        auto now = std::chrono::steady_clock::now();

        // Clean up expired available resources
        while (!m_available_resources.empty()) {
            auto& [handle, resource] = m_available_resources.front();

            bool should_cleanup = false;
            if (m_quota.max_lifetime.count() > 0) {
                auto age =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - handle.created_at());
                should_cleanup = age >= m_quota.max_lifetime;
            }

            if (should_cleanup || !is_resource_healthy(resource.get())) {
                m_available_resources.pop();
                m_active_resources.erase(handle.id());
                m_stats.currently_active--;
                m_stats.total_destroyed++;
                cleaned++;
            } else {
                break;
            }
        }

        return cleaned;
    }

    std::string name() const override { return m_name; }

private:
    std::string m_name;
    std::unique_ptr<IResourceFactory<T>> m_factory;
    ResourceQuota m_quota;
    ResourceUsageStats m_stats;

    mutable std::shared_mutex m_mutex;
    std::unordered_map<std::string, ResourceHandle> m_active_resources;
    std::queue<std::pair<ResourceHandle, std::unique_ptr<T>>>
        m_available_resources;

    std::atomic<size_t> m_resource_counter{0};

    std::string generate_resource_id() {
        return m_name + "_" + std::to_string(m_resource_counter.fetch_add(1));
    }

    ResourceType get_resource_type() const {
        // This would be determined by the factory type
        return ResourceType::Custom;
    }

    std::optional<std::pair<ResourceHandle, std::unique_ptr<T>>>
    try_reuse_resource(std::string_view plugin_id, ResourcePriority priority) {
        (void)plugin_id;  // Suppress unused parameter warning
        (void)priority;   // Suppress unused parameter warning

        if (m_available_resources.empty()) {
            return std::nullopt;
        }

        auto [handle, resource] = std::move(
            const_cast<std::pair<ResourceHandle, std::unique_ptr<T>>&>(
                m_available_resources.front()));
        m_available_resources.pop();

        handle.set_state(ResourceState::InUse);
        handle.update_access_time();

        return std::make_pair(std::move(handle), std::move(resource));
    }

    bool can_reuse_resource(const ResourceHandle& handle, T* resource) const {
        Q_UNUSED(handle)
        return is_resource_healthy(resource);
    }

    bool is_resource_healthy(T* resource) const {
        Q_UNUSED(resource)
        // Default implementation - can be overridden for specific resource
        // types
        return true;
    }

    void cleanup_all_resources() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        // Clear available resources
        while (!m_available_resources.empty()) {
            m_available_resources.pop();
        }

        // Clear active resources
        m_active_resources.clear();
        m_stats.currently_active = 0;
    }
};

// === Main Implementation ===

/**
 * @brief Default resource manager implementation
 */
class ResourceManager : public QObject, public IResourceManager {
    Q_OBJECT

public:
    explicit ResourceManager(QObject* parent = nullptr);
    ~ResourceManager() override;

    // IResourceManager implementation
    qtplugin::expected<void, PluginError> create_pool(
        ResourceType type, std::string_view pool_name,
        const ResourceQuota& quota = {}) override;

    qtplugin::expected<void, PluginError> remove_pool(
        std::string_view pool_name) override;

    ResourceUsageStats get_usage_statistics(
        std::optional<ResourceType> type = std::nullopt,
        std::string_view plugin_id = {}) const override;

    std::vector<ResourceHandle> get_active_resources(
        std::string_view plugin_id = {}) const override;

    qtplugin::expected<void, PluginError> set_plugin_quota(
        std::string_view plugin_id, ResourceType type,
        const ResourceQuota& quota) override;

    qtplugin::expected<ResourceQuota, PluginError> get_plugin_quota(
        std::string_view plugin_id, ResourceType type) const override;

    size_t cleanup_plugin_resources(
        std::string_view plugin_id,
        std::optional<ResourceType> type = std::nullopt) override;

    size_t cleanup_expired_resources(
        std::chrono::milliseconds max_age) override;

    void set_cleanup_interval(std::chrono::milliseconds interval) override;
    std::chrono::milliseconds get_cleanup_interval() const override;

    std::string subscribe_to_events(
        std::function<void(const ResourceHandle&, ResourceState, ResourceState)>
            callback,
        std::optional<ResourceType> type = std::nullopt,
        std::string_view plugin_id = {}) override;

    qtplugin::expected<void, PluginError> unsubscribe_from_events(
        const std::string& subscription_id) override;

    QJsonObject get_statistics() const override;
    std::vector<std::string> get_pool_names() const override;
    bool is_resource_type_supported(ResourceType type) const override;
    size_t get_total_memory_usage(
        std::string_view plugin_id = {}) const override;
    void set_tracking_enabled(bool enabled) override;
    bool is_tracking_enabled() const override;

signals:
    void resource_acquired(const QString& resource_id,
                           const QString& plugin_id);
    void resource_released(const QString& resource_id,
                           const QString& plugin_id);
    void quota_exceeded(const QString& plugin_id, int resource_type);
    void cleanup_completed(int resources_cleaned);

protected:
    qtplugin::expected<void, PluginError> register_factory_impl(
        ResourceType type, std::type_index type_index,
        std::unique_ptr<void, std::function<void(void*)>> factory) override;

    qtplugin::expected<void*, PluginError> get_pool_impl(
        std::string_view pool_name, std::type_index type_index) override;

    qtplugin::expected<void, PluginError> release_resource_impl(
        const ResourceHandle& handle,
        std::unique_ptr<void, std::function<void(void*)>> resource) override;

private slots:
    void perform_cleanup();

private:
    // Resource factories
    std::unordered_map<
        ResourceType,
        std::unordered_map<std::type_index,
                           std::unique_ptr<void, std::function<void(void*)>>>>
        m_factories;

    // Resource pools
    std::unordered_map<std::string,
                       std::unique_ptr<void, std::function<void(void*)>>>
        m_pools;
    std::unordered_map<std::string, std::type_index> m_pool_types;

    // Plugin quotas
    std::unordered_map<std::string,
                       std::unordered_map<ResourceType, ResourceQuota>>
        m_plugin_quotas;

    // Event subscriptions
    struct EventSubscription {
        std::string id;
        std::function<void(const ResourceHandle&, ResourceState, ResourceState)>
            callback;
        std::optional<ResourceType> type_filter;
        std::string plugin_filter;
    };
    std::unordered_map<std::string, std::unique_ptr<EventSubscription>>
        m_event_subscriptions;

    // Cleanup timer
    std::unique_ptr<QTimer> m_cleanup_timer;
    std::chrono::milliseconds m_cleanup_interval{60000};  // 1 minute default

    // Settings
    std::atomic<bool> m_tracking_enabled{true};

    // Thread safety
    mutable std::shared_mutex m_mutex;

    // Helper methods
    std::string generate_subscription_id() const;
    void notify_event(const ResourceHandle& handle, ResourceState old_state,
                      ResourceState new_state);
    void setup_default_factories();
};

/**
 * @brief Create a default resource manager instance
 * @param parent Parent QObject
 * @return Unique pointer to resource manager
 */
std::unique_ptr<IResourceManager> create_resource_manager(
    QObject* parent = nullptr);

}  // namespace qtplugin
