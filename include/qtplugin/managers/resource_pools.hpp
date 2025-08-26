/**
 * @file resource_pools.hpp
 * @brief Specialized resource pool implementations
 * @version 3.0.0
 */

#pragma once

#include <QFile>
#ifdef QTFORGE_HAS_NETWORK
#include <QNetworkAccessManager>
#endif
#ifdef QTFORGE_HAS_SQL
#include <QSqlDatabase>
#endif
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <deque>
#include <queue>
#include "resource_manager.hpp"
#include "resource_manager_impl.hpp"
#include "components/resource_pool.hpp"

namespace qtplugin {

// === Thread Pool ===

/**
 * @brief Specialized thread pool for managing QThread instances
 */
class ThreadPool : public ResourcePool<QThread> {
public:
    explicit ThreadPool(const ResourceQuota& quota = {})
        : ResourcePool<QThread>("thread_pool", ResourceType::Thread) {
        // Set reasonable defaults for thread pool
        ResourceQuota default_quota;
        default_quota.max_instances = QThread::idealThreadCount() * 2;
        default_quota.max_lifetime = std::chrono::hours(1);
        default_quota.min_priority = ResourcePriority::Low;

        if (quota.is_unlimited()) {
            set_quota(default_quota);
        }
    }

    /**
     * @brief Acquire a thread with specific configuration
     * @param plugin_id Plugin requesting the thread
     * @param priority Thread priority
     * @param stack_size Stack size for the thread (optional)
     * @return Thread handle and instance or error
     */
    qtplugin::expected<std::pair<ResourceHandle, std::unique_ptr<QThread>>,
                       PluginError>
    acquire_thread(std::string_view plugin_id,
                   ResourcePriority priority = ResourcePriority::Normal,
                   std::optional<size_t> stack_size = std::nullopt) {
        auto result = acquire_resource(plugin_id, priority);
        if (!result) {
            return result;
        }

        auto& [handle, thread] = result.value();

        // Configure thread if stack size is specified
        if (stack_size) {
            thread->setStackSize(static_cast<uint>(stack_size.value()));
        }

        // Set thread name for debugging
        thread->setObjectName(
            QString("Plugin_%1_Thread_%2")
                .arg(QString::fromStdString(std::string(plugin_id)))
                .arg(QString::fromStdString(handle.id())));

        return result;
    }

    /**
     * @brief Get thread pool statistics
     * @return Extended statistics including thread-specific metrics
     */
    QJsonObject get_thread_statistics() const {
        QJsonObject stats;
        auto base_stats = get_statistics();

        stats["total_created"] = static_cast<qint64>(base_stats.total_created);
        stats["currently_active"] =
            static_cast<qint64>(base_stats.currently_active);
        stats["peak_usage"] = static_cast<qint64>(base_stats.peak_usage);
        stats["ideal_thread_count"] = QThread::idealThreadCount();
        stats["utilization_rate"] = base_stats.utilization_rate();

        return stats;
    }

private:
    ResourceType get_resource_type() const { return ResourceType::Thread; }

    bool is_resource_healthy(QThread* thread) const {
        return thread && !thread->isFinished();
    }
};

// === Timer Pool ===

/**
 * @brief Specialized timer pool for managing QTimer instances
 */
class TimerPool : public ResourcePool<QTimer> {
public:
    explicit TimerPool(const ResourceQuota& quota = {})
        : ResourcePool<QTimer>("timer_pool", ResourceType::Timer) {
        // Set reasonable defaults for timer pool
        ResourceQuota default_quota;
        default_quota.max_instances = 1000;  // Allow many timers
        default_quota.max_lifetime = std::chrono::hours(24);
        default_quota.min_priority = ResourcePriority::Low;

        if (quota.is_unlimited()) {
            set_quota(default_quota);
        }
    }

    /**
     * @brief Acquire a timer with specific configuration
     * @param plugin_id Plugin requesting the timer
     * @param priority Timer priority
     * @param interval Timer interval in milliseconds (optional)
     * @param single_shot Whether timer should be single shot (optional)
     * @return Timer handle and instance or error
     */
    qtplugin::expected<std::pair<ResourceHandle, std::unique_ptr<QTimer>>,
                       PluginError>
    acquire_timer(
        std::string_view plugin_id,
        ResourcePriority priority = ResourcePriority::Normal,
        std::optional<std::chrono::milliseconds> interval = std::nullopt,
        std::optional<bool> single_shot = std::nullopt) {
        auto result = acquire_resource(plugin_id, priority);
        if (!result) {
            return result;
        }

        auto& [handle, timer] = result.value();

        // Configure timer
        if (interval) {
            timer->setInterval(static_cast<int>(interval.value().count()));
        }

        if (single_shot) {
            timer->setSingleShot(single_shot.value());
        }

        // Set timer name for debugging
        timer->setObjectName(
            QString("Plugin_%1_Timer_%2")
                .arg(QString::fromStdString(std::string(plugin_id)))
                .arg(QString::fromStdString(handle.id())));

        return result;
    }

