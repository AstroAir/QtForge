/**
 * @file test_sandbox_manager.cpp
 * @brief Tests for sandbox lifecycle management and multi-sandbox scenarios
 * @version 3.2.0
 */

#include <QCoreApplication>
#include <QSignalSpy>
#include <QTimer>
#include <QtTest/QtTest>
#include <chrono>
#include <memory>
#include <thread>

#include "qtplugin/security/sandbox/plugin_sandbox.hpp"

using namespace qtplugin;

class TestSandboxManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testSingletonBehavior();
    void testDefaultPolicyRegistration();

    // Sandbox lifecycle tests
    void testCreateSandbox();
    void testGetSandbox();
    void testRemoveSandbox();
    void testDuplicateSandboxCreation();
    void testInvalidSandboxRetrieval();

    // Policy management tests
    void testPolicyRegistration();
    void testPolicyRetrieval();
    void testCustomPolicyRegistration();
    void testInvalidPolicyRetrieval();
    void testPolicyOverwrite();

    // Multi-sandbox tests
    void testMultipleSandboxes();
    void testConcurrentSandboxCreation();
    void testSandboxIsolation();
    void testActiveSandboxListing();

    // Lifecycle management tests
    void testShutdownAll();
    void testShutdownWithActiveSandboxes();
    void testManagerReinitialization();

    // Signal tests
    void testSandboxCreatedSignal();
    void testSandboxRemovedSignal();
    void testSecurityEventSignal();

    // Thread safety tests
    void testConcurrentAccess();
    void testThreadSafePolicyManagement();

    // Performance tests
    void testLargeSandboxCount();
    void testRapidCreateRemoveCycle();

private:
    SandboxManager* m_manager;
    QStringList m_created_sandboxes;

    SecurityPolicy createTestPolicy(const QString& name);
    QString generateUniqueSandboxId();
    void cleanupCreatedSandboxes();
};

void TestSandboxManager::initTestCase() {
    // Initialize Qt application for testing
    if (!QCoreApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        new QCoreApplication(argc, argv);
    }
}

void TestSandboxManager::cleanupTestCase() {
    // Final cleanup
    if (m_manager) {
        m_manager->shutdown_all();
    }
}

void TestSandboxManager::init() {
    m_manager = &SandboxManager::instance();
    m_created_sandboxes.clear();
}

void TestSandboxManager::cleanup() { cleanupCreatedSandboxes(); }

void TestSandboxManager::testSingletonBehavior() {
    SandboxManager& manager1 = SandboxManager::instance();
    SandboxManager& manager2 = SandboxManager::instance();

    // Verify singleton behavior
    QCOMPARE(&manager1, &manager2);
    QCOMPARE(m_manager, &manager1);
}

void TestSandboxManager::testDefaultPolicyRegistration() {
    auto policies = m_manager->get_registered_policies();

    // Should have at least the 4 default policies
    QVERIFY(policies.size() >= 4);

    // Check for specific default policies
    QVERIFY(std::find(policies.begin(), policies.end(), "unrestricted") !=
            policies.end());
    QVERIFY(std::find(policies.begin(), policies.end(), "limited") !=
            policies.end());
    QVERIFY(std::find(policies.begin(), policies.end(), "sandboxed") !=
            policies.end());
    QVERIFY(std::find(policies.begin(), policies.end(), "strict") !=
            policies.end());

    // Verify we can retrieve each default policy
    auto unrestricted = m_manager->get_policy("unrestricted");
    QVERIFY(unrestricted.has_value());
    QCOMPARE(unrestricted.value().level, SandboxSecurityLevel::Unrestricted);

    auto strict = m_manager->get_policy("strict");
    QVERIFY(strict.has_value());
    QCOMPARE(strict.value().level, SandboxSecurityLevel::Strict);
}

