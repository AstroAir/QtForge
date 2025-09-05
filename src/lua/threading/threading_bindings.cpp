/**
 * @file threading_bindings.cpp
 * @brief Threading bindings for Lua (stub implementation)
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>
#include <QThread>
#include <QCoreApplication>

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
