/**
 * @file configuration_validator.hpp
 * @brief Configuration validator interface and implementation
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <string>
#include <string_view>
#include "../../utils/error_handling.hpp"
#include "../configuration_manager.hpp"

namespace qtplugin {

// Forward declarations
struct ConfigurationSchema;
struct ConfigurationValidationResult;
enum class ConfigurationScope;

/**
 * @brief Interface for configuration validation
 *
 * The configuration validator handles schema validation, type checking,
 * and configuration verification operations.
 */
class IConfigurationValidator {
public:
    virtual ~IConfigurationValidator() = default;

    /**
     * @brief Validate configuration against schema
     * @param configuration Configuration to validate
     * @param schema Schema to validate against
     * @return Validation result
     */
    virtual ConfigurationValidationResult validate_configuration(
        const QJsonObject& configuration,
        const ConfigurationSchema& schema) const = 0;

    /**
     * @brief Validate single property
     * @param value Property value
     * @param schema Property schema
     * @param property_name Property name
     * @return Validation result
     */
    virtual ConfigurationValidationResult validate_property(
        const QJsonValue& value, const QJsonObject& schema,
        const std::string& property_name) const = 0;

    /**
     * @brief Validate configuration value type
     * @param value Value to validate
     * @param expected_type Expected type name
     * @return true if type matches
     */
    virtual bool validate_type(const QJsonValue& value,
                               const std::string& expected_type) const = 0;

    /**
     * @brief Validate configuration key format
     * @param key Configuration key
     * @return true if key format is valid
     */
    virtual bool validate_key_format(std::string_view key) const = 0;

    /**
     * @brief Get JSON value type name
     * @param value JSON value
     * @return Type name as string
     */
    virtual QString get_json_value_type_name(const QJsonValue& value) const = 0;

    /**
     * @brief Check if configuration has required properties
     * @param configuration Configuration to check
     * @param schema Schema with required properties
     * @return Validation result
     */
    virtual ConfigurationValidationResult validate_required_properties(
        const QJsonObject& configuration,
        const ConfigurationSchema& schema) const = 0;
};

/**
 * @brief Configuration validator implementation
 *
 * Provides comprehensive configuration validation including schema validation,
 * type checking, and format verification.
 */
class ConfigurationValidator : public QObject, public IConfigurationValidator {
    Q_OBJECT

public:
    explicit ConfigurationValidator(QObject* parent = nullptr);
    ~ConfigurationValidator() override;

    // IConfigurationValidator interface
    ConfigurationValidationResult validate_configuration(
        const QJsonObject& configuration,
        const ConfigurationSchema& schema) const override;

    ConfigurationValidationResult validate_property(
        const QJsonValue& value, const QJsonObject& schema,
        const std::string& property_name) const override;

    bool validate_type(const QJsonValue& value,
                       const std::string& expected_type) const override;

    bool validate_key_format(std::string_view key) const override;

    QString get_json_value_type_name(const QJsonValue& value) const override;

    ConfigurationValidationResult validate_required_properties(
        const QJsonObject& configuration,
        const ConfigurationSchema& schema) const override;

signals:
    /**
     * @brief Emitted when validation is performed
     * @param is_valid Whether validation passed
     * @param error_count Number of validation errors
     */
    void validation_performed(bool is_valid, int error_count);

    /**
     * @brief Emitted when validation error occurs
     * @param property_name Property that failed validation
     * @param error_message Error description
     */
    void validation_error(const QString& property_name,
                          const QString& error_message);

private:
    // Helper methods
    bool validate_string_constraints(const QString& value,
                                     const QJsonObject& schema) const;
    bool validate_number_constraints(double value,
                                     const QJsonObject& schema) const;
    bool validate_array_constraints(const QJsonArray& value,
                                    const QJsonObject& schema) const;
    bool validate_object_constraints(const QJsonObject& value,
                                     const QJsonObject& schema) const;

    ConfigurationValidationResult create_validation_result(
        bool is_valid, const std::vector<std::string>& errors = {},
        const std::vector<std::string>& warnings = {}) const;
};

}  // namespace qtplugin
