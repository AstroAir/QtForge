#include <QCoreApplication>
#include <QDebug>
#include <qtplugin/threading/plugin_thread_pool.hpp>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    qtplugin::PluginThreadPool pool;

    // Configure a small pool
    qtplugin::ThreadPoolConfig cfg;
    cfg.max_thread_count = 2;
    cfg.ideal_thread_count = 2;
    auto set_ok = pool.set_config(cfg);
    if (!set_ok) {
        qWarning() << "Failed to set thread pool config:"
                   << static_cast<int>(set_ok.error().code);
    }

    // Submit a few trivial tasks
    auto id1 = pool.submit_task("quick_task", "examples.threading", []() {
        // Simulate a short CPU task
        volatile int s = 0;
        for (int i = 0; i < 100000; ++i)
            s += i;
    });
    auto id2 = pool.submit_task(
        "delayed_task", "examples.threading", []() { QThread::msleep(50); },
        qtplugin::TaskPriority::High);

    if (!id1)
        qWarning() << "Submit id1 failed:"
                   << static_cast<int>(id1.error().code);
    if (!id2)
        qWarning() << "Submit id2 failed:"
                   << static_cast<int>(id2.error().code);

    // Wait for all tasks
    pool.wait_for_all_tasks(std::chrono::milliseconds{5000});

    // Print basic stats
    auto stats = pool.get_statistics();
    qInfo() << "Threading example complete. Executed tasks:"
            << stats.total_tasks_executed
            << "failed:" << stats.total_tasks_failed
            << "active threads:" << stats.active_thread_count;

    return 0;
}
