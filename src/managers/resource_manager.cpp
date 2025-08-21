/**
 * @file resource_manager.cpp
 * @brief Implementation of resource management system
 * @version 3.0.0
 */

#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <algorithm>
#include <qtplugin/managers/resource_manager_impl.hpp>
#include <random>

Q_LOGGING_CATEGORY(resourceLog, "qtplugin.resource")

namespace qtplugin {

// === Utility Functions ===

std::string resource_type_to_string(ResourceType type) {
    switch (type) {
        case ResourceType::Thread:
            return "thread";
        case ResourceType::Timer:
            return "timer";
        case ResourceType::NetworkConnection:
            return "network_connection";
        case ResourceType::FileHandle:
            return "file_handle";
        case ResourceType::DatabaseConnection:
            return "database_connection";
        case ResourceType::Memory:
            return "memory";
        case ResourceType::Custom:
            return "custom";
    }
    return "unknown";
}

std::optional<ResourceType> string_to_resource_type(std::string_view str) {
    if (str == "thread")
        return ResourceType::Thread;
    if (str == "timer")
        return ResourceType::Timer;
    if (str == "network_connection")
        return ResourceType::NetworkConnection;
    if (str == "file_handle")
        return ResourceType::FileHandle;
    if (str == "database_connection")
        return ResourceType::DatabaseConnection;
    if (str == "memory")
        return ResourceType::Memory;
    if (str == "custom")
        return ResourceType::Custom;
    return std::nullopt;
}

std::string resource_state_to_string(ResourceState state) {
    switch (state) {
        case ResourceState::Available:
            return "available";
        case ResourceState::InUse:
            return "in_use";
        case ResourceState::Reserved:
            return "reserved";
        case ResourceState::Cleanup:
            return "cleanup";
        case ResourceState::Error:
            return "error";
    }
    return "unknown";
}

std::string resource_priority_to_string(ResourcePriority priority) {
    switch (priority) {
        case ResourcePriority::Low:
            return "low";
        case ResourcePriority::Normal:
            return "normal";
        case ResourcePriority::High:
            return "high";
        case ResourcePriority::Critical:
            return "critical";
    }
    return "unknown";
}

// === Resource Factories Implementation ===

qtplugin::expected<std::unique_ptr<QThread>, PluginError>
ThreadResourceFactory::create_resource(const ResourceHandle& handle) {
    Q_UNUSED(handle)

    auto thread = std::make_unique<QThread>();
    thread->setObjectName(
        QString("PluginThread_%1").arg(QString::fromStdString(handle.id())));

    qCDebug(resourceLog) << "Created thread resource:" << thread->objectName();

    return thread;
}

bool ThreadResourceFactory::can_create_resource(
    const ResourceHandle& handle) const {
    Q_UNUSED(handle)

    // Check system thread limits
    int max_threads =
        QThread::idealThreadCount() * 4;  // Allow 4x ideal thread count
    int current_threads =
        QCoreApplication::instance()->thread()->children().size();

    return current_threads < max_threads;
}

size_t ThreadResourceFactory::get_estimated_cost(
    const ResourceHandle& handle) const {
    Q_UNUSED(handle)

    // Estimated memory cost of a thread (stack size + overhead)
    return 8 * 1024 * 1024;  // 8MB typical stack size
}

qtplugin::expected<std::unique_ptr<QTimer>, PluginError>
TimerResourceFactory::create_resource(const ResourceHandle& handle) {
    Q_UNUSED(handle)

    auto timer = std::make_unique<QTimer>();
    timer->setObjectName(
        QString("PluginTimer_%1").arg(QString::fromStdString(handle.id())));
    timer->setSingleShot(false);

    qCDebug(resourceLog) << "Created timer resource:" << timer->objectName();

    return timer;
}

bool TimerResourceFactory::can_create_resource(
    const ResourceHandle& handle) const {
    Q_UNUSED(handle)

    // Timers are lightweight, allow many
    return true;
}

size_t TimerResourceFactory::get_estimated_cost(
    const ResourceHandle& handle) const {
    Q_UNUSED(handle)

    // Estimated memory cost of a timer
    return sizeof(QTimer) + 1024;  // Timer object + some overhead
}

qtplugin::expected<std::unique_ptr<MemoryResource>, PluginError>
MemoryResourceFactory::create_resource(const ResourceHandle& handle) {
    auto size_meta = handle.get_metadata("size");
    if (!size_meta) {
        return make_error<std::unique_ptr<MemoryResource>>(
            PluginErrorCode::InvalidArgument,
            "Memory size not specified in handle metadata");
    }

    try {
        size_t size = std::any_cast<size_t>(size_meta.value());

        if (size == 0 || size > 1024 * 1024 * 1024) {  // Max 1GB per allocation
            return make_error<std::unique_ptr<MemoryResource>>(
                PluginErrorCode::InvalidArgument, "Invalid memory size");
        }

        auto memory = std::make_unique<MemoryResource>(size);

        qCDebug(resourceLog) << "Created memory resource:" << size << "bytes";

        return memory;
    } catch (const std::bad_any_cast& e) {
        return make_error<std::unique_ptr<MemoryResource>>(
            PluginErrorCode::InvalidArgument, "Invalid size type in metadata");
    }
}

bool MemoryResourceFactory::can_create_resource(
    const ResourceHandle& handle) const {
    auto size_meta = handle.get_metadata("size");
    if (!size_meta) {
        return false;
    }

    try {
        size_t size = std::any_cast<size_t>(size_meta.value());

        // Check available memory (simplified check)
        return size > 0 && size <= 1024 * 1024 * 1024;  // Max 1GB
    } catch (const std::bad_any_cast&) {
        return false;
    }
}

size_t MemoryResourceFactory::get_estimated_cost(
    const ResourceHandle& handle) const {
    auto size_meta = handle.get_metadata("size");
    if (!size_meta) {
        return 0;
    }

    try {
        return std::any_cast<size_t>(size_meta.value());
    } catch (const std::bad_any_cast&) {
        return 0;
    }
}

// === ResourceManager Implementation ===

ResourceManager::ResourceManager(QObject* parent)
    : QObject(parent), m_cleanup_timer(std::make_unique<QTimer>(this)) {
    // Set up cleanup timer
    m_cleanup_timer->setSingleShot(false);
    m_cleanup_timer->setInterval(static_cast<int>(m_cleanup_interval.count()));
    connect(m_cleanup_timer.get(), &QTimer::timeout, this,
            &ResourceManager::perform_cleanup);
    m_cleanup_timer->start();

    // Set up default factories
    setup_default_factories();

    qCDebug(resourceLog) << "Resource manager initialized";
}

ResourceManager::~ResourceManager() {
    // Stop cleanup timer
    m_cleanup_timer->stop();

    // Clean up all resources
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    // Clear pools
    m_pools.clear();
    m_pool_types.clear();

    // Clear factories
    m_factories.clear();

    qCDebug(resourceLog) << "Resource manager destroyed";
}

qtplugin::expected<void, PluginError> ResourceManager::create_pool(
    ResourceType type, std::string_view pool_name, const ResourceQuota& quota) {
    Q_UNUSED(quota)  // Quota will be used in full implementation
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    std::string pool_name_str(pool_name);

    // Check if pool already exists
    if (m_pools.find(pool_name_str) != m_pools.end()) {
        return make_error<void>(
            qtplugin::PluginErrorCode::AlreadyExists,
            "Resource pool already exists: " + pool_name_str);
    }

    // Find factory for this resource type
    auto type_factories = m_factories.find(type);
    if (type_factories == m_factories.end() || type_factories->second.empty()) {
        return make_error<void>(PluginErrorCode::NotFound,
                                "No factory registered for resource type: " +
                                    resource_type_to_string(type));
    }

    // Create a generic resource pool for the specified type
    // For this implementation, we'll create a basic pool that can handle any
    // resource type
    try {
        // Create a generic resource pool using void* for type erasure
        // This is a simplified implementation that provides basic pool
        // functionality

        struct GenericResourcePool {
            std::string name;
            ResourceType type;
            ResourceQuota quota;
            std::vector<std::unique_ptr<void, std::function<void(void*)>>>
                available_resources;
            std::unordered_map<
                std::string, std::unique_ptr<void, std::function<void(void*)>>>
                in_use_resources;
            std::atomic<size_t> total_created{0};
            std::atomic<size_t> total_acquired{0};
            std::atomic<size_t> total_released{0};
            mutable std::shared_mutex mutex;

            GenericResourcePool(std::string n, ResourceType t,
                                const ResourceQuota& q)
                : name(std::move(n)), type(t), quota(q) {}
        };

        auto pool =
            std::make_unique<GenericResourcePool>(pool_name_str, type, quota);

        // Store with type erasure
        auto deleter = [](void* ptr) {
            delete static_cast<GenericResourcePool*>(ptr);
        };

        m_pools[pool_name_str] =
            std::unique_ptr<void, std::function<void(void*)>>(pool.release(),
                                                              deleter);

        // Store the resource type for this pool
        switch (type) {
            case ResourceType::Thread:
                m_pool_types.emplace(pool_name_str,
                                     std::type_index(typeid(std::thread)));
                break;
            case ResourceType::Timer:
                m_pool_types.emplace(pool_name_str,
                                     std::type_index(typeid(QTimer)));
                break;
            case ResourceType::NetworkConnection:
                m_pool_types.emplace(pool_name_str,
                                     std::type_index(typeid(void*)));
                break;
            case ResourceType::FileHandle:
                m_pool_types.emplace(pool_name_str,
                                     std::type_index(typeid(void*)));
                break;
            case ResourceType::DatabaseConnection:
                m_pool_types.emplace(pool_name_str,
                                     std::type_index(typeid(void*)));
                break;
            case ResourceType::Memory:
                m_pool_types.emplace(pool_name_str,
                                     std::type_index(typeid(void*)));
                break;
            case ResourceType::Custom:
                m_pool_types.emplace(pool_name_str,
                                     std::type_index(typeid(void)));
                break;
        }

    } catch (const std::exception& e) {
        return make_error<void>(
            PluginErrorCode::ResourceUnavailable,
            "Failed to create resource pool: " + std::string(e.what()));
    } catch (...) {
        return make_error<void>(PluginErrorCode::ResourceUnavailable,
                                "Unknown error creating resource pool");
    }

    qCDebug(resourceLog)
        << "Created resource pool:" << QString::fromStdString(pool_name_str)
        << "for type:" << QString::fromStdString(resource_type_to_string(type));

    return make_success();
}

qtplugin::expected<void, PluginError> ResourceManager::remove_pool(
    std::string_view pool_name) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    std::string pool_name_str(pool_name);

