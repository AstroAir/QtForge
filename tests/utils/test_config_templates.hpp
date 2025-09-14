/**
 * @file test_config_templates.hpp
 * @brief Template configurations for QtForge testing
 */

#pragma once

#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <QUuid>

namespace QtForgeTest {

/**
 * @brief Template configurations for testing
 */
class ConfigTemplates {
public:
    /**
     * @brief Basic plugin configuration template
     */
    static QJsonObject basicPluginConfig() {
        QJsonObject config;
        config["enabled"] = true;
        config["auto_start"] = true;
        config["priority"] = "normal";
        config["log_level"] = "debug";
        config["timeout"] = 30000;
        
        QJsonObject settings;
        settings["test_mode"] = true;
        settings["mock_data"] = true;
        config["settings"] = settings;
        
        return config;
    }

    /**
     * @brief Advanced plugin configuration template
     */
    static QJsonObject advancedPluginConfig() {
        QJsonObject config = basicPluginConfig();
        
        // Add advanced features
        config["hot_reload_enabled"] = true;
        config["metrics_collection"] = true;
        config["event_handling"] = true;
        
        QJsonObject security;
        security["sandbox_enabled"] = true;
        security["trust_level"] = "medium";
        security["permissions"] = QJsonArray{"file_read", "file_write", "network"};
        config["security"] = security;
        
        QJsonObject resources;
        resources["memory_limit"] = "100MB";
        resources["cpu_limit"] = "50%";
        resources["thread_limit"] = 10;
        config["resources"] = resources;
        
        return config;
    }

    /**
     * @brief Security test configuration
     */
    static QJsonObject securityTestConfig() {
        QJsonObject config;
        config["test_type"] = "security";
        config["sandbox_enabled"] = true;
        config["trust_level"] = "low";
        config["validate_signatures"] = true;
        config["check_permissions"] = true;
        
        QJsonArray allowed_operations;
        allowed_operations.append("read_config");
        allowed_operations.append("write_log");
        config["allowed_operations"] = allowed_operations;
        
        QJsonArray blocked_operations;
        blocked_operations.append("file_system_access");
        blocked_operations.append("network_access");
        blocked_operations.append("system_calls");
        config["blocked_operations"] = blocked_operations;
        
        return config;
    }

    /**
     * @brief Performance test configuration
     */
    static QJsonObject performanceTestConfig() {
        QJsonObject config;
        config["test_type"] = "performance";
        config["benchmark_enabled"] = true;
        config["memory_profiling"] = true;
        config["cpu_profiling"] = true;
        config["max_execution_time"] = 60000; // 60 seconds
        
        QJsonObject thresholds;
        thresholds["max_memory_mb"] = 512;
        thresholds["max_cpu_percent"] = 80;
        thresholds["max_response_time_ms"] = 1000;
        config["thresholds"] = thresholds;
        
        return config;
    }

    /**
     * @brief Communication test configuration
     */
    static QJsonObject communicationTestConfig() {
        QJsonObject config;
        config["test_type"] = "communication";
        config["message_bus_enabled"] = true;
        config["max_message_size"] = 1048576; // 1MB
        config["message_timeout"] = 5000;
        config["compression_enabled"] = false;
        
        QJsonArray test_topics;
        test_topics.append("test.topic.1");
        test_topics.append("test.topic.2");
        test_topics.append("test.broadcast");
        config["test_topics"] = test_topics;
        
        return config;
    }

    /**
     * @brief Integration test configuration
     */
    static QJsonObject integrationTestConfig() {
        QJsonObject config;
        config["test_type"] = "integration";
        config["multi_plugin_testing"] = true;
        config["cross_language_testing"] = true;
        config["service_contract_testing"] = true;
        
        QJsonArray plugin_combinations;
        QJsonObject combo1;
        combo1["plugins"] = QJsonArray{"plugin_a", "plugin_b"};
        combo1["interaction_type"] = "message_passing";
        plugin_combinations.append(combo1);
        
        QJsonObject combo2;
        combo2["plugins"] = QJsonArray{"plugin_c", "plugin_d"};
        combo2["interaction_type"] = "service_contract";
        plugin_combinations.append(combo2);
        
        config["plugin_combinations"] = plugin_combinations;
        
        return config;
    }

