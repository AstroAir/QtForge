# Advanced Plugin Tutorial

This tutorial guides you through creating sophisticated QtForge plugins with advanced features like dependency injection, event handling, and resource management.

## Prerequisites

- Completed the [Basic Plugin Tutorial](../getting-started/first-plugin.md)
- Understanding of C++ concepts (RAII, smart pointers, templates)
- Familiarity with Qt framework basics

## Tutorial Overview

In this tutorial, you'll build a **Data Processing Pipeline Plugin** that demonstrates:

- Advanced plugin architecture patterns
- Dependency injection and service location
- Event-driven programming
- Resource management and cleanup
- Error handling and recovery
- Performance optimization techniques

## Step 1: Project Setup

### Create Project Structure

```bash
mkdir advanced-data-processor
cd advanced-data-processor
mkdir src include tests resources
```

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(AdvancedDataProcessor VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QtForge REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core)

# Plugin library
add_library(AdvancedDataProcessor SHARED
    src/advanced_data_processor.cpp
    src/data_pipeline.cpp
    src/processing_stage.cpp
    src/resource_manager.cpp
    include/advanced_data_processor.hpp
    include/data_pipeline.hpp
    include/processing_stage.hpp
    include/resource_manager.hpp
)

target_include_directories(AdvancedDataProcessor PRIVATE include)
target_link_libraries(AdvancedDataProcessor
    QtForge::Core
    QtForge::Communication
    QtForge::Utils
    Qt6::Core
)

set_target_properties(AdvancedDataProcessor PROPERTIES
    PREFIX ""
    SUFFIX ".qtplugin"
)
```

## Step 2: Advanced Plugin Architecture

### Plugin Interface (include/advanced_data_processor.hpp)

```cpp
#pragma once

#include <qtforge/core/plugin_interface.hpp>
#include <qtforge/communication/message_bus.hpp>
#include <qtforge/utils/resource_manager.hpp>
#include <memory>
#include <vector>
#include <unordered_map>

class DataPipeline;
class ProcessingStage;

class AdvancedDataProcessor : public qtforge::IPlugin {
public:
    AdvancedDataProcessor();
    ~AdvancedDataProcessor() override;

    // Plugin metadata
    std::string name() const override { return "AdvancedDataProcessor"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override { 
        return "Advanced data processing plugin with pipeline architecture"; 
    }
    std::vector<std::string> dependencies() const override {
        return {"CorePlugin >= 1.0.0", "UtilsPlugin >= 1.0.0"};
    }

    // Plugin lifecycle
    qtforge::expected<void, qtforge::Error> initialize() override;
    qtforge::expected<void, qtforge::Error> activate() override;
    qtforge::expected<void, qtforge::Error> deactivate() override;
    void cleanup() override;

    qtforge::PluginState state() const override { return currentState_; }
    bool isCompatible(const std::string& version) const override;

    // Advanced features
    void registerProcessingStage(std::unique_ptr<ProcessingStage> stage);
    qtforge::expected<void, qtforge::Error> processData(const std::string& inputData);
    
    // Service interface
    std::shared_ptr<DataPipeline> getPipeline() const { return pipeline_; }

private:
    void setupEventHandlers();
    void setupResourceManagement();
    void handleDataRequest(const qtforge::DataRequestMessage& message);
    void handleConfigurationUpdate(const qtforge::ConfigurationMessage& message);

    qtforge::PluginState currentState_;
    std::shared_ptr<DataPipeline> pipeline_;
    std::unique_ptr<qtforge::ResourceManager> resourceManager_;
    std::vector<qtforge::SubscriptionHandle> subscriptions_;
    std::unordered_map<std::string, std::any> configuration_;
};
```

## Step 3: Data Pipeline Implementation

### Data Pipeline (include/data_pipeline.hpp)

```cpp
#pragma once

#include <vector>
#include <memory>
#include <functional>
#include <future>
#include <qtforge/utils/expected.hpp>
#include <qtforge/utils/error.hpp>

class ProcessingStage;

struct ProcessingContext {
    std::string data;
    std::unordered_map<std::string, std::any> metadata;
    std::chrono::system_clock::time_point timestamp;
    std::string correlationId;
};

class DataPipeline {
public:
    DataPipeline();
    ~DataPipeline();