    auto it = m_pools.find(pool_name_str);
    if (it == m_pools.end()) {
        return make_error<void>(PluginErrorCode::NotFound,
                                "Resource pool not found: " + pool_name_str);
    }

    // Remove pool and its type mapping
    m_pools.erase(it);
    m_pool_types.erase(pool_name_str);

    qCDebug(resourceLog) << "Removed resource pool:"
                         << QString::fromStdString(pool_name_str);

    return make_success();
}

ResourceUsageStats ResourceManager::get_usage_statistics(
    std::optional<ResourceType> type, std::string_view plugin_id) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    ResourceUsageStats combined_stats;

    // This is a simplified implementation
    // In practice, you'd aggregate statistics from all relevant pools
    Q_UNUSED(type)
    Q_UNUSED(plugin_id)

    return combined_stats;
}

std::vector<ResourceHandle> ResourceManager::get_active_resources(
    std::string_view plugin_id) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    std::vector<ResourceHandle> handles;

    // This is a simplified implementation
    // In practice, you'd collect handles from all pools
    Q_UNUSED(plugin_id)

    return handles;
}

void ResourceManager::setup_default_factories() {
    // Register default factories
    auto thread_factory = std::make_unique<ThreadResourceFactory>();
    register_factory<QThread>(ResourceType::Thread, std::move(thread_factory));

    auto timer_factory = std::make_unique<TimerResourceFactory>();
    register_factory<QTimer>(ResourceType::Timer, std::move(timer_factory));

    auto memory_factory = std::make_unique<MemoryResourceFactory>();
    register_factory<MemoryResource>(ResourceType::Memory,
                                     std::move(memory_factory));

    qCDebug(resourceLog) << "Default resource factories registered";
}

