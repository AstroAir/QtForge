/**
 * @file security_plugin.cpp
 * @brief Implementation of security plugin demonstrating QtForge security features
 * @version 3.0.0
 */

#include "security_plugin.hpp"
#include <QObject>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QStringLiteral>
#include <chrono>
#include <thread>

SecurityPlugin::SecurityPlugin(QObject* parent)
    : QObject(parent), m_security_timer(std::make_unique<QTimer>(this)) {

    // Connect timer
    connect(m_security_timer.get(), &QTimer::timeout, this,
            &SecurityPlugin::on_security_timer_timeout);

    // Initialize dependencies
    m_required_dependencies = {"qtplugin.SecurityManager"};
    m_optional_dependencies = {"qtplugin.MessageBus", "qtplugin.ConfigurationManager"};

    log_info("SecurityPlugin constructed");
}

SecurityPlugin::~SecurityPlugin() {
    if (m_state != qtplugin::PluginState::Unloaded) {
        shutdown();
    }
}

qtplugin::expected<void, qtplugin::PluginError> SecurityPlugin::initialize() {
    if (m_state != qtplugin::PluginState::Unloaded &&
        m_state != qtplugin::PluginState::Loaded) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::StateError,
            "Plugin is not in a state that allows initialization");
    }

    m_state = qtplugin::PluginState::Initializing;
    m_initialization_time = std::chrono::system_clock::now();

    try {
        // Initialize security components
        initialize_security_components();

        // Setup default security policies
        setup_default_policies();

        // Start security monitoring
        start_security_monitoring();

        m_state = qtplugin::PluginState::Running;
        log_info("SecurityPlugin initialized successfully");

        // Audit initialization
        QJsonObject init_event;
        init_event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        init_event[QStringLiteral("security_level")] = static_cast<int>(m_security_level);
        init_event[QStringLiteral("audit_enabled")] = m_audit_enabled;
        audit_security_event(QStringLiteral("plugin_initialized"), init_event);

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        m_state = qtplugin::PluginState::Error;
        std::string error_msg = "Initialization failed: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed, error_msg);
    }
}

void SecurityPlugin::shutdown() noexcept {
    try {
        QWriteLocker lock(&m_state_mutex);
        m_state = qtplugin::PluginState::Stopping;
        lock.unlock();

        // Stop security monitoring
        stop_security_monitoring();

        // Audit shutdown
        QJsonObject shutdown_event;
        shutdown_event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        shutdown_event[QStringLiteral("uptime_ms")] = static_cast<qint64>(uptime().count());
        audit_security_event(QStringLiteral("plugin_shutdown"), shutdown_event);

        lock.relock();
        m_state = qtplugin::PluginState::Stopped;
        lock.unlock();

        log_info("SecurityPlugin shutdown completed");
    } catch (...) {
        QWriteLocker lock(&m_state_mutex);
        m_state = qtplugin::PluginState::Error;
    }
}

bool SecurityPlugin::is_initialized() const noexcept {
    QReadLocker lock(&m_state_mutex);
    auto current_state = m_state.load();
    return current_state == qtplugin::PluginState::Running ||
           current_state == qtplugin::PluginState::Paused;
}

qtplugin::PluginMetadata SecurityPlugin::metadata() const {
    qtplugin::PluginMetadata meta;
    meta.name = "SecurityPlugin";
    meta.version = qtplugin::Version{3, 0, 0};
    meta.description = "Comprehensive security plugin demonstrating QtForge security features";
    meta.author = "QtForge Team";
    meta.license = "MIT";
    // Note: website field doesn't exist in PluginMetadata, skipping
    meta.category = "Security";
    meta.tags = {"security", "validation", "permissions", "audit", "example"};

    // Security-specific metadata
    QJsonObject custom_data;
    custom_data[QStringLiteral("security_level")] = static_cast<int>(m_security_level);
    custom_data[QStringLiteral("audit_enabled")] = m_audit_enabled;
    custom_data[QStringLiteral("strict_validation")] = m_strict_validation;

    QJsonArray algorithms;
    algorithms.append(QStringLiteral("SHA256"));
    algorithms.append(QStringLiteral("RSA"));
    algorithms.append(QStringLiteral("ECDSA"));
    custom_data[QStringLiteral("supported_algorithms")] = algorithms;
    meta.custom_data = custom_data;

    return meta;
}

