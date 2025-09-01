/**
 * @file resource_monitor.cpp
 * @brief Cross-platform resource monitoring implementation
 * @version 3.2.0
 */

#include "resource_monitor.hpp"
#include <QDebug>
#include <QLoggingCategory>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#include <pdh.h>
#ifdef _MSC_VER
#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "pdh.lib")
#endif
#elif defined(Q_OS_LINUX)
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <sys/resource.h>
#include <sys/times.h>
#elif defined(Q_OS_MACOS)
#include <mach/mach.h>
#include <mach/task.h>
#include <mach/mach_init.h>
#include <sys/resource.h>
#endif

Q_LOGGING_CATEGORY(resourceMonitorLog, "qtplugin.sandbox.resource_monitor");

namespace qtplugin {

SandboxResourceMonitor::SandboxResourceMonitor(QObject* parent) : QObject(parent) {}

SandboxResourceMonitor::~SandboxResourceMonitor() = default;

bool ResourceMonitor::initialize() {
    #ifdef Q_OS_WIN
    return initialize_windows();
    #elif defined(Q_OS_LINUX)
    return initialize_linux();
    #elif defined(Q_OS_MACOS)
    return initialize_macos();
    #else
    qCWarning(resourceMonitorLog) << "Resource monitoring not supported on this platform";
    return false;
    #endif
}

void ResourceMonitor::shutdown() {
    #ifdef Q_OS_WIN
    shutdown_windows();
    #elif defined(Q_OS_LINUX)
    shutdown_linux();
    #elif defined(Q_OS_MACOS)
    shutdown_macos();
    #endif
}

ResourceUsage ResourceMonitor::get_process_usage(qint64 pid) {
    ResourceUsage usage;
    usage.start_time = std::chrono::steady_clock::now();

    #ifdef Q_OS_WIN
    get_windows_process_usage(pid, usage);
    #elif defined(Q_OS_LINUX)
    get_linux_process_usage(pid, usage);
    #elif defined(Q_OS_MACOS)
    get_macos_process_usage(pid, usage);
    #else
    // Fallback - return empty usage
    qCWarning(resourceMonitorLog) << "Process resource monitoring not available";
    #endif

    return usage;
}

ResourceUsage ResourceMonitor::get_system_usage() {
    ResourceUsage usage;
    usage.start_time = std::chrono::steady_clock::now();

    #ifdef Q_OS_WIN
    get_windows_system_usage(usage);
    #elif defined(Q_OS_LINUX)
    get_linux_system_usage(usage);
    #elif defined(Q_OS_MACOS)
    get_macos_system_usage(usage);
    #else
    qCWarning(resourceMonitorLog) << "System resource monitoring not available";
    #endif

    return usage;
}

#ifdef Q_OS_WIN
bool ResourceMonitor::initialize_windows() {
    // Initialize Windows Performance Data Helper (PDH) for CPU monitoring
    if (PdhOpenQuery(nullptr, 0, &m_pdh_query) != ERROR_SUCCESS) {
        qCWarning(resourceMonitorLog) << "Failed to initialize PDH query";
        return false;
    }

    // Add CPU counter
    if (PdhAddCounter(m_pdh_query, L"\\Processor(_Total)\\% Processor Time", 0, &m_cpu_counter) != ERROR_SUCCESS) {
        qCWarning(resourceMonitorLog) << "Failed to add CPU counter";
        PdhCloseQuery(m_pdh_query);
        return false;
    }

    qCDebug(resourceMonitorLog) << "Windows resource monitoring initialized";
    return true;
}

void ResourceMonitor::shutdown_windows() {
    if (m_pdh_query) {
        PdhCloseQuery(m_pdh_query);
        m_pdh_query = nullptr;
    }
}

void ResourceMonitor::get_windows_process_usage(qint64 pid, ResourceUsage& usage) {
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, static_cast<DWORD>(pid));
    if (!process) {
        qCWarning(resourceMonitorLog) << "Failed to open process" << pid;
        return;
    }

    // Get memory usage
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(process, &pmc, sizeof(pmc))) {
        usage.memory_used_mb = pmc.WorkingSetSize / (1024 * 1024);
    }

    // Get CPU time
    FILETIME creation_time, exit_time, kernel_time, user_time;
    if (GetProcessTimes(process, &creation_time, &exit_time, &kernel_time, &user_time)) {
        ULARGE_INTEGER kernel_time_int, user_time_int;
        kernel_time_int.LowPart = kernel_time.dwLowDateTime;
        kernel_time_int.HighPart = kernel_time.dwHighDateTime;
        user_time_int.LowPart = user_time.dwLowDateTime;
        user_time_int.HighPart = user_time.dwHighDateTime;

        // Convert from 100-nanosecond intervals to milliseconds
        ULONGLONG total_time = (kernel_time_int.QuadPart + user_time_int.QuadPart) / 10000;
        usage.cpu_time_used = std::chrono::milliseconds(total_time);
    }

    // Get handle count
    DWORD handle_count;
    if (GetProcessHandleCount(process, &handle_count)) {
        usage.file_handles_used = static_cast<int>(handle_count);
    }

    CloseHandle(process);
}

