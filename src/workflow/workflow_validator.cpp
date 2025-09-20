/**
 * @file workflow_validator.cpp
 * @brief Implementation of comprehensive workflow validation system
 * @version 3.1.0
 */

#include "qtplugin/workflow/workflow_validator.hpp"
#include <QLoggingCategory>
#include <QUuid>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <algorithm>
#include <queue>

Q_LOGGING_CATEGORY(workflowValidatorLog, "qtplugin.workflow.validator")

namespace qtplugin::workflow::validation {

// === ValidationIssue Implementation ===

QJsonObject ValidationIssue::to_json() const {
    QJsonObject json;
    json["issue_id"] = issue_id;
    json["workflow_id"] = workflow_id;
    json["step_id"] = step_id;
    json["plugin_id"] = plugin_id;
    json["rule_type"] = static_cast<int>(rule_type);
    json["severity"] = static_cast<int>(severity);
    json["rule_name"] = rule_name;
    json["message"] = message;
    json["description"] = description;
    json["recommendation"] = recommendation;
    json["issue_data"] = issue_data;
    json["file_path"] = file_path;
    json["line_number"] = line_number;
    json["context"] = context;
    json["auto_fixable"] = auto_fixable;
    json["fix_suggestion"] = fix_suggestion;
    json["fix_data"] = fix_data;
    json["detected_time"] = detected_time.toString(Qt::ISODate);
    
    return json;
}

qtplugin::expected<ValidationIssue, PluginError> ValidationIssue::from_json(const QJsonObject& json) {
    ValidationIssue issue;
    
    if (json.contains("issue_id") && json["issue_id"].isString()) {
        issue.issue_id = json["issue_id"].toString();
    }
    
    if (json.contains("workflow_id") && json["workflow_id"].isString()) {
        issue.workflow_id = json["workflow_id"].toString();
    }
    
    if (json.contains("step_id") && json["step_id"].isString()) {
        issue.step_id = json["step_id"].toString();
    }
    
    if (json.contains("plugin_id") && json["plugin_id"].isString()) {
        issue.plugin_id = json["plugin_id"].toString();
    }
    
    if (json.contains("rule_type") && json["rule_type"].isDouble()) {
        issue.rule_type = static_cast<ValidationRuleType>(json["rule_type"].toInt());
    }
    
    if (json.contains("severity") && json["severity"].isDouble()) {
        issue.severity = static_cast<ValidationSeverity>(json["severity"].toInt());
    }
    
    if (json.contains("rule_name") && json["rule_name"].isString()) {
        issue.rule_name = json["rule_name"].toString();
    }
    
    if (json.contains("message") && json["message"].isString()) {
        issue.message = json["message"].toString();
    }
    
    if (json.contains("description") && json["description"].isString()) {
        issue.description = json["description"].toString();
    }
    
    if (json.contains("recommendation") && json["recommendation"].isString()) {
        issue.recommendation = json["recommendation"].toString();
    }
    
    if (json.contains("issue_data") && json["issue_data"].isObject()) {
        issue.issue_data = json["issue_data"].toObject();
    }
    
    if (json.contains("file_path") && json["file_path"].isString()) {
        issue.file_path = json["file_path"].toString();
    }
    
    if (json.contains("line_number") && json["line_number"].isDouble()) {
        issue.line_number = json["line_number"].toInt();
    }
    
    if (json.contains("context") && json["context"].isString()) {
        issue.context = json["context"].toString();
    }
    
    if (json.contains("auto_fixable") && json["auto_fixable"].isBool()) {
        issue.auto_fixable = json["auto_fixable"].toBool();
    }
    
    if (json.contains("fix_suggestion") && json["fix_suggestion"].isString()) {
        issue.fix_suggestion = json["fix_suggestion"].toString();
    }
    
    if (json.contains("fix_data") && json["fix_data"].isObject()) {
        issue.fix_data = json["fix_data"].toObject();
    }
    
    if (json.contains("detected_time") && json["detected_time"].isString()) {
        issue.detected_time = QDateTime::fromString(json["detected_time"].toString(), Qt::ISODate);
    }
    
    return issue;
}

// === ValidationResult Implementation ===

QJsonObject ValidationResult::to_json() const {
    QJsonObject json;
    json["validation_id"] = validation_id;
    json["workflow_id"] = workflow_id;
    json["is_valid"] = is_valid;
    json["total_issues"] = total_issues;
    json["critical_issues"] = critical_issues;
    json["error_issues"] = error_issues;
    json["warning_issues"] = warning_issues;
    json["info_issues"] = info_issues;
    json["validation_start_time"] = validation_start_time.toString(Qt::ISODate);
    json["validation_end_time"] = validation_end_time.toString(Qt::ISODate);
    json["validation_duration_ms"] = static_cast<int>(validation_duration.count());
    json["summary"] = summary;
    json["execution_recommended"] = execution_recommended;
    json["execution_recommendation"] = execution_recommendation;
    
    QJsonArray issues_array;
    for (const ValidationIssue& issue : issues) {
        issues_array.append(issue.to_json());
    }
    json["issues"] = issues_array;
    
    return json;
}

qtplugin::expected<ValidationResult, PluginError> ValidationResult::from_json(const QJsonObject& json) {
    ValidationResult result;
    
    if (json.contains("validation_id") && json["validation_id"].isString()) {
        result.validation_id = json["validation_id"].toString();
    }
    
    if (json.contains("workflow_id") && json["workflow_id"].isString()) {
        result.workflow_id = json["workflow_id"].toString();
    }
    
    if (json.contains("is_valid") && json["is_valid"].isBool()) {
        result.is_valid = json["is_valid"].toBool();
    }
    
    if (json.contains("total_issues") && json["total_issues"].isDouble()) {
        result.total_issues = json["total_issues"].toInt();
    }
    
    if (json.contains("critical_issues") && json["critical_issues"].isDouble()) {
        result.critical_issues = json["critical_issues"].toInt();
    }
    
    if (json.contains("error_issues") && json["error_issues"].isDouble()) {
        result.error_issues = json["error_issues"].toInt();
    }
    
    if (json.contains("warning_issues") && json["warning_issues"].isDouble()) {
        result.warning_issues = json["warning_issues"].toInt();
    }
    
    if (json.contains("info_issues") && json["info_issues"].isDouble()) {
        result.info_issues = json["info_issues"].toInt();
    }
    
    if (json.contains("validation_start_time") && json["validation_start_time"].isString()) {
        result.validation_start_time = QDateTime::fromString(json["validation_start_time"].toString(), Qt::ISODate);
    }
    
    if (json.contains("validation_end_time") && json["validation_end_time"].isString()) {
        result.validation_end_time = QDateTime::fromString(json["validation_end_time"].toString(), Qt::ISODate);
    }
    
    if (json.contains("validation_duration_ms") && json["validation_duration_ms"].isDouble()) {
        result.validation_duration = std::chrono::milliseconds(json["validation_duration_ms"].toInt());
    }
    
    if (json.contains("summary") && json["summary"].isString()) {
        result.summary = json["summary"].toString();
    }
    
    if (json.contains("execution_recommended") && json["execution_recommended"].isBool()) {
        result.execution_recommended = json["execution_recommended"].toBool();
    }
    
    if (json.contains("execution_recommendation") && json["execution_recommendation"].isString()) {
        result.execution_recommendation = json["execution_recommendation"].toString();
    }
    
    if (json.contains("issues") && json["issues"].isArray()) {
        QJsonArray issues_array = json["issues"].toArray();
        for (const QJsonValue& value : issues_array) {
            if (value.isObject()) {
                auto issue_result = ValidationIssue::from_json(value.toObject());
                if (issue_result) {
                    result.issues.push_back(issue_result.value());
                }
            }
        }
    }
    
    return result;
}

// === DependencyValidator Implementation ===

DependencyValidator::DependencyValidator(QObject* parent)
    : QObject(parent) {
    
    qCDebug(workflowValidatorLog) << "Created dependency validator";
}

std::vector<ValidationIssue> DependencyValidator::validate_dependencies(const Workflow& workflow) const {
    std::vector<ValidationIssue> issues;
    
    // Check for circular dependencies
    if (has_circular_dependencies(workflow)) {
        ValidationIssue issue;
        issue.issue_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        issue.workflow_id = workflow.workflow_id();
        issue.rule_type = ValidationRuleType::Dependency;
        issue.severity = ValidationSeverity::Critical;
        issue.rule_name = "circular_dependency_check";
        issue.message = "Circular dependency detected in workflow";
        issue.description = "The workflow contains circular dependencies that would prevent execution";
        issue.recommendation = "Review and remove circular dependencies between workflow steps";
        issue.detected_time = QDateTime::currentDateTime();
        
        auto cycle = find_dependency_cycle(workflow);
        if (!cycle.empty()) {
            QJsonArray cycle_array;
            for (const QString& step : cycle) {
                cycle_array.append(step);
            }
            issue.issue_data["dependency_cycle"] = cycle_array;
            issue.description += QString(". Cycle: %1").arg(cycle.join(" -> "));
        }
        
        issues.push_back(issue);
    }
    
    // Validate individual step dependencies
    auto steps = workflow.get_steps();
    for (const auto& step : steps) {
        auto step_issues = validate_step_dependencies(step, workflow);
        issues.insert(issues.end(), step_issues.begin(), step_issues.end());
    }
    
    return issues;
}

std::vector<ValidationIssue> DependencyValidator::validate_step_dependencies(const WorkflowStep& step, const Workflow& workflow) const {
    std::vector<ValidationIssue> issues;
    
    // Check if all dependencies exist
    auto dependencies = step.get_dependencies();
    auto all_steps = workflow.get_steps();
    
    std::unordered_set<QString> available_steps;
    for (const auto& available_step : all_steps) {
        available_steps.insert(available_step.step_id());
    }
    
    for (const QString& dependency : dependencies) {
        if (available_steps.find(dependency) == available_steps.end()) {
            ValidationIssue issue;
            issue.issue_id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            issue.workflow_id = workflow.workflow_id();
            issue.step_id = step.step_id();
            issue.rule_type = ValidationRuleType::Dependency;
            issue.severity = ValidationSeverity::Error;
            issue.rule_name = "missing_dependency_check";
            issue.message = QString("Missing dependency: %1").arg(dependency);
            issue.description = QString("Step '%1' depends on step '%2' which does not exist in the workflow").arg(step.step_id(), dependency);
            issue.recommendation = QString("Add step '%1' to the workflow or remove the dependency").arg(dependency);
            issue.detected_time = QDateTime::currentDateTime();
            
            QJsonObject issue_data;
            issue_data["missing_dependency"] = dependency;
            issue_data["dependent_step"] = step.step_id();
            issue.issue_data = issue_data;
            
            issues.push_back(issue);
        }
    }
    
    return issues;
}

std::vector<ValidationIssue> DependencyValidator::validate_plugin_dependencies(const QString& plugin_id) const {
    std::vector<ValidationIssue> issues;
    
    // This would check plugin-specific dependencies
    // For now, we'll create a placeholder implementation
    Q_UNUSED(plugin_id)
    
    return issues;
}

bool DependencyValidator::has_circular_dependencies(const Workflow& workflow) const {
    auto dependency_graph = build_dependency_graph(workflow);
    
    std::unordered_set<QString> visited;
    std::unordered_set<QString> rec_stack;
    std::vector<QString> cycle_path;
    
    for (const auto& [node, dependencies] : dependency_graph) {
        if (visited.find(node) == visited.end()) {
            if (has_cycle_dfs(dependency_graph, node, visited, rec_stack, cycle_path)) {
                return true;
            }
        }
    }
    
    return false;
}

std::vector<QString> DependencyValidator::find_dependency_cycle(const Workflow& workflow) const {
    auto dependency_graph = build_dependency_graph(workflow);
    
    std::unordered_set<QString> visited;
    std::unordered_set<QString> rec_stack;
    std::vector<QString> cycle_path;
    
    for (const auto& [node, dependencies] : dependency_graph) {
        if (visited.find(node) == visited.end()) {
            if (has_cycle_dfs(dependency_graph, node, visited, rec_stack, cycle_path)) {
                return cycle_path;
            }
        }
    }
    
    return {};
}

std::vector<QString> DependencyValidator::get_execution_order(const Workflow& workflow) const {
    auto dependency_graph = build_dependency_graph(workflow);
    return topological_sort(dependency_graph);
}

std::unordered_map<QString, std::vector<QString>> DependencyValidator::build_dependency_graph(const Workflow& workflow) const {
    std::unordered_map<QString, std::vector<QString>> graph;
    
    auto steps = workflow.get_steps();
    for (const auto& step : steps) {
        graph[step.step_id()] = step.get_dependencies();
    }
    
    return graph;
}

bool DependencyValidator::has_cycle_dfs(const std::unordered_map<QString, std::vector<QString>>& graph, 
                                       const QString& node, 
                                       std::unordered_set<QString>& visited, 
                                       std::unordered_set<QString>& rec_stack,
                                       std::vector<QString>& cycle_path) const {
    visited.insert(node);
    rec_stack.insert(node);
    cycle_path.push_back(node);
    
    auto it = graph.find(node);
    if (it != graph.end()) {
        for (const QString& neighbor : it->second) {
            if (rec_stack.find(neighbor) != rec_stack.end()) {
                // Found cycle
                cycle_path.push_back(neighbor);
                return true;
            }
            
            if (visited.find(neighbor) == visited.end()) {
                if (has_cycle_dfs(graph, neighbor, visited, rec_stack, cycle_path)) {
                    return true;
                }
            }
        }
    }
    
    rec_stack.erase(node);
    cycle_path.pop_back();
    return false;
}

std::vector<QString> DependencyValidator::topological_sort(const std::unordered_map<QString, std::vector<QString>>& graph) const {
    std::vector<QString> result;
    std::unordered_map<QString, int> in_degree;
    
    // Calculate in-degrees
    for (const auto& [node, dependencies] : graph) {
        in_degree[node] = 0;
    }
    
    for (const auto& [node, dependencies] : graph) {
        for (const QString& dep : dependencies) {
            in_degree[dep]++;
        }
    }
    
    // Find nodes with no dependencies
    std::queue<QString> queue;
    for (const auto& [node, degree] : in_degree) {
        if (degree == 0) {
            queue.push(node);
        }
    }
    
    // Process nodes
    while (!queue.empty()) {
        QString current = queue.front();
        queue.pop();
        result.push_back(current);
        
        auto it = graph.find(current);
        if (it != graph.end()) {
            for (const QString& dependent : it->second) {
                in_degree[dependent]--;
                if (in_degree[dependent] == 0) {
                    queue.push(dependent);
                }
            }
        }
    }
    
    return result;
}

} // namespace qtplugin::workflow::validation