    /**
     * @brief Monitoring test configuration
     */
    static QJsonObject monitoringTestConfig() {
        QJsonObject config;
        config["test_type"] = "monitoring";
        config["metrics_collection"] = true;
        config["health_checks"] = true;
        config["alerting_enabled"] = false; // Disable for tests
        
        QJsonArray monitored_metrics;
        monitored_metrics.append("cpu_usage");
        monitored_metrics.append("memory_usage");
        monitored_metrics.append("plugin_count");
        monitored_metrics.append("message_throughput");
        config["monitored_metrics"] = monitored_metrics;
        
        QJsonObject collection_intervals;
        collection_intervals["cpu_usage"] = 1000; // 1 second
        collection_intervals["memory_usage"] = 5000; // 5 seconds
        collection_intervals["plugin_count"] = 10000; // 10 seconds
        config["collection_intervals"] = collection_intervals;
        
        return config;
    }

    /**
     * @brief Orchestration test configuration
     */
    static QJsonObject orchestrationTestConfig() {
        QJsonObject config;
        config["test_type"] = "orchestration";
        config["dependency_resolution"] = true;
        config["load_order_optimization"] = true;
        config["circular_dependency_detection"] = true;
        
        QJsonArray test_scenarios;
        QJsonObject scenario1;
        scenario1["name"] = "basic_dependency_chain";
        scenario1["plugins"] = QJsonArray{"base", "middleware", "ui"};
        test_scenarios.append(scenario1);
        
        QJsonObject scenario2;
        scenario2["name"] = "parallel_loading";
        scenario2["plugins"] = QJsonArray{"service_a", "service_b", "service_c"};
        test_scenarios.append(scenario2);
        
        config["test_scenarios"] = test_scenarios;
        
        return config;
    }

    /**
     * @brief Marketplace test configuration
     */
    static QJsonObject marketplaceTestConfig() {
        QJsonObject config;
        config["test_type"] = "marketplace";
        config["mock_marketplace"] = true;
        config["download_simulation"] = true;
        config["signature_verification"] = true;
        
        QJsonArray mock_plugins;
        QJsonObject plugin1;
        plugin1["id"] = "com.test.plugin1";
        plugin1["name"] = "Test Plugin 1";
        plugin1["version"] = "1.0.0";
        plugin1["download_url"] = "https://mock.marketplace.com/plugin1.zip";
        mock_plugins.append(plugin1);
        
        config["mock_plugins"] = mock_plugins;
        
        return config;
    }

    /**
     * @brief Transaction test configuration
     */
    static QJsonObject transactionTestConfig() {
        QJsonObject config;
        config["test_type"] = "transaction";
        config["rollback_testing"] = true;
        config["commit_testing"] = true;
        config["isolation_testing"] = true;
        
        QJsonArray test_operations;
        test_operations.append("plugin_install");
        test_operations.append("plugin_uninstall");
        test_operations.append("plugin_update");
        test_operations.append("configuration_change");
        config["test_operations"] = test_operations;
        
        return config;
    }

    /**
     * @brief Basic plugin metadata template
     */
    static QJsonObject basicPluginMetadata(const QString& name = "TestPlugin") {
        QJsonObject metadata;
        metadata["name"] = name;
        metadata["version"] = "1.0.0";
        metadata["description"] = "A test plugin for QtForge";
        metadata["author"] = "QtForge Test Suite";
        metadata["license"] = "MIT";
        metadata["category"] = "test";
        metadata["id"] = QString("com.qtforge.test.%1").arg(name.toLower());
        
        QJsonArray interfaces;
        interfaces.append("IPlugin");
        metadata["interfaces"] = interfaces;
        
        QJsonObject dependencies;
        dependencies["qtforge_core"] = ">=3.0.0";
        metadata["dependencies"] = dependencies;
        
        QJsonObject capabilities;
        capabilities["supports_hot_reload"] = true;
        capabilities["thread_safe"] = true;
        metadata["capabilities"] = capabilities;
        
        return metadata;
    }

    /**
     * @brief Advanced plugin metadata template
     */
    static QJsonObject advancedPluginMetadata(const QString& name = "AdvancedTestPlugin") {
        QJsonObject metadata = basicPluginMetadata(name);
        
        QJsonArray interfaces;
        interfaces.append("IPlugin");
        interfaces.append("IAdvancedPlugin");
        interfaces.append("IDynamicPlugin");
        metadata["interfaces"] = interfaces;
        
        QJsonObject service_contracts;
        service_contracts["provides"] = QJsonArray{"TestService", "DataProcessor"};
        service_contracts["requires"] = QJsonArray{"LoggingService"};
        metadata["service_contracts"] = service_contracts;
        
        return metadata;
    }
};

} // namespace QtForgeTest