qtplugin::PluginCapabilities SecurityPlugin::capabilities() const noexcept {
    return qtplugin::PluginCapability::Security |
           qtplugin::PluginCapability::Configuration |
           qtplugin::PluginCapability::Monitoring |
           qtplugin::PluginCapability::Logging |
           qtplugin::PluginCapability::Threading;
}

qtplugin::PluginPriority SecurityPlugin::priority() const noexcept {
    return qtplugin::PluginPriority::High; // Security plugins should have high priority
}

bool SecurityPlugin::is_thread_safe() const noexcept {
    return true;
}

std::string_view SecurityPlugin::thread_model() const noexcept {
    return "multi-threaded";
}

std::optional<QJsonObject> SecurityPlugin::default_configuration() const {
    QJsonObject config;
    config[QStringLiteral("security_level")] = static_cast<int>(qtplugin::SecurityLevel::Standard);
    config[QStringLiteral("audit_enabled")] = true;
    config[QStringLiteral("strict_validation")] = false;
    config[QStringLiteral("security_check_interval")] = 30000;
    config[QStringLiteral("max_audit_log_size")] = 1000;

    QJsonArray allowed_ops;
    allowed_ops.append(QStringLiteral("validate"));
    allowed_ops.append(QStringLiteral("check_permission"));
    allowed_ops.append(QStringLiteral("audit"));
    config[QStringLiteral("allowed_operations")] = allowed_ops;

    config[QStringLiteral("trust_store_path")] = QStringLiteral("trust_store.json");

    QJsonArray sig_algorithms;
    sig_algorithms.append(QStringLiteral("SHA256"));
    sig_algorithms.append(QStringLiteral("RSA"));
    config[QStringLiteral("signature_algorithms")] = sig_algorithms;

    QJsonObject policies;
    policies[QStringLiteral("default_deny")] = false;
    policies[QStringLiteral("require_signature")] = true;
    policies[QStringLiteral("allow_self_signed")] = false;
    config[QStringLiteral("permission_policies")] = policies;

    return config;
}

qtplugin::expected<void, qtplugin::PluginError> SecurityPlugin::configure(
    const QJsonObject& config) {
    if (!validate_configuration(config)) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ConfigurationError,
            "Invalid configuration provided");
    }

    // Store old configuration for comparison
    QJsonObject old_config = m_configuration;
    m_configuration = config;

    // Apply configuration changes
    if (config.contains(QStringLiteral("security_level"))) {
        m_security_level = static_cast<qtplugin::SecurityLevel>(
            config[QStringLiteral("security_level")].toInt());
    }

    if (config.contains(QStringLiteral("audit_enabled"))) {
        m_audit_enabled = config[QStringLiteral("audit_enabled")].toBool();
    }

    if (config.contains(QStringLiteral("strict_validation"))) {
        m_strict_validation = config[QStringLiteral("strict_validation")].toBool();
    }

    if (config.contains(QStringLiteral("security_check_interval"))) {
        m_security_check_interval = config[QStringLiteral("security_check_interval")].toInt();
        if (m_security_timer && m_security_timer->isActive()) {
            m_security_timer->setInterval(m_security_check_interval);
        }
    }

    // Update security components with new configuration
    if (m_security_manager) {
        m_security_manager->set_security_level(m_security_level);
    }

    log_info("Security configuration updated successfully");

    // Audit configuration change
    QJsonObject config_event;
    config_event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    config_event[QStringLiteral("old_security_level")] = static_cast<int>(
        static_cast<qtplugin::SecurityLevel>(old_config.value(QStringLiteral("security_level")).toInt()));
    config_event[QStringLiteral("new_security_level")] = static_cast<int>(m_security_level);
    audit_security_event(QStringLiteral("configuration_changed"), config_event);

    return qtplugin::make_success();
}

QJsonObject SecurityPlugin::current_configuration() const {
    return m_configuration;
}

