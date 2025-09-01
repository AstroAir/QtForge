/**
 * @file performance_metrics_message.hpp
 * @brief Performance metrics message implementation for communication examples
 * @version 3.0.0
 */

#pragma once

#include <chrono>
#include <string>
#include <unordered_map>

namespace qtplugin::examples {

/**
 * @brief Performance metrics message for demonstration purposes
 */
class PerformanceMetricsMessage {
public:
    using MetricsMap = std::unordered_map<std::string, double>;

    PerformanceMetricsMessage(const std::string& sender,
                              const MetricsMap& metrics);

    // Accessor methods
    std::string type() const { return "PerformanceMetrics"; }
    std::string sender() const { return m_sender; }
    std::chrono::system_clock::time_point timestamp() const {
        return m_timestamp;
    }

    // Performance metrics specific methods
    const MetricsMap& metrics() const { return m_metrics; }
    double get_metric(const std::string& name) const;
    bool has_metric(const std::string& name) const;

private:
    std::string m_sender;
    MetricsMap m_metrics;
    std::chrono::system_clock::time_point m_timestamp;
};

}  // namespace qtplugin::examples
