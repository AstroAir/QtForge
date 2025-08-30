/**
 * @file security_policy_engine.cpp
 * @brief Implementation of security policy engine
 * @version 3.0.0
 */

#include "../../../include/qtplugin/security/components/security_policy_engine.hpp"
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <algorithm>
#include "../../../include/qtplugin/security/security_manager.hpp"

Q_LOGGING_CATEGORY(securityPolicyLog, "qtplugin.security.policy")

namespace qtplugin {

SecurityPolicyEngine::SecurityPolicyEngine(QObject* parent) : QObject(parent) {
    // Initialize with default policy
    m_policy_config = get_default_policy();
    initialize_default_rules();

    qCDebug(securityPolicyLog) << "Security policy engine initialized";
}

SecurityPolicyEngine::~SecurityPolicyEngine() {
    qCDebug(securityPolicyLog) << "Security policy engine destroyed";
}

SecurityValidationResult SecurityPolicyEngine::evaluate_policy(
    const std::filesystem::path& file_path, const QJsonObject& context) const {
    SecurityValidationResult result;
    result.is_valid = true;
    result.validated_level = SecurityLevel::Basic;

    try {
        // Evaluate all enabled rules in priority order
        auto sorted_rules = m_rules;
        std::sort(sorted_rules.begin(), sorted_rules.end(),
                  [](const SecurityPolicyRule& a, const SecurityPolicyRule& b) {
                      return a.priority > b.priority;  // Higher priority first
                  });

        for (const auto& rule : sorted_rules) {
            if (!rule.enabled) {
                continue;
            }

            bool rule_result = evaluate_rule(rule, file_path, context);

            if (!rule_result) {
                if (rule.action == "deny") {
                    result.is_valid = false;
                    result.errors.push_back("Policy rule violated: " +
                                            rule.name);
                    // Note: Cannot emit signals from const method
                    // emit policy_violation(QString::fromStdString(rule.name),
                    //                       QString::fromStdString(file_path.string()));
                } else if (rule.action == "warn") {
                    result.warnings.push_back("Policy warning: " + rule.name);
                }
            }
        }

        // Check global policy settings
        if (m_policy_config.contains("allowUnsignedPlugins") &&
            !m_policy_config["allowUnsignedPlugins"].toBool()) {
            // This would need integration with SignatureVerifier
            result.warnings.push_back(
                "Unsigned plugins policy check not implemented");
        }

        if (m_policy_config.contains("requireTrustedPublisher") &&
            m_policy_config["requireTrustedPublisher"].toBool()) {
            result.warnings.push_back(
                "Trusted publisher check not implemented");
        }

        // Note: Cannot emit signals from const method
        // emit policy_evaluated(QString::fromStdString(file_path.string()),
        // result.is_valid);

    } catch (const std::exception& e) {
        result.is_valid = false;
        result.errors.push_back("Exception during policy evaluation: " +
                                std::string(e.what()));
    } catch (...) {
        result.is_valid = false;
        result.errors.push_back("Unknown exception during policy evaluation");
    }

    return result;
}

qtplugin::expected<void, PluginError> SecurityPolicyEngine::load_policy(
    const std::filesystem::path& policy_file) {
    try {
        QFile file(QString::fromStdString(policy_file.string()));
        if (!file.open(QIODevice::ReadOnly)) {
            return make_error<void>(
                PluginErrorCode::FileNotFound,
                "Cannot open policy file: " + policy_file.string());
        }

        QJsonParseError parse_error;
        QJsonDocument doc =
            QJsonDocument::fromJson(file.readAll(), &parse_error);

        if (parse_error.error != QJsonParseError::NoError) {
            return make_error<void>(
                PluginErrorCode::InvalidFormat,
                "JSON parse error: " + parse_error.errorString().toStdString());
        }

        m_policy_config = doc.object();

        // Load rules if present
        if (m_policy_config.contains("rules") &&
            m_policy_config["rules"].isArray()) {
            m_rules.clear();
            QJsonArray rules_array = m_policy_config["rules"].toArray();

            for (const auto& rule_value : rules_array) {
                if (rule_value.isObject()) {
                    QJsonObject rule_obj = rule_value.toObject();
                    SecurityPolicyRule rule;
                    rule.name = rule_obj["name"].toString().toStdString();
                    rule.condition =
                        rule_obj["condition"].toString().toStdString();
                    rule.action = rule_obj["action"].toString().toStdString();
                    rule.enabled = rule_obj["enabled"].toBool(true);
                    rule.priority = rule_obj["priority"].toInt(0);

                    m_rules.push_back(rule);
                }
            }
        }

        qCDebug(securityPolicyLog)
            << "Policy loaded from"
            << QString::fromStdString(policy_file.string());
        emit policy_updated();

        return make_success();

    } catch (const std::exception& e) {
        return make_error<void>(
            PluginErrorCode::LoadFailed,
            "Exception loading policy: " + std::string(e.what()));
    }
}

qtplugin::expected<void, PluginError> SecurityPolicyEngine::save_policy(
    const std::filesystem::path& policy_file) const {
    try {
        // Add rules to policy config
        QJsonObject config = m_policy_config;
        QJsonArray rules_array;

        for (const auto& rule : m_rules) {
            QJsonObject rule_obj;
            rule_obj["name"] = QString::fromStdString(rule.name);
            rule_obj["condition"] = QString::fromStdString(rule.condition);
            rule_obj["action"] = QString::fromStdString(rule.action);
            rule_obj["enabled"] = rule.enabled;
            rule_obj["priority"] = rule.priority;
            rules_array.append(rule_obj);
        }

        config["rules"] = rules_array;

        QJsonDocument doc(config);

        QFile file(QString::fromStdString(policy_file.string()));
        if (!file.open(QIODevice::WriteOnly)) {
            return make_error<void>(
                PluginErrorCode::FileNotFound,
                "Cannot create policy file: " + policy_file.string());
        }

        file.write(doc.toJson());

        qCDebug(securityPolicyLog)
            << "Policy saved to"
            << QString::fromStdString(policy_file.string());

        return make_success();

    } catch (const std::exception& e) {
        return make_error<void>(
            PluginErrorCode::LoadFailed,
            "Exception saving policy: " + std::string(e.what()));
    }
}

qtplugin::expected<void, PluginError> SecurityPolicyEngine::add_rule(
    const SecurityPolicyRule& rule) {
    if (rule.name.empty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                                "Rule name cannot be empty");
    }

