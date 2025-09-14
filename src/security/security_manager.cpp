/**
 * @file security_manager.cpp
 * @brief Implementation of security manager
 * @version 3.0.0
 */

#include "qtplugin/security/security_manager.hpp"
#include "qtplugin/security/sandbox/plugin_sandbox.hpp"
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QPluginLoader>
#include <QRegularExpression>
#include <QTimer>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <mutex>
#include "qtplugin/security/components/permission_manager.hpp"
#include "qtplugin/security/components/security_policy_engine.hpp"
#include "qtplugin/security/components/security_validator.hpp"
#include "qtplugin/security/components/signature_verifier.hpp"

namespace qtplugin {

SecurityManager::SecurityManager()
    : m_validator(std::make_unique<SecurityValidator>(nullptr)),
      m_signature_verifier(std::make_unique<SignatureVerifier>(nullptr)),
      m_permission_manager(std::make_unique<PermissionManager>(nullptr)),
      m_policy_engine(std::make_unique<SecurityPolicyEngine>(nullptr)) {
    // Configure signature verifier
    m_signature_verifier->set_signature_verification_enabled(
        m_signature_verification_enabled);

    // Initialize enhanced security features
    initialize_monitoring_timer();

    // Log security manager initialization
    log_security_event("security_manager_initialized",
                      "system",
                      "Security manager started with default configuration",
                      SecurityLevel::Basic);
}

SecurityManager::~SecurityManager() = default;

SecurityValidationResult SecurityManager::validate_plugin(
    const std::filesystem::path& file_path, SecurityLevel required_level) {
    m_validations_performed.fetch_add(1);

    SecurityValidationResult result;
    result.validated_level = SecurityLevel::None;

    // Basic file validation (always performed)
    auto file_result = validate_file_integrity(file_path);
    if (!file_result.is_valid) {
        result.errors.insert(result.errors.end(), file_result.errors.begin(),
                             file_result.errors.end());
        m_validations_failed.fetch_add(1);
        return result;
    }
    result.validated_level = SecurityLevel::Basic;
    result.warnings.insert(result.warnings.end(), file_result.warnings.begin(),
                           file_result.warnings.end());

    // Metadata validation
    if (required_level >= SecurityLevel::Basic) {
        auto metadata_result = validate_metadata(file_path);
        if (!metadata_result.is_valid) {
            result.errors.insert(result.errors.end(),
                                 metadata_result.errors.begin(),
                                 metadata_result.errors.end());
            m_validations_failed.fetch_add(1);
            return result;
        }
        result.warnings.insert(result.warnings.end(),
                               metadata_result.warnings.begin(),
                               metadata_result.warnings.end());
    }

    // Signature validation
    if (required_level >= SecurityLevel::Standard &&
        m_signature_verification_enabled) {
        auto signature_result = validate_signature(file_path);
        if (!signature_result.is_valid) {
            result.errors.insert(result.errors.end(),
                                 signature_result.errors.begin(),
                                 signature_result.errors.end());
            m_validations_failed.fetch_add(1);
            return result;
        }
        result.validated_level = SecurityLevel::Standard;
        result.warnings.insert(result.warnings.end(),
                               signature_result.warnings.begin(),
                               signature_result.warnings.end());
    }

    // Permission validation
    if (required_level >= SecurityLevel::Strict) {
        auto permission_result = validate_permissions(file_path);
        if (!permission_result.is_valid) {
            result.errors.insert(result.errors.end(),
                                 permission_result.errors.begin(),
                                 permission_result.errors.end());
            m_validations_failed.fetch_add(1);
            return result;
        }
        result.validated_level = SecurityLevel::Strict;
        result.warnings.insert(result.warnings.end(),
                               permission_result.warnings.begin(),
                               permission_result.warnings.end());
    }

    result.is_valid = true;
    m_validations_passed.fetch_add(1);

    return result;
}

bool SecurityManager::is_trusted(std::string_view plugin_id) const {
    std::shared_lock lock(m_trusted_plugins_mutex);
    return m_trusted_plugins.find(std::string(plugin_id)) !=
           m_trusted_plugins.end();
}

void SecurityManager::add_trusted_plugin(std::string_view plugin_id,
                                         SecurityLevel trust_level) {
    std::unique_lock lock(m_trusted_plugins_mutex);
    m_trusted_plugins[std::string(plugin_id)] = trust_level;
}

void SecurityManager::remove_trusted_plugin(std::string_view plugin_id) {
    std::unique_lock lock(m_trusted_plugins_mutex);
    m_trusted_plugins.erase(std::string(plugin_id));
}

SecurityLevel SecurityManager::security_level() const noexcept {
    return m_security_level;
}

void SecurityManager::set_security_level(SecurityLevel level) {
    m_security_level = level;
}

QJsonObject SecurityManager::security_statistics() const {
    return QJsonObject{
        {"validations_performed",
         static_cast<qint64>(m_validations_performed.load())},
        {"validations_passed",
         static_cast<qint64>(m_validations_passed.load())},
        {"validations_failed",
         static_cast<qint64>(m_validations_failed.load())},
        {"trusted_plugins_count", static_cast<int>(m_trusted_plugins.size())},
        {"current_security_level", security_level_to_string(m_security_level)},
        {"signature_verification_enabled", m_signature_verification_enabled}};
}

qtplugin::expected<void, PluginError> SecurityManager::load_trusted_plugins(
    const std::filesystem::path& file_path) {
    if (!std::filesystem::exists(file_path)) {
        return make_error<void>(PluginErrorCode::FileNotFound,
                                "Trusted plugins file not found");
    }

    std::ifstream file(file_path);
    if (!file.is_open()) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Cannot open trusted plugins file");
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    QJsonParseError parse_error;
    QJsonDocument doc = QJsonDocument::fromJson(
        QByteArray::fromStdString(content), &parse_error);

    if (parse_error.error != QJsonParseError::NoError) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "Invalid JSON in trusted plugins file: " +
                                    parse_error.errorString().toStdString());
    }

    if (!doc.isObject()) {
        return make_error<void>(
            PluginErrorCode::ConfigurationError,
            "Trusted plugins file must contain a JSON object");
    }

    QJsonObject root = doc.object();
    if (!root.contains("trusted_plugins") ||
        !root["trusted_plugins"].isArray()) {
        return make_error<void>(PluginErrorCode::ConfigurationError,
                                "Missing 'trusted_plugins' array");
    }

    std::unique_lock lock(m_trusted_plugins_mutex);
    m_trusted_plugins.clear();

    QJsonArray plugins = root["trusted_plugins"].toArray();
    for (const auto& plugin_value : plugins) {
        if (!plugin_value.isObject())
            continue;

        QJsonObject plugin = plugin_value.toObject();
        if (!plugin.contains("id") || !plugin["id"].isString())
            continue;

        std::string plugin_id = plugin["id"].toString().toStdString();
        SecurityLevel trust_level = SecurityLevel::Basic;

        if (plugin.contains("trust_level") &&
            plugin["trust_level"].isString()) {
            trust_level = security_level_from_string(
                plugin["trust_level"].toString().toStdString());
        }

        m_trusted_plugins[plugin_id] = trust_level;
    }

    return make_success();
}

