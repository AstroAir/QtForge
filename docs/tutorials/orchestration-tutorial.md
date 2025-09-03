# Orchestration Tutorial

This tutorial teaches you how to create complex workflows using QtForge's orchestration system to coordinate multiple plugins and manage data flow between them.

## Prerequisites

- Completed [Advanced Plugin Tutorial](advanced-plugin-tutorial.md)
- Understanding of workflow concepts
- Familiarity with QtForge plugin architecture

## Tutorial Overview

You'll build a **Document Processing Workflow** that demonstrates:

- Workflow definition and execution
- Plugin coordination and data flow
- Error handling and recovery
- Conditional execution and branching
- Parallel processing capabilities
- Workflow monitoring and logging

## Step 1: Workflow Architecture

### Workflow Components

```cpp
// Workflow definition
class DocumentProcessingWorkflow {
public:
    // Workflow stages
    enum class Stage {
        DocumentIngestion,
        FormatDetection,
        ContentExtraction,
        TextAnalysis,
        MetadataGeneration,
        OutputGeneration
    };
    
    // Workflow configuration
    struct WorkflowConfig {
        std::vector<std::string> inputFormats;
        std::string outputFormat;
        bool enableParallelProcessing = true;
        std::chrono::seconds timeout = std::chrono::seconds(300);
        int maxRetries = 3;
    };
};
```

### Orchestrator Plugin (include/document_orchestrator.hpp)

```cpp
#pragma once

#include <qtforge/core/plugin_interface.hpp>
#include <qtforge/orchestration/plugin_orchestrator.hpp>
#include <qtforge/communication/message_bus.hpp>
#include <memory>
#include <unordered_map>

class DocumentOrchestrator : public qtforge::IPlugin {
public:
    DocumentOrchestrator();
    ~DocumentOrchestrator() override;

    // Plugin interface
    std::string name() const override { return "DocumentOrchestrator"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "Orchestrates document processing workflows";
    }
    
    std::vector<std::string> dependencies() const override {
        return {
            "DocumentIngestionPlugin >= 1.0.0",
            "FormatDetectionPlugin >= 1.0.0", 
            "ContentExtractionPlugin >= 1.0.0",
            "TextAnalysisPlugin >= 1.0.0"
        };
    }

    // Lifecycle
    qtforge::expected<void, qtforge::Error> initialize() override;
    qtforge::expected<void, qtforge::Error> activate() override;
    qtforge::expected<void, qtforge::Error> deactivate() override;
    void cleanup() override;

    qtforge::PluginState state() const override { return currentState_; }
    bool isCompatible(const std::string& version) const override;

    // Workflow management
    qtforge::expected<std::string, qtforge::Error> 
        startWorkflow(const std::string& documentPath, const WorkflowConfig& config);
    
    qtforge::expected<WorkflowStatus, qtforge::Error> 
        getWorkflowStatus(const std::string& workflowId);
    
    qtforge::expected<void, qtforge::Error> 
        cancelWorkflow(const std::string& workflowId);

private:
    void setupWorkflowDefinition();
    void handleWorkflowEvent(const qtforge::WorkflowEvent& event);
    void handlePluginError(const std::string& pluginName, const qtforge::Error& error);

    qtforge::PluginState currentState_;
    std::unique_ptr<qtforge::PluginOrchestrator> orchestrator_;
    std::unordered_map<std::string, WorkflowInstance> activeWorkflows_;
    std::vector<qtforge::SubscriptionHandle> subscriptions_;
};
```

## Step 2: Workflow Definition

### Workflow Builder

```cpp
class WorkflowBuilder {
public:
    WorkflowBuilder& addStage(const std::string& name, const std::string& pluginName) {
        WorkflowStage stage;
        stage.name = name;
        stage.pluginName = pluginName;
        stage.id = generateStageId();
        stages_.push_back(stage);
        return *this;
    }
    
    WorkflowBuilder& addDependency(const std::string& fromStage, const std::string& toStage) {
        dependencies_.emplace_back(fromStage, toStage);
        return *this;
    }
    
    WorkflowBuilder& addConditionalBranch(const std::string& stage, 
                                        const std::string& condition,
                                        const std::string& trueStage,
                                        const std::string& falseStage) {
        ConditionalBranch branch;
        branch.sourceStage = stage;
        branch.condition = condition;
        branch.trueStage = trueStage;
        branch.falseStage = falseStage;
        conditionalBranches_.push_back(branch);
        return *this;
    }
    
    WorkflowBuilder& enableParallelExecution(const std::vector<std::string>& stages) {
        parallelStages_.insert(parallelStages_.end(), stages.begin(), stages.end());
        return *this;
    }
    
    WorkflowDefinition build() {
        WorkflowDefinition definition;
        definition.stages = std::move(stages_);
        definition.dependencies = std::move(dependencies_);
        definition.conditionalBranches = std::move(conditionalBranches_);
        definition.parallelStages = std::move(parallelStages_);
        return definition;
    }

private:
    std::vector<WorkflowStage> stages_;
    std::vector<std::pair<std::string, std::string>> dependencies_;
    std::vector<ConditionalBranch> conditionalBranches_;
    std::vector<std::string> parallelStages_;
    
    std::string generateStageId() {
        return "stage_" + std::to_string(stages_.size());
    }
};
```

