/**
 * @file resource_monitor.hpp
 * @brief Cross-platform resource monitoring for sandbox system
 * @version 3.2.0
 */

#pragma once

#include <QObject>
#include <chrono>
#include "../../../include/qtplugin/security/sandbox/plugin_sandbox.hpp"

#ifdef Q_OS_WIN
#include <windows.h>
#include <pdh.h>
#elif defined(Q_OS_LINUX)
#include <QDir>
#elif defined(Q_OS_MACOS)
// macOS specific includes would go here
#endif

namespace qtplugin {

/**
 * @brief Cross-platform resource monitoring system for sandbox
 */
class SandboxResourceMonitor : public QObject {
    Q_OBJECT

public:
    explicit SandboxResourceMonitor(QObject* parent = nullptr);
    ~SandboxResourceMonitor() override;

    /**
     * @brief Initialize the resource monitor
     * @return True if initialization successful
     */
    bool initialize();

    /**
     * @brief Shutdown the resource monitor
     */
    void shutdown();

    /**
     * @brief Get resource usage for a specific process
     * @param pid Process ID
     * @return Resource usage information
     */
    ResourceUsage get_process_usage(qint64 pid);

    /**
     * @brief Get system-wide resource usage
     * @return System resource usage information
     */
    ResourceUsage get_system_usage();

    /**
     * @brief Check if resource monitoring is available on this platform
     * @return True if monitoring is supported
     */
    static bool is_supported();

signals:
    /**
     * @brief Emitted when resource usage is updated
     */
    void resource_usage_updated(const ResourceUsage& usage);

    /**
     * @brief Emitted when a resource limit is approached
     */
    void resource_limit_warning(const QString& resource, double usage_percentage);

private:
#ifdef Q_OS_WIN
    bool initialize_windows();
    void shutdown_windows();
    void get_windows_process_usage(qint64 pid, ResourceUsage& usage);
    void get_windows_system_usage(ResourceUsage& usage);

    HQUERY m_pdh_query = nullptr;
    HCOUNTER m_cpu_counter = nullptr;

#elif defined(Q_OS_LINUX)
    bool initialize_linux();
    void shutdown_linux();
    void get_linux_process_usage(qint64 pid, ResourceUsage& usage);
    void get_linux_system_usage(ResourceUsage& usage);

#elif defined(Q_OS_MACOS)
    bool initialize_macos();
    void shutdown_macos();
    void get_macos_process_usage(qint64 pid, ResourceUsage& usage);
    void get_macos_system_usage(ResourceUsage& usage);
#endif
};

/**
 * @brief Resource monitoring utilities
 */
class ResourceMonitorUtils {
public:
    /**
     * @brief Convert bytes to megabytes
     */
    static size_t bytes_to_mb(size_t bytes) {
        return bytes / (1024 * 1024);
    }

    /**
     * @brief Convert milliseconds to seconds
     */
    static double ms_to_seconds(std::chrono::milliseconds ms) {
        return ms.count() / 1000.0;
    }

    /**
     * @brief Calculate CPU usage percentage
     */
    static double calculate_cpu_percentage(std::chrono::milliseconds used_time,
                                         std::chrono::milliseconds total_time) {
        if (total_time.count() == 0) return 0.0;
        return (static_cast<double>(used_time.count()) / total_time.count()) * 100.0;
    }

    /**
     * @brief Calculate memory usage percentage
     */
    static double calculate_memory_percentage(size_t used_mb, size_t total_mb) {
        if (total_mb == 0) return 0.0;
        return (static_cast<double>(used_mb) / total_mb) * 100.0;
    }

    /**
     * @brief Format resource usage for display
     */
    static QString format_usage(const ResourceUsage& usage);

    /**
     * @brief Check if usage exceeds threshold percentage
     */
    static bool exceeds_threshold(const ResourceUsage& usage,
                                const ResourceLimits& limits,
                                double threshold_percentage = 80.0);
};

} // namespace qtplugin
