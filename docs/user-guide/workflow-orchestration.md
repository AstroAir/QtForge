# Workflow Orchestration Guide

!!! info "Guide Information"
**Difficulty**: Intermediate  
 **Prerequisites**: Basic plugin development, understanding of workflows  
 **Estimated Time**: 1-2 hours  
 **QtForge Version**: v3.1.0+

## Overview

This guide covers how to create, manage, and execute complex plugin workflows using QtForge's orchestration system. You'll learn how to coordinate multiple plugins, handle dependencies, manage data flow, and implement robust error handling in your workflows.

### What You'll Learn

- [ ] Creating and configuring plugin workflows
- [ ] Managing workflow steps and dependencies
- [ ] Implementing different execution modes (sequential, parallel, conditional)
- [ ] Handling data flow between workflow steps
- [ ] Monitoring workflow execution and progress
- [ ] Implementing error handling and rollback strategies
- [ ] Advanced orchestration patterns and best practices

### Prerequisites

Before starting this guide, you should have:

- [x] QtForge installed and configured
- [x] Basic understanding of plugin development
- [x] Familiarity with plugin management concepts
- [x] Understanding of asynchronous programming concepts

## Getting Started

### Basic Workflow Creation

```cpp
#include <qtplugin/orchestration/plugin_orchestrator.hpp>

class WorkflowManager {
private:
    std::shared_ptr<PluginOrchestrator> m_orchestrator;

public:
    bool initialize() {
        m_orchestrator = PluginOrchestrator::create();

        // Connect to orchestration signals
        connect(m_orchestrator.get(), &PluginOrchestrator::workflow_started,
                this, &WorkflowManager::on_workflow_started);

        connect(m_orchestrator.get(), &PluginOrchestrator::workflow_completed,
                this, &WorkflowManager::on_workflow_completed);

        connect(m_orchestrator.get(), &PluginOrchestrator::workflow_failed,
                this, &WorkflowManager::on_workflow_failed);

        return true;
    }

    bool create_data_processing_workflow() {
        // Create workflow definition
        Workflow workflow("data_processing", "Data Processing Pipeline");
        workflow.set_description("Complete data processing pipeline with validation")
                .set_execution_mode(ExecutionMode::Sequential);

        // Step 1: Load data
        WorkflowStep load_step("load_data", "csv_loader", "load_file");
        load_step.parameters = QJsonObject{
            {"file_path", "input/data.csv"},
            {"delimiter", ","},
            {"has_header", true}
        };
        load_step.timeout = std::chrono::milliseconds(30000);

        // Step 2: Validate data
        WorkflowStep validate_step("validate_data", "data_validator", "validate_schema");
        validate_step.dependencies = {"load_data"};
        validate_step.parameters = QJsonObject{
            {"schema_file", "schemas/data_schema.json"},
            {"strict_mode", true}
        };

        // Step 3: Clean data
        WorkflowStep clean_step("clean_data", "data_cleaner", "clean_dataset");
        clean_step.dependencies = {"validate_data"};
        clean_step.parameters = QJsonObject{
            {"remove_duplicates", true},
            {"handle_missing", "interpolate"}
        };

        // Step 4: Transform data
        WorkflowStep transform_step("transform_data", "data_transformer", "apply_transformations");
        transform_step.dependencies = {"clean_data"};
        transform_step.parameters = QJsonObject{
            {"transformations", QJsonArray{
                QJsonObject{{"type", "normalize"}, {"columns", QJsonArray{"value1", "value2"}}},
                QJsonObject{{"type", "encode"}, {"column", "category"}}
            }}
        };

        // Step 5: Save results
        WorkflowStep save_step("save_results", "csv_writer", "save_file");
        save_step.dependencies = {"transform_data"};
        save_step.parameters = QJsonObject{
            {"output_path", "output/processed_data.csv"},
            {"include_metadata", true}
        };

        // Add steps to workflow
        workflow.add_step(load_step)
                .add_step(validate_step)
                .add_step(clean_step)
                .add_step(transform_step)
                .add_step(save_step);

        // Register workflow
        auto result = m_orchestrator->register_workflow(workflow);
        if (result) {
            qDebug() << "Data processing workflow registered successfully";
            return true;
        } else {
            qWarning() << "Failed to register workflow:" << result.error().message();
            return false;
        }
    }

    QString execute_workflow(const QString& workflow_id, const QJsonObject& initial_data = {}) {
        auto execution_result = m_orchestrator->execute_workflow(workflow_id, initial_data);
        if (execution_result) {
            QString execution_id = execution_result.value();
            qDebug() << "Workflow execution started:" << execution_id;

            // Start monitoring execution
            monitor_workflow_execution(execution_id);

            return execution_id;
        } else {
            qWarning() << "Failed to execute workflow:" << execution_result.error().message();
            return QString();
        }
    }

private:
    void monitor_workflow_execution(const QString& execution_id) {
        // Create timer for monitoring
        auto timer = new QTimer(this);
        timer->setInterval(2000); // Check every 2 seconds

        connect(timer, &QTimer::timeout, [this, execution_id, timer]() {
            auto status_result = m_orchestrator->get_execution_status(execution_id);
            if (status_result) {
                auto status = status_result.value();

                qDebug() << "Workflow" << execution_id << "status:";
                qDebug() << "  Progress:" << status["progress"].toDouble() * 100 << "%";
                qDebug() << "  Current step:" << status["current_step"].toString();
                qDebug() << "  Status:" << status["status"].toString();

                // Check if workflow is complete
                QString workflow_status = status["status"].toString();
                if (workflow_status == "completed" || workflow_status == "failed" || workflow_status == "cancelled") {
                    timer->stop();
                    timer->deleteLater();

                    if (workflow_status == "completed") {
                        handle_workflow_completion(execution_id);
                    } else {
                        handle_workflow_failure(execution_id, workflow_status);
                    }
                }
            }
        });

        timer->start();
    }

    void handle_workflow_completion(const QString& execution_id) {
        qDebug() << "Workflow completed successfully:" << execution_id;

        // Get final results
        auto results = m_orchestrator->get_step_results(execution_id);
        if (results) {
            auto step_results = results.value();
            qDebug() << "Workflow produced" << step_results.size() << "step results";

            for (const auto& step_result : step_results) {
                qDebug() << "Step" << step_result.step_id << ":"
                         << "Status:" << static_cast<int>(step_result.status)
                         << "Duration:" << step_result.execution_time().count() << "ms";
            }
        }
    }

    void handle_workflow_failure(const QString& execution_id, const QString& status) {
        qWarning() << "Workflow failed:" << execution_id << "Status:" << status;

        // Get error details
        auto results = m_orchestrator->get_step_results(execution_id);
        if (results) {
            auto step_results = results.value();
            for (const auto& step_result : step_results) {
                if (step_result.status == StepStatus::Failed) {
                    qWarning() << "Failed step:" << step_result.step_id
                               << "Error:" << step_result.error_message;
                }
            }
        }
    }

private slots:
    void on_workflow_started(const QString& execution_id, const QString& workflow_id) {
        qDebug() << "Workflow started:" << workflow_id << "Execution ID:" << execution_id;
    }

    void on_workflow_completed(const QString& execution_id, const QJsonObject& results) {
        qDebug() << "Workflow completed:" << execution_id;
        qDebug() << "Results:" << results;
    }

    void on_workflow_failed(const QString& execution_id, const QString& error) {
        qWarning() << "Workflow failed:" << execution_id << "Error:" << error;
    }
};
```

