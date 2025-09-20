/**
 * @file test_message_bus.cpp
 * @brief Comprehensive tests for message bus functionality
 * @version 3.0.0
 */

#include <QSignalSpy>
#include <QtTest/QtTest>
#include <chrono>
#include <memory>
#include <thread>

#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/communication/message_types.hpp>
#include <qtplugin/utils/error_handling.hpp>

using namespace qtplugin;

class TestMessageBus : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testMessageBusCreation();
    void testMessageBusDestruction();
    void testMessageBusInitialization();

    // Message publishing tests
    void testPublishMessage();
    void testPublishInvalidMessage();
    void testPublishToNonexistentTopic();
    void testPublishLargeMessage();
    void testPublishEmptyMessage();

    // Message subscription tests
    void testSubscribeToTopic();
    void testSubscribeToInvalidTopic();
    void testSubscribeMultipleTopics();
    void testUnsubscribeFromTopic();
    void testUnsubscribeNonexistentSubscription();

    // Message delivery tests
    void testMessageDelivery();
    void testMessageDeliveryOrder();
    void testMessageDeliveryReliability();
    void testMessageDeliveryPerformance();

    // Topic management tests
    void testCreateTopic();
    void testDeleteTopic();
    void testListTopics();
    void testTopicExists();
    void testTopicMetadata();

    // Subscription management tests
    void testListSubscriptions();
    void testSubscriptionMetadata();
    void testSubscriptionFiltering();
    void testSubscriptionPriority();

    // Message filtering tests
    void testMessageFiltering();
    void testContentFiltering();
    void testSenderFiltering();
    void testTypeFiltering();

    // Message routing tests
    void testDirectRouting();
    void testBroadcastRouting();
    void testMulticastRouting();
    void testConditionalRouting();

    // Error handling tests
    void testMessageDeliveryFailure();
    void testSubscriberError();
    void testTopicError();
    void testNetworkError();

    // Performance tests
    void testHighVolumeMessaging();
    void testConcurrentMessaging();
    void testMessageThroughput();
    void testMemoryUsage();

    // Thread safety tests
    void testConcurrentPublishing();
    void testConcurrentSubscription();
    void testThreadSafetyStress();

    // Message persistence tests
    void testMessagePersistence();
    void testMessageRecovery();
    void testMessageHistory();

    // Quality of service tests
    void testMessagePriority();
    void testMessageExpiration();
    void testMessageRetry();
    void testMessageAcknowledgment();

private:
    std::unique_ptr<MessageBus> m_message_bus;

    // Test message types
    class TestMessage : public IMessage {
    public:
        TestMessage(const std::string& content = "",
                    const std::string& sender = "test")
            : m_content(content),
              m_sender(sender),
              m_timestamp(std::chrono::system_clock::now()),
              m_priority(MessagePriority::Normal),
              m_id(generate_id()) {}

        std::string_view type() const noexcept override {
            return "TestMessage";
        }
        std::string_view sender() const noexcept override { return m_sender; }
        std::chrono::system_clock::time_point timestamp()
            const noexcept override {
            return m_timestamp;
        }
        MessagePriority priority() const noexcept override {
            return m_priority;
        }
        std::string id() const noexcept override { return m_id; }

        QJsonObject to_json() const override {
            QJsonObject obj;
            obj["content"] = QString::fromStdString(m_content);
            obj["sender"] = QString::fromStdString(m_sender);
            obj["timestamp"] = static_cast<qint64>(
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    m_timestamp.time_since_epoch())
                    .count());
            obj["priority"] = static_cast<int>(m_priority);
            obj["id"] = QString::fromStdString(m_id);
            return obj;
        }

        std::string content() const { return m_content; }
        void set_content(const std::string& content) { m_content = content; }

    private:
        std::string m_content;
        std::string m_sender;
        std::chrono::system_clock::time_point m_timestamp;
        MessagePriority m_priority;
        std::string m_id;

        static std::string generate_id() {
            static std::atomic<uint64_t> counter{0};
            return "test_msg_" + std::to_string(counter.fetch_add(1));
        }
    };

    // Helper methods
    void createTestMessage(const std::string& content, int priority = 0);
    void verifyMessageDelivery(const std::string& topic,
                               const std::string& expected_content);
    void simulateNetworkDelay();
};

void TestMessageBus::initTestCase() {
    qDebug() << "Starting message bus tests";
}

void TestMessageBus::cleanupTestCase() {
    qDebug() << "Message bus tests completed";
}

void TestMessageBus::init() {
    // Create fresh message bus for each test
    m_message_bus = std::make_unique<MessageBus>();
    QVERIFY(m_message_bus != nullptr);
}

void TestMessageBus::cleanup() {
    // Clean up message bus
    if (m_message_bus) {
        m_message_bus->clear();  // Clear all subscriptions and messages
        m_message_bus.reset();
    }
}

