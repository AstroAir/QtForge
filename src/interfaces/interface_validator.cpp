/**
 * @file interface_validator.cpp
 * @brief Implementation of interface validation utilities
 */

#include "qtplugin/interfaces/interface_validator.hpp"
#include <QRegularExpression>
#include <algorithm>

namespace qtplugin {

expected<void, PluginError> InterfaceValidator::register_interface(
    const InterfaceMetadata& metadata) {
    
    // Validate interface ID format
    if (!is_valid_interface_id(metadata.interface_id)) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                               QString("Invalid interface ID format: %1")
                               .arg(metadata.interface_id).toStdString());
    }
    
    // Check for duplicate registration
    if (m_interfaces.find(metadata.interface_id) != m_interfaces.end()) {
        return make_error<void>(PluginErrorCode::AlreadyExists,
                               QString("Interface already registered: %1")
                               .arg(metadata.interface_id).toStdString());
    }
    
    m_interfaces[metadata.interface_id] = metadata;
    return make_success();
}

InterfaceValidationResult InterfaceValidator::validate_interface(
    const QString& interface_id) const {
    
    InterfaceValidationResult result;
    
    auto it = m_interfaces.find(interface_id);
    if (it == m_interfaces.end()) {
        result.add_error(InterfaceValidationError::UnknownDependency,
                        QString("Interface not registered: %1").arg(interface_id));
        return result;
    }
    
    const auto& metadata = it->second;
    
    // Check if deprecated
    if (metadata.deprecated) {
        result.add_warning(QString("Interface %1 is deprecated").arg(interface_id));
        if (!metadata.replacement.isEmpty()) {
            result.add_suggestion(QString("Consider upgrading to %1")
                                 .arg(metadata.replacement));
        }
    }
    
    // Validate dependencies
    for (const auto& dep : metadata.dependencies) {
        if (m_interfaces.find(dep) == m_interfaces.end()) {
            result.add_error(InterfaceValidationError::UnknownDependency,
                            QString("Unknown dependency: %1").arg(dep));
        }
    }
    
    return result;
}

InterfaceValidationResult InterfaceValidator::validate_all_interfaces() const {
    InterfaceValidationResult overall_result;
    
    // Validate each interface individually
    for (const auto& [interface_id, metadata] : m_interfaces) {
        auto result = validate_interface(interface_id);
        
        // Merge results
        if (!result.is_valid) {
            overall_result.is_valid = false;
        }
        overall_result.errors.insert(overall_result.errors.end(),
                                   result.errors.begin(), result.errors.end());
        overall_result.warnings.insert(overall_result.warnings.end(),
                                      result.warnings.begin(), result.warnings.end());
        overall_result.suggestions.insert(overall_result.suggestions.end(),
                                         result.suggestions.begin(), result.suggestions.end());
    }
    
    // Check for interface conflicts
    auto conflicts = find_interface_conflicts();
    for (const auto& [id1, id2] : conflicts) {
        overall_result.add_error(InterfaceValidationError::DuplicateInterfaceId,
                                QString("Interface conflict: %1 vs %2").arg(id1, id2));
    }
    
    // Check for circular dependencies
    auto circular_deps = find_circular_dependencies();
    for (const auto& cycle : circular_deps) {
        overall_result.add_error(InterfaceValidationError::CircularDependency,
                                QString("Circular dependency: %1").arg(cycle.join(" -> ")));
    }
    
    return overall_result;
}

std::vector<std::pair<QString, QString>> InterfaceValidator::find_interface_conflicts() const {
    std::vector<std::pair<QString, QString>> conflicts;
    
    // Group interfaces by base name
    std::unordered_map<QString, std::vector<QString>> base_name_groups;
    
    for (const auto& [interface_id, metadata] : m_interfaces) {
        auto [base_name, version] = parse_interface_id(interface_id);
        base_name_groups[base_name].push_back(interface_id);
    }
    
    // Check for version conflicts within same base name
    for (const auto& [base_name, interface_ids] : base_name_groups) {
        if (interface_ids.size() > 1) {
            // Multiple versions of same interface - check for major version conflicts
            for (size_t i = 0; i < interface_ids.size(); ++i) {
                for (size_t j = i + 1; j < interface_ids.size(); ++j) {
                    auto [name1, ver1] = parse_interface_id(interface_ids[i]);
                    auto [name2, ver2] = parse_interface_id(interface_ids[j]);
                    
                    // Major version differences indicate potential conflicts
                    if (ver1.major() != ver2.major()) {
                        conflicts.emplace_back(interface_ids[i], interface_ids[j]);
                    }
                }
            }
        }
    }
    
    return conflicts;
}