qtplugin::expected<void, PluginError> ResourceManager::set_plugin_quota(
    std::string_view plugin_id, ResourceType type, const ResourceQuota& quota) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    std::string plugin_id_str(plugin_id);
    m_plugin_quotas[plugin_id_str][type] = quota;

    qCDebug(resourceLog)
        << "Set quota for plugin:" << QString::fromStdString(plugin_id_str)
        << "type:" << QString::fromStdString(resource_type_to_string(type))
        << "max_instances:" << quota.max_instances;

    return make_success();
}

qtplugin::expected<ResourceQuota, PluginError>
ResourceManager::get_plugin_quota(std::string_view plugin_id,
                                  ResourceType type) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    std::string plugin_id_str(plugin_id);
    auto plugin_it = m_plugin_quotas.find(plugin_id_str);
    if (plugin_it == m_plugin_quotas.end()) {
        return make_error<ResourceQuota>(
            PluginErrorCode::NotFound,
            "No quota found for plugin: " + plugin_id_str);
    }

    auto type_it = plugin_it->second.find(type);
    if (type_it == plugin_it->second.end()) {
        return make_error<ResourceQuota>(PluginErrorCode::NotFound,
                                         "No quota found for resource type: " +
                                             resource_type_to_string(type));
    }

    return type_it->second;
}

