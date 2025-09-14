/**
 * @file security_manager.hpp
 * @brief Security manager for plugin validation and sandboxing
 * @version 3.0.0
 */

#pragma once

#include <QJsonObject>
#include <QObject>
#include <QTimer>
#include <atomic>
#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "../utils/error_handling.hpp"

class QJsonObject;

namespace qtplugin {

// Forward declarations
class ISecurityValidator;
class ISignatureVerifier;
class IPermissionManager;
class ISecurityPolicyEngine;

/**
 * @brief Security levels for plugin validation
 */
enum class SecurityLevel {
    None = 0,        ///< No security validation
    Basic = 1,       ///< Basic file and metadata validation
    Standard = 2,    ///< Standard security checks including signatures
    Moderate = 2,    ///< Alias for Standard (for backward compatibility)
    Strict = 3,      ///< Strict validation with sandboxing
    Permissive = 1,  ///< Alias for Basic (for backward compatibility)
    Maximum = 4      ///< Maximum security with full isolation
};

/**
 * @brief Security validation result
 */
struct SecurityValidationResult {
    bool is_valid = false;
    SecurityLevel validated_level = SecurityLevel::None;
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
    QJsonObject details;

    /**
     * @brief Check if validation passed
     */
    bool passed() const noexcept { return is_valid; }

    /**
     * @brief Check if there are warnings
     */
    bool has_warnings() const noexcept { return !warnings.empty(); }

    /**
     * @brief Check if there are errors
     */
    bool has_errors() const noexcept { return !errors.empty(); }
};

/**
 * @brief Security manager interface
 */
class ISecurityManager {
public:
    virtual ~ISecurityManager() = default;

    /**
     * @brief Validate plugin security
     * @param file_path Path to plugin file
     * @param required_level Required security level
     * @return Validation result
     */
    virtual SecurityValidationResult validate_plugin(
        const std::filesystem::path& file_path,
        SecurityLevel required_level) = 0;

    /**
     * @brief Check if plugin is trusted
     * @param plugin_id Plugin identifier
     * @return true if plugin is trusted
     */
    virtual bool is_trusted(std::string_view plugin_id) const = 0;

    /**
     * @brief Add plugin to trusted list
     * @param plugin_id Plugin identifier
     * @param trust_level Trust level to assign
     */
    virtual void add_trusted_plugin(std::string_view plugin_id,
                                    SecurityLevel trust_level) = 0;

    /**
     * @brief Remove plugin from trusted list
     * @param plugin_id Plugin identifier
     */
    virtual void remove_trusted_plugin(std::string_view plugin_id) = 0;

    /**
     * @brief Get current security level
     * @return Current security level
     */
    virtual SecurityLevel security_level() const noexcept = 0;

    /**
     * @brief Set security level
     * @param level Security level to set
     */
    virtual void set_security_level(SecurityLevel level) = 0;

    /**
     * @brief Get security statistics
     * @return Security statistics as JSON
     */
    virtual QJsonObject security_statistics() const = 0;
};

/**
 * @brief Default security manager implementation
 */
class SecurityManager : public ISecurityManager {
public:
    SecurityManager();
    ~SecurityManager() override;

    // ISecurityManager implementation
    SecurityValidationResult validate_plugin(
        const std::filesystem::path& file_path,
        SecurityLevel required_level) override;
    bool is_trusted(std::string_view plugin_id) const override;
    void add_trusted_plugin(std::string_view plugin_id,
                            SecurityLevel trust_level) override;
    void remove_trusted_plugin(std::string_view plugin_id) override;
    SecurityLevel security_level() const noexcept override;
    SecurityLevel get_security_level() const noexcept {
        return security_level();
    }
    void set_security_level(SecurityLevel level) override;
    QJsonObject security_statistics() const override;

    // Additional getter methods for testing
    uint64_t get_validations_performed() const noexcept {
        return m_validations_performed.load();
    }
    uint64_t get_violations_detected() const noexcept {
        return m_violations_detected.load();
    }

    // Methods for testing (normally private)
    SecurityValidationResult validate_metadata(
        const std::filesystem::path& file_path) const;
    SecurityValidationResult validate_signature(
        const std::filesystem::path& file_path) const;
    bool is_safe_file_path(const std::filesystem::path& file_path) const;

    /**
     * @brief Load trusted plugins list from file
     * @param file_path Path to trusted plugins file
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> load_trusted_plugins(
        const std::filesystem::path& file_path);

    /**
     * @brief Save trusted plugins list to file
     * @param file_path Path to save trusted plugins file
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> save_trusted_plugins(
        const std::filesystem::path& file_path) const;

    /**
     * @brief Enable or disable signature verification
     * @param enabled true to enable signature verification
     */
    void set_signature_verification_enabled(bool enabled);

    /**
     * @brief Check if signature verification is enabled
     * @return true if signature verification is enabled
     */
    bool is_signature_verification_enabled() const noexcept;

    // === Enhanced Security Features ===

    /**
     * @brief Enable plugin sandboxing
     * @param enabled true to enable sandboxing
     */
    void set_sandboxing_enabled(bool enabled);

    /**
     * @brief Check if sandboxing is enabled
     * @return true if sandboxing is enabled
     */
    bool is_sandboxing_enabled() const noexcept;

    /**
     * @brief Create sandbox environment for plugin
     * @param plugin_id Plugin identifier
     * @param permissions Permissions to grant to the plugin
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> create_sandbox(
        std::string_view plugin_id,
        const std::vector<std::string>& permissions);

    /**
     * @brief Destroy sandbox environment for plugin
     * @param plugin_id Plugin identifier
     * @return Success or error information
     */
    qtplugin::expected<void, PluginError> destroy_sandbox(
        std::string_view plugin_id);

