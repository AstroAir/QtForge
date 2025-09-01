/**
 * @file sandbox_usage_example.cpp
 * @brief Example demonstrating the enhanced sandbox system usage
 * @version 3.2.0
 */

#include "qtplugin/security/sandbox/plugin_sandbox.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <memory>

using namespace qtplugin;

class SandboxExample : public QObject {
    Q_OBJECT

public:
    SandboxExample(QObject* parent = nullptr) : QObject(parent) {}

    void run() {
        qDebug() << "=== QtForge Sandbox System Example ===";
        
        // Example 1: Create a sandbox with strict policy
        example_strict_sandbox();
        
        // Example 2: Use SandboxManager for multiple sandboxes
        example_sandbox_manager();
        
        // Example 3: Custom security policy
        example_custom_policy();
        
        // Example 4: Resource monitoring
        example_resource_monitoring();
    }

private slots:
    void on_execution_completed(int exit_code, const QJsonObject& result) {
        qDebug() << "Plugin execution completed with exit code:" << exit_code;
        qDebug() << "Result:" << result;
    }

    void on_resource_limit_exceeded(const QString& resource, const QJsonObject& usage) {
        qWarning() << "Resource limit exceeded for:" << resource;
        qWarning() << "Usage:" << usage;
    }

    void on_security_violation(const QString& violation, const QJsonObject& details) {
        qWarning() << "Security violation detected:" << violation;
        qWarning() << "Details:" << details;
    }

private:
    void example_strict_sandbox() {
        qDebug() << "\n--- Example 1: Strict Sandbox ---";
        
        // Create a strict security policy
        SecurityPolicy strict_policy = SecurityPolicy::create_strict_policy();
        
        // Create sandbox with strict policy
        auto sandbox = std::make_unique<PluginSandbox>(strict_policy);
        
        // Connect signals
        connect(sandbox.get(), &PluginSandbox::execution_completed,
                this, &SandboxExample::on_execution_completed);
        connect(sandbox.get(), &PluginSandbox::resource_limit_exceeded,
                this, &SandboxExample::on_resource_limit_exceeded);
        connect(sandbox.get(), &PluginSandbox::security_violation,
                this, &SandboxExample::on_security_violation);
        
        // Initialize sandbox
        auto init_result = sandbox->initialize();
        if (!init_result) {
            qWarning() << "Failed to initialize sandbox:" << init_result.error().message.c_str();
            return;
        }
        
        qDebug() << "Strict sandbox initialized successfully";
        qDebug() << "Policy:" << strict_policy.policy_name;
        qDebug() << "Security level:" << static_cast<int>(strict_policy.level);
        
        // Note: In a real application, you would execute a plugin here
        // sandbox->execute_plugin("/path/to/plugin", PluginType::Native);
        
        sandbox->shutdown();
    }

    void example_sandbox_manager() {
        qDebug() << "\n--- Example 2: Sandbox Manager ---";
        
        // Get the singleton instance
        SandboxManager& manager = SandboxManager::instance();
        
        // List available policies
        auto policies = manager.get_registered_policies();
        qDebug() << "Available security policies:";
        for (const QString& policy_name : policies) {
            qDebug() << " -" << policy_name;
        }
        
        // Get a predefined policy
        auto policy_result = manager.get_policy("sandboxed");
        if (!policy_result) {
            qWarning() << "Failed to get policy:" << policy_result.error().message.c_str();
            return;
        }
        
        SecurityPolicy policy = policy_result.value();
        
        // Create multiple sandboxes
        auto sandbox1_result = manager.create_sandbox("test_sandbox_1", policy);
        auto sandbox2_result = manager.create_sandbox("test_sandbox_2", policy);
        
        if (sandbox1_result && sandbox2_result) {
            qDebug() << "Created two sandboxes successfully";
            
            // List active sandboxes
            auto active_sandboxes = manager.get_active_sandboxes();
            qDebug() << "Active sandboxes:" << active_sandboxes.size();
            for (const QString& sandbox_id : active_sandboxes) {
                qDebug() << " -" << sandbox_id;
            }
            
            // Clean up
            manager.remove_sandbox("test_sandbox_1");
            manager.remove_sandbox("test_sandbox_2");
        }
    }

