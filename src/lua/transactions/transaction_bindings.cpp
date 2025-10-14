/**
 * @file transaction_bindings.cpp
 * @brief Transaction bindings for Lua (stub implementation)
 * @version 3.2.0
 */

#include <QDebug>
#include <QLoggingCategory>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include <qtplugin/workflow/transactions.hpp>
#include "../qt_conversions.hpp"

Q_LOGGING_CATEGORY(transactionBindingsLog, "qtforge.lua.transactions");

namespace qtforge_lua {

#ifdef QTFORGE_LUA_BINDINGS

void register_transaction_bindings(sol::state& lua) {
    qCDebug(transactionBindingsLog) << "Registering transaction bindings...";

    // Create qtforge.transactions namespace
    sol::table qtforge = lua["qtforge"];
    sol::table transactions =
        qtforge["transactions"].get_or_create<sol::table>();

    // Transaction state enum
    lua.new_enum<qtplugin::TransactionState>(
        "TransactionState",
        {{"Active", qtplugin::TransactionState::Active},
         {"Preparing", qtplugin::TransactionState::Preparing},
         {"Prepared", qtplugin::TransactionState::Prepared},
         {"Committing", qtplugin::TransactionState::Committing},
         {"Committed", qtplugin::TransactionState::Committed},
         {"Aborting", qtplugin::TransactionState::Aborting},
         {"Aborted", qtplugin::TransactionState::Aborted},
         {"Failed", qtplugin::TransactionState::Failed},
         {"Timeout", qtplugin::TransactionState::Timeout}});

    // Isolation level enum
    lua.new_enum<qtplugin::IsolationLevel>(
        "IsolationLevel",
        {{"ReadUncommitted", qtplugin::IsolationLevel::ReadUncommitted},
         {"ReadCommitted", qtplugin::IsolationLevel::ReadCommitted},
         {"RepeatableRead", qtplugin::IsolationLevel::RepeatableRead},
         {"Serializable", qtplugin::IsolationLevel::Serializable}});

    // Plugin transaction manager (singleton)
    auto manager_type = lua.new_usertype<qtplugin::PluginTransactionManager>(
        "PluginTransactionManager");
    manager_type["begin_transaction"] =
        &qtplugin::PluginTransactionManager::begin_transaction;
    manager_type["commit_transaction"] =
        &qtplugin::PluginTransactionManager::commit_transaction;
    manager_type["rollback_transaction"] =
        &qtplugin::PluginTransactionManager::rollback_transaction;
    manager_type["has_transaction"] =
        &qtplugin::PluginTransactionManager::has_transaction;
    manager_type["get_active_transactions"] =
        &qtplugin::PluginTransactionManager::get_active_transactions;
    manager_type["set_default_timeout"] =
        &qtplugin::PluginTransactionManager::set_default_timeout;
    manager_type["get_default_timeout"] =
        &qtplugin::PluginTransactionManager::get_default_timeout;
    manager_type["clear_completed_transactions"] =
        &qtplugin::PluginTransactionManager::clear_completed_transactions;

    // Factory function for singleton access
    transactions["get_transaction_manager"] =
        []() -> qtplugin::PluginTransactionManager& {
        return qtplugin::PluginTransactionManager::instance();
    };

    qCDebug(transactionBindingsLog)
        << "Transaction bindings registered successfully";
}

#else  // QTFORGE_LUA_BINDINGS not defined

namespace sol {
class state;
}

void register_transaction_bindings(sol::state& lua) {
    (void)lua;
    qCWarning(transactionBindingsLog)
        << "Transaction bindings not available - Lua support not compiled";
}

#endif  // QTFORGE_LUA_BINDINGS

}  // namespace qtforge_lua