    /**
     * @brief Get timer pool statistics
     * @return Extended statistics including timer-specific metrics
     */
    QJsonObject get_timer_statistics() const {
        QJsonObject stats;
        auto base_stats = get_statistics();

        stats["total_created"] = static_cast<qint64>(base_stats.total_created);
        stats["currently_active"] =
            static_cast<qint64>(base_stats.currently_active);
        stats["peak_usage"] = static_cast<qint64>(base_stats.peak_usage);
        stats["utilization_rate"] = base_stats.utilization_rate();

        return stats;
    }

private:
    ResourceType get_resource_type() const { return ResourceType::Timer; }

    bool is_resource_healthy(QTimer* timer) const { return timer != nullptr; }
};

// === Memory Pool ===

/**
 * @brief Specialized memory pool for managing memory allocations
 */
class MemoryPool : public ResourcePool<MemoryResource> {
public:
    explicit MemoryPool(const ResourceQuota& quota = {})
        : ResourcePool<MemoryResource>("memory_pool", ResourceType::Memory) {
        // Set reasonable defaults for memory pool
        ResourceQuota default_quota;
        default_quota.max_instances = 1000;
        default_quota.max_memory_bytes = 1024 * 1024 * 1024;  // 1GB total
        default_quota.max_lifetime = std::chrono::hours(1);
        default_quota.min_priority = ResourcePriority::Low;

        if (quota.is_unlimited()) {
            set_quota(default_quota);
        }
    }

    /**
     * @brief Acquire memory with specific size
     * @param plugin_id Plugin requesting the memory
     * @param size Memory size in bytes
     * @param priority Memory allocation priority
     * @return Memory handle and instance or error
     */
    qtplugin::expected<
        std::pair<ResourceHandle, std::unique_ptr<MemoryResource>>, PluginError>
    acquire_memory(std::string_view plugin_id, size_t size,
                   ResourcePriority priority = ResourcePriority::Normal) {
        // Create handle with size metadata
        std::string resource_id = generate_memory_id();
        ResourceHandle handle(resource_id, ResourceType::Memory,
                              std::string(plugin_id));
        handle.set_priority(priority);
        handle.set_metadata("size", size);

        // Check quota before allocation
        auto quota = get_quota();
        if (quota.max_memory_bytes > 0) {
            size_t current_usage = get_current_memory_usage();
            if (current_usage + size > quota.max_memory_bytes) {
                return make_error<
                    std::pair<ResourceHandle, std::unique_ptr<MemoryResource>>>(
                    PluginErrorCode::ResourceUnavailable,
                    "Memory quota exceeded");
            }
        }

        auto result = acquire_resource(plugin_id, priority);
        if (!result) {
            return result;
        }

        m_total_allocated_bytes += size;

        return result;
    }

    /**
     * @brief Release memory resource
     * @param handle Memory handle
     * @param memory Memory resource
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> release_memory(
        const ResourceHandle& handle, std::unique_ptr<MemoryResource> memory) {
        if (memory) {
            m_total_allocated_bytes -= memory->size();
        }

        return release_resource(handle, std::move(memory));
    }

    /**
     * @brief Get memory pool statistics
     * @return Extended statistics including memory-specific metrics
     */
    QJsonObject get_memory_statistics() const {
        QJsonObject stats;
        auto base_stats = get_statistics();

        stats["total_created"] = static_cast<qint64>(base_stats.total_created);
        stats["currently_active"] =
            static_cast<qint64>(base_stats.currently_active);
        stats["peak_usage"] = static_cast<qint64>(base_stats.peak_usage);
        stats["total_allocated_bytes"] =
            static_cast<qint64>(m_total_allocated_bytes);
        stats["utilization_rate"] = base_stats.utilization_rate();

        return stats;
    }

    /**
     * @brief Get current memory usage
     * @return Current memory usage in bytes
     */
    size_t get_current_memory_usage() const { return m_total_allocated_bytes; }

private:
    std::atomic<size_t> m_total_allocated_bytes{0};
    std::atomic<size_t> m_memory_counter{0};

