#include "qtplugin/security/components/security_policy_engine.hpp"
#include <QtCore/QDebug>

namespace qtplugin {
namespace security {
namespace components {

SecurityPolicyEngine::SecurityPolicyEngine(QObject* parent)
    : QObject(parent), m_strictMode(false) {}

bool SecurityPolicyEngine::initialize() {
    qDebug() << "SecurityPolicyEngine initialized";
    return true;
}

bool SecurityPolicyEngine::validatePlugin(const QString& pluginPath) {
    Q_UNUSED(pluginPath)
    // Stub implementation - always return true for testing
    return true;
}

bool SecurityPolicyEngine::isOperationAllowed(const QString& operation,
                                              const QVariant& context) {
    Q_UNUSED(operation)
    Q_UNUSED(context)
    // Stub implementation - always allow operations for testing
    return true;
}

void SecurityPolicyEngine::setConfiguration(const QVariantMap& config) {
    m_configuration = config;
    emit configurationChanged();
}

QVariantMap SecurityPolicyEngine::getConfiguration() const {
    return m_configuration;
}

void SecurityPolicyEngine::setStrictMode(bool enabled) {
    if (m_strictMode != enabled) {
        m_strictMode = enabled;
        emit configurationChanged();
    }
}

bool SecurityPolicyEngine::isStrictModeEnabled() const { return m_strictMode; }

}  // namespace components
}  // namespace security
}  // namespace qtplugin