### Parallel Workflow Execution

```cpp
bool create_parallel_processing_workflow() {
    Workflow workflow("parallel_media", "Parallel Media Processing");
    workflow.set_description("Process different media types in parallel")
            .set_execution_mode(ExecutionMode::Parallel);

    // Independent parallel steps
    WorkflowStep process_images("process_images", "image_processor", "batch_process");
    process_images.parameters = QJsonObject{
        {"input_directory", "media/images/"},
        {"output_directory", "processed/images/"},
        {"format", "jpg"},
        {"quality", 85}
    };

    WorkflowStep process_videos("process_videos", "video_processor", "batch_process");
    process_videos.parameters = QJsonObject{
        {"input_directory", "media/videos/"},
        {"output_directory", "processed/videos/"},
        {"codec", "h264"},
        {"bitrate", "2M"}
    };

    WorkflowStep process_audio("process_audio", "audio_processor", "batch_process");
    process_audio.parameters = QJsonObject{
        {"input_directory", "media/audio/"},
        {"output_directory", "processed/audio/"},
        {"format", "mp3"},
        {"bitrate", "320k"}
    };

    // Aggregation step that depends on all parallel steps
    WorkflowStep create_manifest("create_manifest", "manifest_creator", "create_media_manifest");
    create_manifest.dependencies = {"process_images", "process_videos", "process_audio"};
    create_manifest.parameters = QJsonObject{
        {"output_file", "processed/manifest.json"},
        {"include_metadata", true}
    };

    // Final packaging step
    WorkflowStep package_results("package_results", "archiver", "create_archive");
    package_results.dependencies = {"create_manifest"};
    package_results.parameters = QJsonObject{
        {"source_directory", "processed/"},
        {"archive_name", "media_package.zip"},
        {"compression_level", 6}
    };

    workflow.add_step(process_images)
            .add_step(process_videos)
            .add_step(process_audio)
            .add_step(create_manifest)
            .add_step(package_results);

    return m_orchestrator->register_workflow(workflow).has_value();
}
```