### Document Processing Workflow Definition

```cpp
void DocumentOrchestrator::setupWorkflowDefinition() {
    auto workflow = WorkflowBuilder()
        // Sequential stages
        .addStage("ingestion", "DocumentIngestionPlugin")
        .addStage("format_detection", "FormatDetectionPlugin")
        .addStage("content_extraction", "ContentExtractionPlugin")
        
        // Parallel processing branches
        .addStage("text_analysis", "TextAnalysisPlugin")
        .addStage("metadata_generation", "MetadataGenerationPlugin")
        .addStage("image_processing", "ImageProcessingPlugin")
        
        // Final stage
        .addStage("output_generation", "OutputGenerationPlugin")
        
        // Dependencies
        .addDependency("ingestion", "format_detection")
        .addDependency("format_detection", "content_extraction")
        .addDependency("content_extraction", "text_analysis")
        .addDependency("content_extraction", "metadata_generation")
        .addDependency("content_extraction", "image_processing")
        .addDependency("text_analysis", "output_generation")
        .addDependency("metadata_generation", "output_generation")
        .addDependency("image_processing", "output_generation")
        
        // Conditional branching
        .addConditionalBranch("format_detection", "hasImages", 
                            "image_processing", "text_analysis")
        
        // Parallel execution
        .enableParallelExecution({"text_analysis", "metadata_generation", "image_processing"})
        
        .build();
    
    orchestrator_->setWorkflowDefinition(workflow);
}
```

## Step 3: Workflow Execution Engine

### Workflow Executor

```cpp
class WorkflowExecutor {
public:
    WorkflowExecutor(qtforge::PluginOrchestrator* orchestrator)
        : orchestrator_(orchestrator) {}
    
    qtforge::expected<std::string, qtforge::Error> 
    executeWorkflow(const WorkflowDefinition& definition, 
                   const WorkflowContext& context) {
        
        std::string workflowId = generateWorkflowId();
        
        try {
            // Create workflow instance
            auto instance = std::make_unique<WorkflowInstance>();
            instance->id = workflowId;
            instance->definition = definition;
            instance->context = context;
            instance->status = WorkflowStatus::Running;
            instance->startTime = std::chrono::system_clock::now();
            
            // Start execution
            auto result = executeNextStages(instance.get());
            if (!result) {
                instance->status = WorkflowStatus::Failed;
                instance->error = result.error();
                return result.error();
            }
            
            // Store instance
            activeInstances_[workflowId] = std::move(instance);
            
            return workflowId;
            
        } catch (const std::exception& e) {
            return qtforge::Error("Workflow execution failed: " + std::string(e.what()));
        }
    }
    
private:
    qtforge::expected<void, qtforge::Error> 
    executeNextStages(WorkflowInstance* instance) {
        
        auto readyStages = findReadyStages(instance);
        if (readyStages.empty()) {
            // Check if workflow is complete
            if (allStagesCompleted(instance)) {
                instance->status = WorkflowStatus::Completed;
                instance->endTime = std::chrono::system_clock::now();
                return {};
            } else {
                return qtforge::Error("No ready stages found, workflow may be deadlocked");
            }
        }
        
        // Execute ready stages
        for (const auto& stage : readyStages) {
            auto result = executeStage(instance, stage);
            if (!result) {
                return result.error();
            }
        }
        
        return {};
    }
    
    qtforge::expected<void, qtforge::Error> 
    executeStage(WorkflowInstance* instance, const WorkflowStage& stage) {
        
        qtforge::Logger::info("WorkflowExecutor", 
            "Executing stage: " + stage.name + " (Plugin: " + stage.pluginName + ")");
        
        // Find plugin
        auto plugin = orchestrator_->findPlugin(stage.pluginName);
        if (!plugin) {
            return qtforge::Error("Plugin not found: " + stage.pluginName);
        }
        
        // Prepare stage context
        StageContext stageContext;
        stageContext.workflowId = instance->id;
        stageContext.stageName = stage.name;
        stageContext.inputData = getStageInputData(instance, stage);
        stageContext.configuration = stage.configuration;
        
        // Execute stage
        auto result = plugin->executeStage(stageContext);
        if (!result) {
            instance->failedStages.push_back(stage.name);
            return qtforge::Error("Stage execution failed: " + stage.name + 
                                " - " + result.error().message());
        }
        
        // Store stage result
        instance->stageResults[stage.name] = result.value();
        instance->completedStages.push_back(stage.name);
        
        // Continue with next stages
        return executeNextStages(instance);
    }
    
    qtforge::PluginOrchestrator* orchestrator_;
    std::unordered_map<std::string, std::unique_ptr<WorkflowInstance>> activeInstances_;
};
```

