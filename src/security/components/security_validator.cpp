/**
 * @file security_validator.cpp
 * @brief Implementation of security validator
 * @version 3.0.0
 */

#include "../../../include/qtplugin/security/components/security_validator.hpp"
#include <QDebug>
#include <QFileInfo>
#include <QLoggingCategory>
#include <algorithm>
#include <fstream>
#include "../../../include/qtplugin/security/security_manager.hpp"

Q_LOGGING_CATEGORY(securityValidatorLog, "qtplugin.security.validator")

namespace qtplugin {

SecurityValidator::SecurityValidator(QObject* parent) : QObject(parent) {
    qCDebug(securityValidatorLog) << "Security validator initialized";
}

SecurityValidator::~SecurityValidator() {
    qCDebug(securityValidatorLog) << "Security validator destroyed";
}

SecurityValidationResult SecurityValidator::validate_file_integrity(
    const std::filesystem::path& file_path) const {
    SecurityValidationResult result;
    result.is_valid = false;
    result.validated_level = SecurityLevel::None;

    try {
        // Check if file exists and is readable
        if (!file_exists_and_readable(file_path)) {
            result.errors.push_back("File does not exist or is not readable: " +
                                    file_path.string());
            // Note: Cannot emit signals from const method
            // emit
            // validation_performed(QString::fromStdString(file_path.string()),
            // false);
            return result;
        }

        // Check file size limits
        if (!check_file_size_limits(file_path)) {
            result.errors.push_back("File size exceeds maximum allowed limit");
            // Note: Cannot emit signals from const method
            // emit
            // security_violation_detected(QString::fromStdString(file_path.string()),
            // "File size limit exceeded");
            return result;
        }

        // Check file permissions
        if (!check_file_permissions(file_path)) {
            result.warnings.push_back("File has unusual permissions");
        }

        // Check for valid extension
        if (!has_valid_extension(file_path)) {
            result.errors.push_back("Invalid file extension: " +
                                    file_path.extension().string());
            return result;
        }

        // Check for safe file path
        if (!is_safe_file_path(file_path)) {
            result.errors.push_back("Unsafe file path detected");
            // Note: Cannot emit signals from const method
            // emit
            // security_violation_detected(QString::fromStdString(file_path.string()),
            // "Unsafe file path");
            return result;
        }

        result.is_valid = true;
        result.validated_level = SecurityLevel::Basic;

    } catch (const std::exception& e) {
        result.errors.push_back("Exception during file integrity validation: " +
                                std::string(e.what()));
    } catch (...) {
        result.errors.push_back(
            "Unknown exception during file integrity validation");
    }

    // Note: Cannot emit signals from const method
    // emit validation_performed(QString::fromStdString(file_path.string()),
    // result.is_valid);
    return result;
}

SecurityValidationResult SecurityValidator::validate_metadata(
    const std::filesystem::path& file_path) const {
    SecurityValidationResult result;
    result.is_valid = false;
    result.validated_level = SecurityLevel::None;

    try {
        // Basic file validation first
        auto file_result = validate_file_integrity(file_path);
        if (!file_result.is_valid) {
            return file_result;
        }

        // Check if file appears to be a valid plugin format
        // This is a simplified check - in a real implementation you would
        // parse the actual plugin metadata
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            result.errors.push_back("Cannot open file for metadata validation");
            return result;
        }

        // Read first few bytes to check for valid format
        char header[16];
        file.read(header, sizeof(header));

        if (file.gcount() < 4) {
            result.errors.push_back("File too small to contain valid metadata");
            return result;
        }

        // Simple format validation (this would be more sophisticated in
        // practice)
        bool has_valid_format = true;

        // Check for common executable formats
        if (file_path.extension() == ".dll" || file_path.extension() == ".so" ||
            file_path.extension() == ".dylib") {
            // Check for PE/ELF/Mach-O headers (simplified)
            if (header[0] == 'M' && header[1] == 'Z') {
                // PE format (Windows DLL)
                has_valid_format = true;
            } else if (header[0] == 0x7f && header[1] == 'E' &&
                       header[2] == 'L' && header[3] == 'F') {
                // ELF format (Linux SO)
                has_valid_format = true;
            } else if (static_cast<unsigned char>(header[0]) == 0xfe &&
                       static_cast<unsigned char>(header[1]) == 0xed &&
                       static_cast<unsigned char>(header[2]) == 0xfa) {
                // Mach-O format (macOS dylib)
                has_valid_format = true;
            } else {
                has_valid_format = false;
            }
        }

        if (!has_valid_format) {
            result.errors.push_back("Invalid file format for plugin");
            return result;
        }

        result.is_valid = true;
        result.validated_level = SecurityLevel::Basic;

    } catch (const std::exception& e) {
        result.errors.push_back("Exception during metadata validation: " +
                                std::string(e.what()));
    } catch (...) {
        result.errors.push_back("Unknown exception during metadata validation");
    }

