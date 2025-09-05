/**
 * @file transaction_bindings.cpp
 * @brief Transaction system Python bindings
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/transactions/plugin_transaction_manager.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_transactions(py::module& m) {
    // Transaction state enum
    py::enum_<TransactionState>(m, "TransactionState")
        .value("Active", TransactionState::Active)
        .value("Preparing", TransactionState::Preparing)
        .value("Prepared", TransactionState::Prepared)
        .value("Committing", TransactionState::Committing)
        .value("Committed", TransactionState::Committed)
        .value("Aborting", TransactionState::Aborting)
        .value("Aborted", TransactionState::Aborted)
        .value("Failed", TransactionState::Failed)
        .value("Timeout", TransactionState::Timeout)
        .export_values();

    // Isolation level enum
    py::enum_<IsolationLevel>(m, "IsolationLevel")
        .value("ReadUncommitted", IsolationLevel::ReadUncommitted)
        .value("ReadCommitted", IsolationLevel::ReadCommitted)
        .value("RepeatableRead", IsolationLevel::RepeatableRead)
        .value("Serializable", IsolationLevel::Serializable)
        .export_values();

    // Operation type enum
    py::enum_<OperationType>(m, "OperationType")
        .value("Read", OperationType::Read)
        .value("Write", OperationType::Write)
        .value("Execute", OperationType::Execute)
        .value("Configure", OperationType::Configure)
        .value("Custom", OperationType::Custom)
        .export_values();

    // Transaction operation
    py::class_<TransactionOperation>(m, "TransactionOperation")
        .def(py::init<>())
        .def(py::init<const QString&, const QString&, OperationType>())
        .def_readwrite("operation_id", &TransactionOperation::operation_id)
        .def_readwrite("plugin_id", &TransactionOperation::plugin_id)
        .def_readwrite("type", &TransactionOperation::type)
        .def_readwrite("method_name", &TransactionOperation::method_name)
        .def_readwrite("parameters", &TransactionOperation::parameters)
        .def_readwrite("rollback_data", &TransactionOperation::rollback_data)
        .def_readwrite("priority", &TransactionOperation::priority)
        .def("__repr__", [](const TransactionOperation& op) {
            return "<TransactionOperation id='" +
                   op.operation_id.toStdString() + "' plugin='" +
                   op.plugin_id.toStdString() + "'>";
        });

    // Transaction participant interface
    py::class_<ITransactionParticipant,
               std::shared_ptr<ITransactionParticipant>>(
        m, "ITransactionParticipant")
        .def("prepare", &ITransactionParticipant::prepare)
        .def("commit", &ITransactionParticipant::commit)
        .def("abort", &ITransactionParticipant::abort)
        .def("supports_transactions",
             &ITransactionParticipant::supports_transactions)
        .def("supported_isolation_level",
             &ITransactionParticipant::supported_isolation_level)
        .def("__repr__", [](const ITransactionParticipant& participant) {
            return "<ITransactionParticipant supports=" +
                   std::string(participant.supports_transactions() ? "true"
                                                                   : "false") +
                   ">";
        });

    // Transaction context
    py::class_<TransactionContext>(m, "TransactionContext")
        .def(py::init<const QString&, IsolationLevel>(),
             py::arg("transaction_id"),
             py::arg("isolation") = IsolationLevel::ReadCommitted)
        .def("transaction_id", &TransactionContext::transaction_id)
        .def("state", &TransactionContext::state)
        .def("isolation_level", &TransactionContext::isolation_level)
        .def("start_time", &TransactionContext::start_time)
        .def("add_operation", &TransactionContext::add_operation)
        .def("get_operations", &TransactionContext::get_operations)
        .def("get_participants", &TransactionContext::get_participants)
        .def("set_timeout", &TransactionContext::set_timeout)
        .def("get_timeout", &TransactionContext::get_timeout)
        .def("__repr__", [](const TransactionContext& ctx) {
            return "<TransactionContext id='" +
                   ctx.transaction_id().toStdString() +
                   "' state=" + std::to_string(static_cast<int>(ctx.state())) +
                   ">";
        });

    // Plugin transaction manager (singleton)
    py::class_<PluginTransactionManager>(m, "PluginTransactionManager")
        .def_static("instance", &PluginTransactionManager::instance,
                    py::return_value_policy::reference)
        .def("begin_transaction", &PluginTransactionManager::begin_transaction)
        .def("commit_transaction",
             &PluginTransactionManager::commit_transaction)
        .def("rollback_transaction", &PluginTransactionManager::rollback_transaction)
        .def("add_operation", &PluginTransactionManager::add_operation)
        .def("register_participant",
             &PluginTransactionManager::register_participant)
        .def("unregister_participant",
             &PluginTransactionManager::unregister_participant)
        .def("get_transaction", &PluginTransactionManager::get_transaction)
        .def("has_transaction", &PluginTransactionManager::has_transaction)
        .def("get_active_transactions",
             &PluginTransactionManager::get_active_transactions)
        .def("set_default_timeout",
             &PluginTransactionManager::set_default_timeout)
        .def("get_default_timeout",
             &PluginTransactionManager::get_default_timeout)
        .def("clear_completed_transactions",
             &PluginTransactionManager::clear_completed_transactions)
        .def("__repr__", [](const PluginTransactionManager& manager) {
            auto active = manager.get_active_transactions();
            return "<PluginTransactionManager active=" +
                   std::to_string(active.size()) + ">";
        });

    // Utility functions
    m.def(
        "get_transaction_manager",
        []() -> PluginTransactionManager& {
            return PluginTransactionManager::instance();
        },
        py::return_value_policy::reference,
        "Get the PluginTransactionManager singleton instance");

    m.def(
        "create_transaction_operation",
        [](const std::string& op_id, const std::string& plugin_id,
           OperationType type) -> TransactionOperation {
            return TransactionOperation(QString::fromStdString(op_id),
                                        QString::fromStdString(plugin_id),
                                        type);
        },
        py::arg("operation_id"), py::arg("plugin_id"), py::arg("type"),
        "Create a new TransactionOperation instance");

    m.def(
        "create_transaction_context",
        [](const std::string& transaction_id,
           IsolationLevel isolation =
               IsolationLevel::ReadCommitted) -> TransactionContext {
            return TransactionContext(QString::fromStdString(transaction_id),
                                      isolation);
        },
        py::arg("transaction_id"),
        py::arg("isolation") = IsolationLevel::ReadCommitted,
        "Create a new TransactionContext instance");

    // Helper functions for common transaction patterns
    m.def(
        "execute_atomic_operation",
        [](const std::vector<TransactionOperation>& operations,
           IsolationLevel isolation = IsolationLevel::ReadCommitted) -> bool {
            try {
                auto& manager = PluginTransactionManager::instance();

                // Begin transaction
                auto tx_result = manager.begin_transaction(isolation);
                if (!tx_result) {
                    return false;
                }

                QString tx_id = tx_result.value();

                // Add all operations
                for (const auto& op : operations) {
                    auto add_result = manager.add_operation(tx_id, op);
                    if (!add_result) {
                        manager.rollback_transaction(tx_id);
                        return false;
                    }
                }

                // Commit transaction
                auto commit_result = manager.commit_transaction(tx_id);
                return commit_result.has_value();

            } catch (...) {
                return false;
            }
        },
        py::arg("operations"),
        py::arg("isolation") = IsolationLevel::ReadCommitted,
        "Execute multiple operations atomically within a single transaction");
}

}  // namespace qtforge_python