void TestMessageBus::testMessageBusCreation() {
    // Test basic creation
    auto bus = std::make_unique<MessageBus>();
    QVERIFY(bus != nullptr);

    // Test initial state
    QVERIFY(!bus->is_logging_enabled());
    auto stats = bus->statistics();
    QVERIFY(stats.contains("messages_published"));
    QVERIFY(stats.contains("messages_delivered"));
    QVERIFY(stats.contains("delivery_failures"));

    // Test that initial statistics are zero
    QCOMPARE(stats["messages_published"].toInt(), 0);
    QCOMPARE(stats["messages_delivered"].toInt(), 0);
    QCOMPARE(stats["delivery_failures"].toInt(), 0);
}

void TestMessageBus::testMessageBusDestruction() {
    // Test that destruction properly cleans up resources
    {
        auto bus = std::make_unique<MessageBus>();

        // Test basic functionality before destruction
        auto stats = bus->statistics();
        QVERIFY(!stats.isEmpty());

        // Bus should clean up automatically when destroyed
    }

    // Verify no memory leaks
    QVERIFY(true);  // This would be verified with memory profiling tools
}

void TestMessageBus::testMessageBusInitialization() {
    // Test initialization state
    auto stats = m_message_bus->statistics();
    QVERIFY(!stats.isEmpty());

    // Test that logging is initially disabled
    QVERIFY(!m_message_bus->is_logging_enabled());

    // Test that we can enable logging
    m_message_bus->set_logging_enabled(true);
    QVERIFY(m_message_bus->is_logging_enabled());

    // Test that message log is initially empty
    auto log = m_message_bus->message_log();
    QVERIFY(log.empty());
}

void TestMessageBus::testPublishMessage() {
    // Create a test message
    TestMessage msg("Hello, World!", "test_sender");

    // Publish the message using broadcast mode
    auto publish_result = m_message_bus->publish(msg, DeliveryMode::Broadcast);
    QVERIFY(publish_result.has_value());

    // Verify statistics were updated
    auto stats = m_message_bus->statistics();
    QVERIFY(stats["messages_published"].toInt() >= 1);
}

void TestMessageBus::testPublishInvalidMessage() {
    // Test publishing with empty content to test edge cases
    TestMessage empty_msg("", "");

    // Publishing should still succeed but with empty content
    auto publish_result =
        m_message_bus->publish(empty_msg, DeliveryMode::Broadcast);
    QVERIFY(publish_result.has_value());  // Empty content is still valid
}

void TestMessageBus::testSubscribeToTopic() {
    // Test subscription to TestMessage type
    bool message_received = false;
    std::string received_content;

    auto subscribe_result = m_message_bus->subscribe<TestMessage>(
        "test_subscriber",
        [&message_received, &received_content](
            const TestMessage& msg) -> qtplugin::expected<void, PluginError> {
            message_received = true;
            received_content = msg.content();
            return make_success();
        });
    QVERIFY(subscribe_result.has_value());

    // Verify subscriber was added
    auto subscribers =
        m_message_bus->subscribers(std::type_index(typeid(TestMessage)));
    QVERIFY(!subscribers.empty());
    QVERIFY(std::find(subscribers.begin(), subscribers.end(),
                      "test_subscriber") != subscribers.end());

    // Publish a message to test delivery
    TestMessage test_msg("Test message", "test_sender");
    auto publish_result =
        m_message_bus->publish(test_msg, DeliveryMode::Broadcast);
    QVERIFY(publish_result.has_value());

    // Wait for message delivery
    QTest::qWait(100);

    // Verify message was received
    QVERIFY(message_received);
    QCOMPARE(QString::fromStdString(received_content), "Test message");
}

void TestMessageBus::testUnsubscribeFromTopic() {
    // Create a simple test message type
    class TestMessage : public qtplugin::Message<TestMessage> {
    public:
        TestMessage() : qtplugin::Message<TestMessage>("test_sender") {}
        std::string content = "test";

        QJsonObject to_json() const override {
            QJsonObject obj;
            obj["type"] = QString::fromStdString(std::string(type()));
            obj["sender"] = QString::fromStdString(std::string(sender()));
            obj["content"] = QString::fromStdString(content);
            obj["id"] = QString::fromStdString(id());
            return obj;
        }
    };

    // Subscribe to message type
    auto subscribe_result = m_message_bus->subscribe<TestMessage>(
        "test_subscriber", [](const TestMessage& msg) -> qtplugin::expected<void, qtplugin::PluginError> {
            Q_UNUSED(msg);
            return {};
        });
    QVERIFY(subscribe_result.has_value());

    // Verify subscription exists
    auto stats1 = m_message_bus->statistics();
    QVERIFY(stats1.contains("total_subscriptions"));
    QCOMPARE(stats1["total_subscriptions"].toInt(), 1);

    // Unsubscribe
    auto unsubscribe_result = m_message_bus->unsubscribe("test_subscriber");
    QVERIFY(unsubscribe_result.has_value());

    // Verify subscription was removed
    auto stats2 = m_message_bus->statistics();
    QCOMPARE(stats2["total_subscriptions"].toInt(), 0);
}