qtplugin::expected<void, PluginError> SecurityManager::save_trusted_plugins(
    const std::filesystem::path& file_path) const {
    QJsonArray plugins_array;

    {
        std::shared_lock lock(m_trusted_plugins_mutex);
        for (const auto& [plugin_id, trust_level] : m_trusted_plugins) {
            QJsonObject plugin{
                {"id", QString::fromStdString(plugin_id)},
                {"trust_level", security_level_to_string(trust_level)}};
            plugins_array.append(plugin);
        }
    }

    QJsonObject root{{"version", "1.0"}, {"trusted_plugins", plugins_array}};

    QJsonDocument doc(root);

    std::ofstream file(file_path);
    if (!file.is_open()) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Cannot create trusted plugins file");
    }

    std::string json_string = doc.toJson().toStdString();
    file.write(json_string.c_str(), json_string.size());
    file.close();

    if (file.fail()) {
        return make_error<void>(PluginErrorCode::FileSystemError,
                                "Failed to write trusted plugins file");
    }

    return make_success();
}

void SecurityManager::set_signature_verification_enabled(bool enabled) {
    m_signature_verification_enabled = enabled;
}

bool SecurityManager::is_signature_verification_enabled() const noexcept {
    return m_signature_verification_enabled;
}

SecurityValidationResult SecurityManager::validate_file_integrity(
    const std::filesystem::path& file_path) const {
    SecurityValidationResult result;

    if (!is_safe_file_path(file_path)) {
        result.errors.push_back("Unsafe file path detected");
        return result;
    }

    if (!has_valid_extension(file_path)) {
        result.errors.push_back("Invalid file extension");
        return result;
    }

    QFileInfo file_info(QString::fromStdString(file_path.string()));
    if (!file_info.exists()) {
        result.errors.push_back("File does not exist");
        return result;
    }

    if (!file_info.isFile()) {
        result.errors.push_back("Path is not a regular file");
        return result;
    }

    if (!file_info.isReadable()) {
        result.errors.push_back("File is not readable");
        return result;
    }

    // Check file size (reasonable limits)
    qint64 file_size = file_info.size();
    if (file_size == 0) {
        result.errors.push_back("File is empty");
        return result;
    }

    if (file_size > 100 * 1024 * 1024) {  // 100MB limit
        result.warnings.push_back("File is very large (>100MB)");
    }

    result.is_valid = true;
    return result;
}

