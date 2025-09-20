/**
 * @file workflow_validator.hpp
 * @brief Comprehensive workflow validation and verification system
 * @version 3.1.0
 */

#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "../utils/error_handling.hpp"
#include "workflow.hpp"

namespace qtplugin::workflow::validation {

/**
 * @brief Validation severity levels
 */
enum class ValidationSeverity {
    Info = 0,
    Warning = 1,
    Error = 2,
    Critical = 3
};

/**
 * @brief Validation rule types
 */
enum class ValidationRuleType {
    Dependency = 0,         // Dependency validation
    Plugin = 1,             // Plugin availability
    Configuration = 2,      // Configuration correctness
    Resource = 3,           // Resource availability
    Security = 4,           // Security constraints
    Performance = 5,        // Performance considerations
    Compatibility = 6,      // Version compatibility
    Business = 7            // Business logic validation
};

/**
 * @brief Validation issue information
 */
struct ValidationIssue {
    QString issue_id;
    QString workflow_id;
    QString step_id;
    QString plugin_id;
    
    // Issue classification
    ValidationRuleType rule_type;
    ValidationSeverity severity;
    QString rule_name;
    
    // Issue details
    QString message;
    QString description;
    QString recommendation;
    QJsonObject issue_data;
    
    // Location information
    QString file_path;
    int line_number{-1};
    QString context;
    
    // Resolution information
    bool auto_fixable{false};
    QString fix_suggestion;
    QJsonObject fix_data;
    
    // Timing
    QDateTime detected_time;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<ValidationIssue, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Validation result
 */
struct ValidationResult {
    QString validation_id;
    QString workflow_id;
    bool is_valid{true};
    
    // Issue summary
    int total_issues{0};
    int critical_issues{0};
    int error_issues{0};
    int warning_issues{0};
    int info_issues{0};
    
    // Issues by category
    std::vector<ValidationIssue> issues;
    std::unordered_map<ValidationRuleType, std::vector<ValidationIssue>> issues_by_type;
    
    // Timing information
    QDateTime validation_start_time;
    QDateTime validation_end_time;
    std::chrono::milliseconds validation_duration{0};
    
    // Summary
    QString summary;
    bool execution_recommended{true};
    QString execution_recommendation;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<ValidationResult, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Validation rule definition
 */
struct ValidationRule {
    QString rule_id;
    QString rule_name;
    ValidationRuleType rule_type;
    ValidationSeverity default_severity;
    
    // Rule configuration
    bool enabled{true};
    QJsonObject rule_config;
    
    // Rule function
    std::function<std::vector<ValidationIssue>(const Workflow&, const QJsonObject&)> validation_func;
    
    // Conditions
    std::function<bool(const Workflow&)> should_apply;
    
    // Serialization
    QJsonObject to_json() const;
    static qtplugin::expected<ValidationRule, PluginError> from_json(const QJsonObject& json);
};

/**
 * @brief Dependency validator
 */
class DependencyValidator : public QObject {
    Q_OBJECT

public:
    explicit DependencyValidator(QObject* parent = nullptr);
    
    // Dependency validation
    std::vector<ValidationIssue> validate_dependencies(const Workflow& workflow) const;
    std::vector<ValidationIssue> validate_step_dependencies(const WorkflowStep& step, const Workflow& workflow) const;
    std::vector<ValidationIssue> validate_plugin_dependencies(const QString& plugin_id) const;
    
    // Circular dependency detection
    bool has_circular_dependencies(const Workflow& workflow) const;
    std::vector<QString> find_dependency_cycle(const Workflow& workflow) const;
    
    // Dependency graph analysis
    std::vector<QString> get_execution_order(const Workflow& workflow) const;
    std::unordered_map<QString, std::vector<QString>> build_dependency_graph(const Workflow& workflow) const;

private:
    // Helper methods
    bool has_cycle_dfs(const std::unordered_map<QString, std::vector<QString>>& graph, 
                       const QString& node, 
                       std::unordered_set<QString>& visited, 
                       std::unordered_set<QString>& rec_stack,
                       std::vector<QString>& cycle_path) const;
    
    std::vector<QString> topological_sort(const std::unordered_map<QString, std::vector<QString>>& graph) const;
};

/**
 * @brief Plugin validator
 */
class PluginValidator : public QObject {
    Q_OBJECT

public:
    explicit PluginValidator(QObject* parent = nullptr);
    
    // Plugin validation
    std::vector<ValidationIssue> validate_plugin_availability(const Workflow& workflow) const;
    std::vector<ValidationIssue> validate_plugin_compatibility(const QString& plugin_id, const QJsonObject& config) const;
    std::vector<ValidationIssue> validate_plugin_methods(const QString& plugin_id, const std::vector<QString>& required_methods) const;
    
    // Plugin information
    bool is_plugin_available(const QString& plugin_id) const;
    QString get_plugin_version(const QString& plugin_id) const;
    std::vector<QString> get_plugin_methods(const QString& plugin_id) const;
    QJsonObject get_plugin_metadata(const QString& plugin_id) const;

private:
    // Helper methods
    bool check_version_compatibility(const QString& required_version, const QString& available_version) const;
    bool check_method_signature(const QString& plugin_id, const QString& method_name, const QJsonObject& expected_signature) const;
};

/**
 * @brief Configuration validator
 */
class ConfigurationValidator : public QObject {
    Q_OBJECT

public:
    explicit ConfigurationValidator(QObject* parent = nullptr);
    
    // Configuration validation
    std::vector<ValidationIssue> validate_workflow_configuration(const Workflow& workflow) const;
    std::vector<ValidationIssue> validate_step_configuration(const WorkflowStep& step) const;
    std::vector<ValidationIssue> validate_plugin_configuration(const QString& plugin_id, const QJsonObject& config) const;
    