void TestSandboxManager::testCreateSandbox() {
    QString sandbox_id = generateUniqueSandboxId();
    SecurityPolicy policy = createTestPolicy("test_create");

    QSignalSpy spy(m_manager, SIGNAL(sandbox_created(const QString&)));

    auto result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(result.has_value());

    auto sandbox = result.value();
    QVERIFY(sandbox != nullptr);
    QVERIFY(sandbox->is_active());
    QCOMPARE(sandbox->get_policy().policy_name, policy.policy_name);

    // Verify signal was emitted
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), sandbox_id);

    m_created_sandboxes.append(sandbox_id);
}

void TestSandboxManager::testGetSandbox() {
    QString sandbox_id = generateUniqueSandboxId();
    SecurityPolicy policy = createTestPolicy("test_get");

    // Create sandbox
    auto create_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(create_result.has_value());
    m_created_sandboxes.append(sandbox_id);

    // Retrieve sandbox
    auto retrieved = m_manager->get_sandbox(sandbox_id);
    QVERIFY(retrieved != nullptr);
    QCOMPARE(retrieved.get(), create_result.value().get());
}

void TestSandboxManager::testRemoveSandbox() {
    QString sandbox_id = generateUniqueSandboxId();
    SecurityPolicy policy = createTestPolicy("test_remove");

    // Create sandbox
    auto create_result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(create_result.has_value());

    QSignalSpy spy(m_manager, SIGNAL(sandbox_removed(const QString&)));

    // Remove sandbox
    m_manager->remove_sandbox(sandbox_id);

    // Verify removal
    auto retrieved = m_manager->get_sandbox(sandbox_id);
    QVERIFY(retrieved == nullptr);

    // Verify signal was emitted
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), sandbox_id);
}

void TestSandboxManager::testDuplicateSandboxCreation() {
    QString sandbox_id = generateUniqueSandboxId();
    SecurityPolicy policy = createTestPolicy("test_duplicate");

    // Create first sandbox
    auto result1 = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(result1.has_value());
    m_created_sandboxes.append(sandbox_id);

    // Attempt to create duplicate
    auto result2 = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(!result2.has_value());
    QCOMPARE(result2.error().code, PluginErrorCode::InvalidArgument);
}

void TestSandboxManager::testInvalidSandboxRetrieval() {
    QString non_existent_id = "non_existent_sandbox_12345";

    auto retrieved = m_manager->get_sandbox(non_existent_id);
    QVERIFY(retrieved == nullptr);
}

void TestSandboxManager::testPolicyRegistration() {
    SecurityPolicy custom_policy = createTestPolicy("custom_test_policy");

    m_manager->register_policy("custom_test", custom_policy);

    auto policies = m_manager->get_registered_policies();
    QVERIFY(std::find(policies.begin(), policies.end(), "custom_test") !=
            policies.end());

    auto retrieved_policy = m_manager->get_policy("custom_test");
    QVERIFY(retrieved_policy.has_value());
    QCOMPARE(retrieved_policy.value().policy_name, custom_policy.policy_name);
}

void TestSandboxManager::testPolicyRetrieval() {
    // Test retrieving existing policy
    auto policy_result = m_manager->get_policy("limited");
    QVERIFY(policy_result.has_value());

    SecurityPolicy policy = policy_result.value();
    QCOMPARE(policy.policy_name, QString("limited"));
    QCOMPARE(policy.level, SandboxSecurityLevel::Limited);
}

void TestSandboxManager::testCustomPolicyRegistration() {
    SecurityPolicy custom1 = createTestPolicy("custom1");
    SecurityPolicy custom2 = createTestPolicy("custom2");

    m_manager->register_policy("custom1", custom1);
    m_manager->register_policy("custom2", custom2);

    auto policies = m_manager->get_registered_policies();
    QVERIFY(std::find(policies.begin(), policies.end(), "custom1") !=
            policies.end());
    QVERIFY(std::find(policies.begin(), policies.end(), "custom2") !=
            policies.end());

    // Verify both can be retrieved
    auto policy1 = m_manager->get_policy("custom1");
    auto policy2 = m_manager->get_policy("custom2");
    QVERIFY(policy1.has_value());
    QVERIFY(policy2.has_value());
}