### Conditional Workflow Execution

```cpp
bool create_conditional_workflow() {
    Workflow workflow("data_analysis", "Conditional Data Analysis");
    workflow.set_description("Analyze data with conditional processing paths")
            .set_execution_mode(ExecutionMode::Conditional);

    // Initial data inspection
    WorkflowStep inspect_data("inspect_data", "data_inspector", "analyze_structure");
    inspect_data.parameters = QJsonObject{{"input_file", "data/dataset.csv"}};

    // Conditional steps based on data characteristics
    WorkflowStep small_dataset_processing("process_small", "simple_processor", "quick_analysis");
    small_dataset_processing.dependencies = {"inspect_data"};
    small_dataset_processing.parameters = QJsonObject{
        {"condition", "row_count < 10000"},
        {"algorithm", "basic_stats"}
    };

    WorkflowStep large_dataset_processing("process_large", "advanced_processor", "distributed_analysis");
    large_dataset_processing.dependencies = {"inspect_data"};
    large_dataset_processing.parameters = QJsonObject{
        {"condition", "row_count >= 10000"},
        {"algorithm", "advanced_ml"},
        {"use_clustering", true}
    };

    // Conditional visualization
    WorkflowStep simple_visualization("viz_simple", "chart_generator", "create_basic_charts");
    simple_visualization.dependencies = {"process_small"};

    WorkflowStep advanced_visualization("viz_advanced", "dashboard_generator", "create_dashboard");
    advanced_visualization.dependencies = {"process_large"};

    // Final report generation (depends on either processing path)
    WorkflowStep generate_report("generate_report", "report_generator", "create_analysis_report");
    generate_report.dependencies = {"viz_simple", "viz_advanced"}; // OR dependency
    generate_report.parameters = QJsonObject{
        {"template", "analysis_template.html"},
        {"include_raw_data", false}
    };

    workflow.add_step(inspect_data)
            .add_step(small_dataset_processing)
            .add_step(large_dataset_processing)
            .add_step(simple_visualization)
            .add_step(advanced_visualization)
            .add_step(generate_report);

    return m_orchestrator->register_workflow(workflow).has_value();
}
```

## Advanced Orchestration Patterns

### Error Handling and Rollback

```cpp
class RobustWorkflowManager {
private:
    std::shared_ptr<PluginOrchestrator> m_orchestrator;
    std::shared_ptr<PluginTransactionManager> m_transaction_manager;

public:
    bool create_transactional_workflow() {
        Workflow workflow("transactional_update", "Transactional System Update");
        workflow.set_description("Update system components with rollback capability")
                .set_execution_mode(ExecutionMode::Sequential);

        // Step 1: Begin transaction
        WorkflowStep begin_transaction("begin_tx", "transaction_manager", "begin_transaction");
        begin_transaction.parameters = QJsonObject{
            {"isolation_level", "ReadCommitted"},
            {"timeout", 300000} // 5 minutes
        };

        // Step 2: Backup current state
        WorkflowStep backup_state("backup", "backup_manager", "create_backup");
        backup_state.dependencies = {"begin_tx"};
        backup_state.parameters = QJsonObject{
            {"backup_location", "backups/"},
            {"include_config", true},
            {"include_data", true}
        };
        backup_state.critical = true; // Failure stops workflow

        // Step 3: Update database schema
        WorkflowStep update_schema("update_db", "database_migrator", "migrate_schema");
        update_schema.dependencies = {"backup"};
        update_schema.max_retries = 2;
        update_schema.retry_delay = std::chrono::milliseconds(5000);

        // Step 4: Update application code
        WorkflowStep update_code("update_app", "code_deployer", "deploy_update");
        update_code.dependencies = {"update_db"};
        update_code.parameters = QJsonObject{
            {"deployment_package", "updates/app_v2.1.0.zip"},
            {"restart_services", true}
        };

        // Step 5: Verify update
        WorkflowStep verify_update("verify", "system_verifier", "verify_system_health");
        verify_update.dependencies = {"update_app"};
        verify_update.timeout = std::chrono::milliseconds(60000);

        // Step 6: Commit transaction
        WorkflowStep commit_transaction("commit_tx", "transaction_manager", "commit_transaction");
        commit_transaction.dependencies = {"verify"};

        // Add rollback steps for each critical operation
        WorkflowStep rollback_code("rollback_app", "code_deployer", "rollback_deployment");
        WorkflowStep rollback_schema("rollback_db", "database_migrator", "rollback_migration");
        WorkflowStep restore_backup("restore", "backup_manager", "restore_backup");

        workflow.add_step(begin_transaction)
                .add_step(backup_state)
                .add_step(update_schema)
                .add_step(update_code)
                .add_step(verify_update)
                .add_step(commit_transaction)
                .add_rollback_step("update_app", rollback_code)
                .add_rollback_step("update_db", rollback_schema)
                .add_rollback_step("backup", restore_backup);

        return m_orchestrator->register_workflow(workflow).has_value();
    }

    void handle_workflow_failure_with_rollback(const QString& execution_id) {
        qWarning() << "Workflow failed, initiating rollback:" << execution_id;

        // Get failed step information
        auto results = m_orchestrator->get_step_results(execution_id);
        if (results) {
            auto step_results = results.value();

            // Find the first failed step
            for (const auto& step_result : step_results) {
                if (step_result.status == StepStatus::Failed) {
                    qWarning() << "Failed at step:" << step_result.step_id;

                    // Trigger rollback from this point
                    initiate_rollback_from_step(execution_id, step_result.step_id);
                    break;
                }
            }
        }
    }

private:
    void initiate_rollback_from_step(const QString& execution_id, const QString& failed_step) {
        // Create rollback workflow
        Workflow rollback_workflow("rollback_" + execution_id, "Rollback Workflow");
        rollback_workflow.set_execution_mode(ExecutionMode::Sequential);

        // Add appropriate rollback steps based on failed step
        if (failed_step == "update_app" || failed_step == "verify") {
            WorkflowStep rollback_app("rollback_app", "code_deployer", "rollback_deployment");
            rollback_workflow.add_step(rollback_app);
        }

        if (failed_step == "update_db" || failed_step == "update_app" || failed_step == "verify") {
            WorkflowStep rollback_db("rollback_db", "database_migrator", "rollback_migration");
            rollback_workflow.add_step(rollback_db);
        }

        // Always restore backup as final step
        WorkflowStep restore("restore_backup", "backup_manager", "restore_backup");
        rollback_workflow.add_step(restore);

        // Execute rollback workflow
        m_orchestrator->register_workflow(rollback_workflow);
        auto rollback_result = m_orchestrator->execute_workflow(rollback_workflow.id());

        if (rollback_result) {
            qDebug() << "Rollback workflow started:" << rollback_result.value();
        } else {
            qCritical() << "Failed to start rollback workflow:" << rollback_result.error().message();
        }
    }
};
```

