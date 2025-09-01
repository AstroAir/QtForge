/**
 * @file security_plugin.hpp
 * @brief Security plugin demonstrating QtForge security features
 * @version 3.0.0
 *
 * This plugin demonstrates comprehensive security functionality including:
 * - Plugin validation and signature verification
 * - Permission management and security policies
 * - Trust levels and security monitoring
 * - Secure communication patterns
 */

#pragma once

#include <QJsonObject>
#include <QMutex>
#include <QObject>
#include <QReadWriteLock>
#include <QTimer>
#include <atomic>
#include <memory>
#include <unordered_map>
#include <vector>

#include "qtplugin/communication/message_bus.hpp"
#include "qtplugin/core/plugin_interface.hpp"
#include "qtplugin/security/components/permission_manager.hpp"
#include "qtplugin/security/components/security_policy_engine.hpp"
#include "qtplugin/security/components/security_validator.hpp"
#include "qtplugin/security/components/signature_verifier.hpp"
#include "qtplugin/security/security_manager.hpp"
#include "qtplugin/utils/error_handling.hpp"

/**
 * @brief Security plugin demonstrating QtForge security features
 *
 * This plugin showcases:
 * - Security validation and verification
 * - Permission management
 * - Trust level enforcement
 * - Secure plugin loading
 * - Security monitoring and auditing
 */
class SecurityPlugin : public QObject, public qtplugin::IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.SecurityPlugin" FILE "security_plugin.json")
    Q_INTERFACES(qtplugin::IPlugin)

public:
    explicit SecurityPlugin(QObject* parent = nullptr);
    ~SecurityPlugin() override;

    // === IPlugin Interface ===
    // Basic plugin information
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    qtplugin::Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;
    qtplugin::PluginState state() const noexcept override;

    // Lifecycle methods
    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    bool is_initialized() const noexcept override;

    qtplugin::PluginMetadata metadata() const override;
    qtplugin::PluginCapabilities capabilities() const noexcept override;
    qtplugin::PluginPriority priority() const noexcept override;
    bool is_thread_safe() const noexcept override;
    std::string_view thread_model() const noexcept override;

    // === Configuration Management ===
    std::optional<QJsonObject> default_configuration() const override;
    qtplugin::expected<void, qtplugin::PluginError> configure(
        const QJsonObject& config) override;
    QJsonObject current_configuration() const override;
    bool validate_configuration(const QJsonObject& config) const override;

    // === Command Execution ===
    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params) override;
    std::vector<std::string> available_commands() const override;

    // === Lifecycle Management ===
    qtplugin::expected<void, qtplugin::PluginError> pause() override;
    qtplugin::expected<void, qtplugin::PluginError> resume() override;
    qtplugin::expected<void, qtplugin::PluginError> restart() override;

    // === Dependency Management ===
    std::vector<std::string> dependencies() const override;
    std::vector<std::string> optional_dependencies() const override;
    bool dependencies_satisfied() const override;

    // === Monitoring ===
    std::chrono::milliseconds uptime() const override;
    QJsonObject performance_metrics() const override;
    QJsonObject resource_usage() const override;
    void clear_errors() override;

    // === Security-Specific Methods ===

    /**
     * @brief Validate a plugin file using security manager
     * @param file_path Path to plugin file
     * @param required_level Required security level
     * @return Validation result
     */
    qtplugin::SecurityValidationResult validate_plugin_file(
        const QString& file_path, qtplugin::SecurityLevel required_level =
                                      qtplugin::SecurityLevel::Standard);

    /**
     * @brief Check permissions for a specific operation
     * @param operation Operation to check
     * @param context Security context
     * @return Permission check result
     */
    bool check_permission(const QString& operation,
                          const QJsonObject& context = {});

    /**
     * @brief Set security policy for the plugin
     * @param policy_name Policy name
     * @param policy_config Policy configuration
     * @return Success or error
     */
    qtplugin::expected<void, qtplugin::PluginError> set_security_policy(
        const QString& policy_name, const QJsonObject& policy_config);

    /**
     * @brief Get current security status
     * @return Security status information
     */
    QJsonObject get_security_status() const;

    /**
     * @brief Audit security events
     * @param event_type Type of security event
     * @param details Event details
     */
    void audit_security_event(const QString& event_type,
                              const QJsonObject& details);

private slots:
    void on_security_timer_timeout();
    void on_security_event_received();

private:
    // === Security Components ===
    std::unique_ptr<qtplugin::ISecurityManager> m_security_manager;
    std::unique_ptr<qtplugin::IPermissionManager> m_permission_manager;
    std::unique_ptr<qtplugin::ISecurityValidator> m_security_validator;
    std::unique_ptr<qtplugin::ISignatureVerifier> m_signature_verifier;
    std::unique_ptr<qtplugin::ISecurityPolicyEngine> m_policy_engine;

    // === State Management ===
    std::atomic<qtplugin::PluginState> m_state{qtplugin::PluginState::Unloaded};
    std::atomic<bool> m_dependencies_satisfied{false};
    mutable QReadWriteLock m_state_mutex;

    // === Configuration ===
    QJsonObject m_configuration;
    qtplugin::SecurityLevel m_security_level{qtplugin::SecurityLevel::Standard};
    bool m_audit_enabled{true};
    bool m_strict_validation{false};
    int m_security_check_interval{30000};  // 30 seconds

    // === Monitoring ===
    std::unique_ptr<QTimer> m_security_timer;
    std::chrono::system_clock::time_point m_initialization_time;

    // === Metrics ===
    mutable QMutex m_metrics_mutex;
    std::atomic<uint64_t> m_validation_count{0};
    std::atomic<uint64_t> m_permission_checks{0};
    std::atomic<uint64_t> m_security_violations{0};
    std::atomic<uint64_t> m_audit_events{0};

    // === Security Audit ===
    mutable QMutex m_audit_mutex;
    std::vector<QJsonObject> m_audit_log;
    static constexpr size_t MAX_AUDIT_LOG_SIZE = 1000;

    // === Dependencies ===
    std::vector<std::string> m_required_dependencies;
    std::vector<std::string> m_optional_dependencies;

    // === Error Handling ===
    mutable QMutex m_error_mutex;
    std::vector<std::string> m_error_log;
    std::string m_last_error;
    std::atomic<uint64_t> m_error_count{0};
    static constexpr size_t MAX_ERROR_LOG_SIZE = 100;

    // === Command Handlers ===
    QJsonObject handle_validate_command(const QJsonObject& params);
    QJsonObject handle_permission_command(const QJsonObject& params);
    QJsonObject handle_policy_command(const QJsonObject& params);
    QJsonObject handle_audit_command(const QJsonObject& params);
    QJsonObject handle_status_command(const QJsonObject& params);
    QJsonObject handle_security_test_command(const QJsonObject& params);

    // === Helper Methods ===
    void log_error(const std::string& error);
    void log_info(const std::string& message);
    void update_metrics();
    void initialize_security_components();
    void setup_default_policies();
    void start_security_monitoring();
    void stop_security_monitoring();

public:
    // === Plugin Factory ===
    static std::unique_ptr<SecurityPlugin> create_instance();
    static qtplugin::PluginMetadata get_static_metadata();
};