void ResourceMonitor::get_windows_system_usage(ResourceUsage& usage) {
    // Get system memory info
    MEMORYSTATUSEX mem_status;
    mem_status.dwLength = sizeof(mem_status);
    if (GlobalMemoryStatusEx(&mem_status)) {
        usage.memory_used_mb = (mem_status.ullTotalPhys - mem_status.ullAvailPhys) / (1024 * 1024);
    }

    // Get CPU usage using PDH
    if (m_pdh_query) {
        PdhCollectQueryData(m_pdh_query);
        PDH_FMT_COUNTERVALUE counter_value;
        if (PdhGetFormattedCounterValue(m_cpu_counter, PDH_FMT_DOUBLE, nullptr, &counter_value) == ERROR_SUCCESS) {
            // Convert CPU percentage to approximate time
            usage.cpu_time_used = std::chrono::milliseconds(static_cast<long long>(counter_value.doubleValue * 10));
        }
    }
}

#elif defined(Q_OS_LINUX)

bool ResourceMonitor::initialize_linux() {
    // Check if /proc filesystem is available
    if (access("/proc/stat", R_OK) != 0) {
        qCWarning(resourceMonitorLog) << "/proc filesystem not accessible";
        return false;
    }

    qCDebug(resourceMonitorLog) << "Linux resource monitoring initialized";
    return true;
}

void ResourceMonitor::shutdown_linux() {
    // Nothing specific to cleanup on Linux
}

void ResourceMonitor::get_linux_process_usage(qint64 pid, ResourceUsage& usage) {
    // Read from /proc/[pid]/stat for process information
    std::string stat_path = "/proc/" + std::to_string(pid) + "/stat";
    std::ifstream stat_file(stat_path);

    if (!stat_file.is_open()) {
        qCWarning(resourceMonitorLog) << "Failed to open" << QString::fromStdString(stat_path);
        return;
    }

    std::string line;
    if (std::getline(stat_file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;

        while (iss >> token) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 24) {
            // CPU time (user + system time in clock ticks)
            long utime = std::stol(tokens[13]);  // User time
            long stime = std::stol(tokens[14]);  // System time
            long clock_ticks_per_sec = sysconf(_SC_CLK_TCK);

            long total_time_ms = ((utime + stime) * 1000) / clock_ticks_per_sec;
            usage.cpu_time_used = std::chrono::milliseconds(total_time_ms);

            // Virtual memory size (in bytes)
            long vsize = std::stol(tokens[22]);
            usage.memory_used_mb = vsize / (1024 * 1024);
        }
    }

    // Read memory information from /proc/[pid]/status
    std::string status_path = "/proc/" + std::to_string(pid) + "/status";
    std::ifstream status_file(status_path);

    if (status_file.is_open()) {
        std::string line;
        while (std::getline(status_file, line)) {
            if (line.find("VmRSS:") == 0) {
                std::istringstream iss(line);
                std::string key, value, unit;
                iss >> key >> value >> unit;
                usage.memory_used_mb = std::stol(value) / 1024; // Convert from KB to MB
                break;
            }
        }
    }

    // Count file descriptors
    std::string fd_path = "/proc/" + std::to_string(pid) + "/fd";
    QDir fd_dir(QString::fromStdString(fd_path));
    if (fd_dir.exists()) {
        usage.file_handles_used = fd_dir.entryList(QDir::Files).size();
    }
}

void ResourceMonitor::get_linux_system_usage(ResourceUsage& usage) {
    // Read system memory from /proc/meminfo
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        long total_mem = 0, available_mem = 0;

        while (std::getline(meminfo, line)) {
            if (line.find("MemTotal:") == 0) {
                std::istringstream iss(line);
                std::string key, value;
                iss >> key >> value;
                total_mem = std::stol(value);
            } else if (line.find("MemAvailable:") == 0) {
                std::istringstream iss(line);
                std::string key, value;
                iss >> key >> value;
                available_mem = std::stol(value);
            }
        }

        if (total_mem > 0 && available_mem > 0) {
            usage.memory_used_mb = (total_mem - available_mem) / 1024; // Convert from KB to MB
        }
    }

    // Read CPU information from /proc/stat
    std::ifstream stat("/proc/stat");
    if (stat.is_open()) {
        std::string line;
        if (std::getline(stat, line) && line.find("cpu ") == 0) {
            std::istringstream iss(line);
            std::string cpu;
            long user, nice, system, idle;
            iss >> cpu >> user >> nice >> system >> idle;

            long total_time = user + nice + system + idle;
            long used_time = user + nice + system;

            if (total_time > 0) {
                // Convert to approximate milliseconds (simplified)
                usage.cpu_time_used = std::chrono::milliseconds(used_time * 10);
            }
        }
    }
}

#endif

} // namespace qtplugin