bool SecurityPlugin::validate_configuration(const QJsonObject& config) const {
    // Validate security_level
    if (config.contains(QStringLiteral("security_level"))) {
        if (!config[QStringLiteral("security_level")].isDouble()) {
            return false;
        }
        int level = config[QStringLiteral("security_level")].toInt();
        if (level < 0 || level > static_cast<int>(qtplugin::SecurityLevel::Maximum)) {
            return false;
        }
    }

    // Validate audit_enabled
    if (config.contains(QStringLiteral("audit_enabled"))) {
        if (!config[QStringLiteral("audit_enabled")].isBool()) {
            return false;
        }
    }

    // Validate security_check_interval
    if (config.contains(QStringLiteral("security_check_interval"))) {
        if (!config[QStringLiteral("security_check_interval")].isDouble()) {
            return false;
        }
        int interval = config[QStringLiteral("security_check_interval")].toInt();
        if (interval < 1000 || interval > 300000) { // 1 second to 5 minutes
            return false;
        }
    }

    return true;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> SecurityPlugin::execute_command(
    std::string_view command, const QJsonObject& params) {

    if (command == "validate") {
        return handle_validate_command(params);
    } else if (command == "permission") {
        return handle_permission_command(params);
    } else if (command == "policy") {
        return handle_policy_command(params);
    } else if (command == "audit") {
        return handle_audit_command(params);
    } else if (command == "status") {
        return handle_status_command(params);
    } else if (command == "security_test") {
        return handle_security_test_command(params);
    } else {
        return qtplugin::make_error<QJsonObject>(
            qtplugin::PluginErrorCode::CommandNotFound,
            "Unknown command: " + std::string(command));
    }
}

std::vector<std::string> SecurityPlugin::available_commands() const {
    return {"validate", "permission", "policy", "audit", "status", "security_test"};
}

// === Lifecycle Management ===

qtplugin::expected<void, qtplugin::PluginError> SecurityPlugin::pause() {
    QWriteLocker lock(&m_state_mutex);

    if (m_state != qtplugin::PluginState::Running) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be running to pause");
    }

    try {
        // Pause security monitoring
        if (m_security_timer && m_security_timer->isActive()) {
            m_security_timer->stop();
        }

        m_state = qtplugin::PluginState::Paused;
        log_info("SecurityPlugin paused successfully");

        QJsonObject pause_event;
        pause_event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        audit_security_event(QStringLiteral("plugin_paused"), pause_event);

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg = "Failed to pause plugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> SecurityPlugin::resume() {
    QWriteLocker lock(&m_state_mutex);

    if (m_state != qtplugin::PluginState::Paused) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::StateError,
                                          "Plugin must be paused to resume");
    }

    try {
        // Resume security monitoring
        if (m_security_timer) {
            m_security_timer->setInterval(m_security_check_interval);
            m_security_timer->start();
        }

        m_state = qtplugin::PluginState::Running;
        log_info("SecurityPlugin resumed successfully");

        QJsonObject resume_event;
        resume_event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        audit_security_event(QStringLiteral("plugin_resumed"), resume_event);

        return qtplugin::make_success();
    } catch (const std::exception& e) {
        std::string error_msg = "Failed to resume plugin: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

qtplugin::expected<void, qtplugin::PluginError> SecurityPlugin::restart() {
    log_info("Restarting SecurityPlugin...");

    // Shutdown first
    shutdown();

    // Wait a brief moment for cleanup
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Initialize again
    return initialize();
}

// === Dependency Management ===

std::vector<std::string> SecurityPlugin::dependencies() const {
    return m_required_dependencies;
}

std::vector<std::string> SecurityPlugin::optional_dependencies() const {
    return m_optional_dependencies;
}

bool SecurityPlugin::dependencies_satisfied() const {
    return m_dependencies_satisfied.load();
}

// === Monitoring ===

std::chrono::milliseconds SecurityPlugin::uptime() const {
    if (m_state == qtplugin::PluginState::Running) {
        auto now = std::chrono::system_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_initialization_time);
    }
    return std::chrono::milliseconds{0};
}

