/**
 * @file configuration_validator.cpp
 * @brief Implementation of configuration validator
 * @version 3.0.0
 */

#include "../qtplugin/managers/components/configuration_validator.hpp"
#include <QDebug>
#include <QJsonArray>
#include <QLoggingCategory>
#include <QRegularExpression>
#include <algorithm>

Q_LOGGING_CATEGORY(configValidatorLog, "qtplugin.config.validator")

namespace qtplugin {

ConfigurationValidator::ConfigurationValidator(QObject* parent)
    : QObject(parent) {
    qCDebug(configValidatorLog) << "Configuration validator initialized";
}

ConfigurationValidator::~ConfigurationValidator() {
    qCDebug(configValidatorLog) << "Configuration validator destroyed";
}

ConfigurationValidationResult ConfigurationValidator::validate_configuration(
    const QJsonObject& configuration, const ConfigurationSchema& schema) const {
    ConfigurationValidationResult result;
    result.is_valid = true;

    try {
        // Validate required properties
        auto required_result =
            validate_required_properties(configuration, schema);
        if (!required_result.is_valid) {
            result.errors.insert(result.errors.end(),
                                 required_result.errors.begin(),
                                 required_result.errors.end());
            result.is_valid = false;
        }

        // Validate each property in the configuration
        for (auto it = configuration.begin(); it != configuration.end(); ++it) {
            const QString& key = it.key();
            const QJsonValue& value = it.value();

            // Check if property is defined in schema
            QJsonObject properties =
                schema.schema.value("properties").toObject();
            if (properties.contains(key)) {
                auto property_schema = properties.value(key).toObject();
                auto property_result = validate_property(value, property_schema,
                                                         key.toStdString());

                if (!property_result.is_valid) {
                    result.errors.insert(result.errors.end(),
                                         property_result.errors.begin(),
                                         property_result.errors.end());
                    result.is_valid = false;
                    // Note: Cannot emit signals from const method
                    // emit validation_error(key,
                    // QString::fromStdString(property_result.errors.front()));
                }

                result.warnings.insert(result.warnings.end(),
                                       property_result.warnings.begin(),
                                       property_result.warnings.end());
            } else if (schema.strict_mode) {
                result.errors.push_back("Unknown property: " +
                                        key.toStdString());
                result.is_valid = false;
                // Note: Cannot emit signals from const method
                // emit validation_error(key, "Unknown property");
            }
        }

        // Note: Cannot emit signals from const method
        // emit validation_performed(result.is_valid,
        // static_cast<int>(result.errors.size()));

    } catch (const std::exception& e) {
        result.is_valid = false;
        result.errors.push_back("Exception during validation: " +
                                std::string(e.what()));
    }

    return result;
}

ConfigurationValidationResult ConfigurationValidator::validate_property(
    const QJsonValue& value, const QJsonObject& schema,
    const std::string& property_name) const {
    ConfigurationValidationResult result;
    result.is_valid = true;

    try {
        // Check type
        if (schema.contains("type")) {
            QString expected_type = schema["type"].toString();
            if (!validate_type(value, expected_type.toStdString())) {
                result.errors.push_back(
                    "Property '" + property_name +
                    "' has invalid type. Expected: " +
                    expected_type.toStdString() +
                    ", Got: " + get_json_value_type_name(value).toStdString());
                result.is_valid = false;
                return result;
            }
        }

        // Type-specific validation
        if (value.isString()) {
            if (!validate_string_constraints(value.toString(), schema)) {
                result.errors.push_back("Property '" + property_name +
                                        "' violates string constraints");
                result.is_valid = false;
            }
        } else if (value.isDouble()) {
            if (!validate_number_constraints(value.toDouble(), schema)) {
                result.errors.push_back("Property '" + property_name +
                                        "' violates number constraints");
                result.is_valid = false;
            }
        } else if (value.isArray()) {
            if (!validate_array_constraints(value.toArray(), schema)) {
                result.errors.push_back("Property '" + property_name +
                                        "' violates array constraints");
                result.is_valid = false;
            }
        } else if (value.isObject()) {
            if (!validate_object_constraints(value.toObject(), schema)) {
                result.errors.push_back("Property '" + property_name +
                                        "' violates object constraints");
                result.is_valid = false;
            }
        }

        // Check enum values
        if (schema.contains("enum") && schema["enum"].isArray()) {
            QJsonArray enum_values = schema["enum"].toArray();
            bool found = false;
            for (const auto& enum_value : enum_values) {
                if (enum_value == value) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                result.errors.push_back("Property '" + property_name +
                                        "' value not in allowed enum values");
                result.is_valid = false;
            }
        }

    } catch (const std::exception& e) {
        result.is_valid = false;
        result.errors.push_back("Exception validating property '" +
                                property_name + "': " + std::string(e.what()));
    }

    return result;
}

bool ConfigurationValidator::validate_type(
    const QJsonValue& value, const std::string& expected_type) const {
    if (expected_type == "string") {
        return value.isString();
    } else if (expected_type == "number") {
        return value.isDouble();
    } else if (expected_type == "integer") {
        return value.isDouble() &&
               (value.toDouble() == static_cast<int>(value.toDouble()));
    } else if (expected_type == "boolean") {
        return value.isBool();
    } else if (expected_type == "array") {
        return value.isArray();
    } else if (expected_type == "object") {
        return value.isObject();
    } else if (expected_type == "null") {
        return value.isNull();
    }

    return false;
}

bool ConfigurationValidator::validate_key_format(std::string_view key) const {
    if (key.empty()) {
        return false;
    }

    // Check for valid characters (alphanumeric, underscore, dot, hyphen)
    QRegularExpression key_regex("^[a-zA-Z0-9_.-]+$");
    QString key_str = QString::fromStdString(std::string(key));

    return key_regex.match(key_str).hasMatch();
}

QString ConfigurationValidator::get_json_value_type_name(
    const QJsonValue& value) const {
    if (value.isString()) {
        return "string";
    } else if (value.isDouble()) {
        return "number";
    } else if (value.isBool()) {
        return "boolean";
    } else if (value.isArray()) {
        return "array";
    } else if (value.isObject()) {
        return "object";
    } else if (value.isNull()) {
        return "null";
    } else if (value.isUndefined()) {
        return "undefined";
    }

    return "unknown";
}

ConfigurationValidationResult
ConfigurationValidator::validate_required_properties(
    const QJsonObject& configuration, const ConfigurationSchema& schema) const {
    ConfigurationValidationResult result;
    result.is_valid = true;

    if (schema.schema.contains("required") &&
        schema.schema["required"].isArray()) {
        QJsonArray required = schema.schema["required"].toArray();
        for (const auto& req : required) {
            QString required_property = req.toString();
            if (!configuration.contains(required_property)) {
                result.errors.push_back("Required property '" +
                                        required_property.toStdString() +
                                        "' is missing");
                result.is_valid = false;
            }
        }
    }

    return result;
}

bool ConfigurationValidator::validate_string_constraints(
    const QString& value, const QJsonObject& schema) const {
    // Check minimum length
    if (schema.contains("minLength")) {
        int min_length = schema["minLength"].toInt();
        if (value.length() < min_length) {
            return false;
        }
    }

    // Check maximum length
    if (schema.contains("maxLength")) {
        int max_length = schema["maxLength"].toInt();
        if (value.length() > max_length) {
            return false;
        }
    }

    // Check pattern
    if (schema.contains("pattern")) {
        QString pattern = schema["pattern"].toString();
        QRegularExpression regex(pattern);
        if (!regex.match(value).hasMatch()) {
            return false;
        }
    }

    return true;
}

bool ConfigurationValidator::validate_number_constraints(
    double value, const QJsonObject& schema) const {
    // Check minimum value
    if (schema.contains("minimum")) {
        double minimum = schema["minimum"].toDouble();
        if (value < minimum) {
            return false;
        }
    }

    // Check maximum value
    if (schema.contains("maximum")) {
        double maximum = schema["maximum"].toDouble();
        if (value > maximum) {
            return false;
        }
    }

    // Check exclusive minimum
    if (schema.contains("exclusiveMinimum")) {
        double exclusive_min = schema["exclusiveMinimum"].toDouble();
        if (value <= exclusive_min) {
            return false;
        }
    }

    // Check exclusive maximum
    if (schema.contains("exclusiveMaximum")) {
        double exclusive_max = schema["exclusiveMaximum"].toDouble();
        if (value >= exclusive_max) {
            return false;
        }
    }

    return true;
}

bool ConfigurationValidator::validate_array_constraints(
    const QJsonArray& value, const QJsonObject& schema) const {
    // Check minimum items
    if (schema.contains("minItems")) {
        int min_items = schema["minItems"].toInt();
        if (value.size() < min_items) {
            return false;
        }
    }

    // Check maximum items
    if (schema.contains("maxItems")) {
        int max_items = schema["maxItems"].toInt();
        if (value.size() > max_items) {
            return false;
        }
    }

    // Check unique items
    if (schema.contains("uniqueItems") && schema["uniqueItems"].toBool()) {
        QJsonArray sorted_array = value;
        // Simple uniqueness check (this could be more sophisticated)
        for (int i = 0; i < sorted_array.size(); ++i) {
            for (int j = i + 1; j < sorted_array.size(); ++j) {
                if (sorted_array[i] == sorted_array[j]) {
                    return false;
                }
            }
        }
    }

    return true;
}

bool ConfigurationValidator::validate_object_constraints(
    const QJsonObject& value, const QJsonObject& schema) const {
    // Check minimum properties
    if (schema.contains("minProperties")) {
        int min_properties = schema["minProperties"].toInt();
        if (value.size() < min_properties) {
            return false;
        }
    }

    // Check maximum properties
    if (schema.contains("maxProperties")) {
        int max_properties = schema["maxProperties"].toInt();
        if (value.size() > max_properties) {
            return false;
        }
    }

    return true;
}

ConfigurationValidationResult ConfigurationValidator::create_validation_result(
    bool is_valid, const std::vector<std::string>& errors,
    const std::vector<std::string>& warnings) const {
    ConfigurationValidationResult result;
    result.is_valid = is_valid;
    result.errors = errors;
    result.warnings = warnings;
    return result;
}

}  // namespace qtplugin
