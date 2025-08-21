/**
 * @file security_policy_engine.hpp
 * @brief Security policy engine interface and implementation
 * @version 3.0.0
 */

#pragma once

#include "../../utils/error_handling.hpp"
#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include <QObject>
#include <QJsonObject>

namespace qtplugin {

// Forward declarations
struct SecurityValidationResult;
enum class SecurityLevel;

/**
 * @brief Security policy rule
 */
struct SecurityPolicyRule {
    std::string name;
    std::string condition;
    std::string action;
    bool enabled = true;
    int priority = 0;
};

/**
 * @brief Interface for security policy evaluation
 * 
 * The security policy engine handles policy evaluation, rule enforcement,
 * and security decision making based on configurable policies.
 */
class ISecurityPolicyEngine {
public:
    virtual ~ISecurityPolicyEngine() = default;
    
    /**
     * @brief Evaluate security policy for a plugin
     * @param file_path Path to plugin file
     * @param context Additional context for evaluation
     * @return Policy evaluation result
     */
    virtual SecurityValidationResult evaluate_policy(const std::filesystem::path& file_path,
                                                    const QJsonObject& context = {}) const = 0;
    
    /**
     * @brief Load security policy from file
     * @param policy_file Path to policy file
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> 
    load_policy(const std::filesystem::path& policy_file) = 0;
    
    /**
     * @brief Save security policy to file
     * @param policy_file Path to policy file
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> 
    save_policy(const std::filesystem::path& policy_file) const = 0;
    
    /**
     * @brief Add security policy rule
     * @param rule Policy rule to add
     * @return Success or error
     */
    virtual qtplugin::expected<void, PluginError> 
    add_rule(const SecurityPolicyRule& rule) = 0;
    
    /**
     * @brief Remove security policy rule
     * @param rule_name Name of rule to remove
     */
    virtual void remove_rule(const std::string& rule_name) = 0;
    
    /**
     * @brief Get all policy rules
     * @return Vector of policy rules
     */
    virtual std::vector<SecurityPolicyRule> get_rules() const = 0;
    
    /**
     * @brief Get default security policy
     * @return Default policy as JSON object
     */
    virtual QJsonObject get_default_policy() const = 0;
    
    /**
     * @brief Set policy configuration
     * @param config Policy configuration
     */
    virtual void set_policy_config(const QJsonObject& config) = 0;
    
    /**
     * @brief Get current policy configuration
     * @return Current policy configuration
     */
    virtual QJsonObject get_policy_config() const = 0;
};

/**
 * @brief Security policy engine implementation
 * 
 * Evaluates security policies and enforces rules based on configurable
 * policy definitions and plugin context.
 */
class SecurityPolicyEngine : public QObject, public ISecurityPolicyEngine {
    Q_OBJECT
    
public:
    explicit SecurityPolicyEngine(QObject* parent = nullptr);
    ~SecurityPolicyEngine() override;
    
    // ISecurityPolicyEngine interface
    SecurityValidationResult evaluate_policy(const std::filesystem::path& file_path,
                                            const QJsonObject& context = {}) const override;
    qtplugin::expected<void, PluginError> 
    load_policy(const std::filesystem::path& policy_file) override;
    qtplugin::expected<void, PluginError> 
    save_policy(const std::filesystem::path& policy_file) const override;
    qtplugin::expected<void, PluginError> 
    add_rule(const SecurityPolicyRule& rule) override;
    void remove_rule(const std::string& rule_name) override;
    std::vector<SecurityPolicyRule> get_rules() const override;
    QJsonObject get_default_policy() const override;
    void set_policy_config(const QJsonObject& config) override;
    QJsonObject get_policy_config() const override;

signals:
    /**
     * @brief Emitted when policy is evaluated
     * @param file_path Path that was evaluated
     * @param result Evaluation result
     */
    void policy_evaluated(const QString& file_path, bool result);
    
    /**
     * @brief Emitted when policy rule is violated
     * @param rule_name Name of violated rule
     * @param file_path Path where violation occurred
     */
    void policy_violation(const QString& rule_name, const QString& file_path);
    
    /**
     * @brief Emitted when policy is updated
     */
    void policy_updated();

private:
    QJsonObject m_policy_config;
    std::vector<SecurityPolicyRule> m_rules;
    
    // Helper methods
    bool evaluate_rule(const SecurityPolicyRule& rule, 
                      const std::filesystem::path& file_path,
                      const QJsonObject& context) const;
    bool evaluate_condition(const std::string& condition,
                           const std::filesystem::path& file_path,
                           const QJsonObject& context) const;
    SecurityValidationResult create_policy_result(bool is_valid,
                                                 const std::vector<std::string>& errors = {},
                                                 const std::vector<std::string>& warnings = {}) const;
    void initialize_default_rules();
};

} // namespace qtplugin