    // Pipeline configuration
    void addStage(std::unique_ptr<ProcessingStage> stage);
    void removeStage(const std::string& stageName);
    void clearStages();

    // Processing
    qtforge::expected<ProcessingContext, qtforge::Error> 
        process(const ProcessingContext& input);
    
    std::future<qtforge::expected<ProcessingContext, qtforge::Error>>
        processAsync(const ProcessingContext& input);

    // Pipeline management
    void setErrorHandler(std::function<void(const qtforge::Error&)> handler);
    void setProgressCallback(std::function<void(size_t, size_t)> callback);
    
    // Statistics
    size_t getStageCount() const { return stages_.size(); }
    std::vector<std::string> getStageNames() const;
    
private:
    std::vector<std::unique_ptr<ProcessingStage>> stages_;
    std::function<void(const qtforge::Error&)> errorHandler_;
    std::function<void(size_t, size_t)> progressCallback_;
    mutable std::mutex pipelineMutex_;
};
```

### Processing Stage Base Class (include/processing_stage.hpp)

```cpp
#pragma once

#include <string>
#include <chrono>
#include <qtforge/utils/expected.hpp>
#include <qtforge/utils/error.hpp>

struct ProcessingContext;

class ProcessingStage {
public:
    ProcessingStage(const std::string& name) : name_(name) {}
    virtual ~ProcessingStage() = default;

    // Core processing interface
    virtual qtforge::expected<ProcessingContext, qtforge::Error> 
        process(const ProcessingContext& input) = 0;

    // Stage metadata
    virtual std::string name() const { return name_; }
    virtual std::string description() const = 0;
    virtual std::vector<std::string> requiredMetadata() const { return {}; }
    virtual std::vector<std::string> producedMetadata() const { return {}; }

    // Configuration
    virtual void configure(const std::unordered_map<std::string, std::any>& config) {}
    virtual bool isConfigured() const { return true; }

    // Performance monitoring
    std::chrono::milliseconds getLastProcessingTime() const { return lastProcessingTime_; }
    size_t getProcessedCount() const { return processedCount_; }
    size_t getErrorCount() const { return errorCount_; }

protected:
    void recordProcessingTime(std::chrono::milliseconds time) { 
        lastProcessingTime_ = time; 
        processedCount_++;
    }
    void recordError() { errorCount_++; }

private:
    std::string name_;
    std::chrono::milliseconds lastProcessingTime_{0};
    std::atomic<size_t> processedCount_{0};
    std::atomic<size_t> errorCount_{0};
};
```

## Step 4: Concrete Processing Stages

### Data Validation Stage

```cpp
class DataValidationStage : public ProcessingStage {
public:
    DataValidationStage() : ProcessingStage("DataValidation") {}

    std::string description() const override {
        return "Validates input data format and content";
    }

    qtforge::expected<ProcessingContext, qtforge::Error> 
    process(const ProcessingContext& input) override {
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            ProcessingContext output = input;
            
            // Validate data is not empty
            if (input.data.empty()) {
                recordError();
                return qtforge::Error("Input data is empty");
            }
            
            // Validate data format (example: JSON)
            if (!isValidJson(input.data)) {
                recordError();
                return qtforge::Error("Invalid JSON format");
            }
            
            // Add validation metadata
            output.metadata["validated"] = true;
            output.metadata["validation_timestamp"] = 
                std::chrono::system_clock::now();
            
            auto end = std::chrono::high_resolution_clock::now();
            recordProcessingTime(
                std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
            
            return output;
            
        } catch (const std::exception& e) {
            recordError();
            return qtforge::Error("Validation failed: " + std::string(e.what()));
        }
    }

private:
    bool isValidJson(const std::string& data) {
        // Simple JSON validation (in real implementation, use proper JSON parser)
        return !data.empty() && 
               data.front() == '{' && 
               data.back() == '}';
    }
};
```

### Data Transformation Stage

```cpp
class DataTransformationStage : public ProcessingStage {
public:
    DataTransformationStage() : ProcessingStage("DataTransformation") {}