SecurityValidationResult SecurityManager::validate_metadata(
    const std::filesystem::path& file_path) const {
    SecurityValidationResult result;
    result.is_valid = false;

    try {
        // Check if file exists
        if (!std::filesystem::exists(file_path)) {
            result.errors.push_back("Plugin file does not exist: " +
                                    file_path.string());
            return result;
        }

        // Load plugin to extract metadata
        QPluginLoader loader(QString::fromStdString(file_path.string()));
        QJsonObject metadata = loader.metaData();

        if (metadata.isEmpty()) {
            result.errors.push_back("Failed to load plugin metadata");
            return result;
        }

        // Validate required metadata fields
        QJsonObject plugin_metadata = metadata["MetaData"].toObject();

        // Check for required fields
        std::vector<std::string> required_fields = {"name", "version",
                                                    "author"};
        for (const auto& field : required_fields) {
            if (!plugin_metadata.contains(QString::fromStdString(field))) {
                result.errors.push_back("Missing required metadata field: " +
                                        field);
            }
        }

        // Validate name field
        if (plugin_metadata.contains("name")) {
            QString name = plugin_metadata["name"].toString();
            if (name.isEmpty()) {
                result.errors.push_back("Plugin name cannot be empty");
            } else if (name.length() > 100) {
                result.warnings.push_back(
                    "Plugin name is unusually long (>100 characters)");
            }

            // Check for suspicious characters in name
            if (name.contains(QRegularExpression("[<>:\"|?*\\\\]"))) {
                result.errors.push_back(
                    "Plugin name contains invalid characters");
            }
        }

        // Validate version field
        if (plugin_metadata.contains("version")) {
            QString version = plugin_metadata["version"].toString();
            if (version.isEmpty()) {
                result.errors.push_back("Plugin version cannot be empty");
            } else {
                // Basic version format validation (semantic versioning)
                QRegularExpression version_regex(
                    "^\\d+\\.\\d+\\.\\d+(?:-[a-zA-Z0-9.-]+)?(?:\\+[a-zA-Z0-9.-]"
                    "+)?$");
                if (!version_regex.match(version).hasMatch()) {
                    result.warnings.push_back(
                        "Plugin version does not follow semantic versioning "
                        "format");
                }
            }
        }

        // Validate author field
        if (plugin_metadata.contains("author")) {
            QString author = plugin_metadata["author"].toString();
            if (author.isEmpty()) {
                result.warnings.push_back("Plugin author is empty");
            } else if (author.length() > 200) {
                result.warnings.push_back(
                    "Plugin author field is unusually long");
            }
        }

        // Check for suspicious metadata
        if (plugin_metadata.contains("description")) {
            QString description = plugin_metadata["description"].toString();
            if (description.length() > 1000) {
                result.warnings.push_back(
                    "Plugin description is unusually long");
            }
        }

        // Validate dependencies if present
        if (plugin_metadata.contains("dependencies")) {
            QJsonValue deps = plugin_metadata["dependencies"];
            if (deps.isArray()) {
                QJsonArray deps_array = deps.toArray();
                if (deps_array.size() > 50) {
                    result.warnings.push_back(
                        "Plugin has an unusually large number of dependencies");
                }

                for (const auto& dep : deps_array) {
                    if (!dep.isString() || dep.toString().isEmpty()) {
                        result.errors.push_back(
                            "Invalid dependency specification");
                    }
                }
            }
        }

        // If no errors, validation passed
        if (result.errors.empty()) {
            result.is_valid = true;
            result.validated_level = SecurityLevel::Basic;
        }

        // Store metadata details
        result.details["metadata"] = plugin_metadata;
        result.details["file_path"] =
            QString::fromStdString(file_path.string());

    } catch (const std::exception& e) {
        result.errors.push_back("Exception during metadata validation: " +
                                std::string(e.what()));
    } catch (...) {
        result.errors.push_back("Unknown exception during metadata validation");
    }

    return result;
}