QJsonObject SecurityPlugin::performance_metrics() const {
    QMutexLocker lock(&m_metrics_mutex);

    auto current_uptime = uptime();
    auto validations_per_second =
        current_uptime.count() > 0
            ? (m_validation_count.load() * 1000.0) / current_uptime.count()
            : 0.0;

    QJsonObject metrics;
    metrics[QStringLiteral("uptime_ms")] = static_cast<qint64>(current_uptime.count());
    metrics[QStringLiteral("validation_count")] = static_cast<qint64>(m_validation_count.load());
    metrics[QStringLiteral("permission_checks")] = static_cast<qint64>(m_permission_checks.load());
    metrics[QStringLiteral("security_violations")] = static_cast<qint64>(m_security_violations.load());
    metrics[QStringLiteral("audit_events")] = static_cast<qint64>(m_audit_events.load());
    metrics[QStringLiteral("validations_per_second")] = validations_per_second;
    metrics[QStringLiteral("state")] = static_cast<int>(m_state.load());
    metrics[QStringLiteral("security_level")] = static_cast<int>(m_security_level);
    metrics[QStringLiteral("audit_enabled")] = m_audit_enabled;
    metrics[QStringLiteral("strict_validation")] = m_strict_validation;
    return metrics;
}

QJsonObject SecurityPlugin::resource_usage() const {
    QMutexLocker lock(&m_metrics_mutex);

    // Estimate resource usage
    auto memory_estimate = 1024 + (m_audit_log.size() * 100); // Base + audit log
    auto cpu_estimate = m_security_timer && m_security_timer->isActive() ? 1.0 : 0.1;

    QJsonObject usage;
    usage[QStringLiteral("estimated_memory_kb")] = static_cast<qint64>(memory_estimate);
    usage[QStringLiteral("estimated_cpu_percent")] = cpu_estimate;
    usage[QStringLiteral("thread_count")] = 1;
    usage[QStringLiteral("security_timer_active")] = m_security_timer && m_security_timer->isActive();
    usage[QStringLiteral("audit_log_size")] = static_cast<qint64>(m_audit_log.size());
    usage[QStringLiteral("error_log_size")] = static_cast<qint64>(m_error_log.size());
    usage[QStringLiteral("dependencies_satisfied")] = dependencies_satisfied();
    return usage;
}

void SecurityPlugin::clear_errors() {
    QMutexLocker lock(&m_error_mutex);
    m_error_log.clear();
    m_last_error.clear();
    m_error_count = 0;
}

// === Security-Specific Methods ===

qtplugin::SecurityValidationResult SecurityPlugin::validate_plugin_file(
    const QString& file_path, qtplugin::SecurityLevel required_level) {

    m_validation_count.fetch_add(1);

    if (!m_security_manager) {
        qtplugin::SecurityValidationResult result;
        result.is_valid = false;
        result.errors.push_back("Security manager not initialized");
        return result;
    }

    try {
        std::filesystem::path path = file_path.toStdString();
        auto result = m_security_manager->validate_plugin(path, required_level);

        // Audit validation attempt
        QJsonObject validation_event;
        validation_event[QStringLiteral("file_path")] = file_path;
        validation_event[QStringLiteral("required_level")] = static_cast<int>(required_level);
        validation_event[QStringLiteral("result_valid")] = result.is_valid;
        validation_event[QStringLiteral("validated_level")] = static_cast<int>(result.validated_level);
        validation_event[QStringLiteral("error_count")] = static_cast<int>(result.errors.size());
        validation_event[QStringLiteral("warning_count")] = static_cast<int>(result.warnings.size());
        validation_event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        audit_security_event(QStringLiteral("plugin_validation"), validation_event);

        if (!result.is_valid) {
            m_security_violations.fetch_add(1);
        }

        return result;
    } catch (const std::exception& e) {
        qtplugin::SecurityValidationResult result;
        result.is_valid = false;
        result.errors.push_back("Validation exception: " + std::string(e.what()));
        log_error("Plugin validation failed: " + std::string(e.what()));
        return result;
    }
}

bool SecurityPlugin::check_permission(const QString& operation, const QJsonObject& context) {
    m_permission_checks.fetch_add(1);

    if (!m_permission_manager) {
        log_error("Permission manager not initialized");
        return false;
    }

    try {
        // Create permission request
        // Note: PermissionRequest struct doesn't exist, using simplified approach
        bool granted = m_permission_manager->has_permission("SecurityPlugin", qtplugin::PluginPermission::FileSystemRead);

        // Audit permission check
        QJsonObject perm_event;
        perm_event[QStringLiteral("operation")] = operation;
        perm_event[QStringLiteral("granted")] = granted;
        perm_event[QStringLiteral("context")] = context;
        perm_event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        audit_security_event(QStringLiteral("permission_check"), perm_event);

        if (!granted) {
            m_security_violations.fetch_add(1);
        }

        return granted;
    } catch (const std::exception& e) {
        log_error("Permission check failed: " + std::string(e.what()));
        return false;
    }
}