    std::string description() const override {
        return "Transforms data according to configured rules";
    }

    std::vector<std::string> requiredMetadata() const override {
        return {"validated"};
    }

    std::vector<std::string> producedMetadata() const override {
        return {"transformed", "transformation_rules"};
    }

    void configure(const std::unordered_map<std::string, std::any>& config) override {
        auto it = config.find("transformation_rules");
        if (it != config.end()) {
            try {
                transformationRules_ = std::any_cast<std::vector<std::string>>(it->second);
                configured_ = true;
            } catch (const std::bad_any_cast& e) {
                configured_ = false;
            }
        }
    }

    bool isConfigured() const override { return configured_; }

    qtforge::expected<ProcessingContext, qtforge::Error> 
    process(const ProcessingContext& input) override {
        auto start = std::chrono::high_resolution_clock::now();
        
        if (!isConfigured()) {
            recordError();
            return qtforge::Error("Transformation stage not configured");
        }
        
        try {
            ProcessingContext output = input;
            
            // Apply transformation rules
            std::string transformedData = input.data;
            for (const auto& rule : transformationRules_) {
                transformedData = applyTransformationRule(transformedData, rule);
            }
            
            output.data = transformedData;
            output.metadata["transformed"] = true;
            output.metadata["transformation_rules"] = transformationRules_;
            
            auto end = std::chrono::high_resolution_clock::now();
            recordProcessingTime(
                std::chrono::duration_cast<std::chrono::milliseconds>(end - start));
            
            return output;
            
        } catch (const std::exception& e) {
            recordError();
            return qtforge::Error("Transformation failed: " + std::string(e.what()));
        }
    }

private:
    std::string applyTransformationRule(const std::string& data, const std::string& rule) {
        // Example transformation rules
        if (rule == "uppercase") {
            std::string result = data;
            std::transform(result.begin(), result.end(), result.begin(), ::toupper);
            return result;
        } else if (rule == "remove_whitespace") {
            std::string result = data;
            result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
            return result;
        }
        return data;
    }

    std::vector<std::string> transformationRules_;
    bool configured_ = false;
};
```

## Step 5: Plugin Implementation

### Main Plugin Implementation (src/advanced_data_processor.cpp)

```cpp
#include "advanced_data_processor.hpp"
#include "data_pipeline.hpp"
#include "processing_stage.hpp"
#include <qtforge/communication/message_bus.hpp>
#include <qtforge/utils/logger.hpp>

AdvancedDataProcessor::AdvancedDataProcessor() 
    : currentState_(qtforge::PluginState::Unloaded) {
}

AdvancedDataProcessor::~AdvancedDataProcessor() {
    cleanup();
}

qtforge::expected<void, qtforge::Error> AdvancedDataProcessor::initialize() {
    try {
        qtforge::Logger::info(name(), "Initializing advanced data processor...");
        
        // Create data pipeline
        pipeline_ = std::make_shared<DataPipeline>();
        
        // Setup resource management
        setupResourceManagement();
        
        // Register default processing stages
        registerProcessingStage(std::make_unique<DataValidationStage>());
        registerProcessingStage(std::make_unique<DataTransformationStage>());
        
        // Setup event handlers
        setupEventHandlers();
        
        currentState_ = qtforge::PluginState::Initialized;
        qtforge::Logger::info(name(), "Plugin initialized successfully");
        
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        return qtforge::Error("Initialization failed: " + std::string(e.what()));
    }
}

void AdvancedDataProcessor::setupEventHandlers() {
    auto& messageBus = qtforge::MessageBus::instance();
    
    // Handle data processing requests
    subscriptions_.emplace_back(
        messageBus.subscribe<qtforge::DataRequestMessage>("data.process",
            [this](const qtforge::DataRequestMessage& msg) {
                handleDataRequest(msg);
            })
    );
    
    // Handle configuration updates
    subscriptions_.emplace_back(
        messageBus.subscribe<qtforge::ConfigurationMessage>("config.update",
            [this](const qtforge::ConfigurationMessage& msg) {
                handleConfigurationUpdate(msg);
            })
    );
}

void AdvancedDataProcessor::registerProcessingStage(std::unique_ptr<ProcessingStage> stage) {
    if (pipeline_) {
        qtforge::Logger::info(name(), "Registering processing stage: " + stage->name());
        pipeline_->addStage(std::move(stage));
    }
}

qtforge::expected<void, qtforge::Error> AdvancedDataProcessor::processData(const std::string& inputData) {
    if (!pipeline_) {
        return qtforge::Error("Pipeline not initialized");
    }
    
    ProcessingContext context;
    context.data = inputData;
    context.timestamp = std::chrono::system_clock::now();
    context.correlationId = qtforge::generateUuid();
    
    auto result = pipeline_->process(context);
    if (!result) {
        return qtforge::Error("Processing failed: " + result.error().message());
    }
    
    // Publish result
    auto& messageBus = qtforge::MessageBus::instance();
    qtforge::DataResultMessage resultMsg;
    resultMsg.data = result.value().data;
    resultMsg.correlationId = context.correlationId;
    resultMsg.processingTime = std::chrono::system_clock::now() - context.timestamp;
    
    messageBus.publish("data.result", resultMsg);
    
    return {};
}

// Plugin factory functions
extern "C" QTFORGE_EXPORT qtforge::IPlugin* createPlugin() {
    return new AdvancedDataProcessor();
}

extern "C" QTFORGE_EXPORT void destroyPlugin(qtforge::IPlugin* plugin) {
    delete plugin;
}
```

## Step 6: Testing the Advanced Plugin

### Unit Tests (tests/test_advanced_plugin.cpp)

```cpp
#include <gtest/gtest.h>
#include "advanced_data_processor.hpp"
#include "data_pipeline.hpp"

class AdvancedPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin_ = std::make_unique<AdvancedDataProcessor>();
    }
    
    void TearDown() override {
        plugin_->cleanup();
    }
    
    std::unique_ptr<AdvancedDataProcessor> plugin_;
};