SecurityValidationResult SecurityManager::validate_signature(
    const std::filesystem::path& file_path) const {
    SecurityValidationResult result;
    result.is_valid = false;

    if (!m_signature_verification_enabled) {
        result.warnings.push_back("Signature verification is disabled");
        result.is_valid = true;
        result.validated_level = SecurityLevel::Basic;
        return result;
    }

    try {
        // Check if file exists
        if (!std::filesystem::exists(file_path)) {
            result.errors.push_back(
                "Plugin file does not exist for signature validation");
            return result;
        }

        // Get file size for validation
        auto file_size = std::filesystem::file_size(file_path);
        if (file_size == 0) {
            result.errors.push_back("Plugin file is empty");
            return result;
        }

        // Check for common signature file extensions or embedded signatures
        std::filesystem::path sig_file = file_path;
        sig_file.replace_extension(file_path.extension().string() + ".sig");

        bool has_signature_file = std::filesystem::exists(sig_file);

        // Look for embedded signature in plugin metadata
        QPluginLoader loader(QString::fromStdString(file_path.string()));
        QJsonObject metadata = loader.metaData();
        bool has_embedded_signature = false;

        if (!metadata.isEmpty()) {
            QJsonObject plugin_metadata = metadata["MetaData"].toObject();
            has_embedded_signature =
                plugin_metadata.contains("signature") ||
                plugin_metadata.contains("digital_signature") ||
                plugin_metadata.contains("checksum");
        }

        if (!has_signature_file && !has_embedded_signature) {
            if (m_security_level >= SecurityLevel::Standard) {
                result.errors.push_back(
                    "No digital signature found for plugin");
                return result;
            } else {
                result.warnings.push_back(
                    "No digital signature found for plugin");
            }
        }

        // Basic file integrity check using checksum
        if (has_embedded_signature) {
            QJsonObject plugin_metadata = metadata["MetaData"].toObject();

            if (plugin_metadata.contains("checksum")) {
                QString expected_checksum =
                    plugin_metadata["checksum"].toString();
                if (!expected_checksum.isEmpty()) {
                    // Calculate actual file checksum (simplified
                    // implementation)
                    QFile file(QString::fromStdString(file_path.string()));
                    if (file.open(QIODevice::ReadOnly)) {
                        QCryptographicHash hash(QCryptographicHash::Sha256);
                        hash.addData(file.readAll());
                        QString actual_checksum = hash.result().toHex();

                        if (actual_checksum.toLower() !=
                            expected_checksum.toLower()) {
                            result.errors.push_back(
                                "File checksum verification failed");
                            return result;
                        } else {
                            result.details["checksum_verified"] = true;
                        }
                    }
                }
            }
        }

        // If we reach here, basic validation passed
        result.is_valid = true;
        result.validated_level = has_signature_file || has_embedded_signature
                                     ? SecurityLevel::Standard
                                     : SecurityLevel::Basic;

        if (has_signature_file) {
            result.details["signature_file"] =
                QString::fromStdString(sig_file.string());
        }
        if (has_embedded_signature) {
            result.details["embedded_signature"] = true;
        }

    } catch (const std::exception& e) {
        result.errors.push_back("Exception during signature validation: " +
                                std::string(e.what()));
    } catch (...) {
        result.errors.push_back(
            "Unknown exception during signature validation");
    }

    return result;
}

SecurityValidationResult SecurityManager::validate_permissions(
    const std::filesystem::path& file_path) const {
    SecurityValidationResult result;
    result.is_valid = false;

    try {
        // Check if file exists
        if (!std::filesystem::exists(file_path)) {
            result.errors.push_back(
                "Plugin file does not exist for permission validation");
            return result;
        }

        // Check file permissions
        auto file_status = std::filesystem::status(file_path);
        auto file_perms = file_status.permissions();

        // Ensure file is readable
        if ((file_perms & std::filesystem::perms::owner_read) ==
                std::filesystem::perms::none &&
            (file_perms & std::filesystem::perms::group_read) ==
                std::filesystem::perms::none &&
            (file_perms & std::filesystem::perms::others_read) ==
                std::filesystem::perms::none) {
            result.errors.push_back("Plugin file is not readable");
            return result;
        }

        // Check for overly permissive permissions
        if ((file_perms & std::filesystem::perms::others_write) !=
            std::filesystem::perms::none) {
            result.warnings.push_back(
                "Plugin file is writable by others - potential security risk");
        }

        // Load plugin metadata to check requested permissions
        QPluginLoader loader(QString::fromStdString(file_path.string()));
        QJsonObject metadata = loader.metaData();

        if (!metadata.isEmpty()) {
            QJsonObject plugin_metadata = metadata["MetaData"].toObject();

            // Check for requested permissions
            if (plugin_metadata.contains("permissions")) {
                QJsonValue permissions = plugin_metadata["permissions"];

                if (permissions.isArray()) {
                    QJsonArray perms_array = permissions.toArray();
                    std::vector<std::string> dangerous_permissions = {
                        "file_system_write", "network_access",
                        "system_commands",   "registry_access",
                        "process_creation",  "dll_injection"};

                    for (const auto& perm : perms_array) {
                        if (perm.isString()) {
                            std::string perm_str =
                                perm.toString().toStdString();

                            // Check for dangerous permissions
                            for (const auto& dangerous :
                                 dangerous_permissions) {
                                if (perm_str == dangerous) {
                                    if (m_security_level >=
                                        SecurityLevel::Standard) {
                                        result.warnings.push_back(
                                            "Plugin requests dangerous "
                                            "permission: " +
                                            perm_str);
                                    }
                                }
                            }

                            // Check for privilege escalation attempts
                            if (perm_str.find("admin") != std::string::npos ||
                                perm_str.find("root") != std::string::npos ||
                                perm_str.find("elevated") !=
                                    std::string::npos) {
                                result.errors.push_back(
                                    "Plugin requests elevated privileges: " +
                                    perm_str);
                                return result;
                            }
                        }
                    }

                    result.details["requested_permissions"] = perms_array;
                } else if (permissions.isString()) {
                    result.warnings.push_back(
                        "Plugin permissions should be specified as an array");
                }
            }

            // Check for suspicious capabilities
            if (plugin_metadata.contains("capabilities")) {
                QJsonValue capabilities = plugin_metadata["capabilities"];
                if (capabilities.isArray()) {
                    QJsonArray caps_array = capabilities.toArray();

                    for (const auto& cap : caps_array) {
                        if (cap.isString()) {
                            std::string cap_str = cap.toString().toStdString();

                            // Check for potentially dangerous capabilities
                            if (cap_str == "system_access" ||
                                cap_str == "unrestricted") {
                                result.warnings.push_back(
                                    "Plugin declares potentially dangerous "
                                    "capability: " +
                                    cap_str);
                            }
                        }
                    }
                }
            }
        }

        // Check file location - plugins should be in designated directories
        std::string file_path_str = file_path.string();
        std::vector<std::string> safe_directories = {
            "plugins", "extensions", "addons", "lib", "libraries"};

        bool in_safe_directory = false;
        for (const auto& safe_dir : safe_directories) {
            if (file_path_str.find(safe_dir) != std::string::npos) {
                in_safe_directory = true;
                break;
            }
        }

        if (!in_safe_directory) {
            result.warnings.push_back(
                "Plugin is not located in a standard plugin directory");
        }

        // If we reach here, validation passed
        result.is_valid = true;
        result.validated_level = SecurityLevel::Basic;

        // Upgrade security level based on checks
        if (result.warnings.empty()) {
            result.validated_level = SecurityLevel::Standard;
        }

    } catch (const std::exception& e) {
        result.errors.push_back("Exception during permission validation: " +
                                std::string(e.what()));
    } catch (...) {
        result.errors.push_back(
            "Unknown exception during permission validation");
    }

    return result;
}

