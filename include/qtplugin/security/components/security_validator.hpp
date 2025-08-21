/**
 * @file security_validator.hpp
 * @brief Security validator interface and implementation
 * @version 3.0.0
 */

#pragma once

#include "../../utils/error_handling.hpp"
#include <filesystem>
#include <string>
#include <vector>
#include <QObject>

namespace qtplugin {

// Forward declarations
struct SecurityValidationResult;
enum class SecurityLevel;

/**
 * @brief Interface for core security validation
 * 
 * The security validator handles basic file integrity checks,
 * metadata validation, and core security operations.
 */
class ISecurityValidator {
public:
    virtual ~ISecurityValidator() = default;
    
    /**
     * @brief Validate file integrity
     * @param file_path Path to file to validate
     * @return Validation result
     */
    virtual SecurityValidationResult validate_file_integrity(const std::filesystem::path& file_path) const = 0;
    
    /**
     * @brief Validate plugin metadata
     * @param file_path Path to plugin file
     * @return Validation result
     */
    virtual SecurityValidationResult validate_metadata(const std::filesystem::path& file_path) const = 0;
    
    /**
     * @brief Check if file path is safe
     * @param file_path Path to check
     * @return true if path is safe
     */
    virtual bool is_safe_file_path(const std::filesystem::path& file_path) const = 0;
    
    /**
     * @brief Check if file has valid extension
     * @param file_path Path to check
     * @return true if extension is valid
     */
    virtual bool has_valid_extension(const std::filesystem::path& file_path) const = 0;
    
    /**
     * @brief Get allowed file extensions
     * @return Vector of allowed extensions
     */
    virtual std::vector<std::string> get_allowed_extensions() const = 0;
    
    /**
     * @brief Validate basic security requirements
     * @param file_path Path to plugin file
     * @param required_level Required security level
     * @return Validation result
     */
    virtual SecurityValidationResult validate_basic_security(const std::filesystem::path& file_path,
                                                            SecurityLevel required_level) const = 0;
};

/**
 * @brief Security validator implementation
 * 
 * Provides core security validation functionality including file integrity
 * checks, metadata validation, and basic security requirements.
 */
class SecurityValidator : public QObject, public ISecurityValidator {
    Q_OBJECT
    
public:
    explicit SecurityValidator(QObject* parent = nullptr);
    ~SecurityValidator() override;
    
    // ISecurityValidator interface
    SecurityValidationResult validate_file_integrity(const std::filesystem::path& file_path) const override;
    SecurityValidationResult validate_metadata(const std::filesystem::path& file_path) const override;
    bool is_safe_file_path(const std::filesystem::path& file_path) const override;
    bool has_valid_extension(const std::filesystem::path& file_path) const override;
    std::vector<std::string> get_allowed_extensions() const override;
    SecurityValidationResult validate_basic_security(const std::filesystem::path& file_path,
                                                    SecurityLevel required_level) const override;

signals:
    /**
     * @brief Emitted when validation is performed
     * @param file_path Path that was validated
     * @param is_valid Whether validation passed
     */
    void validation_performed(const QString& file_path, bool is_valid);
    
    /**
     * @brief Emitted when security violation is detected
     * @param file_path Path where violation was detected
     * @param violation Description of violation
     */
    void security_violation_detected(const QString& file_path, const QString& violation);

private:
    // Helper methods
    bool file_exists_and_readable(const std::filesystem::path& file_path) const;
    bool check_file_size_limits(const std::filesystem::path& file_path) const;
    bool check_file_permissions(const std::filesystem::path& file_path) const;
    SecurityValidationResult create_validation_result(bool is_valid, 
                                                     const std::vector<std::string>& errors = {},
                                                     const std::vector<std::string>& warnings = {}) const;
};

} // namespace qtplugin
