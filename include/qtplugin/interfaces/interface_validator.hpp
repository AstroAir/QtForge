/**
 * @file interface_validator.hpp
 * @brief Interface validation utilities to prevent conflicts and ensure consistency
 * @version 1.0.0
 */

#pragma once

#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "../utils/error_handling.hpp"
#include "../utils/version.hpp"

namespace qtplugin {

/**
 * @brief Interface validation error types
 */
enum class InterfaceValidationError {
    DuplicateInterfaceId,      ///< Multiple interfaces with same ID
    VersionConflict,           ///< Incompatible interface versions
    MissingRequiredMethod,     ///< Required method not implemented
    InvalidMethodSignature,    ///< Method signature doesn't match interface
    InconsistentMetadata,      ///< Interface metadata inconsistency
    CircularDependency,        ///< Circular dependency between interfaces
    UnknownDependency,         ///< Dependency on unknown interface
    InvalidInterfaceId,        ///< Malformed interface ID
    DeprecatedInterface        ///< Using deprecated interface version
};

/**
 * @brief Interface validation result
 */
struct InterfaceValidationResult {
    bool is_valid = true;
    std::vector<InterfaceValidationError> errors;
    std::vector<QString> warnings;
    std::vector<QString> suggestions;
    
    /**
     * @brief Add validation error
     */
    void add_error(InterfaceValidationError error, const QString& message = {}) {
        is_valid = false;
        errors.push_back(error);
        if (!message.isEmpty()) {
            warnings.push_back(QString("Error: %1").arg(message));
        }
    }
    
    /**
     * @brief Add validation warning
     */
    void add_warning(const QString& message) {
        warnings.push_back(QString("Warning: %1").arg(message));
    }
    
    /**
     * @brief Add suggestion
     */
    void add_suggestion(const QString& message) {
        suggestions.push_back(QString("Suggestion: %1").arg(message));
    }
};

/**
 * @brief Interface metadata for validation
 */
struct InterfaceMetadata {
    QString interface_id;           ///< Interface identifier (e.g., "qtplugin.IUIPlugin/3.1")
    Version version;                ///< Interface version
    QString name;                   ///< Human-readable interface name
    QString description;            ///< Interface description
    QStringList required_methods;   ///< Required method signatures
    QStringList optional_methods;   ///< Optional method signatures
    QStringList dependencies;       ///< Interface dependencies
    bool deprecated = false;        ///< Whether interface is deprecated
    QString replacement;            ///< Replacement interface if deprecated
    QJsonObject custom_metadata;    ///< Custom validation metadata
};

/**
 * @brief Interface validator for preventing conflicts and ensuring consistency
 */
class InterfaceValidator {
public:
    /**
     * @brief Register interface metadata for validation
     * @param metadata Interface metadata
     * @return Success or error
     */
    expected<void, PluginError> register_interface(const InterfaceMetadata& metadata);
    
    /**
     * @brief Validate interface consistency
     * @param interface_id Interface to validate
     * @return Validation result
     */
    InterfaceValidationResult validate_interface(const QString& interface_id) const;
    
    /**
     * @brief Validate all registered interfaces
     * @return Overall validation result
     */
    InterfaceValidationResult validate_all_interfaces() const;
    
    /**
     * @brief Check for interface conflicts
     * @return List of conflicting interface pairs
     */
    std::vector<std::pair<QString, QString>> find_interface_conflicts() const;
    
    /**
     * @brief Check for circular dependencies
     * @return List of circular dependency chains
     */
    std::vector<QStringList> find_circular_dependencies() const;
    
    /**
     * @brief Get interface metadata
     * @param interface_id Interface identifier
     * @return Interface metadata or error
     */
    expected<InterfaceMetadata, PluginError> get_interface_metadata(
        const QString& interface_id) const;
    
    /**
     * @brief Check interface compatibility
     * @param interface_id Interface identifier
     * @param required_version Minimum required version
     * @return true if compatible
     */
    bool is_interface_compatible(const QString& interface_id, 
                                const Version& required_version) const;
    
    /**
     * @brief Get deprecated interfaces
     * @return List of deprecated interface IDs with replacements
     */
    std::vector<std::pair<QString, QString>> get_deprecated_interfaces() const;
    
    /**
     * @brief Suggest interface upgrades
     * @param current_interface_id Current interface being used
     * @return Suggested upgrade path
     */
    QStringList suggest_interface_upgrades(const QString& current_interface_id) const;
    
    /**
     * @brief Clear all registered interfaces
     */
    void clear();
    
    /**
     * @brief Get all registered interface IDs
     * @return List of interface IDs
     */
    QStringList get_registered_interfaces() const;

private:
    std::unordered_map<QString, InterfaceMetadata> m_interfaces;
    
    /**
     * @brief Parse interface ID to extract base name and version
     * @param interface_id Full interface ID
     * @return Pair of base name and version
     */
    std::pair<QString, Version> parse_interface_id(const QString& interface_id) const;
    
    /**
     * @brief Check for circular dependency recursively
     * @param interface_id Starting interface
     * @param visited Set of visited interfaces
     * @param path Current dependency path
     * @return true if circular dependency found
     */
    bool has_circular_dependency(const QString& interface_id,
                                std::unordered_set<QString>& visited,
                                QStringList& path) const;
    
    /**
     * @brief Validate interface ID format
     * @param interface_id Interface ID to validate
     * @return true if valid format
     */
    bool is_valid_interface_id(const QString& interface_id) const;
};

/**
 * @brief Global interface validator instance
 * @return Reference to global validator
 */
InterfaceValidator& global_interface_validator();

/**
 * @brief Convenience macro for registering interface metadata
 */
#define QTPLUGIN_REGISTER_INTERFACE(interface_id, version_major, version_minor, version_patch, name, description) \
    do { \
        qtplugin::InterfaceMetadata metadata; \
        metadata.interface_id = interface_id; \
        metadata.version = qtplugin::Version{version_major, version_minor, version_patch}; \
        metadata.name = name; \
        metadata.description = description; \
        qtplugin::global_interface_validator().register_interface(metadata); \
    } while(0)

}  // namespace qtplugin