bool SecurityManager::is_safe_file_path(
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

    return true;
}

bool SecurityManager::has_valid_extension(
    const std::filesystem::path& file_path) const {
    auto extensions = get_allowed_extensions();
    std::string extension = file_path.extension().string();

    return std::find(extensions.begin(), extensions.end(), extension) !=
           extensions.end();
}

std::vector<std::string> SecurityManager::get_allowed_extensions() const {
    return {".dll", ".so", ".dylib", ".qtplugin"};
}

// Factory implementation

std::unique_ptr<ISecurityManager> SecurityManagerFactory::create_default() {
    return std::make_unique<SecurityManager>();
}

std::unique_ptr<SecurityManager> SecurityManagerFactory::create_with_level(
    SecurityLevel level) {
    auto manager = std::make_unique<SecurityManager>();
    manager->set_security_level(level);
    return manager;
}

// Utility functions

const char* security_level_to_string(SecurityLevel level) noexcept {
    switch (level) {
        case SecurityLevel::None:
            return "None";
        case SecurityLevel::Basic:
            return "Basic";
        case SecurityLevel::Standard:
            return "Standard";
        case SecurityLevel::Strict:
            return "Strict";
        case SecurityLevel::Maximum:
            return "Maximum";
    }
    return "Unknown";
}

SecurityLevel security_level_from_string(std::string_view str) noexcept {
    if (str == "None")
        return SecurityLevel::None;
    if (str == "Basic")
        return SecurityLevel::Basic;
    if (str == "Standard")
        return SecurityLevel::Standard;
    if (str == "Strict")
        return SecurityLevel::Strict;
    if (str == "Maximum")
        return SecurityLevel::Maximum;
    return SecurityLevel::None;
}

// === Enhanced Security Features Implementation ===

// Local sandbox implementation removed - using real PluginSandbox class

// Sandboxing implementation
void SecurityManager::set_sandboxing_enabled(bool enabled) {
    m_sandboxing_enabled = enabled;
    log_security_event("sandboxing_configuration_changed",
                      "system",
                      enabled ? "Sandboxing enabled" : "Sandboxing disabled",
                      SecurityLevel::Standard);
}

bool SecurityManager::is_sandboxing_enabled() const noexcept {
    return m_sandboxing_enabled;
}

