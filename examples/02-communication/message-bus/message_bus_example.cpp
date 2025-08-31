/**
 * @file message_bus_example.cpp
 * @brief Advanced MessageBus example implementation
 * @version 3.0.0
 */

#include "message_bus_example.hpp"
#include "../filters/message_filters.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <iostream>
#include <random>
#include <thread>

namespace qtplugin::examples {

AdvancedMessageBusExample::AdvancedMessageBusExample(QObject* parent)
    : QObject(parent)
    , m_message_bus(std::make_unique<MessageBus>(this))
    , m_statistics_collector(std::make_unique<MessageStatisticsCollector>())
    , m_latency_measurer(std::make_unique<utils::MessageLatencyMeasurer>())
    , m_batch_processor(std::make_unique<utils::MessageBatchProcessor>(m_batch_size))
    , m_monitoring_timer(new QTimer(this))
    , m_start_time(std::chrono::steady_clock::now())
{
    // Connect monitoring timer
    connect(m_monitoring_timer, &QTimer::timeout,
            this, &AdvancedMessageBusExample::on_monitoring_timer);

    setup_subscriptions();
}

AdvancedMessageBusExample::~AdvancedMessageBusExample() {
    cleanup_subscriptions();
}

int AdvancedMessageBusExample::run_example() {
    std::cout << "QtForge Communication Examples - Advanced MessageBus\n";
    std::cout << "==================================================\n\n";

    try {
        demonstrate_basic_messaging();
        demonstrate_priority_messaging();
        demonstrate_filtered_subscriptions();
        demonstrate_message_statistics();
        demonstrate_error_handling();
        demonstrate_performance_monitoring();
        demonstrate_subscription_management();
        demonstrate_message_batching();

        std::cout << "\n🎉 Advanced MessageBus example completed successfully!\n";

        // Show final statistics
        auto stats = get_statistics();
        std::cout << "Final Statistics:\n";
        std::cout << QJsonDocument(stats).toJson().toStdString() << "\n";

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Error during example: " << e.what() << "\n";
        return 1;
    }
}

QJsonObject AdvancedMessageBusExample::get_statistics() const {
    std::lock_guard<std::mutex> lock(m_stats_mutex);

    QJsonObject stats;
    stats["messages_sent"] = static_cast<qint64>(m_messages_sent.load());
    stats["messages_received"] = static_cast<qint64>(m_messages_received.load());
    stats["messages_failed"] = static_cast<qint64>(m_messages_failed.load());

    // Add collector statistics
    if (m_statistics_collector) {
        stats["detailed_stats"] = m_statistics_collector->get_statistics();
    }

    // Add message bus statistics
    if (m_message_bus) {
        stats["bus_stats"] = m_message_bus->statistics();
    }

    // Runtime information
    auto runtime = std::chrono::steady_clock::now() - m_start_time;
    stats["runtime_seconds"] = std::chrono::duration_cast<std::chrono::seconds>(runtime).count();

    return stats;
}

void AdvancedMessageBusExample::reset_statistics() {
    std::lock_guard<std::mutex> lock(m_stats_mutex);

    m_messages_sent = 0;
    m_messages_received = 0;
    m_messages_failed = 0;
    m_message_type_counts.clear();
    m_message_latencies.clear();

    if (m_statistics_collector) {
        m_statistics_collector->reset_statistics();
    }

    if (m_latency_measurer) {
        m_latency_measurer->clear_measurements();
    }

    if (m_batch_processor) {
        m_batch_processor->clear_batches();
    }

    m_start_time = std::chrono::steady_clock::now();
}

void AdvancedMessageBusExample::demonstrate_basic_messaging() {
    std::cout << "🔄 Demonstrating Basic Messaging...\n";

    // Create and publish a system event
    auto message = utils::create_test_system_event(
        SystemEventMessage::EventType::SystemStartup,
        "example_app");

    auto result = m_message_bus->publish(message, DeliveryMode::Reliable);
    if (result.has_value()) {
        m_messages_sent++;
        m_statistics_collector->record_message_sent("system_event");
        log_message_activity("System startup event published successfully");
    } else {
        m_messages_failed++;
        m_statistics_collector->record_message_failed("system_event", result.error().message);
        log_message_activity("Failed to publish system startup event");
    }

    // Publish performance metrics
    auto metrics = utils::create_test_performance_metrics("example_app", 25.5, 1024*1024*50);
    result = m_message_bus->publish(metrics, DeliveryMode::Fast);
    if (result.has_value()) {
        m_messages_sent++;
        m_statistics_collector->record_message_sent("performance_metrics");
        log_message_activity("Performance metrics published successfully");
    }

    // Allow time for message processing
    QCoreApplication::processEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void AdvancedMessageBusExample::demonstrate_priority_messaging() {
    std::cout << "\n⚡ Demonstrating Priority Messaging...\n";

    // Publish messages with different priorities
    std::vector<SystemEventMessage::Priority> priorities = {
        SystemEventMessage::Priority::Low,
        SystemEventMessage::Priority::Normal,
        SystemEventMessage::Priority::High,
        SystemEventMessage::Priority::Critical
    };

    for (auto priority : priorities) {
        auto message = utils::create_test_system_event(
            SystemEventMessage::EventType::ConfigurationChanged,
            "config_manager");

        // Set priority-specific data
        QJsonObject data = utils::generate_random_test_data();
        data["priority_level"] = static_cast<int>(priority);
        data["message"] = QString("Priority %1 message").arg(static_cast<int>(priority));
        message->set_data(data);

        auto result = m_message_bus->publish(message, DeliveryMode::Reliable);
        if (result.has_value()) {
            m_messages_sent++;
            m_statistics_collector->record_message_sent("system_event");
            log_message_activity(QString("Priority %1 message published").arg(static_cast<int>(priority)));
        }
    }

    QCoreApplication::processEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void AdvancedMessageBusExample::demonstrate_filtered_subscriptions() {
    std::cout << "\n🔍 Demonstrating Filtered Subscriptions...\n";

    // Create a filter for high-priority messages only
    PriorityMessageFilter high_priority_filter(SystemEventMessage::Priority::High);

    log_message_activity("High-priority filter active");

    // Publish mixed priority messages
    std::vector<SystemEventMessage::Priority> test_priorities = {
        SystemEventMessage::Priority::Low,
        SystemEventMessage::Priority::High,
        SystemEventMessage::Priority::Normal,
        SystemEventMessage::Priority::Critical
    };

    for (auto priority : test_priorities) {
        auto message = utils::create_test_system_event(
            SystemEventMessage::EventType::ErrorOccurred,
            "error_handler");

        QJsonObject data = utils::generate_random_test_data();
        data["error_code"] = 500 + static_cast<int>(priority);
        data["severity"] = static_cast<int>(priority);
        message->set_data(data);

        m_message_bus->publish(message, DeliveryMode::Reliable);
        m_messages_sent++;
        m_statistics_collector->record_message_sent("system_event");
    }

    log_message_activity(QString("Published %1 messages with mixed priorities").arg(test_priorities.size()));

    QCoreApplication::processEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void AdvancedMessageBusExample::demonstrate_message_statistics() {
    std::cout << "\n📊 Demonstrating Message Statistics...\n";

    auto stats = get_statistics();
    std::cout << "   Current Statistics:\n";
    std::cout << "   - Messages sent: " << m_messages_sent.load() << "\n";
    std::cout << "   - Messages received: " << m_messages_received.load() << "\n";
    std::cout << "   - Messages failed: " << m_messages_failed.load() << "\n";

    if (stats.contains("detailed_stats")) {
        auto detailed = stats["detailed_stats"].toObject();
        if (detailed.contains("runtime_seconds")) {
            std::cout << "   - Runtime: " << detailed["runtime_seconds"].toInt() << " seconds\n";
        }
    }

    if (stats.contains("bus_stats")) {
        auto bus_stats = stats["bus_stats"].toObject();
        if (bus_stats.contains("total_messages")) {
            std::cout << "   - Bus total messages: " << bus_stats["total_messages"].toInt() << "\n";
        }
        if (bus_stats.contains("active_subscriptions")) {
            std::cout << "   - Active subscriptions: " << bus_stats["active_subscriptions"].toInt() << "\n";
        }
    }
}

void AdvancedMessageBusExample::setup_subscriptions() {
    // Subscribe to system events
    auto system_event_result = m_message_bus->subscribe<SystemEventMessage>(
        "advanced_example",
        [this](std::shared_ptr<SystemEventMessage> message) {
            this->on_system_event_received(message);
        }
    );

    if (system_event_result.has_value()) {
        log_message_activity("Subscribed to system events");
    }

    // Subscribe to performance metrics
    auto metrics_result = m_message_bus->subscribe<PerformanceMetricsMessage>(
        "advanced_example",
        [this](std::shared_ptr<PerformanceMetricsMessage> message) {
            this->on_performance_metrics_received(message);
        }
    );

    if (metrics_result.has_value()) {
        log_message_activity("Subscribed to performance metrics");
    }
}

void AdvancedMessageBusExample::cleanup_subscriptions() {
    // Unsubscribe from all topics
    for (const auto& subscription_id : m_subscription_ids) {
        m_message_bus->unsubscribe("advanced_example");
    }
    m_subscription_ids.clear();
}

void AdvancedMessageBusExample::on_system_event_received(std::shared_ptr<SystemEventMessage> message) {
    m_messages_received++;
    m_statistics_collector->record_message_received("system_event");

    std::lock_guard<std::mutex> lock(m_stats_mutex);
    m_message_type_counts["system_event"]++;

    if (m_verbose_logging) {
        log_message_activity(QString("System event received: %1 (priority: %2)")
                           .arg(static_cast<int>(message->event_type()))
                           .arg(static_cast<int>(message->priority())));
    }
}

void AdvancedMessageBusExample::on_performance_metrics_received(std::shared_ptr<PerformanceMetricsMessage> message) {
    m_messages_received++;
    m_statistics_collector->record_message_received("performance_metrics");

    std::lock_guard<std::mutex> lock(m_stats_mutex);
    m_message_type_counts["performance_metrics"]++;

    if (m_verbose_logging) {
        log_message_activity(QString("Performance metrics received: CPU=%1%, Memory=%2MB")
                           .arg(message->cpu_usage())
                           .arg(message->memory_usage() / (1024*1024)));
    }
}

void AdvancedMessageBusExample::on_monitoring_timer() {
    // Publish periodic performance metrics
    auto metrics = utils::create_test_performance_metrics(
        "monitoring_system",
        static_cast<double>(rand() % 100),
        1024 * 1024 * (50 + rand() % 100));

    m_message_bus->publish(metrics, DeliveryMode::Fast);
    m_messages_sent++;
    m_statistics_collector->record_message_sent("performance_metrics");
}

void AdvancedMessageBusExample::demonstrate_error_handling() {
    std::cout << "\n🛡️ Demonstrating Error Handling...\n";

    // Try to publish to a non-existent topic (simulated error)
    auto message = utils::create_test_system_event(
        SystemEventMessage::EventType::ErrorOccurred,
        "error_simulator");

    // Simulate error conditions
    QJsonObject error_data = utils::generate_random_test_data();
    error_data["error_type"] = "simulated_error";
    error_data["should_fail"] = true;
    message->set_data(error_data);

    auto result = m_message_bus->publish(message, DeliveryMode::Reliable);
    if (!result.has_value()) {
        m_messages_failed++;
        m_statistics_collector->record_message_failed("system_event", result.error().message);
        log_message_activity(QString("Expected error occurred: %1").arg(result.error().message.c_str()));
    } else {
        log_message_activity("Error simulation message published");
        m_messages_sent++;
        m_statistics_collector->record_message_sent("system_event");
    }
}

void AdvancedMessageBusExample::demonstrate_performance_monitoring() {
    std::cout << "\n⚡ Demonstrating Performance Monitoring...\n";

    start_performance_monitoring();

    // Generate load for performance testing
    log_message_activity("Generating message load...");
    for (int i = 0; i < 50; ++i) {
        auto message = utils::create_test_system_event(
            SystemEventMessage::EventType::PluginLoaded,
            "load_tester");

        QJsonObject data = utils::generate_random_test_data();
        data["iteration"] = i;
        data["batch_id"] = "performance_test";
        message->set_data(data);

        m_message_bus->publish(message, DeliveryMode::Fast);
        m_messages_sent++;
        m_statistics_collector->record_message_sent("system_event");

        // Small delay to simulate realistic load
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    QCoreApplication::processEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    stop_performance_monitoring();
    log_message_activity("Performance monitoring completed");
}

void AdvancedMessageBusExample::demonstrate_subscription_management() {
    std::cout << "\n📡 Demonstrating Subscription Management...\n";

    // Show current subscriptions
    auto subscribers = m_message_bus->subscribers(std::type_index(typeid(SystemEventMessage)));
    log_message_activity(QString("Current system event subscribers: %1").arg(subscribers.size()));

    // Test subscription lifecycle
    log_message_activity("Testing subscription lifecycle...");

    // This would involve creating temporary subscriptions and cleaning them up
    log_message_activity("Subscription management demonstrated");
}

void AdvancedMessageBusExample::demonstrate_message_batching() {
    std::cout << "\n📦 Demonstrating Message Batching...\n";

    // Simulate batch message processing
    std::vector<std::shared_ptr<SystemEventMessage>> batch_messages;

    for (int i = 0; i < 10; ++i) {
        auto message = utils::create_test_system_event(
            SystemEventMessage::EventType::ConfigurationChanged,
            "batch_processor");

        QJsonObject data = utils::generate_random_test_data();
        data["batch_index"] = i;
        data["batch_size"] = 10;
        message->set_data(data);

        batch_messages.push_back(message);
        m_batch_processor->add_message(message);
    }

    // Publish batch
    log_message_activity(QString("Publishing batch of %1 messages...").arg(batch_messages.size()));
    for (const auto& message : batch_messages) {
        m_message_bus->publish(message, DeliveryMode::Reliable);
        m_messages_sent++;
        m_statistics_collector->record_message_sent("system_event");
    }

    QCoreApplication::processEvents();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    log_message_activity("Batch processing completed");
}

void AdvancedMessageBusExample::start_performance_monitoring() {
    m_monitoring_timer->start(m_monitoring_interval.count());
    log_message_activity("Performance monitoring started");
}

void AdvancedMessageBusExample::stop_performance_monitoring() {
    m_monitoring_timer->stop();
    log_message_activity("Performance monitoring stopped");
}

void AdvancedMessageBusExample::publish_test_messages() {
    // Publish a variety of test messages
    for (int i = 0; i < 5; ++i) {
        auto system_event = utils::create_test_system_event(
            SystemEventMessage::EventType::PluginLoaded,
            "test_publisher");

        auto metrics = utils::create_test_performance_metrics(
            "test_publisher",
            static_cast<double>(rand() % 100),
            1024 * 1024 * (rand() % 100));

        m_message_bus->publish(system_event, DeliveryMode::Reliable);
        m_message_bus->publish(metrics, DeliveryMode::Fast);

        m_messages_sent += 2;
        m_statistics_collector->record_message_sent("system_event");
        m_statistics_collector->record_message_sent("performance_metrics");
    }
}

void AdvancedMessageBusExample::log_message_activity(const QString& activity) {
    if (m_verbose_logging) {
        std::cout << "   " << activity.toStdString() << "\n";
    }
}

} // namespace qtplugin::examples