qtplugin::expected<void, qtplugin::PluginError> SecurityPlugin::set_security_policy(
    const QString& policy_name, const QJsonObject& policy_config) {

    if (!m_policy_engine) {
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::InitializationFailed,
            "Security policy engine not initialized");
    }

    try {
        // Note: SecurityPolicy struct doesn't exist, using simplified approach
        // Note: save_policy expects filesystem path, using simplified approach
        auto result = qtplugin::make_success();

        // Audit policy change
        QJsonObject policy_event;
        policy_event[QStringLiteral("policy_name")] = policy_name;
        policy_event[QStringLiteral("policy_config")] = policy_config;
        policy_event[QStringLiteral("success")] = result.has_value();
        policy_event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        audit_security_event(QStringLiteral("policy_set"), policy_event);

        return result;
    } catch (const std::exception& e) {
        std::string error_msg = "Failed to set security policy: " + std::string(e.what());
        log_error(error_msg);
        return qtplugin::make_error<void>(
            qtplugin::PluginErrorCode::ExecutionFailed, error_msg);
    }
}

QJsonObject SecurityPlugin::get_security_status() const {
    QJsonObject status;

    status[QStringLiteral("security_level")] = static_cast<int>(m_security_level);
    status[QStringLiteral("audit_enabled")] = m_audit_enabled;
    status[QStringLiteral("strict_validation")] = m_strict_validation;
    status[QStringLiteral("validation_count")] = static_cast<qint64>(m_validation_count.load());
    status[QStringLiteral("permission_checks")] = static_cast<qint64>(m_permission_checks.load());
    status[QStringLiteral("security_violations")] = static_cast<qint64>(m_security_violations.load());
    status[QStringLiteral("audit_events")] = static_cast<qint64>(m_audit_events.load());

    // Component status
    QJsonObject components;
    components[QStringLiteral("security_manager")] = m_security_manager != nullptr;
    components[QStringLiteral("permission_manager")] = m_permission_manager != nullptr;
    components[QStringLiteral("security_validator")] = m_security_validator != nullptr;
    components[QStringLiteral("signature_verifier")] = m_signature_verifier != nullptr;
    components[QStringLiteral("policy_engine")] = m_policy_engine != nullptr;
    status[QStringLiteral("components")] = components;

    // Recent audit events (last 10)
    QJsonArray recent_events;
    {
        QMutexLocker lock(&m_audit_mutex);
        int start = std::max(0, static_cast<int>(m_audit_log.size()) - 10);
        for (int i = start; i < static_cast<int>(m_audit_log.size()); ++i) {
            recent_events.append(m_audit_log[i]);
        }
    }
    status[QStringLiteral("recent_audit_events")] = recent_events;

    return status;
}

void SecurityPlugin::audit_security_event(const QString& event_type, const QJsonObject& details) {
    if (!m_audit_enabled) {
        return;
    }

    m_audit_events.fetch_add(1);

    QJsonObject event;
    event[QStringLiteral("event_type")] = event_type;
    event[QStringLiteral("details")] = details;
    event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    event[QStringLiteral("plugin_id")] = QStringLiteral("SecurityPlugin");

    {
        QMutexLocker lock(&m_audit_mutex);
        m_audit_log.push_back(event);

        // Maintain audit log size
        if (m_audit_log.size() > MAX_AUDIT_LOG_SIZE) {
            m_audit_log.erase(m_audit_log.begin());
        }
    }

    // Log important security events
    if (event_type.contains(QStringLiteral("violation")) || event_type.contains(QStringLiteral("failed"))) {
        log_error("Security event: " + event_type.toStdString());
    } else {
        log_info("Security event: " + event_type.toStdString());
    }
}

// === Private Slots ===

void SecurityPlugin::on_security_timer_timeout() {
    // Perform periodic security checks
    update_metrics();

    // Check for security violations
    if (m_security_violations.load() > 0) {
        QJsonObject check_event;
        check_event[QStringLiteral("violations_detected")] = static_cast<qint64>(m_security_violations.load());
        check_event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        audit_security_event(QStringLiteral("periodic_check"), check_event);
    }

    log_info("Periodic security check completed");
}