qtplugin::expected<void, PluginError> SecurityManager::create_sandbox(
    std::string_view plugin_id,
    const std::vector<std::string>& permissions) {

    if (!m_sandboxing_enabled) {
        return make_error<void>(PluginErrorCode::SecurityViolation,
                               "Sandboxing is not enabled");
    }

    if (plugin_id.empty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                               "Plugin ID cannot be empty");
    }

    if (!validate_sandbox_permissions(permissions)) {
        return make_error<void>(PluginErrorCode::SecurityViolation,
                               "Invalid or dangerous permissions requested");
    }

    std::unique_lock lock(m_sandbox_mutex);

    // Check if sandbox already exists
    auto it = m_plugin_sandboxes.find(std::string(plugin_id));
    if (it != m_plugin_sandboxes.end() && it->second->is_active()) {
        return make_error<void>(PluginErrorCode::StateError,
                               "Sandbox already exists for plugin: " + std::string(plugin_id));
    }

    try {
        auto sandbox = create_plugin_sandbox(plugin_id, permissions);
        m_plugin_sandboxes[std::string(plugin_id)] = std::move(sandbox);

        log_security_event("sandbox_created",
                          std::string(plugin_id),
                          "Plugin sandbox created with " + std::to_string(permissions.size()) + " permissions",
                          SecurityLevel::Standard);

        return make_success();

    } catch (const std::exception& e) {
        return make_error<void>(PluginErrorCode::SystemError,
                               "Failed to create sandbox: " + std::string(e.what()));
    }
}

qtplugin::expected<void, PluginError> SecurityManager::destroy_sandbox(
    std::string_view plugin_id) {

    if (plugin_id.empty()) {
        return make_error<void>(PluginErrorCode::InvalidParameters,
                               "Plugin ID cannot be empty");
    }

    std::unique_lock lock(m_sandbox_mutex);

    auto it = m_plugin_sandboxes.find(std::string(plugin_id));
    if (it == m_plugin_sandboxes.end()) {
        return make_error<void>(PluginErrorCode::StateError,
                               "Sandbox not found for plugin: " + std::string(plugin_id));
    }

    it->second->shutdown();
    m_plugin_sandboxes.erase(it);

    log_security_event("sandbox_destroyed",
                      std::string(plugin_id),
                      "Plugin sandbox destroyed",
                      SecurityLevel::Basic);

    return make_success();
}

// Runtime monitoring implementation
void SecurityManager::start_runtime_monitoring(std::chrono::milliseconds interval) {
    if (m_runtime_monitoring_active) {
        return; // Already active
    }

    m_monitoring_interval = interval;
    m_runtime_monitoring_active = true;

    if (m_monitoring_timer) {
        m_monitoring_timer->start(static_cast<int>(interval.count()));
    }

    log_security_event("runtime_monitoring_started",
                      "system",
                      "Runtime security monitoring activated with " + std::to_string(interval.count()) + "ms interval",
                      SecurityLevel::Standard);
}

void SecurityManager::stop_runtime_monitoring() {
    if (!m_runtime_monitoring_active) {
        return; // Not active
    }

    m_runtime_monitoring_active = false;

    if (m_monitoring_timer) {
        m_monitoring_timer->stop();
    }

    log_security_event("runtime_monitoring_stopped",
                      "system",
                      "Runtime security monitoring deactivated",
                      SecurityLevel::Basic);
}

bool SecurityManager::is_runtime_monitoring_active() const noexcept {
    return m_runtime_monitoring_active;
}

SecurityValidationResult SecurityManager::monitor_plugin_behavior(
    std::string_view plugin_id) const {

    SecurityValidationResult result;
    result.is_valid = true;
    result.validated_level = SecurityLevel::Basic;

    try {
        // Check resource usage
        auto resource_result = check_resource_usage(plugin_id);
        if (!resource_result.is_valid) {
            result.errors.insert(result.errors.end(),
                                resource_result.errors.begin(),
                                resource_result.errors.end());
            result.is_valid = false;
        }
        result.warnings.insert(result.warnings.end(),
                              resource_result.warnings.begin(),
                              resource_result.warnings.end());

        // Get cached behavior data
        std::shared_lock lock(m_monitoring_mutex);
        auto cache_it = m_runtime_behavior_cache.find(std::string(plugin_id));
        if (cache_it != m_runtime_behavior_cache.end()) {

            // Check for privilege escalation attempts
            auto privilege_result = detect_privilege_escalation(plugin_id, cache_it->second);
            if (!privilege_result.is_valid) {
                result.errors.insert(result.errors.end(),
                                    privilege_result.errors.begin(),
                                    privilege_result.errors.end());
                result.is_valid = false;
                m_violations_detected.fetch_add(1);
            }
            result.warnings.insert(result.warnings.end(),
                                  privilege_result.warnings.begin(),
                                  privilege_result.warnings.end());
        }

        if (result.is_valid) {
            result.validated_level = SecurityLevel::Standard;
        }

    } catch (const std::exception& e) {
        result.is_valid = false;
        result.errors.push_back("Exception during behavior monitoring: " + std::string(e.what()));
    }

    return result;
}

// Audit logging implementation
void SecurityManager::set_audit_logging_enabled(
    bool enabled,
    const std::filesystem::path& log_file_path) {

    m_audit_logging_enabled = enabled;

    if (enabled && !log_file_path.empty()) {
        m_audit_log_file = log_file_path;
    }

    log_security_event("audit_logging_configuration_changed",
                      "system",
                      enabled ? "Audit logging enabled" : "Audit logging disabled",
                      SecurityLevel::Standard);
}

