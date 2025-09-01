/**
 * @file performance_metrics_message.cpp
 * @brief Performance metrics message implementation for communication examples
 * @version 3.0.0
 */

#include "performance_metrics_message.hpp"

namespace qtplugin::examples {

PerformanceMetricsMessage::PerformanceMetricsMessage(const std::string& sender, 
                                                   const MetricsMap& metrics)
    : m_sender(sender)
    , m_metrics(metrics)
    , m_timestamp(std::chrono::system_clock::now())
{
}

double PerformanceMetricsMessage::get_metric(const std::string& name) const {
    auto it = m_metrics.find(name);
    return (it != m_metrics.end()) ? it->second : 0.0;
}

bool PerformanceMetricsMessage::has_metric(const std::string& name) const {
    return m_metrics.find(name) != m_metrics.end();
}

} // namespace qtplugin::examples