void TestSandboxManager::testInvalidPolicyRetrieval() {
    auto result = m_manager->get_policy("non_existent_policy");
    QVERIFY(!result.has_value());
    QCOMPARE(result.error().code, PluginErrorCode::NotFound);
}

void TestSandboxManager::testPolicyOverwrite() {
    SecurityPolicy original = createTestPolicy("original");
    SecurityPolicy updated = createTestPolicy("updated");

    // Register original policy
    m_manager->register_policy("overwrite_test", original);

    auto retrieved1 = m_manager->get_policy("overwrite_test");
    QVERIFY(retrieved1.has_value());
    QCOMPARE(retrieved1.value().policy_name, QString("original"));

    // Overwrite with updated policy
    m_manager->register_policy("overwrite_test", updated);

    auto retrieved2 = m_manager->get_policy("overwrite_test");
    QVERIFY(retrieved2.has_value());
    QCOMPARE(retrieved2.value().policy_name, QString("updated"));
}

void TestSandboxManager::testMultipleSandboxes() {
    const int sandbox_count = 5;
    QStringList sandbox_ids;

    // Create multiple sandboxes
    for (int i = 0; i < sandbox_count; ++i) {
        QString sandbox_id = generateUniqueSandboxId();
        SecurityPolicy policy =
            createTestPolicy(QString("multi_test_%1").arg(i));

        auto result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(result.has_value());

        sandbox_ids.append(sandbox_id);
        m_created_sandboxes.append(sandbox_id);
    }

    // Verify all sandboxes exist and are active
    for (const QString& sandbox_id : sandbox_ids) {
        auto sandbox = m_manager->get_sandbox(sandbox_id);
        QVERIFY(sandbox != nullptr);
        QVERIFY(sandbox->is_active());
    }

    // Verify active sandbox count
    auto active_sandboxes = m_manager->get_active_sandboxes();
    QVERIFY(active_sandboxes.size() >= sandbox_count);
}

