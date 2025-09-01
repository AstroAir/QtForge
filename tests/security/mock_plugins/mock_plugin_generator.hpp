/**
 * @file mock_plugin_generator.hpp
 * @brief Generator for mock plugins used in sandbox testing
 * @version 3.2.0
 */

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

/**
 * @brief Generates various types of mock plugins for testing sandbox
 * functionality
 */
class MockPluginGenerator : public QObject {
    Q_OBJECT

public:
    explicit MockPluginGenerator(const QString& output_dir,
                                 QObject* parent = nullptr);
    ~MockPluginGenerator() override;

    /**
     * @brief Create a well-behaved plugin that executes normally
     * @param name Plugin name
     * @return Path to created plugin file
     */
    QString createBehavingPlugin(const QString& name);

    /**
     * @brief Create a plugin that consumes excessive resources
     * @param name Plugin name
     * @return Path to created plugin file
     */
    QString createResourceHungryPlugin(const QString& name);

    /**
     * @brief Create a plugin that attempts malicious operations
     * @param name Plugin name
     * @return Path to created plugin file
     */
    QString createMaliciousPlugin(const QString& name);

    /**
     * @brief Create a plugin that crashes during execution
     * @param name Plugin name
     * @return Path to created plugin file
     */
    QString createCrashingPlugin(const QString& name);

    /**
     * @brief Create a plugin that runs for a specified duration
     * @param name Plugin name
     * @param duration_seconds How long the plugin should run
     * @return Path to created plugin file
     */
    QString createLongRunningPlugin(const QString& name, int duration_seconds);

    /**
     * @brief Create a plugin that attempts to access specific files
     * @param name Plugin name
     * @param file_paths List of file paths to attempt to access
     * @return Path to created plugin file
     */
    QString createFileAccessPlugin(const QString& name,
                                   const QStringList& file_paths);

    /**
     * @brief Create a plugin that attempts network access
     * @param name Plugin name
     * @param hosts List of hosts to attempt to connect to
     * @return Path to created plugin file
     */
    QString createNetworkAccessPlugin(const QString& name,
                                      const QStringList& hosts);

    /**
     * @brief Create a comprehensive test suite of plugins
     * @param suite_name Base name for the test suite
     * @return List of paths to created plugin files
     */
    QStringList createTestSuite(const QString& suite_name);

    /**
     * @brief Get list of all created plugin files
     * @return List of plugin file paths
     */
    QStringList getCreatedPlugins() const { return m_created_plugins; }

    /**
     * @brief Clean up all created plugin files
     */
    void cleanup();

private:
    QString m_output_dir;
    QStringList m_created_plugins;

    /**
     * @brief Create a Python plugin file with the given content
     * @param name Plugin name
     * @param script_content Python script content
     * @return Path to created plugin file
     */
    QString createPythonPlugin(const QString& name,
                               const QString& script_content);
};

/**
 * @brief Test scenarios for mock plugins
 */
enum class MockPluginScenario {
    WellBehaved,      ///< Normal execution
    ResourceHungry,   ///< High resource consumption
    Malicious,        ///< Security violations
    Crashing,         ///< Runtime crashes
    LongRunning,      ///< Extended execution time
    FileAccess,       ///< File system access attempts
    NetworkAccess,    ///< Network access attempts
    ProcessCreation,  ///< Process creation attempts
    SystemCalls       ///< System call attempts
};

/**
 * @brief Plugin behavior configuration
 */
struct MockPluginConfig {
    MockPluginScenario scenario = MockPluginScenario::WellBehaved;
    QString name;
    int duration_seconds = 1;
    QStringList target_files;
    QStringList target_hosts;
    bool should_crash = false;
    bool consume_memory = false;
    bool consume_cpu = false;

    /**
     * @brief Create a default configuration for a scenario
     */
    static MockPluginConfig forScenario(MockPluginScenario scenario,
                                        const QString& name = "test_plugin");
};

/**
 * @brief Utility functions for mock plugin testing
 */
class MockPluginUtils {
public:
    /**
     * @brief Validate plugin output format
     * @param output Plugin output string
     * @return True if output format is valid
     */
    static bool validatePluginOutput(const QString& output);

    /**
     * @brief Parse JSON result from plugin output
     * @param output Plugin output string
     * @return Parsed JSON object
     */
    static QJsonObject parsePluginResult(const QString& output);

    /**
     * @brief Check if plugin attempted security violations
     * @param output Plugin output string
     * @return List of attempted violations
     */
    static QStringList extractSecurityViolations(const QString& output);

    /**
     * @brief Estimate plugin resource usage from output
     * @param output Plugin output string
     * @return Resource usage information
     */
    static QJsonObject extractResourceUsage(const QString& output);

    /**
     * @brief Create a temporary directory for plugin testing
     * @return Path to temporary directory
     */
    static QString createTestDirectory();

    /**
     * @brief Create test files for file access testing
     * @param directory Directory to create files in
     * @return List of created file paths
     */
    static QStringList createTestFiles(const QString& directory);

    /**
     * @brief Clean up test directory and files
     * @param directory Directory to clean up
     */
    static void cleanupTestDirectory(const QString& directory);
};