    // Check if rule already exists
    auto it = std::find_if(m_rules.begin(), m_rules.end(),
                           [&rule](const SecurityPolicyRule& existing) {
                               return existing.name == rule.name;
                           });

    if (it != m_rules.end()) {
        // Update existing rule
        *it = rule;
    } else {
        // Add new rule
        m_rules.push_back(rule);
    }

    qCDebug(securityPolicyLog)
        << "Policy rule added/updated:" << QString::fromStdString(rule.name);
    emit policy_updated();

    return make_success();
}

void SecurityPolicyEngine::remove_rule(const std::string& rule_name) {
    auto it = std::find_if(m_rules.begin(), m_rules.end(),
                           [&rule_name](const SecurityPolicyRule& rule) {
                               return rule.name == rule_name;
                           });

    if (it != m_rules.end()) {
        m_rules.erase(it);
        qCDebug(securityPolicyLog)
            << "Policy rule removed:" << QString::fromStdString(rule_name);
        emit policy_updated();
    }
}

std::vector<SecurityPolicyRule> SecurityPolicyEngine::get_rules() const {
    return m_rules;
}

QJsonObject SecurityPolicyEngine::get_default_policy() const {
    QJsonObject policy;

    // Default security policy settings
    policy["allowUnsignedPlugins"] = false;
    policy["requireTrustedPublisher"] = true;
    policy["sandboxMode"] = true;
    policy["maxMemoryUsage"] = 100 * 1024 * 1024;  // 100MB
    policy["maxCpuUsage"] = 80.0;                  // 80%
    policy["allowNetworkAccess"] = false;
    policy["allowFileSystemAccess"] = false;
    policy["allowRegistryAccess"] = false;
    policy["maxFileSize"] = 50 * 1024 * 1024;  // 50MB

    return policy;
}

void SecurityPolicyEngine::set_policy_config(const QJsonObject& config) {
    m_policy_config = config;
    qCDebug(securityPolicyLog) << "Policy configuration updated";
    emit policy_updated();
}

QJsonObject SecurityPolicyEngine::get_policy_config() const {
    return m_policy_config;
}

bool SecurityPolicyEngine::evaluate_rule(const SecurityPolicyRule& rule,
                                         const std::filesystem::path& file_path,
                                         const QJsonObject& context) const {
    return evaluate_condition(rule.condition, file_path, context);
}

bool SecurityPolicyEngine::evaluate_condition(
    const std::string& condition, const std::filesystem::path& file_path,
    const QJsonObject& context) const {
    // This is a simplified condition evaluator
    // In a real implementation, you would have a proper expression parser

    Q_UNUSED(context);

    if (condition == "file_exists") {
        return std::filesystem::exists(file_path);
    } else if (condition == "file_size_ok") {
        try {
            auto size = std::filesystem::file_size(file_path);
            auto max_size =
                m_policy_config["maxFileSize"].toInt(50 * 1024 * 1024);
            return size <= static_cast<size_t>(max_size);
        } catch (...) {
            return false;
        }
    } else if (condition == "valid_extension") {
        std::string ext = file_path.extension().string();
        return ext == ".dll" || ext == ".so" || ext == ".dylib" ||
               ext == ".qtplugin";
    }

    // Default: condition passes
    return true;
}

SecurityValidationResult SecurityPolicyEngine::create_policy_result(
    bool is_valid, const std::vector<std::string>& errors,
    const std::vector<std::string>& warnings) const {
    SecurityValidationResult result;
    result.is_valid = is_valid;
    result.errors = errors;
    result.warnings = warnings;
    result.validated_level =
        is_valid ? SecurityLevel::Standard : SecurityLevel::None;
    return result;
}

void SecurityPolicyEngine::initialize_default_rules() {
    // Add some default security rules
    SecurityPolicyRule file_exists_rule;
    file_exists_rule.name = "file_must_exist";
    file_exists_rule.condition = "file_exists";
    file_exists_rule.action = "deny";
    file_exists_rule.enabled = true;
    file_exists_rule.priority = 100;
    m_rules.push_back(file_exists_rule);

    SecurityPolicyRule file_size_rule;
    file_size_rule.name = "file_size_limit";
    file_size_rule.condition = "file_size_ok";
    file_size_rule.action = "deny";
    file_size_rule.enabled = true;
    file_size_rule.priority = 90;
    m_rules.push_back(file_size_rule);

    SecurityPolicyRule extension_rule;
    extension_rule.name = "valid_extension";
    extension_rule.condition = "valid_extension";
    extension_rule.action = "deny";
    extension_rule.enabled = true;
    extension_rule.priority = 80;
    m_rules.push_back(extension_rule);
}

}  // namespace qtplugin