    // Schema validation
    std::vector<ValidationIssue> validate_against_schema(const QJsonObject& data, const QJsonObject& schema, const QString& context) const;
    
    // Configuration completeness
    std::vector<ValidationIssue> check_required_parameters(const QJsonObject& config, const std::vector<QString>& required_params, const QString& context) const;
    std::vector<ValidationIssue> check_parameter_types(const QJsonObject& config, const std::unordered_map<QString, QString>& expected_types, const QString& context) const;

private:
    // Schema validation helpers
    bool validate_json_type(const QJsonValue& value, const QString& expected_type) const;
    std::vector<ValidationIssue> validate_object_properties(const QJsonObject& object, const QJsonObject& schema, const QString& context) const;
};

/**
 * @brief Resource validator
 */
class ResourceValidator : public QObject {
    Q_OBJECT

public:
    explicit ResourceValidator(QObject* parent = nullptr);
    
    // Resource validation
    std::vector<ValidationIssue> validate_resource_availability(const Workflow& workflow) const;
    std::vector<ValidationIssue> validate_file_resources(const std::vector<QString>& file_paths) const;
    std::vector<ValidationIssue> validate_network_resources(const std::vector<QString>& urls) const;
    std::vector<ValidationIssue> validate_database_resources(const std::vector<QString>& connection_strings) const;
    
    // Resource capacity
    std::vector<ValidationIssue> check_memory_requirements(const Workflow& workflow) const;
    std::vector<ValidationIssue> check_disk_space_requirements(const Workflow& workflow) const;
    std::vector<ValidationIssue> check_network_bandwidth_requirements(const Workflow& workflow) const;

private:
    // Resource checking helpers
    bool is_file_accessible(const QString& file_path) const;
    bool is_url_reachable(const QString& url) const;
    bool is_database_accessible(const QString& connection_string) const;
    
    qint64 estimate_memory_usage(const Workflow& workflow) const;
    qint64 estimate_disk_usage(const Workflow& workflow) const;
    qint64 estimate_bandwidth_usage(const Workflow& workflow) const;
};

/**
 * @brief Comprehensive workflow validator
 */
class WorkflowValidator : public QObject {
    Q_OBJECT

public:
    explicit WorkflowValidator(QObject* parent = nullptr);
    ~WorkflowValidator() override;
    
    // Service lifecycle
    qtplugin::expected<void, PluginError> initialize();
    void shutdown();
    bool is_initialized() const { return m_initialized; }
    
    // Validation execution
    qtplugin::expected<ValidationResult, PluginError> validate_workflow(const Workflow& workflow) const;
    qtplugin::expected<ValidationResult, PluginError> validate_workflow_with_config(const Workflow& workflow, const QJsonObject& validation_config) const;
    
    // Quick validation
    qtplugin::expected<bool, PluginError> is_workflow_valid(const Workflow& workflow) const;
    qtplugin::expected<std::vector<ValidationIssue>, PluginError> get_critical_issues(const Workflow& workflow) const;
    
    // Rule management
    void register_validation_rule(const ValidationRule& rule);
    void unregister_validation_rule(const QString& rule_id);
    void enable_validation_rule(const QString& rule_id, bool enabled = true);
    
    std::vector<ValidationRule> get_validation_rules() const;
    std::optional<ValidationRule> get_validation_rule(const QString& rule_id) const;
    
    // Configuration
    void set_validation_config(const QJsonObject& config);
    QJsonObject get_validation_config() const;
    
    // Auto-fix capabilities
    qtplugin::expected<Workflow, PluginError> auto_fix_workflow(const Workflow& workflow, const ValidationResult& validation_result) const;
    std::vector<ValidationIssue> get_auto_fixable_issues(const ValidationResult& validation_result) const;
    
    // Statistics
    size_t total_validations_performed() const { return m_total_validations; }
    size_t successful_validations() const { return m_successful_validations; }
    size_t failed_validations() const { return m_failed_validations; }
    
    // Singleton access
    static WorkflowValidator& instance();

signals:
    void validation_started(const QString& workflow_id);
    void validation_completed(const QString& validation_id, bool is_valid);
    void validation_rule_registered(const QString& rule_id);
    void validation_rule_unregistered(const QString& rule_id);
    void critical_issue_detected(const QString& workflow_id, const ValidationIssue& issue);

private:
    bool m_initialized{false};
    
    // Specialized validators
    std::unique_ptr<DependencyValidator> m_dependency_validator;
    std::unique_ptr<PluginValidator> m_plugin_validator;
    std::unique_ptr<ConfigurationValidator> m_configuration_validator;
    std::unique_ptr<ResourceValidator> m_resource_validator;
    
    // Validation rules
    std::unordered_map<QString, ValidationRule> m_validation_rules;
    QJsonObject m_validation_config;
    
    // Statistics
    mutable size_t m_total_validations{0};
    mutable size_t m_successful_validations{0};
    mutable size_t m_failed_validations{0};
    
    // Helper methods
    QString generate_validation_id() const;
    QString generate_issue_id() const;
    
    ValidationResult create_validation_result(const QString& workflow_id, const std::vector<ValidationIssue>& issues) const;
    void categorize_issues(ValidationResult& result) const;
    void generate_validation_summary(ValidationResult& result) const;
    
    // Built-in validation rules
    void initialize_built_in_rules();
    std::vector<ValidationIssue> validate_basic_workflow_structure(const Workflow& workflow) const;
    std::vector<ValidationIssue> validate_workflow_metadata(const Workflow& workflow) const;
};

} // namespace qtplugin::workflow::validation