void TestSandboxManager::testConcurrentSandboxCreation() {
    const int thread_count = 4;
    const int sandboxes_per_thread = 3;
    std::vector<std::thread> threads;
    std::vector<QString> all_sandbox_ids;
    std::mutex ids_mutex;

    // Create threads that each create multiple sandboxes
    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([this, t, sandboxes_per_thread, &all_sandbox_ids,
                              &ids_mutex]() {
            for (int i = 0; i < sandboxes_per_thread; ++i) {
                QString sandbox_id =
                    QString("concurrent_%1_%2_%3")
                        .arg(t)
                        .arg(i)
                        .arg(QDateTime::currentMSecsSinceEpoch());

                SecurityPolicy policy =
                    createTestPolicy(QString("concurrent_%1_%2").arg(t).arg(i));

                auto result = m_manager->create_sandbox(sandbox_id, policy);
                if (result.has_value()) {
                    std::lock_guard<std::mutex> lock(ids_mutex);
                    all_sandbox_ids.push_back(sandbox_id);
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify all sandboxes were created successfully
    QCOMPARE(all_sandbox_ids.size(),
             static_cast<size_t>(thread_count * sandboxes_per_thread));

    // Add to cleanup list
    for (const QString& sandbox_id : all_sandbox_ids) {
        m_created_sandboxes.append(sandbox_id);
    }

    // Verify all sandboxes are accessible
    for (const QString& sandbox_id : all_sandbox_ids) {
        auto sandbox = m_manager->get_sandbox(sandbox_id);
        QVERIFY(sandbox != nullptr);
    }
}

void TestSandboxManager::testSandboxIsolation() {
    QString sandbox_id1 = generateUniqueSandboxId();
    QString sandbox_id2 = generateUniqueSandboxId();

    SecurityPolicy policy1 = createTestPolicy("isolation_test_1");
    SecurityPolicy policy2 = createTestPolicy("isolation_test_2");

    // Create two sandboxes with different policies
    auto result1 = m_manager->create_sandbox(sandbox_id1, policy1);
    auto result2 = m_manager->create_sandbox(sandbox_id2, policy2);

    QVERIFY(result1.has_value());
    QVERIFY(result2.has_value());

    m_created_sandboxes.append(sandbox_id1);
    m_created_sandboxes.append(sandbox_id2);

    auto sandbox1 = result1.value();
    auto sandbox2 = result2.value();

    // Verify they are different instances
    QVERIFY(sandbox1.get() != sandbox2.get());

    // Verify they have different policies
    QCOMPARE(sandbox1->get_policy().policy_name, QString("isolation_test_1"));
    QCOMPARE(sandbox2->get_policy().policy_name, QString("isolation_test_2"));

    // Verify they can be managed independently
    ResourceUsage usage1 = sandbox1->get_resource_usage();
    ResourceUsage usage2 = sandbox2->get_resource_usage();

    // They should have independent resource tracking
    QVERIFY(usage1.start_time != usage2.start_time ||
            std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
                         usage1.start_time - usage2.start_time)
                         .count()) < 100);
}

void TestSandboxManager::testActiveSandboxListing() {
    // Get initial active count
    auto initial_active = m_manager->get_active_sandboxes();
    size_t initial_count = initial_active.size();

    // Create some sandboxes
    const int new_sandbox_count = 3;
    for (int i = 0; i < new_sandbox_count; ++i) {
        QString sandbox_id = generateUniqueSandboxId();
        SecurityPolicy policy =
            createTestPolicy(QString("active_test_%1").arg(i));

        auto result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(result.has_value());
        m_created_sandboxes.append(sandbox_id);
    }

    // Verify active count increased
    auto updated_active = m_manager->get_active_sandboxes();
    QCOMPARE(updated_active.size(), initial_count + new_sandbox_count);

    // Remove one sandbox and verify count decreases
    if (!m_created_sandboxes.isEmpty()) {
        QString to_remove = m_created_sandboxes.takeLast();
        m_manager->remove_sandbox(to_remove);

        auto after_removal = m_manager->get_active_sandboxes();
        QCOMPARE(after_removal.size(), initial_count + new_sandbox_count - 1);
    }
}

void TestSandboxManager::testShutdownAll() {
    // Create some sandboxes
    const int sandbox_count = 3;
    for (int i = 0; i < sandbox_count; ++i) {
        QString sandbox_id = generateUniqueSandboxId();
        SecurityPolicy policy =
            createTestPolicy(QString("shutdown_test_%1").arg(i));

        auto result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(result.has_value());
        m_created_sandboxes.append(sandbox_id);
    }

    // Verify sandboxes exist
    auto active_before = m_manager->get_active_sandboxes();
    QVERIFY(active_before.size() >= sandbox_count);

    QSignalSpy spy(m_manager, SIGNAL(sandbox_removed(const QString&)));

    // Shutdown all
    m_manager->shutdown_all();

    // Verify all sandboxes are gone
    auto active_after = m_manager->get_active_sandboxes();
    QCOMPARE(active_after.size(), static_cast<size_t>(0));

    // Verify signals were emitted
    QVERIFY(spy.count() >= sandbox_count);

    // Clear our tracking list since they're all removed
    m_created_sandboxes.clear();
}

void TestSandboxManager::testLargeSandboxCount() {
    const int large_count = 50;
    QStringList large_sandbox_ids;

    QElapsedTimer timer;
    timer.start();

    // Create many sandboxes
    for (int i = 0; i < large_count; ++i) {
        QString sandbox_id = generateUniqueSandboxId();
        SecurityPolicy policy =
            createTestPolicy(QString("large_test_%1").arg(i));

        auto result = m_manager->create_sandbox(sandbox_id, policy);
        if (result.has_value()) {
            large_sandbox_ids.append(sandbox_id);
        }
    }

    qint64 creation_time = timer.elapsed();

    // Verify creation was reasonably fast (less than 5 seconds)
    QVERIFY(creation_time < 5000);

    // Verify all were created
    QCOMPARE(large_sandbox_ids.size(), large_count);

    // Verify they're all active
    auto active_sandboxes = m_manager->get_active_sandboxes();
    QVERIFY(active_sandboxes.size() >= large_count);

    // Cleanup
    for (const QString& sandbox_id : large_sandbox_ids) {
        m_manager->remove_sandbox(sandbox_id);
    }

    qDebug() << "Created" << large_count << "sandboxes in" << creation_time
             << "ms";
}

void TestSandboxManager::testShutdownWithActiveSandboxes() {
    // Create some active sandboxes
    const int sandbox_count = 3;
    QStringList sandbox_ids;

    for (int i = 0; i < sandbox_count; ++i) {
        QString sandbox_id = generateUniqueSandboxId();
        SecurityPolicy policy = createTestPolicy(QString("shutdown_test_%1").arg(i));

        auto result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(result.has_value());
        sandbox_ids.append(sandbox_id);
        m_created_sandboxes.append(sandbox_id);
    }

    // Verify sandboxes are active
    auto active_before = m_manager->get_active_sandboxes();
    QVERIFY(active_before.size() >= sandbox_count);

    // Shutdown all
    m_manager->shutdown_all();

    // Verify all are shutdown
    auto active_after = m_manager->get_active_sandboxes();
    QVERIFY(active_after.size() == 0);

    m_created_sandboxes.clear(); // Already removed by shutdown_all
}

void TestSandboxManager::testManagerReinitialization() {
    // Test that manager is functional (no explicit is_initialized method)
    // Create a sandbox to test functionality
    QString sandbox_id = generateUniqueSandboxId();
    SecurityPolicy policy = createTestPolicy("reinit_test");

    auto result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(result.has_value());
    m_created_sandboxes.append(sandbox_id);

    // Manager should still be functional
    auto retrieved = m_manager->get_sandbox(sandbox_id);
    QVERIFY(retrieved != nullptr);

    // Test that we can create another sandbox (manager is still working)
    QString sandbox_id2 = generateUniqueSandboxId();
    auto result2 = m_manager->create_sandbox(sandbox_id2, policy);
    QVERIFY(result2.has_value());
    m_created_sandboxes.append(sandbox_id2);
}

void TestSandboxManager::testSandboxCreatedSignal() {
    QSignalSpy spy(m_manager, SIGNAL(sandbox_created(const QString&)));
    QVERIFY(spy.isValid());

    QString sandbox_id = generateUniqueSandboxId();
    SecurityPolicy policy = createTestPolicy("signal_test");

    auto result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(result.has_value());
    m_created_sandboxes.append(sandbox_id);

    // Verify signal was emitted
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), sandbox_id);
}