    return result;
}

bool SecurityValidator::is_safe_file_path(
    const std::filesystem::path& file_path) const {
    std::string path_str = file_path.string();

    // Check for path traversal attempts
    if (path_str.find("..") != std::string::npos) {
        return false;
    }

    // Check for suspicious characters
    if (path_str.find_first_of("<>:\"|?*") != std::string::npos) {
        return false;
    }

    // Check for null bytes
    if (path_str.find('\0') != std::string::npos) {
        return false;
    }

    return true;
}

bool SecurityValidator::has_valid_extension(
    const std::filesystem::path& file_path) const {
    auto extensions = get_allowed_extensions();
    std::string extension = file_path.extension().string();

    return std::find(extensions.begin(), extensions.end(), extension) !=
           extensions.end();
}

std::vector<std::string> SecurityValidator::get_allowed_extensions() const {
    return {".dll", ".so", ".dylib", ".qtplugin"};
}

SecurityValidationResult SecurityValidator::validate_basic_security(
    const std::filesystem::path& file_path,
    SecurityLevel required_level) const {
    SecurityValidationResult result;

    // Always perform file integrity validation
    result = validate_file_integrity(file_path);
    if (!result.is_valid) {
        return result;
    }

    // Perform metadata validation if required
    if (required_level >= SecurityLevel::Basic) {
        auto metadata_result = validate_metadata(file_path);
        if (!metadata_result.is_valid) {
            result.errors.insert(result.errors.end(),
                                 metadata_result.errors.begin(),
                                 metadata_result.errors.end());
            result.warnings.insert(result.warnings.end(),
                                   metadata_result.warnings.begin(),
                                   metadata_result.warnings.end());
            result.is_valid = false;
            return result;
        }
        result.warnings.insert(result.warnings.end(),
                               metadata_result.warnings.begin(),
                               metadata_result.warnings.end());
    }

    result.validated_level = SecurityLevel::Basic;
    return result;
}

bool SecurityValidator::file_exists_and_readable(
    const std::filesystem::path& file_path) const {
    try {
        return std::filesystem::exists(file_path) &&
               std::filesystem::is_regular_file(file_path);
    } catch (...) {
        return false;
    }
}

bool SecurityValidator::check_file_size_limits(
    const std::filesystem::path& file_path) const {
    try {
        const size_t MAX_PLUGIN_SIZE = 100 * 1024 * 1024;  // 100MB limit
        auto file_size = std::filesystem::file_size(file_path);
        return file_size <= MAX_PLUGIN_SIZE;
    } catch (...) {
        return false;
    }
}

bool SecurityValidator::check_file_permissions(
    const std::filesystem::path& file_path) const {
    try {
        QFileInfo file_info(QString::fromStdString(file_path.string()));

        // Check if file is executable (which is expected for plugins)
        if (!file_info.isExecutable()) {
            return false;
        }

        // Check if file is writable by others (security risk)
        if (file_info.permission(QFileDevice::WriteOther)) {
            return false;
        }

        return true;
    } catch (...) {
        return false;
    }
}

SecurityValidationResult SecurityValidator::create_validation_result(
    bool is_valid, const std::vector<std::string>& errors,
    const std::vector<std::string>& warnings) const {
    SecurityValidationResult result;
    result.is_valid = is_valid;
    result.errors = errors;
    result.warnings = warnings;
    result.validated_level =
        is_valid ? SecurityLevel::Basic : SecurityLevel::None;
    return result;
}

}  // namespace qtplugin

#include "security_validator.moc"