size_t ResourceManager::cleanup_plugin_resources(
    std::string_view plugin_id, std::optional<ResourceType> type) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    size_t cleaned = 0;

    // This is a simplified implementation
    // In practice, you'd iterate through all pools and clean up resources for
    // the specific plugin
    Q_UNUSED(plugin_id)
    Q_UNUSED(type)

    qCDebug(resourceLog) << "Cleaned up" << cleaned << "resources for plugin:"
                         << QString::fromStdString(std::string(plugin_id));

    return cleaned;
}

size_t ResourceManager::cleanup_expired_resources(
    std::chrono::milliseconds max_age) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    size_t cleaned = 0;

    // This is a simplified implementation
    // In practice, you'd iterate through all pools and clean up expired
    // resources
    Q_UNUSED(max_age)

    qCDebug(resourceLog) << "Cleaned up" << cleaned << "expired resources";

    return cleaned;
}

void ResourceManager::set_cleanup_interval(std::chrono::milliseconds interval) {
    m_cleanup_interval = interval;

    if (interval.count() > 0) {
        m_cleanup_timer->setInterval(static_cast<int>(interval.count()));
        if (!m_cleanup_timer->isActive()) {
            m_cleanup_timer->start();
        }
    } else {
        m_cleanup_timer->stop();
    }

    qCDebug(resourceLog) << "Set cleanup interval to:" << interval.count()
                         << "ms";
}

std::chrono::milliseconds ResourceManager::get_cleanup_interval() const {
    return m_cleanup_interval;
}

std::string ResourceManager::subscribe_to_events(
    std::function<void(const ResourceHandle&, ResourceState, ResourceState)>
        callback,
    std::optional<ResourceType> type, std::string_view plugin_id) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    std::string subscription_id = generate_subscription_id();

    auto subscription = std::make_unique<EventSubscription>();
    subscription->id = subscription_id;
    subscription->callback = std::move(callback);
    subscription->type_filter = type;
    subscription->plugin_filter = plugin_id;

    m_event_subscriptions[subscription_id] = std::move(subscription);

    qCDebug(resourceLog) << "Created event subscription:"
                         << QString::fromStdString(subscription_id);

    return subscription_id;
}

qtplugin::expected<void, PluginError> ResourceManager::unsubscribe_from_events(
    const std::string& subscription_id) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    auto it = m_event_subscriptions.find(subscription_id);
    if (it == m_event_subscriptions.end()) {
        return make_error<void>(
            PluginErrorCode::NotFound,
            "Event subscription not found: " + subscription_id);
    }

    m_event_subscriptions.erase(it);

    qCDebug(resourceLog) << "Removed event subscription:"
                         << QString::fromStdString(subscription_id);

    return make_success();
}

QJsonObject ResourceManager::get_statistics() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    QJsonObject stats;

    stats["pools_count"] = static_cast<qint64>(m_pools.size());
    stats["factories_count"] = static_cast<qint64>(m_factories.size());
    stats["event_subscriptions_count"] =
        static_cast<qint64>(m_event_subscriptions.size());
    stats["plugin_quotas_count"] = static_cast<qint64>(m_plugin_quotas.size());
    stats["cleanup_interval_ms"] =
        static_cast<qint64>(m_cleanup_interval.count());
    stats["tracking_enabled"] = m_tracking_enabled.load();

    // Add pool statistics
    QJsonArray pools_array;
    for (const auto& [pool_name, pool] : m_pools) {
        QJsonObject pool_stats;
        pool_stats["name"] = QString::fromStdString(pool_name);
        // Add more pool-specific statistics here
        pools_array.append(pool_stats);
    }
    stats["pools"] = pools_array;

    return stats;
}