## Step 4: Error Handling and Recovery

### Error Recovery Strategies

```cpp
class WorkflowErrorHandler {
public:
    enum class RecoveryStrategy {
        Retry,
        Skip,
        Fallback,
        Abort
    };
    
    struct ErrorPolicy {
        RecoveryStrategy strategy = RecoveryStrategy::Retry;
        int maxRetries = 3;
        std::chrono::seconds retryDelay = std::chrono::seconds(5);
        std::string fallbackStage;
        bool continueOnError = false;
    };
    
    qtforge::expected<void, qtforge::Error> 
    handleStageError(WorkflowInstance* instance, 
                    const std::string& stageName,
                    const qtforge::Error& error) {
        
        auto policy = getErrorPolicy(stageName);
        
        switch (policy.strategy) {
            case RecoveryStrategy::Retry:
                return handleRetry(instance, stageName, error, policy);
                
            case RecoveryStrategy::Skip:
                return handleSkip(instance, stageName, error);
                
            case RecoveryStrategy::Fallback:
                return handleFallback(instance, stageName, error, policy);
                
            case RecoveryStrategy::Abort:
                return handleAbort(instance, stageName, error);
        }
        
        return qtforge::Error("Unknown recovery strategy");
    }
    
private:
    qtforge::expected<void, qtforge::Error> 
    handleRetry(WorkflowInstance* instance, 
               const std::string& stageName,
               const qtforge::Error& error,
               const ErrorPolicy& policy) {
        
        auto& retryCount = instance->stageRetries[stageName];
        
        if (retryCount >= policy.maxRetries) {
            return qtforge::Error("Max retries exceeded for stage: " + stageName);
        }
        
        retryCount++;
        
        qtforge::Logger::warning("WorkflowErrorHandler", 
            "Retrying stage " + stageName + " (attempt " + 
            std::to_string(retryCount) + "/" + std::to_string(policy.maxRetries) + ")");
        
        // Schedule retry after delay
        scheduleRetry(instance, stageName, policy.retryDelay);
        
        return {};
    }
    
    std::unordered_map<std::string, ErrorPolicy> errorPolicies_;
};
```

## Step 5: Workflow Monitoring

### Workflow Monitor

```cpp
class WorkflowMonitor {
public:
    struct WorkflowMetrics {
        std::string workflowId;
        WorkflowStatus status;
        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point endTime;
        std::chrono::milliseconds totalDuration;
        
        size_t totalStages;
        size_t completedStages;
        size_t failedStages;
        size_t skippedStages;
        
        std::unordered_map<std::string, std::chrono::milliseconds> stageDurations;
        std::vector<std::string> errorMessages;
    };
    
    void recordWorkflowStart(const std::string& workflowId) {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        WorkflowMetrics metrics;
        metrics.workflowId = workflowId;
        metrics.status = WorkflowStatus::Running;
        metrics.startTime = std::chrono::system_clock::now();
        
        workflowMetrics_[workflowId] = metrics;
    }
    
    void recordStageCompletion(const std::string& workflowId,
                              const std::string& stageName,
                              std::chrono::milliseconds duration) {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        auto it = workflowMetrics_.find(workflowId);
        if (it != workflowMetrics_.end()) {
            it->second.completedStages++;
            it->second.stageDurations[stageName] = duration;
        }
    }
    
    void recordWorkflowCompletion(const std::string& workflowId, 
                                 WorkflowStatus finalStatus) {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        auto it = workflowMetrics_.find(workflowId);
        if (it != workflowMetrics_.end()) {
            it->second.status = finalStatus;
            it->second.endTime = std::chrono::system_clock::now();
            it->second.totalDuration = 
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    it->second.endTime - it->second.startTime);
        }
    }
    
    WorkflowMetrics getMetrics(const std::string& workflowId) const {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        auto it = workflowMetrics_.find(workflowId);
        if (it != workflowMetrics_.end()) {
            return it->second;
        }
        
        return WorkflowMetrics{};
    }
    
    std::vector<WorkflowMetrics> getAllMetrics() const {
        std::lock_guard<std::mutex> lock(metricsMutex_);
        
        std::vector<WorkflowMetrics> result;
        for (const auto& pair : workflowMetrics_) {
            result.push_back(pair.second);
        }
        
        return result;
    }
    
private:
    mutable std::mutex metricsMutex_;
    std::unordered_map<std::string, WorkflowMetrics> workflowMetrics_;
};
```