TEST_F(AdvancedPluginTest, InitializationSucceeds) {
    auto result = plugin_->initialize();
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(plugin_->state(), qtforge::PluginState::Initialized);
}

TEST_F(AdvancedPluginTest, PipelineProcessing) {
    plugin_->initialize();
    
    std::string testData = R"({"test": "data", "value": 123})";
    auto result = plugin_->processData(testData);
    
    EXPECT_TRUE(result.has_value());
}

TEST_F(AdvancedPluginTest, InvalidDataHandling) {
    plugin_->initialize();
    
    std::string invalidData = "";
    auto result = plugin_->processData(invalidData);
    
    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().message().find("empty"), std::string::npos);
}
```

## Step 7: Advanced Features

### Performance Monitoring

```cpp
class PerformanceMonitor {
public:
    void recordProcessingTime(const std::string& stage, std::chrono::milliseconds time) {
        std::lock_guard<std::mutex> lock(mutex_);
        processingTimes_[stage].push_back(time);
    }
    
    std::chrono::milliseconds getAverageTime(const std::string& stage) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = processingTimes_.find(stage);
        if (it == processingTimes_.end() || it->second.empty()) {
            return std::chrono::milliseconds(0);
        }
        
        auto total = std::accumulate(it->second.begin(), it->second.end(), 
                                   std::chrono::milliseconds(0));
        return total / it->second.size();
    }
    
private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::vector<std::chrono::milliseconds>> processingTimes_;
};
```

## Key Learning Points

1. **Advanced Architecture**: Modular design with clear separation of concerns
2. **Resource Management**: Proper RAII and cleanup patterns
3. **Error Handling**: Comprehensive error handling with recovery strategies
4. **Performance**: Monitoring and optimization techniques
5. **Extensibility**: Plugin architecture that supports easy extension
6. **Testing**: Comprehensive unit testing strategies

## Next Steps

- **[Orchestration Tutorial](orchestration-tutorial.md)**: Learn workflow orchestration
- **[Python Integration Tutorial](python-integration-tutorial.md)**: Python plugin development
- **[Plugin Architecture Guide](../user-guide/plugin-architecture.md)**: Advanced architectural patterns
- **[Performance Optimization](../user-guide/performance-optimization.md)**: Performance tuning techniques