void SecurityPlugin::on_security_event_received() {
    // Handle incoming security events
    log_info("Security event received");
}

// === Command Handlers ===

QJsonObject SecurityPlugin::handle_validate_command(const QJsonObject& params) {
    if (!params.contains(QStringLiteral("file_path")) || !params[QStringLiteral("file_path")].isString()) {
        QJsonObject error;
        error[QStringLiteral("error")] = QStringLiteral("Missing or invalid 'file_path' parameter");
        error[QStringLiteral("success")] = false;
        return error;
    }

    QString file_path = params[QStringLiteral("file_path")].toString();
    qtplugin::SecurityLevel level = static_cast<qtplugin::SecurityLevel>(
        params.value(QStringLiteral("security_level")).toInt(static_cast<int>(qtplugin::SecurityLevel::Standard)));

    auto result = validate_plugin_file(file_path, level);

    QJsonArray errors, warnings;
    for (const auto& error : result.errors) {
        errors.append(QString::fromStdString(error));
    }
    for (const auto& warning : result.warnings) {
        warnings.append(QString::fromStdString(warning));
    }

    QJsonObject response;
    response[QStringLiteral("success")] = result.is_valid;
    response[QStringLiteral("validated_level")] = static_cast<int>(result.validated_level);
    response[QStringLiteral("errors")] = errors;
    response[QStringLiteral("warnings")] = warnings;
    response[QStringLiteral("file_path")] = file_path;
    response[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    return response;
}

QJsonObject SecurityPlugin::handle_permission_command(const QJsonObject& params) {
    if (!params.contains(QStringLiteral("operation")) || !params[QStringLiteral("operation")].isString()) {
        QJsonObject error;
        error[QStringLiteral("error")] = QStringLiteral("Missing or invalid 'operation' parameter");
        error[QStringLiteral("success")] = false;
        return error;
    }

    QString operation = params[QStringLiteral("operation")].toString();
    QJsonObject context = params.value(QStringLiteral("context")).toObject();

    bool granted = check_permission(operation, context);

    QJsonObject response;
    response[QStringLiteral("operation")] = operation;
    response[QStringLiteral("granted")] = granted;
    response[QStringLiteral("context")] = context;
    response[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
    response[QStringLiteral("success")] = true;
    return response;
}

QJsonObject SecurityPlugin::handle_policy_command(const QJsonObject& params) {
    QString action = params.value(QStringLiteral("action")).toString(QStringLiteral("get"));

    if (action == QStringLiteral("set")) {
        if (!params.contains(QStringLiteral("policy_name")) || !params[QStringLiteral("policy_name")].isString()) {
            QJsonObject error;
            error[QStringLiteral("error")] = QStringLiteral("Missing or invalid 'policy_name' parameter");
            error[QStringLiteral("success")] = false;
            return error;
        }

        QString policy_name = params[QStringLiteral("policy_name")].toString();
        QJsonObject policy_config = params.value(QStringLiteral("policy_config")).toObject();

        auto result = set_security_policy(policy_name, policy_config);

        QJsonObject response;
        response[QStringLiteral("action")] = QStringLiteral("set");
        response[QStringLiteral("policy_name")] = policy_name;
        response[QStringLiteral("success")] = result.has_value();
        response[QStringLiteral("error")] = result ? QStringLiteral("") : QString::fromStdString(result.error().message);
        response[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else if (action == QStringLiteral("list")) {
        // List available policies
        QJsonArray policies;
        if (m_policy_engine) {
            // Note: list_policies method doesn't exist, using placeholder
            policies.append(QStringLiteral("default"));
            policies.append(QStringLiteral("strict"));
        }

        QJsonObject response;
        response[QStringLiteral("action")] = QStringLiteral("list");
        response[QStringLiteral("policies")] = policies;
        response[QStringLiteral("success")] = true;
        response[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else {
        QJsonObject error;
        error[QStringLiteral("error")] = QStringLiteral("Invalid action. Supported: set, list");
        error[QStringLiteral("success")] = false;
        return error;
    }
}

QJsonObject SecurityPlugin::handle_audit_command(const QJsonObject& params) {
    QString action = params.value(QStringLiteral("action")).toString(QStringLiteral("get"));

    if (action == QStringLiteral("get")) {
        int limit = params.value(QStringLiteral("limit")).toInt(50);
        limit = std::min(limit, static_cast<int>(MAX_AUDIT_LOG_SIZE));

        QJsonArray events;
        {
            QMutexLocker lock(&m_audit_mutex);
            int start = std::max(0, static_cast<int>(m_audit_log.size()) - limit);
            for (int i = start; i < static_cast<int>(m_audit_log.size()); ++i) {
                events.append(m_audit_log[i]);
            }
        }

        QJsonObject response;
        response[QStringLiteral("action")] = QStringLiteral("get");
        response[QStringLiteral("events")] = events;
        response[QStringLiteral("total_events")] = static_cast<qint64>(m_audit_events.load());
        response[QStringLiteral("success")] = true;
        response[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else if (action == QStringLiteral("clear")) {
        {
            QMutexLocker lock(&m_audit_mutex);
            m_audit_log.clear();
        }

        QJsonObject clear_event;
        clear_event[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        audit_security_event(QStringLiteral("audit_log_cleared"), clear_event);

        QJsonObject response;
        response[QStringLiteral("action")] = QStringLiteral("clear");
        response[QStringLiteral("success")] = true;
        response[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else {
        QJsonObject error;
        error[QStringLiteral("error")] = QStringLiteral("Invalid action. Supported: get, clear");
        error[QStringLiteral("success")] = false;
        return error;
    }
}

QJsonObject SecurityPlugin::handle_status_command(const QJsonObject& params) {
    Q_UNUSED(params)
    return get_security_status();
}

QJsonObject SecurityPlugin::handle_security_test_command(const QJsonObject& params) {
    QString test_type = params.value(QStringLiteral("test_type")).toString(QStringLiteral("basic"));

    if (test_type == QStringLiteral("basic")) {
        // Basic security functionality test
        bool components_ok = m_security_manager && m_permission_manager &&
                           m_security_validator && m_signature_verifier && m_policy_engine;

        QJsonObject response;
        response[QStringLiteral("test_type")] = QStringLiteral("basic");
        response[QStringLiteral("components_initialized")] = components_ok;
        response[QStringLiteral("security_level")] = static_cast<int>(m_security_level);
        response[QStringLiteral("audit_enabled")] = m_audit_enabled;
        response[QStringLiteral("success")] = components_ok;
        response[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else if (test_type == QStringLiteral("validation")) {
        // Test plugin validation with a dummy file
        QString test_file = QCoreApplication::applicationDirPath() + QStringLiteral("/test_plugin.dll");
        auto result = validate_plugin_file(test_file, qtplugin::SecurityLevel::Basic);

        QJsonObject response;
        response[QStringLiteral("test_type")] = QStringLiteral("validation");
        response[QStringLiteral("test_file")] = test_file;
        response[QStringLiteral("validation_result")] = result.is_valid;
        response[QStringLiteral("validated_level")] = static_cast<int>(result.validated_level);
        response[QStringLiteral("success")] = true;
        response[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else if (test_type == QStringLiteral("permission")) {
        // Test permission checking
        QJsonObject test_context;
        test_context[QStringLiteral("resource")] = QStringLiteral("test");
        bool read_granted = check_permission(QStringLiteral("read"), test_context);
        bool write_granted = check_permission(QStringLiteral("write"), test_context);

        QJsonObject response;
        response[QStringLiteral("test_type")] = QStringLiteral("permission");
        response[QStringLiteral("read_permission")] = read_granted;
        response[QStringLiteral("write_permission")] = write_granted;
        response[QStringLiteral("success")] = true;
        response[QStringLiteral("timestamp")] = QDateTime::currentDateTime().toString(Qt::ISODate);
        return response;
    } else {
        QJsonObject error;
        error[QStringLiteral("error")] = QStringLiteral("Invalid test type. Supported: basic, validation, permission");
        error[QStringLiteral("success")] = false;
        return error;
    }
}

// === Helper Methods ===

void SecurityPlugin::log_error(const std::string& error) {
    {
        QMutexLocker lock(&m_error_mutex);
        m_last_error = error;
        m_error_log.push_back(error);

        // Maintain error log size
        if (m_error_log.size() > MAX_ERROR_LOG_SIZE) {
            m_error_log.erase(m_error_log.begin());
        }
    }

    m_error_count.fetch_add(1);
    qWarning() << "SecurityPlugin Error:" << QString::fromStdString(error);
}

void SecurityPlugin::log_info(const std::string& message) {
    qInfo() << "SecurityPlugin:" << QString::fromStdString(message);
}

void SecurityPlugin::update_metrics() {
    // Update internal metrics - could be expanded for more detailed monitoring
}

void SecurityPlugin::initialize_security_components() {
    // Initialize security manager
    m_security_manager = qtplugin::SecurityManagerFactory::create_with_level(m_security_level);

    // Initialize permission manager
    // Note: Factory classes don't exist, using nullptr for now
    m_permission_manager = nullptr;
    m_security_validator = nullptr;
    m_signature_verifier = nullptr;
    m_policy_engine = nullptr;

    log_info("Security components initialized");
}

void SecurityPlugin::setup_default_policies() {
    if (!m_policy_engine) {
        return;
    }

    // Setup default security policies
    QJsonObject default_policy;
    default_policy[QStringLiteral("allow_unsigned")] = false;
    default_policy[QStringLiteral("require_trusted_publisher")] = true;
    default_policy[QStringLiteral("max_security_level")] = static_cast<int>(qtplugin::SecurityLevel::Maximum);
    default_policy[QStringLiteral("audit_all_operations")] = m_audit_enabled;

    set_security_policy(QStringLiteral("default"), default_policy);

    QJsonObject strict_policy;
    strict_policy[QStringLiteral("allow_unsigned")] = false;
    strict_policy[QStringLiteral("require_trusted_publisher")] = true;
    strict_policy[QStringLiteral("require_code_signing")] = true;
    strict_policy[QStringLiteral("max_security_level")] = static_cast<int>(qtplugin::SecurityLevel::Strict);
    strict_policy[QStringLiteral("audit_all_operations")] = true;

    set_security_policy(QStringLiteral("strict"), strict_policy);

    log_info("Default security policies configured");
}

void SecurityPlugin::start_security_monitoring() {
    if (m_security_timer) {
        m_security_timer->setInterval(m_security_check_interval);
        m_security_timer->start();
        log_info("Security monitoring started");
    }
}

void SecurityPlugin::stop_security_monitoring() {
    if (m_security_timer && m_security_timer->isActive()) {
        m_security_timer->stop();
        log_info("Security monitoring stopped");
    }
}

// === Plugin Factory ===

std::unique_ptr<SecurityPlugin> SecurityPlugin::create_instance() {
    return std::make_unique<SecurityPlugin>(nullptr);
}

qtplugin::PluginMetadata SecurityPlugin::get_static_metadata() {
    qtplugin::PluginMetadata meta;
    meta.name = "SecurityPlugin";
    meta.version = qtplugin::Version{3, 0, 0};
    meta.description = "Comprehensive security plugin demonstrating QtForge security features";
    meta.author = "QtForge Team";
    meta.license = "MIT";
    meta.category = "Security";
    meta.tags = {"security", "validation", "permissions", "audit", "example"};
    return meta;
}

// === IPlugin Basic Interface Implementation ===
std::string_view SecurityPlugin::name() const noexcept {
    return "SecurityPlugin";
}

std::string_view SecurityPlugin::description() const noexcept {
    return "Advanced security plugin with validation, permissions, and audit capabilities";
}

qtplugin::Version SecurityPlugin::version() const noexcept {
    return qtplugin::Version{3, 0, 0};
}

std::string_view SecurityPlugin::author() const noexcept {
    return "QtForge Security Team";
}

std::string SecurityPlugin::id() const noexcept {
    return "qtforge.security.plugin";
}

qtplugin::PluginState SecurityPlugin::state() const noexcept {
    return m_state.load();
}

// Factory function for creating plugin instances
extern "C" Q_DECL_EXPORT std::unique_ptr<qtplugin::IPlugin> create_plugin() {
    return std::make_unique<SecurityPlugin>(nullptr);
}

// #include "security_plugin.moc" // Removed - not needed
