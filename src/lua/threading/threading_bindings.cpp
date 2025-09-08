/**
 * @file threading_bindings.cpp
 * @brief Threading bindings for Lua (stub implementation)
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>
#include <QThread>
#include <QCoreApplication>
#include <QTimer>
#include <QThreadPool>
#include <QMutex>
#include <memory>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

Q_LOGGING_CATEGORY(threadingBindingsLog, "qtforge.lua.threading");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

void register_threading_bindings(sol::state& lua) {
    qCDebug(threadingBindingsLog) << "Registering threading bindings...";

    // Create qtforge.threading namespace
    sol::table qtforge = lua["qtforge"];
    sol::table threading = qtforge.get_or_create<sol::table>("threading");

    // Basic threading utilities
    threading["get_thread_count"] = []() -> int {
        return QThread::idealThreadCount();
    };

    threading["current_thread_id"] = []() -> std::string {
        return QString::number(reinterpret_cast<quintptr>(QThread::currentThread())).toStdString();
    };

    threading["is_main_thread"] = []() -> bool {
        return QThread::currentThread() == QCoreApplication::instance()->thread();
    };

    // Thread priority enum
    lua.new_enum<QThread::Priority>("ThreadPriority", {
        {"IdlePriority", QThread::IdlePriority},
        {"LowestPriority", QThread::LowestPriority},
        {"LowPriority", QThread::LowPriority},
        {"NormalPriority", QThread::NormalPriority},
        {"HighPriority", QThread::HighPriority},
        {"HighestPriority", QThread::HighestPriority},
        {"TimeCriticalPriority", QThread::TimeCriticalPriority},
        {"InheritPriority", QThread::InheritPriority}
    });

    // Thread utilities
    threading["sleep"] = [](int milliseconds) {
        QThread::msleep(milliseconds);
    };

    threading["yield"] = []() {
        QThread::yieldCurrentThread();
    };

    // Thread pool utilities
    threading["get_global_thread_pool_max_threads"] = []() -> int {
        return QThreadPool::globalInstance()->maxThreadCount();
    };

    threading["set_global_thread_pool_max_threads"] = [](int max_threads) {
        QThreadPool::globalInstance()->setMaxThreadCount(max_threads);
    };

    threading["get_global_thread_pool_active_threads"] = []() -> int {
        return QThreadPool::globalInstance()->activeThreadCount();
    };

    // Mutex utilities (basic wrapper)
    auto mutex_type = lua.new_usertype<QMutex>("Mutex",
        sol::constructors<QMutex()>()
    );
    mutex_type["lock"] = &QMutex::lock;
    mutex_type["unlock"] = &QMutex::unlock;
    mutex_type["try_lock"] = &QMutex::tryLock;

    // Factory functions
    threading["create_mutex"] = []() {
        return std::make_shared<QMutex>();
    };

    // Timer utilities
    threading["create_timer"] = [&lua](int interval_ms, const sol::function& callback) -> std::shared_ptr<QTimer> {
        auto timer = std::make_shared<QTimer>();
        timer->setInterval(interval_ms);

        // Connect callback (note: this is a simplified approach)
        QObject::connect(timer.get(), &QTimer::timeout, [callback]() {
            try {
                callback();
            } catch (const std::exception& e) {
                qCWarning(threadingBindingsLog) << "Timer callback error:" << e.what();
            }
        });

        return timer;
    };

    // QTimer bindings
    auto timer_type = lua.new_usertype<QTimer>("Timer");
    timer_type["start"] = sol::overload(
        static_cast<void(QTimer::*)()>(&QTimer::start),
        static_cast<void(QTimer::*)(int)>(&QTimer::start)
    );
    timer_type["stop"] = &QTimer::stop;
    timer_type["set_interval"] = &QTimer::setInterval;
    timer_type["interval"] = &QTimer::interval;
    timer_type["is_active"] = &QTimer::isActive;
    timer_type["set_single_shot"] = &QTimer::setSingleShot;
    timer_type["is_single_shot"] = &QTimer::isSingleShot;

    qCDebug(threadingBindingsLog) << "Threading bindings registered successfully";
}

#else // QTFORGE_LUA_BINDINGS not defined

namespace sol { class state; }

void register_threading_bindings(sol::state& lua) {
    (void)lua;
    qCWarning(threadingBindingsLog) << "Threading bindings not available - Lua support not compiled";
}

#endif // QTFORGE_LUA_BINDINGS

} // namespace qtforge_lua