    void example_custom_policy() {
        qDebug() << "\n--- Example 3: Custom Security Policy ---";
        
        // Create a custom security policy
        SecurityPolicy custom_policy;
        custom_policy.level = SandboxSecurityLevel::Limited;
        custom_policy.policy_name = "custom_development";
        custom_policy.description = "Custom policy for development plugins";
        
        // Set custom resource limits
        custom_policy.limits.cpu_time_limit = std::chrono::minutes(15);
        custom_policy.limits.memory_limit_mb = 1024;
        custom_policy.limits.disk_space_limit_mb = 500;
        custom_policy.limits.max_file_handles = 200;
        custom_policy.limits.max_network_connections = 50;
        custom_policy.limits.execution_timeout = std::chrono::minutes(10);
        
        // Set custom permissions
        custom_policy.permissions.allow_file_system_read = true;
        custom_policy.permissions.allow_file_system_write = true;
        custom_policy.permissions.allow_network_access = true;
        custom_policy.permissions.allow_process_creation = false;
        custom_policy.permissions.allow_system_calls = false;
        custom_policy.permissions.allow_registry_access = false;
        custom_policy.permissions.allow_environment_access = false;
        
        // Set allowed directories
        custom_policy.permissions.allowed_directories = {
            QStandardPaths::writableLocation(QStandardPaths::TempLocation),
            QStandardPaths::writableLocation(QStandardPaths::CacheLocation),
            QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/MyApp"
        };
        
        // Set allowed hosts
        custom_policy.permissions.allowed_hosts = {
            "api.myapp.com",
            "*.trusted-domain.com",
            "localhost"
        };
        
        // Set blocked APIs
        custom_policy.permissions.blocked_apis = {
            "system", "exec", "CreateProcess", "ShellExecute"
        };
        
        // Register the custom policy
        SandboxManager& manager = SandboxManager::instance();
        manager.register_policy("custom_development", custom_policy);
        
        qDebug() << "Custom policy registered successfully";
        qDebug() << "Policy JSON:" << custom_policy.to_json();
    }

    void example_resource_monitoring() {
        qDebug() << "\n--- Example 4: Resource Monitoring ---";
        
        // Create a sandbox with limited resources for monitoring
        SecurityPolicy limited_policy = SecurityPolicy::create_limited_policy();
        
        // Reduce limits for demonstration
        limited_policy.limits.memory_limit_mb = 128;
        limited_policy.limits.cpu_time_limit = std::chrono::seconds(30);
        
        auto sandbox = std::make_unique<PluginSandbox>(limited_policy);
        
        // Connect to resource monitoring signals
        connect(sandbox.get(), &PluginSandbox::resource_usage_updated,
                [](const ResourceUsage& usage) {
                    qDebug() << "Resource usage update:";
                    qDebug() << " - CPU time:" << usage.cpu_time_used.count() << "ms";
                    qDebug() << " - Memory:" << usage.memory_used_mb << "MB";
                    qDebug() << " - File handles:" << usage.file_handles_used;
                    qDebug() << " - Network connections:" << usage.network_connections_used;
                });
        
        // Initialize and demonstrate resource monitoring
        auto init_result = sandbox->initialize();
        if (init_result) {
            qDebug() << "Resource monitoring sandbox initialized";
            
            // Get current resource usage
            ResourceUsage current_usage = sandbox->get_resource_usage();
            qDebug() << "Initial resource usage:" << current_usage.to_json();
            
            // Check if usage exceeds limits
            bool exceeds = current_usage.exceeds_limits(limited_policy.limits);
            qDebug() << "Exceeds limits:" << (exceeds ? "Yes" : "No");
        }
        
        sandbox->shutdown();
    }
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    SandboxExample example;
    example.run();
    
    // Exit after a short delay to allow async operations to complete
    QTimer::singleShot(1000, &app, &QCoreApplication::quit);
    
    return app.exec();
}

#include "sandbox_usage_example.moc"
