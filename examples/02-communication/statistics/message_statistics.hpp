/**
 * @file message_statistics.hpp
 * @brief Message statistics tracking for communication examples
 * @version 3.0.0
 */

#pragma once

#include <string>
#include <unordered_map>
#include <atomic>
#include <chrono>

namespace qtplugin::examples {

/**
 * @brief Simple message statistics tracker
 */
class MessageStatistics {
public:
    MessageStatistics();

    // Statistics tracking
    void record_message_sent(const std::string& type);
    void record_message_received(const std::string& type);
    void record_processing_time(const std::string& type, std::chrono::milliseconds duration);

    // Statistics retrieval
    uint64_t get_sent_count(const std::string& type) const;
    uint64_t get_received_count(const std::string& type) const;
    uint64_t get_total_sent() const;
    uint64_t get_total_received() const;
    double get_average_processing_time(const std::string& type) const;

    // Reset statistics
    void reset();

private:
    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::atomic<uint64_t>> m_sent_counts;
    std::unordered_map<std::string, std::atomic<uint64_t>> m_received_counts;
    std::unordered_map<std::string, std::atomic<uint64_t>> m_processing_times;
    std::unordered_map<std::string, std::atomic<uint64_t>> m_processing_count;
};

} // namespace qtplugin::examples
