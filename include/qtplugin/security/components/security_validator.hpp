#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <filesystem>
#include <memory>
#include <vector>

namespace qtplugin {
namespace security {
namespace components {

/**
 * @brief Validation result structure
 */
struct ValidationResult_t {
    bool is_valid = false;
    std::vector<std::string> errors;
    std::string details;
};

/**
 * @brief Security validator for plugin integrity and safety checks
 *
 * This is a stub implementation to satisfy test compilation requirements.
 * The actual security validator would implement comprehensive security
 * validation including signature verification, malware scanning, and
 * integrity checks.
 */
class SecurityValidator : public QObject {
    Q_OBJECT

public:
    enum ValidationResult {
        Valid = 0,
        Invalid = 1,
        Suspicious = 2,
        Malicious = 3,
        Unknown = 4
    };
    Q_ENUM(ValidationResult)

    explicit SecurityValidator(QObject* parent = nullptr);
    virtual ~SecurityValidator() = default;

    /**
     * @brief Initialize the security validator
     * @return true if initialization successful, false otherwise
     */
    bool initialize();

    /**
     * @brief Validate plugin metadata for security compliance
     * @param metadataPath Path to plugin metadata file
     * @return Validation result
     */
    ValidationResult validate_metadata(const QString& metadataPath);

    /**
     * @brief Validate plugin metadata using filesystem path
     * @param metadataPath Filesystem path to plugin metadata file
     * @return Validation result structure
     */
    ValidationResult_t validate_metadata(
        const std::filesystem::path& metadataPath);

    /**
     * @brief Validate file integrity
     * @param filePath Path to file to validate
     * @return Validation result structure
     */
    ValidationResult_t validate_file_integrity(const std::string& filePath);

    /**
     * @brief Validate plugin binary for security threats
     * @param binaryPath Path to plugin binary file
     * @return Validation result
     */
    ValidationResult validate_binary(const QString& binaryPath);

    /**
     * @brief Perform comprehensive plugin validation
     * @param pluginPath Path to plugin directory or file
     * @return Validation result
     */
    ValidationResult validate_plugin(const QString& pluginPath);

    /**
     * @brief Check plugin permissions and capabilities
     * @param pluginPath Path to plugin
     * @param requestedPermissions List of requested permissions
     * @return true if permissions are acceptable, false otherwise
     */
    bool validate_permissions(const QString& pluginPath,
                              const QStringList& requestedPermissions);

    /**
     * @brief Get detailed validation report
     * @return Detailed validation report as string
     */
    QString getValidationReport() const;

    /**
     * @brief Set validation strictness level
     * @param level Strictness level (0-10, where 10 is most strict)
     */
    void setStrictnessLevel(int level);

    /**
     * @brief Get current validation strictness level
     * @return Current strictness level
     */
    int getStrictnessLevel() const;

signals:
    /**
     * @brief Emitted when validation is complete
     * @param result Validation result
     * @param details Additional details about the validation
     */
    void validationComplete(ValidationResult result, const QString& details);

    /**
     * @brief Emitted when a security threat is detected
     * @param threat Description of the detected threat
     * @param severity Threat severity level
     */
    void threatDetected(const QString& threat, int severity);

private:
    int m_strictnessLevel = 5;
    QString m_validationReport;
};

}  // namespace components
}  // namespace security
}  // namespace qtplugin