bool SecurityManager::is_audit_logging_enabled() const noexcept {
    return m_audit_logging_enabled;
}

QJsonArray SecurityManager::get_audit_log(size_t limit) const {
    std::shared_lock lock(m_audit_log_mutex);

    QJsonArray result;

    size_t start_index = 0;
    if (limit > 0 && m_audit_log_entries.size() > limit) {
        start_index = m_audit_log_entries.size() - limit;
    }

    for (size_t i = start_index; i < m_audit_log_entries.size(); ++i) {
        result.append(m_audit_log_entries[i]);
    }

    return result;
}

void SecurityManager::clear_audit_log() {
    std::unique_lock lock(m_audit_log_mutex);
    m_audit_log_entries.clear();

    log_security_event("audit_log_cleared",
                      "system",
                      "Security audit log cleared",
                      SecurityLevel::Basic);
}

SecurityValidationResult SecurityManager::validate_runtime_behavior(
    std::string_view plugin_id,
    const QJsonObject& behavior_data) const {

    SecurityValidationResult result;
    result.is_valid = true;
    result.validated_level = SecurityLevel::Basic;

    try {
        // Store behavior data in cache
        {
            std::unique_lock lock(m_monitoring_mutex);
            m_runtime_behavior_cache[std::string(plugin_id)] = behavior_data;
        }

        // Validate behavior patterns
        if (behavior_data.contains("suspicious_activity") &&
            behavior_data["suspicious_activity"].toBool()) {
            result.warnings.push_back("Suspicious activity detected in plugin behavior");
        }

        if (behavior_data.contains("resource_usage")) {
            QJsonObject resource_usage = behavior_data["resource_usage"].toObject();

            // Check CPU usage
            if (resource_usage.contains("cpu_percent")) {
                double cpu_usage = resource_usage["cpu_percent"].toDouble();
                if (cpu_usage > 80.0) {
                    result.warnings.push_back(("High CPU usage detected: " + QString::number(cpu_usage) + "%").toStdString());
                }
                if (cpu_usage > 95.0) {
                    result.errors.push_back(("Excessive CPU usage: " + QString::number(cpu_usage) + "%").toStdString());
                    result.is_valid = false;
                }
            }

            // Check memory usage
            if (resource_usage.contains("memory_mb")) {
                double memory_usage = resource_usage["memory_mb"].toDouble();
                if (memory_usage > 500.0) {
                    result.warnings.push_back(("High memory usage detected: " + QString::number(memory_usage) + "MB").toStdString());
                }
                if (memory_usage > 1000.0) {
                    result.errors.push_back(("Excessive memory usage: " + QString::number(memory_usage) + "MB").toStdString());
                    result.is_valid = false;
                }
            }
        }

        // Check for permission violations
        if (behavior_data.contains("permission_violations")) {
            QJsonArray violations = behavior_data["permission_violations"].toArray();
            for (const auto& violation : violations) {
                result.errors.push_back("Permission violation: " + violation.toString().toStdString());
                result.is_valid = false;
                m_violations_detected.fetch_add(1);
            }
        }

        if (result.is_valid && result.warnings.empty()) {
            result.validated_level = SecurityLevel::Standard;
        }

    } catch (const std::exception& e) {
        result.is_valid = false;
        result.errors.push_back("Exception during runtime behavior validation: " + std::string(e.what()));
    }

    return result;
}