    /**
     * @brief Start runtime security monitoring
     * @param interval Monitoring interval in milliseconds
     */
    void start_runtime_monitoring(std::chrono::milliseconds interval = std::chrono::seconds(30));

    /**
     * @brief Stop runtime security monitoring
     */
    void stop_runtime_monitoring();

    /**
     * @brief Check if runtime monitoring is active
     * @return true if monitoring is active
     */
    bool is_runtime_monitoring_active() const noexcept;

    /**
     * @brief Monitor plugin runtime behavior
     * @param plugin_id Plugin identifier
     * @return Security assessment result
     */
    SecurityValidationResult monitor_plugin_behavior(
        std::string_view plugin_id) const;

    /**
     * @brief Enable security audit logging
     * @param enabled true to enable audit logging
     * @param log_file_path Path to audit log file (optional)
     */
    void set_audit_logging_enabled(
        bool enabled,
        const std::filesystem::path& log_file_path = {});

    /**
     * @brief Check if audit logging is enabled
     * @return true if audit logging is enabled
     */
    bool is_audit_logging_enabled() const noexcept;

    /**
     * @brief Get security audit log entries
     * @param limit Maximum number of entries to return (0 for all)
     * @return JSON array of audit log entries
     */
    QJsonArray get_audit_log(size_t limit = 100) const;

    /**
     * @brief Clear security audit log
     */
    void clear_audit_log();

    /**
     * @brief Validate plugin at runtime
     * @param plugin_id Plugin identifier
     * @param behavior_data Runtime behavior data
     * @return Validation result
     */
    SecurityValidationResult validate_runtime_behavior(
        std::string_view plugin_id,
        const QJsonObject& behavior_data) const;

private:
    // Security components
    std::unique_ptr<ISecurityValidator> m_validator;
    std::unique_ptr<ISignatureVerifier> m_signature_verifier;
    std::unique_ptr<IPermissionManager> m_permission_manager;
    std::unique_ptr<ISecurityPolicyEngine> m_policy_engine;

    SecurityLevel m_security_level = SecurityLevel::Basic;
    bool m_signature_verification_enabled = false;

    mutable std::shared_mutex m_trusted_plugins_mutex;
    std::unordered_map<std::string, SecurityLevel> m_trusted_plugins;

    // Statistics
    mutable std::atomic<uint64_t> m_validations_performed{0};
    mutable std::atomic<uint64_t> m_validations_passed{0};
    mutable std::atomic<uint64_t> m_validations_failed{0};
    mutable std::atomic<uint64_t> m_violations_detected{0};

    // Validation methods
    SecurityValidationResult validate_file_integrity(
        const std::filesystem::path& file_path) const;
    SecurityValidationResult validate_permissions(
        const std::filesystem::path& file_path) const;

    // Helper methods
    bool has_valid_extension(const std::filesystem::path& file_path) const;
    std::vector<std::string> get_allowed_extensions() const;

    // === Enhanced Security Features - Private Members ===

    // Sandbox management
    bool m_sandboxing_enabled = false;
    mutable std::shared_mutex m_sandbox_mutex;
    std::unordered_map<std::string, std::unique_ptr<class PluginSandbox>> m_plugin_sandboxes;

    // Runtime monitoring
    bool m_runtime_monitoring_active = false;
    std::unique_ptr<QTimer> m_monitoring_timer;
    std::chrono::milliseconds m_monitoring_interval{30000};
    mutable std::shared_mutex m_monitoring_mutex;
    mutable std::unordered_map<std::string, QJsonObject> m_runtime_behavior_cache;

    // Audit logging
    bool m_audit_logging_enabled = false;
    std::filesystem::path m_audit_log_file;
    mutable std::shared_mutex m_audit_log_mutex;
    mutable std::vector<QJsonObject> m_audit_log_entries;
    static constexpr size_t MAX_AUDIT_LOG_SIZE = 10000;

    // Enhanced validation methods
    void log_security_event(
        const std::string& event_type,
        const std::string& plugin_id,
        const std::string& details,
        SecurityLevel severity = SecurityLevel::Basic) const;

    SecurityValidationResult perform_threat_analysis(
        const std::filesystem::path& file_path) const;

    SecurityValidationResult check_resource_usage(
        std::string_view plugin_id) const;

    SecurityValidationResult detect_privilege_escalation(
        std::string_view plugin_id,
        const QJsonObject& behavior_data) const;

    void initialize_monitoring_timer();
    void on_monitoring_timeout();

    // Sandbox helper methods
    std::unique_ptr<class PluginSandbox> create_plugin_sandbox(
        std::string_view plugin_id,
        const std::vector<std::string>& permissions);

    bool validate_sandbox_permissions(
        const std::vector<std::string>& permissions) const;
};

/**
 * @brief Security manager factory
 */
class SecurityManagerFactory {
public:
    /**
     * @brief Create default security manager
     * @return Unique pointer to security manager
     */
    static std::unique_ptr<ISecurityManager> create_default();

    /**
     * @brief Create security manager with specific level
     * @param level Initial security level
     * @return Unique pointer to security manager
     */
    static std::unique_ptr<SecurityManager> create_with_level(
        SecurityLevel level);
};

/**
 * @brief Convert security level to string
 * @param level Security level
 * @return String representation
 */
const char* security_level_to_string(SecurityLevel level) noexcept;

/**
 * @brief Parse security level from string
 * @param str String representation
 * @return Security level, or None if invalid
 */
SecurityLevel security_level_from_string(std::string_view str) noexcept;

}  // namespace qtplugin