std::vector<std::string> ResourceManager::get_pool_names() const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    std::vector<std::string> names;
    names.reserve(m_pools.size());

    for (const auto& [pool_name, pool] : m_pools) {
        names.push_back(pool_name);
    }

    return names;
}

bool ResourceManager::is_resource_type_supported(ResourceType type) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    auto it = m_factories.find(type);
    return it != m_factories.end() && !it->second.empty();
}

size_t ResourceManager::get_total_memory_usage(
    std::string_view plugin_id) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    size_t total = 0;

    // This is a simplified implementation
    // In practice, you'd sum up memory usage from all pools
    Q_UNUSED(plugin_id)

    return total;
}

void ResourceManager::set_tracking_enabled(bool enabled) {
    m_tracking_enabled.store(enabled);
    qCDebug(resourceLog) << "Resource tracking"
                         << (enabled ? "enabled" : "disabled");
}

bool ResourceManager::is_tracking_enabled() const {
    return m_tracking_enabled.load();
}

void ResourceManager::perform_cleanup() {
    if (!m_tracking_enabled.load()) {
        return;
    }

    size_t total_cleaned = 0;

    // Cleanup expired resources (older than 1 hour by default)
    total_cleaned += cleanup_expired_resources(std::chrono::hours(1));

    if (total_cleaned > 0) {
        emit cleanup_completed(static_cast<int>(total_cleaned));
        qCDebug(resourceLog) << "Automatic cleanup completed, cleaned"
                             << total_cleaned << "resources";
    }
}

std::string ResourceManager::generate_subscription_id() const {
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

void ResourceManager::notify_event(const ResourceHandle& handle,
                                   ResourceState old_state,
                                   ResourceState new_state) {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    for (const auto& [sub_id, subscription] : m_event_subscriptions) {
        if (subscription) {
            // Check filters
            bool matches = true;

            if (subscription->type_filter &&
                subscription->type_filter.value() != handle.type()) {
                matches = false;
            }

            if (matches && !subscription->plugin_filter.empty() &&
                subscription->plugin_filter != handle.plugin_id()) {
                matches = false;
            }

            if (matches) {
                try {
                    subscription->callback(handle, old_state, new_state);
                } catch (const std::exception& e) {
                    qCWarning(resourceLog)
                        << "Exception in resource event callback:" << e.what();
                }
            }
        }
    }
}

// Protected method implementations
qtplugin::expected<void, PluginError> ResourceManager::register_factory_impl(
    ResourceType type, std::type_index type_index,
    std::unique_ptr<void, std::function<void(void*)>> factory) {
    std::unique_lock<std::shared_mutex> lock(m_mutex);

    m_factories[type][type_index] = std::move(factory);

    qCDebug(resourceLog) << "Registered factory for resource type:"
                         << QString::fromStdString(
                                resource_type_to_string(type));

    return make_success();
}

qtplugin::expected<void*, PluginError> ResourceManager::get_pool_impl(
    std::string_view pool_name, std::type_index type_index) {
    std::shared_lock<std::shared_mutex> lock(m_mutex);

    std::string pool_name_str(pool_name);

    auto pool_it = m_pools.find(pool_name_str);
    if (pool_it == m_pools.end()) {
        return make_error<void*>(PluginErrorCode::NotFound,
                                 "Resource pool not found: " + pool_name_str);
    }

    auto type_it = m_pool_types.find(pool_name_str);
    if (type_it == m_pool_types.end() || type_it->second != type_index) {
        return make_error<void*>(PluginErrorCode::InvalidArgument,
                                 "Pool type mismatch for: " + pool_name_str);
    }

    return pool_it->second.get();
}

qtplugin::expected<void, PluginError> ResourceManager::release_resource_impl(
    const ResourceHandle& handle,
    std::unique_ptr<void, std::function<void(void*)>> resource) {
    // This is a simplified implementation
    // In practice, you'd find the appropriate pool and release the resource
    Q_UNUSED(handle)
    Q_UNUSED(resource)

    return make_success();
}

// Factory function
std::unique_ptr<IResourceManager> create_resource_manager(QObject* parent) {
    return std::make_unique<ResourceManager>(parent);
}

}  // namespace qtplugin