std::vector<QStringList> InterfaceValidator::find_circular_dependencies() const {
    std::vector<QStringList> circular_deps;
    std::unordered_set<QString> visited;
    
    for (const auto& [interface_id, metadata] : m_interfaces) {
        if (visited.find(interface_id) == visited.end()) {
            QStringList path;
            std::unordered_set<QString> current_visited;
            
            if (has_circular_dependency(interface_id, current_visited, path)) {
                circular_deps.push_back(path);
            }
            
            // Mark all interfaces in this path as visited
            for (const auto& id : path) {
                visited.insert(id);
            }
        }
    }
    
    return circular_deps;
}

expected<InterfaceMetadata, PluginError> InterfaceValidator::get_interface_metadata(
    const QString& interface_id) const {
    
    auto it = m_interfaces.find(interface_id);
    if (it == m_interfaces.end()) {
        return make_error<InterfaceMetadata>(PluginErrorCode::NotFound,
                                           QString("Interface not found: %1")
                                           .arg(interface_id).toStdString());
    }
    
    return it->second;
}

bool InterfaceValidator::is_interface_compatible(const QString& interface_id,
                                               const Version& required_version) const {
    auto it = m_interfaces.find(interface_id);
    if (it == m_interfaces.end()) {
        return false;
    }
    
    const auto& metadata = it->second;
    
    // Check semantic versioning compatibility
    // Same major version, minor version >= required
    return metadata.version.major() == required_version.major() &&
           metadata.version >= required_version;
}

std::vector<std::pair<QString, QString>> InterfaceValidator::get_deprecated_interfaces() const {
    std::vector<std::pair<QString, QString>> deprecated;
    
    for (const auto& [interface_id, metadata] : m_interfaces) {
        if (metadata.deprecated) {
            deprecated.emplace_back(interface_id, metadata.replacement);
        }
    }
    
    return deprecated;
}

QStringList InterfaceValidator::suggest_interface_upgrades(
    const QString& current_interface_id) const {
    
    QStringList suggestions;
    
    auto [base_name, current_version] = parse_interface_id(current_interface_id);
    
    // Find newer versions of the same interface
    for (const auto& [interface_id, metadata] : m_interfaces) {
        auto [other_base_name, other_version] = parse_interface_id(interface_id);
        
        if (base_name == other_base_name && other_version > current_version) {
            suggestions.append(interface_id);
        }
    }
    
    // Sort by version (newest first)
    std::sort(suggestions.begin(), suggestions.end(),
              [this](const QString& a, const QString& b) {
                  auto [name_a, ver_a] = parse_interface_id(a);
                  auto [name_b, ver_b] = parse_interface_id(b);
                  return ver_a > ver_b;
              });
    
    return suggestions;
}

void InterfaceValidator::clear() {
    m_interfaces.clear();
}

QStringList InterfaceValidator::get_registered_interfaces() const {
    QStringList interfaces;
    for (const auto& [interface_id, metadata] : m_interfaces) {
        interfaces.append(interface_id);
    }
    return interfaces;
}

std::pair<QString, Version> InterfaceValidator::parse_interface_id(
    const QString& interface_id) const {
    
    // Expected format: "namespace.InterfaceName/major.minor.patch" or "namespace.InterfaceName/major.minor"
    QRegularExpression regex(R"(^(.+)/(\d+)\.(\d+)(?:\.(\d+))?$)");
    auto match = regex.match(interface_id);
    
    if (match.hasMatch()) {
        QString base_name = match.captured(1);
        int major = match.captured(2).toInt();
        int minor = match.captured(3).toInt();
        int patch = match.captured(4).isEmpty() ? 0 : match.captured(4).toInt();
        
        return {base_name, Version{major, minor, patch}};
    }
    
    // Fallback: treat entire string as base name with version 0.0.0
    return {interface_id, Version{0, 0, 0}};
}

bool InterfaceValidator::has_circular_dependency(const QString& interface_id,
                                               std::unordered_set<QString>& visited,
                                               QStringList& path) const {
    
    if (visited.find(interface_id) != visited.end()) {
        // Found a cycle
        path.append(interface_id);
        return true;
    }
    
    auto it = m_interfaces.find(interface_id);
    if (it == m_interfaces.end()) {
        return false;
    }
    
    visited.insert(interface_id);
    path.append(interface_id);
    
    const auto& metadata = it->second;
    for (const auto& dep : metadata.dependencies) {
        if (has_circular_dependency(dep, visited, path)) {
            return true;
        }
    }
    
    // Backtrack
    visited.erase(interface_id);
    path.removeLast();
    return false;
}

bool InterfaceValidator::is_valid_interface_id(const QString& interface_id) const {
    // Expected format: "namespace.InterfaceName/version"
    QRegularExpression regex(R"(^[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*\/\d+\.\d+(\.\d+)?$)");
    return regex.match(interface_id).hasMatch();
}

InterfaceValidator& global_interface_validator() {
    static InterfaceValidator instance;
    return instance;
}

}  // namespace qtplugin
