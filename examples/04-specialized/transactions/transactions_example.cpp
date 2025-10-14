#include <QCoreApplication>
#include <QDebug>
#include <qtplugin/workflow/transactions.hpp>

using namespace qtplugin::workflow::transactions;

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    auto& txm = PluginTransactionManager::instance();

    // Begin a transaction
    auto txid_exp = txm.begin_transaction(IsolationLevel::ReadCommitted,
                                          std::chrono::milliseconds{10000});
    if (!txid_exp) {
        qWarning() << "Begin transaction failed:"
                   << static_cast<int>(txid_exp.error().code);
        return 1;
    }
    const QString txid = txid_exp.value();
    qInfo() << "Transaction started:" << txid;

    // Add a simple custom operation (execute_func + rollback_func)
    TransactionOperation op{"op1", "examples.tx", OperationType::Execute};
    op.execute_func = []() {
        QJsonObject out;
        out["status"] = "ok";
        return qtplugin::expected<QJsonObject, qtplugin::PluginError>(out);
    };
    op.rollback_func = []() {
        return qtplugin::expected<void, qtplugin::PluginError>({});
    };

    auto add_ok = txm.add_operation(txid, op);
    if (!add_ok) {
        qWarning() << "Add operation failed:"
                   << static_cast<int>(add_ok.error().code);
        txm.rollback_transaction(txid);
        return 1;
    }

    auto prep_ok = txm.prepare_transaction(txid);
    if (!prep_ok) {
        qWarning() << "Prepare failed, rolling back:"
                   << static_cast<int>(prep_ok.error().code);
        txm.rollback_transaction(txid);
        return 1;
    }

    auto commit_ok = txm.commit_transaction(txid);
    if (!commit_ok) {
        qWarning() << "Commit failed, attempting rollback:"
                   << static_cast<int>(commit_ok.error().code);
        txm.rollback_transaction(txid);
        return 1;
    }

    qInfo() << "Transaction committed successfully";
    return 0;
}