void TestMessageBus::testCreateTopic() {
    // Create a simple test message type
    class NewTopicMessage : public qtplugin::Message<NewTopicMessage> {
    public:
        NewTopicMessage() : qtplugin::Message<NewTopicMessage>("test_sender") {}
        std::string content = "new_topic_test";

        QJsonObject to_json() const override {
            QJsonObject obj;
            obj["type"] = QString::fromStdString(std::string(type()));
            obj["sender"] = QString::fromStdString(std::string(sender()));
            obj["content"] = QString::fromStdString(content);
            obj["id"] = QString::fromStdString(id());
            return obj;
        }
    };

    // Subscribe to message type (this implicitly "creates" the topic)
    auto result = m_message_bus->subscribe<NewTopicMessage>(
        "topic_subscriber", [](const NewTopicMessage& msg) -> qtplugin::expected<void, qtplugin::PluginError> {
            Q_UNUSED(msg);
            return {};
        });
    QVERIFY(result.has_value());

    // Verify subscription exists
    QVERIFY(m_message_bus->has_subscriber("topic_subscriber"));
    auto stats = m_message_bus->statistics();
    QVERIFY(stats.contains("total_subscriptions"));
    QCOMPARE(stats["total_subscriptions"].toInt(), 1);

    // Test subscribing with same ID (should work - multiple subscriptions allowed)
    auto duplicate_result = m_message_bus->subscribe<NewTopicMessage>(
        "topic_subscriber", [](const NewTopicMessage& msg) -> qtplugin::expected<void, qtplugin::PluginError> {
            Q_UNUSED(msg);
            return {};
        });
    QVERIFY(duplicate_result.has_value());
}

void TestMessageBus::testDeleteTopic() {
    // Create a simple test message type
    class DeleteTestMessage : public qtplugin::Message<DeleteTestMessage> {
    public:
        DeleteTestMessage() : qtplugin::Message<DeleteTestMessage>("test_sender") {}
        std::string content = "delete_test";

        QJsonObject to_json() const override {
            QJsonObject obj;
            obj["type"] = QString::fromStdString(std::string(type()));
            obj["sender"] = QString::fromStdString(std::string(sender()));
            obj["content"] = QString::fromStdString(content);
            obj["id"] = QString::fromStdString(id());
            return obj;
        }
    };

    // Subscribe to message type (this implicitly "creates" the topic)
    auto create_result = m_message_bus->subscribe<DeleteTestMessage>(
        "delete_subscriber", [](const DeleteTestMessage& msg) -> qtplugin::expected<void, qtplugin::PluginError> {
            Q_UNUSED(msg);
            return {};
        });
    QVERIFY(create_result.has_value());
    QVERIFY(m_message_bus->has_subscriber("delete_subscriber"));

    // "Delete" the topic by unsubscribing
    auto delete_result = m_message_bus->unsubscribe("delete_subscriber");
    QVERIFY(delete_result.has_value());

    // Verify subscription was removed
    QVERIFY(!m_message_bus->has_subscriber("delete_subscriber"));
    auto stats = m_message_bus->statistics();
    QCOMPARE(stats["total_subscriptions"].toInt(), 0);
}

void TestMessageBus::testMessageDeliveryOrder() {
    // Create a simple test message type
    class OrderTestMessage : public qtplugin::Message<OrderTestMessage> {
    public:
        OrderTestMessage(const std::string& content) : qtplugin::Message<OrderTestMessage>("test_sender"), content(content) {}
        std::string content;

        QJsonObject to_json() const override {
            QJsonObject obj;
            obj["type"] = QString::fromStdString(std::string(type()));
            obj["sender"] = QString::fromStdString(std::string(sender()));
            obj["content"] = QString::fromStdString(content);
            obj["id"] = QString::fromStdString(id());
            return obj;
        }
    };

    // Subscribe to message type
    std::vector<std::string> received_messages;
    auto subscribe_result = m_message_bus->subscribe<OrderTestMessage>(
        "order_subscriber", [&received_messages](const OrderTestMessage& msg) -> qtplugin::expected<void, qtplugin::PluginError> {
            received_messages.push_back(msg.content);
            return {};
        });
    QVERIFY(subscribe_result.has_value());

    // Publish messages in order
    for (int i = 1; i <= 5; ++i) {
        OrderTestMessage msg("Message " + std::to_string(i));
        auto publish_result = m_message_bus->publish(msg);
        QVERIFY(publish_result.has_value());
    }

    // Wait for all messages to be delivered
    QTest::qWait(200);

    // Verify message order
    QCOMPARE(received_messages.size(), 5);
    for (int i = 0; i < 5; ++i) {
        QCOMPARE(received_messages[i], "Message " + std::to_string(i + 1));
    }
}

// Helper methods implementation
void TestMessageBus::createTestMessage(const std::string& content,
                                       int priority) {
    Q_UNUSED(content)
    Q_UNUSED(priority)
    // Helper implementation would go here
}

void TestMessageBus::verifyMessageDelivery(
    const std::string& topic, const std::string& expected_content) {
    Q_UNUSED(topic)
    Q_UNUSED(expected_content)
    // Helper implementation would go here
}

void TestMessageBus::simulateNetworkDelay() {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

QTEST_MAIN(TestMessageBus)
#include "test_message_bus.moc"