    std::string generate_memory_id() {
        return "memory_" + std::to_string(m_memory_counter.fetch_add(1));
    }

    ResourceType get_resource_type() const { return ResourceType::Memory; }

    bool is_resource_healthy(MemoryResource* memory) const {
        return memory && memory->data() != nullptr;
    }
};

#ifdef QTFORGE_HAS_NETWORK
// === Network Connection Pool ===

/**
 * @brief Network connection resource
 */
class NetworkConnection {
public:
    explicit NetworkConnection(const QString& host, int port = 80)
        : m_host(host),
          m_port(port),
          m_manager(std::make_unique<QNetworkAccessManager>()) {}

    QNetworkAccessManager* manager() { return m_manager.get(); }
    const QString& host() const { return m_host; }
    int port() const { return m_port; }

    bool is_connected() const {
        // Simplified connection check
        return m_manager != nullptr;
    }

private:
    QString m_host;
    int m_port;
    std::unique_ptr<QNetworkAccessManager> m_manager;
};

/**
 * @brief Network connection factory
 */
class NetworkConnectionFactory : public IResourceFactory<NetworkConnection> {
public:
    qtplugin::expected<std::unique_ptr<NetworkConnection>, PluginError>
    create_resource(const ResourceHandle& handle) override {
        auto host_meta = handle.get_metadata("host");
        auto port_meta = handle.get_metadata("port");

        if (!host_meta) {
            return make_error<std::unique_ptr<NetworkConnection>>(
                PluginErrorCode::InvalidArgument,
                "Host not specified in handle metadata");
        }

        try {
            QString host = QString::fromStdString(
                std::any_cast<std::string>(host_meta.value()));
            int port = port_meta ? std::any_cast<int>(port_meta.value()) : 80;

            auto connection = std::make_unique<NetworkConnection>(host, port);

            return connection;
        } catch (const std::bad_any_cast& e) {
            return make_error<std::unique_ptr<NetworkConnection>>(
                PluginErrorCode::InvalidArgument, "Invalid metadata types");
        }
    }

    bool can_create_resource(const ResourceHandle& handle) const override {
        auto host_meta = handle.get_metadata("host");
        return host_meta.has_value();
    }

    size_t get_estimated_cost(const ResourceHandle& handle) const override {
        Q_UNUSED(handle)
        return sizeof(NetworkConnection) + sizeof(QNetworkAccessManager) +
               4096;  // Estimated overhead
    }

    std::string name() const override { return "network_connection"; }
};

/**
 * @brief Specialized network connection pool
 */
class NetworkConnectionPool : public ResourcePool<NetworkConnection> {
public:
    explicit NetworkConnectionPool(const ResourceQuota& quota = {})
        : ResourcePool<NetworkConnection>("network_pool", ResourceType::NetworkConnection) {
        // Set reasonable defaults for network pool
        ResourceQuota default_quota;
        default_quota.max_instances = 100;  // Reasonable connection limit
        default_quota.max_lifetime = std::chrono::minutes(30);
        default_quota.min_priority = ResourcePriority::Normal;

        if (quota.is_unlimited()) {
            set_quota(default_quota);
        }
    }

    /**
     * @brief Acquire a network connection
     * @param plugin_id Plugin requesting the connection
     * @param host Target host
     * @param port Target port
     * @param priority Connection priority
     * @return Connection handle and instance or error
     */
    qtplugin::expected<
        std::pair<ResourceHandle, std::unique_ptr<NetworkConnection>>,
        PluginError>
    acquire_connection(std::string_view plugin_id, const QString& host,
                       int port = 80,
                       ResourcePriority priority = ResourcePriority::Normal) {
        // Create handle with connection metadata
        std::string resource_id = generate_connection_id();
        ResourceHandle handle(resource_id, ResourceType::NetworkConnection,
                              std::string(plugin_id));
        handle.set_priority(priority);
        handle.set_metadata("host", host.toStdString());
        handle.set_metadata("port", port);

        auto result = acquire_resource(plugin_id, priority);
        return result;
    }

private:
    std::atomic<size_t> m_connection_counter{0};

    std::string generate_connection_id() {
        return "connection_" +
               std::to_string(m_connection_counter.fetch_add(1));
    }

    ResourceType get_resource_type() const {
        return ResourceType::NetworkConnection;
    }

    bool is_resource_healthy(NetworkConnection* connection) const {
        return connection && connection->is_connected();
    }
};

#endif // QTFORGE_HAS_NETWORK

}  // namespace qtplugin