## Step 6: Testing the Orchestration System

### Integration Tests

```cpp
#include <gtest/gtest.h>
#include "document_orchestrator.hpp"

class OrchestrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        orchestrator_ = std::make_unique<DocumentOrchestrator>();
        orchestrator_->initialize();
        orchestrator_->activate();
    }
    
    void TearDown() override {
        orchestrator_->cleanup();
    }
    
    std::unique_ptr<DocumentOrchestrator> orchestrator_;
};

TEST_F(OrchestrationTest, SimpleWorkflowExecution) {
    WorkflowConfig config;
    config.inputFormats = {"pdf", "docx"};
    config.outputFormat = "json";
    
    auto result = orchestrator_->startWorkflow("test_document.pdf", config);
    ASSERT_TRUE(result.has_value());
    
    std::string workflowId = result.value();
    
    // Wait for completion
    auto status = waitForWorkflowCompletion(workflowId, std::chrono::seconds(30));
    EXPECT_EQ(status.value().status, WorkflowStatus::Completed);
}

TEST_F(OrchestrationTest, ErrorHandlingAndRecovery) {
    // Test with invalid document
    WorkflowConfig config;
    auto result = orchestrator_->startWorkflow("invalid_document.xyz", config);
    
    if (result.has_value()) {
        auto status = waitForWorkflowCompletion(result.value(), std::chrono::seconds(10));
        EXPECT_EQ(status.value().status, WorkflowStatus::Failed);
    }
}

TEST_F(OrchestrationTest, ParallelProcessing) {
    WorkflowConfig config;
    config.enableParallelProcessing = true;
    
    auto result = orchestrator_->startWorkflow("large_document.pdf", config);
    ASSERT_TRUE(result.has_value());
    
    // Verify parallel stages execute concurrently
    auto metrics = getWorkflowMetrics(result.value());
    
    // Check that parallel stages have overlapping execution times
    auto textAnalysisStart = metrics.stageStartTimes["text_analysis"];
    auto metadataStart = metrics.stageStartTimes["metadata_generation"];
    
    auto timeDiff = std::abs(std::chrono::duration_cast<std::chrono::milliseconds>(
        textAnalysisStart - metadataStart).count());
    
    EXPECT_LT(timeDiff, 1000); // Should start within 1 second of each other
}
```

## Key Learning Points

1. **Workflow Design**: Creating flexible, maintainable workflow definitions
2. **Plugin Coordination**: Managing complex plugin interactions and dependencies
3. **Error Handling**: Implementing robust error recovery strategies
4. **Parallel Processing**: Optimizing performance through parallel execution
5. **Monitoring**: Comprehensive workflow monitoring and metrics collection
6. **Testing**: Integration testing strategies for complex workflows

## Best Practices

1. **Modular Design**: Keep workflow stages independent and reusable
2. **Error Recovery**: Implement appropriate recovery strategies for different error types
3. **Performance**: Use parallel processing where possible
4. **Monitoring**: Comprehensive logging and metrics collection
5. **Testing**: Test both success and failure scenarios

## Next Steps

- **[Python Integration Tutorial](python-integration-tutorial.md)**: Python workflow development
- **[Advanced Orchestration](../user-guide/workflow-orchestration.md)**: Advanced orchestration patterns
- **[Performance Optimization](../user-guide/performance-optimization.md)**: Workflow performance tuning
- **[Monitoring Guide](../user-guide/monitoring.md)**: Advanced monitoring techniques