## Best Practices

### Do's ✅

- **Design for Failure**: Always include error handling and rollback strategies
- **Use Appropriate Execution Modes**: Choose the right execution mode for your use case
- **Set Realistic Timeouts**: Configure appropriate timeouts for each step
- **Monitor Progress**: Implement comprehensive progress monitoring
- **Document Dependencies**: Clearly document step dependencies and data flow
- **Test Workflows Thoroughly**: Test all execution paths and error scenarios

### Don'ts ❌

- **Don't Create Circular Dependencies**: Ensure workflow steps form a directed acyclic graph
- **Don't Ignore Resource Limits**: Consider memory and CPU usage of parallel workflows
- **Don't Skip Validation**: Always validate workflow definitions before execution
- **Don't Hardcode Parameters**: Use configurable parameters for flexibility
- **Don't Forget Cleanup**: Ensure proper resource cleanup after workflow completion

## Troubleshooting

### Common Issues

#### Issue 1: Workflow Execution Hangs

**Symptoms:**

- Workflow starts but never progresses
- Steps remain in "Pending" status
- No error messages generated

**Solutions:**

1. Check plugin availability and loading status
2. Verify step dependencies are correctly defined
3. Check for circular dependencies in workflow definition
4. Ensure required plugins are initialized and running

#### Issue 2: Step Execution Failures

**Symptoms:**

- Individual steps fail with errors
- Workflow stops at failed step
- Error messages in step results

**Solutions:**

1. Verify plugin method names and parameters
2. Check plugin state and initialization
3. Review step timeout settings
4. Implement retry logic for transient failures

## Next Steps

After completing this guide, you might want to:

- [ ] [Advanced Orchestration Guide](advanced-orchestration.md) - Visual workflows and event-driven execution
- [ ] [Transaction Management Guide](transaction-management.md) - Atomic workflow operations
- [ ] [Monitoring and Performance Guide](monitoring-optimization.md) - Workflow performance optimization

## Related Resources

### Documentation

- [PluginOrchestrator API](../api/orchestration/plugin-orchestrator.md) - Detailed API reference
- [Workflow API](../api/orchestration/workflow.md) - Workflow definition reference
- [Advanced Orchestrator API](../api/orchestration/advanced-orchestrator.md) - Enhanced orchestration features

### Examples

- [Orchestration Examples](../examples/orchestration-examples.md) - Complete workflow examples
- [Error Handling Examples](../examples/error-handling-examples.md) - Robust error handling patterns

---

_Last updated: December 2024 | QtForge v3.1.0_