// Private helper methods implementation
void SecurityManager::log_security_event(
    const std::string& event_type,
    const std::string& plugin_id,
    const std::string& details,
    SecurityLevel severity) const {

    if (!m_audit_logging_enabled) {
        return;
    }

    QJsonObject log_entry;
    log_entry["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    log_entry["event_type"] = QString::fromStdString(event_type);
    log_entry["plugin_id"] = QString::fromStdString(plugin_id);
    log_entry["details"] = QString::fromStdString(details);
    log_entry["severity"] = security_level_to_string(severity);
    log_entry["security_level"] = static_cast<int>(m_security_level);

    std::unique_lock lock(m_audit_log_mutex);

    // Add to memory log
    m_audit_log_entries.push_back(log_entry);

    // Maintain log size limit
    if (m_audit_log_entries.size() > MAX_AUDIT_LOG_SIZE) {
        m_audit_log_entries.erase(m_audit_log_entries.begin());
    }

    // Write to file if configured
    if (!m_audit_log_file.empty()) {
        try {
            std::ofstream log_file(m_audit_log_file, std::ios::app);
            if (log_file.is_open()) {
                QJsonDocument doc(log_entry);
                log_file << doc.toJson(QJsonDocument::Compact).toStdString() << std::endl;
            }
        } catch (const std::exception&) {
            // Ignore file writing errors to prevent recursive logging
        }
    }
}

SecurityValidationResult SecurityManager::perform_threat_analysis(
    const std::filesystem::path& file_path) const {

    SecurityValidationResult result;
    result.is_valid = true;
    result.validated_level = SecurityLevel::Basic;

    try {
        // Basic threat detection patterns
        std::vector<std::string> threat_signatures = {
            "CreateProcess", "WriteProcessMemory", "VirtualAlloc",
            "LoadLibrary", "GetProcAddress", "ShellExecute"
        };

        // Read file content for basic pattern matching
        std::ifstream file(file_path, std::ios::binary);
        if (file.is_open()) {
            std::string content((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());

            for (const auto& signature : threat_signatures) {
                if (content.find(signature) != std::string::npos) {
                    result.warnings.push_back("Potentially dangerous API call detected: " + signature);
                }
            }
        }

        result.validated_level = SecurityLevel::Standard;

    } catch (const std::exception& e) {
        result.warnings.push_back("Threat analysis failed: " + std::string(e.what()));
    }

    return result;
}

SecurityValidationResult SecurityManager::check_resource_usage(
    std::string_view plugin_id) const {

    SecurityValidationResult result;
    result.is_valid = true;
    result.validated_level = SecurityLevel::Basic;

    // This is a simplified implementation
    // In a real system, you would integrate with OS-level resource monitoring
    result.details["resource_check"] = "basic";
    result.details["plugin_id"] = QString::fromStdString(std::string(plugin_id));

    return result;
}

SecurityValidationResult SecurityManager::detect_privilege_escalation(
    std::string_view plugin_id,
    const QJsonObject& behavior_data) const {

    Q_UNUSED(plugin_id)  // TODO: Implement privilege escalation detection
    Q_UNUSED(behavior_data)

    SecurityValidationResult result;
    result.is_valid = true;
    result.validated_level = SecurityLevel::Basic;

    // Check for common privilege escalation patterns
    if (behavior_data.contains("process_operations")) {
        QJsonArray operations = behavior_data["process_operations"].toArray();

        for (const auto& op : operations) {
            if (op.isString()) {
                std::string operation = op.toString().toStdString();

                if (operation.find("elevated") != std::string::npos ||
                    operation.find("admin") != std::string::npos ||
                    operation.find("root") != std::string::npos) {
                    result.errors.push_back("Privilege escalation attempt detected: " + operation);
                    result.is_valid = false;
                }
            }
        }
    }

    return result;
}

void SecurityManager::initialize_monitoring_timer() {
    // Note: QTimer initialization would typically require a QObject parent
    // This is a simplified implementation
    // m_monitoring_timer = std::make_unique<QTimer>(this);
    // connect(m_monitoring_timer.get(), &QTimer::timeout, this, &SecurityManager::on_monitoring_timeout);
}

void SecurityManager::on_monitoring_timeout() {
    if (!m_runtime_monitoring_active) {
        return;
    }

    // Monitor all active plugins
    std::shared_lock sandbox_lock(m_sandbox_mutex);
    for (const auto& [plugin_id, sandbox] : m_plugin_sandboxes) {
        if (sandbox->is_active()) {
            auto result = monitor_plugin_behavior(plugin_id);
            if (!result.is_valid) {
                log_security_event("security_violation_detected",
                                  plugin_id,
                                  "Runtime monitoring detected security violations",
                                  SecurityLevel::Strict);
            }
        }
    }
}

std::unique_ptr<PluginSandbox> SecurityManager::create_plugin_sandbox(
    std::string_view plugin_id,
    const std::vector<std::string>& permissions) {

    // Create security policy from permissions
    SecurityPolicy policy;
    policy.policy_name = QString::fromStdString(std::string(plugin_id) + "_policy");
    policy.description = QString("Security policy for plugin: %1").arg(QString::fromStdString(std::string(plugin_id)));

    // Convert permissions to SecurityPermissions
    for (const auto& perm : permissions) {
        if (perm == "file_read") policy.permissions.allow_file_system_read = true;
        else if (perm == "file_write") policy.permissions.allow_file_system_write = true;
        else if (perm == "network_access") policy.permissions.allow_network_access = true;
        else if (perm == "system_access") policy.permissions.allow_system_calls = true;
        else if (perm == "process_creation") policy.permissions.allow_process_creation = true;
    }

    return std::make_unique<PluginSandbox>(policy);
}

bool SecurityManager::validate_sandbox_permissions(
    const std::vector<std::string>& permissions) const {

    // Define dangerous permissions that require special handling
    std::vector<std::string> dangerous_permissions = {
        "system_admin", "file_system_root", "process_create_admin",
        "registry_write_system", "network_unrestricted"
    };

    for (const auto& permission : permissions) {
        for (const auto& dangerous : dangerous_permissions) {
            if (permission == dangerous) {
                if (m_security_level < SecurityLevel::Maximum) {
                    return false; // Dangerous permission not allowed at current security level
                }
            }
        }
    }

    return true;
}

}  // namespace qtplugin