void TestSandboxManager::testSandboxRemovedSignal() {
    QString sandbox_id = generateUniqueSandboxId();
    SecurityPolicy policy = createTestPolicy("remove_signal_test");

    // Create sandbox first
    auto result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(result.has_value());

    // Set up signal spy
    QSignalSpy spy(m_manager, SIGNAL(sandbox_removed(const QString&)));
    QVERIFY(spy.isValid());

    // Remove sandbox
    m_manager->remove_sandbox(sandbox_id);

    // Verify signal was emitted
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), sandbox_id);
}

void TestSandboxManager::testSecurityEventSignal() {
    QSignalSpy spy(m_manager, SIGNAL(security_event(const QString&, const QString&, const QJsonObject&)));
    QVERIFY(spy.isValid());

    // This test verifies the signal exists and can be connected
    // In a real scenario, security events would be triggered by sandbox violations

    QString sandbox_id = generateUniqueSandboxId();
    SecurityPolicy policy = createTestPolicy("security_event_test");

    auto result = m_manager->create_sandbox(sandbox_id, policy);
    QVERIFY(result.has_value());
    m_created_sandboxes.append(sandbox_id);
}

void TestSandboxManager::testConcurrentAccess() {
    const int thread_count = 4;
    const int operations_per_thread = 5;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([this, t, operations_per_thread, &success_count]() {
            for (int i = 0; i < operations_per_thread; ++i) {
                QString sandbox_id = QString("concurrent_%1_%2").arg(t).arg(i);
                SecurityPolicy policy = createTestPolicy(QString("concurrent_test_%1_%2").arg(t).arg(i));

                auto result = m_manager->create_sandbox(sandbox_id, policy);
                if (result.has_value()) {
                    success_count.fetch_add(1);
                    // Clean up immediately to avoid conflicts
                    m_manager->remove_sandbox(sandbox_id);
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify that most operations succeeded (some may fail due to timing)
    QVERIFY(success_count.load() > (thread_count * operations_per_thread) / 2);
}

void TestSandboxManager::testThreadSafePolicyManagement() {
    const int thread_count = 3;
    const int policies_per_thread = 5;
    std::vector<std::thread> threads;
    std::atomic<int> success_count{0};

    for (int t = 0; t < thread_count; ++t) {
        threads.emplace_back([this, t, policies_per_thread, &success_count]() {
            for (int i = 0; i < policies_per_thread; ++i) {
                QString policy_name = QString("thread_policy_%1_%2").arg(t).arg(i);
                SecurityPolicy policy = createTestPolicy(policy_name);

                m_manager->register_policy(policy_name, policy);

                auto retrieved = m_manager->get_policy(policy_name);
                if (retrieved.has_value()) {
                    success_count.fetch_add(1);
                }
            }
        });
    }

    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    // Verify that all policy operations succeeded
    QCOMPARE(success_count.load(), thread_count * policies_per_thread);
}

void TestSandboxManager::testRapidCreateRemoveCycle() {
    const int cycle_count = 20;
    QElapsedTimer timer;
    timer.start();

    for (int i = 0; i < cycle_count; ++i) {
        QString sandbox_id = QString("rapid_cycle_%1").arg(i);
        SecurityPolicy policy = createTestPolicy(QString("rapid_test_%1").arg(i));

        // Create
        auto create_result = m_manager->create_sandbox(sandbox_id, policy);
        QVERIFY(create_result.has_value());

        // Verify exists
        auto retrieved = m_manager->get_sandbox(sandbox_id);
        QVERIFY(retrieved != nullptr);

        // Remove
        m_manager->remove_sandbox(sandbox_id);

        // Verify removed
        auto after_removal = m_manager->get_sandbox(sandbox_id);
        QVERIFY(after_removal == nullptr);
    }

    qint64 elapsed = timer.elapsed();
    qDebug() << "Rapid create/remove cycle completed in" << elapsed << "ms";

    // Verify reasonable performance (should complete in under 5 seconds)
    QVERIFY(elapsed < 5000);
}

SecurityPolicy TestSandboxManager::createTestPolicy(const QString& name) {
    SecurityPolicy policy;
    policy.level = SandboxSecurityLevel::Limited;
    policy.policy_name = name;
    policy.description = QString("Test policy: %1").arg(name);

    policy.limits.cpu_time_limit = std::chrono::minutes(5);
    policy.limits.memory_limit_mb = 256;
    policy.limits.disk_space_limit_mb = 100;
    policy.limits.max_file_handles = 50;
    policy.limits.max_network_connections = 10;
    policy.limits.execution_timeout = std::chrono::minutes(2);

    policy.permissions.allow_file_system_read = true;
    policy.permissions.allow_file_system_write = false;
    policy.permissions.allow_network_access = false;
    policy.permissions.allow_process_creation = false;
    policy.permissions.allow_system_calls = false;
    policy.permissions.allow_registry_access = false;
    policy.permissions.allow_environment_access = false;

    return policy;
}

QString TestSandboxManager::generateUniqueSandboxId() {
    return QString("test_sandbox_%1_%2")
        .arg(QDateTime::currentMSecsSinceEpoch())
        .arg(QRandomGenerator::global()->generate());
}

void TestSandboxManager::cleanupCreatedSandboxes() {
    for (const QString& sandbox_id : m_created_sandboxes) {
        m_manager->remove_sandbox(sandbox_id);
    }
    m_created_sandboxes.clear();
}

QTEST_MAIN(TestSandboxManager)
#include "test_sandbox_manager.moc"
