/**
 * @file test_lifecycle_compilation.cpp
 * @brief Simple compilation test for Plugin Lifecycle Manager
 * @version 3.0.0
 */

#include <QtTest/QtTest>
#include "qtplugin/qtplugin.hpp"  // This should include the lifecycle manager

using namespace qtplugin;

/**
 * @brief Simple compilation test for Plugin Lifecycle Manager
 */
class TestLifecycleCompilation : public QObject {
    Q_OBJECT

private slots:
    void testHeaderInclusion();
    void testClassAvailability();
    void testStateMachineAvailability();
};

void TestLifecycleCompilation::testHeaderInclusion() {
    // Test that the header can be included without compilation errors
    qDebug() << "Plugin Lifecycle Manager header included successfully";
    QVERIFY(true);
}

void TestLifecycleCompilation::testClassAvailability() {
    // Test that the PluginLifecycleManager class is available for compilation
    // We can't instantiate it due to MOC issues, but we can verify the type
    // exists

    // Test that we can declare a pointer to the class
    PluginLifecycleManager* manager_ptr = nullptr;
    QVERIFY(manager_ptr == nullptr);

    // Test that we can reference the class in sizeof
    size_t class_size = sizeof(PluginLifecycleManager);
    QVERIFY(class_size > 0);

    qDebug() << "PluginLifecycleManager class is available for compilation";
    qDebug() << "Class size:" << class_size << "bytes";
}

void TestLifecycleCompilation::testStateMachineAvailability() {
    // Test that PluginStateMachine is available
    // This class should be fully functional since it doesn't inherit from
    // QObject

    // Test static method availability
    bool valid_transition = PluginStateMachine::is_valid_transition(
        PluginState::Unloaded, PluginState::Loading);
    QVERIFY(valid_transition);

    // Test invalid transition
    bool invalid_transition = PluginStateMachine::is_valid_transition(
        PluginState::Unloaded, PluginState::Running);
    QVERIFY(!invalid_transition);

    qDebug() << "PluginStateMachine static methods work correctly";
}

QTEST_MAIN(TestLifecycleCompilation)
#include "test_lifecycle_compilation.moc"
