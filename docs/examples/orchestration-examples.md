# Orchestration Examples

This document provides comprehensive examples of workflow orchestration in QtForge, demonstrating various patterns for coordinating complex multi-plugin operations.

## Overview

Orchestration examples cover:
- **Sequential Workflows**: Step-by-step processing pipelines
- **Parallel Workflows**: Concurrent execution patterns
- **Conditional Workflows**: Decision-based routing
- **Event-Driven Workflows**: Reactive orchestration patterns
- **Error Handling**: Robust error recovery and compensation
- **Dynamic Workflows**: Runtime workflow modification

## Sequential Processing Pipeline

### Document Processing Workflow

```cpp
// Document processing orchestration
class DocumentProcessingOrchestrator : public qtforge::IPlugin {
public:
    DocumentProcessingOrchestrator() : currentState_(qtforge::PluginState::Unloaded) {}
    
    // Plugin interface
    std::string name() const override { return "DocumentProcessingOrchestrator"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "Orchestrates document processing through multiple stages";
    }
    
    std::vector<std::string> dependencies() const override {
        return {"DocumentParserPlugin >= 1.0.0", "ContentAnalyzerPlugin >= 1.0.0", 
                "DocumentEnricherPlugin >= 1.0.0", "DocumentStoragePlugin >= 1.0.0"};
    }
    
    qtforge::expected<void, qtforge::Error> initialize() override {
        try {
            // Setup workflow stages
            setupWorkflowStages();
            
            // Setup message handlers
            setupMessageHandlers();
            
            currentState_ = qtforge::PluginState::Initialized;
            return {};
            
        } catch (const std::exception& e) {
            currentState_ = qtforge::PluginState::Error;
            return qtforge::Error("Orchestrator initialization failed: " + std::string(e.what()));
        }
    }
    
    qtforge::expected<ProcessingResult, qtforge::Error> processDocument(const DocumentRequest& request) {
        WorkflowContext context;
        context.requestId = generateRequestId();
        context.document = request.document;
        context.options = request.options;
        context.startTime = std::chrono::system_clock::now();
        
        qtforge::Logger::info(name(), "Starting document processing workflow: " + context.requestId);
        
        // Execute workflow stages sequentially
        for (const auto& stage : workflowStages_) {
            auto result = executeStage(stage, context);
            if (!result) {
                // Handle stage failure
                auto compensation = compensateFailedStage(stage, context);
                return result.error();
            }
            
            // Update context with stage results
            context.stageResults[stage.name] = result.value();
            
            qtforge::Logger::debug(name(), 
                "Completed stage: " + stage.name + " for request: " + context.requestId);
        }
        
        // Create final result
        ProcessingResult finalResult;
        finalResult.requestId = context.requestId;
        finalResult.success = true;
        finalResult.processedDocument = context.document;
        finalResult.processingTime = std::chrono::system_clock::now() - context.startTime;
        finalResult.stageResults = context.stageResults;
        
        qtforge::Logger::info(name(), "Document processing completed: " + context.requestId);
        
        return finalResult;
    }

private:
    struct WorkflowStage {
        std::string name;
        std::string pluginName;
        std::string operation;
        std::map<std::string, std::any> parameters;
        std::chrono::seconds timeout = std::chrono::seconds(30);
        bool required = true;
        std::function<qtforge::expected<StageResult, qtforge::Error>(const WorkflowContext&)> executor;
    };
    
    struct WorkflowContext {
        std::string requestId;
        Document document;
        ProcessingOptions options;
        std::chrono::system_clock::time_point startTime;
        std::map<std::string, StageResult> stageResults;
    };
    
    qtforge::PluginState currentState_;
    std::vector<WorkflowStage> workflowStages_;
    std::vector<qtforge::SubscriptionHandle> subscriptions_;
    
    void setupWorkflowStages() {
        // Stage 1: Document Parsing
        WorkflowStage parseStage;
        parseStage.name = "document_parsing";
        parseStage.pluginName = "DocumentParserPlugin";
        parseStage.operation = "parse_document";
        parseStage.required = true;
        parseStage.executor = [this](const WorkflowContext& context) {
            return executeParsingStage(context);
        };
        workflowStages_.push_back(parseStage);
        
        // Stage 2: Content Analysis
        WorkflowStage analysisStage;
        analysisStage.name = "content_analysis";
        analysisStage.pluginName = "ContentAnalyzerPlugin";
        analysisStage.operation = "analyze_content";
        analysisStage.required = true;
        analysisStage.executor = [this](const WorkflowContext& context) {
            return executeAnalysisStage(context);
        };
        workflowStages_.push_back(analysisStage);
        
        // Stage 3: Document Enrichment
        WorkflowStage enrichmentStage;
        enrichmentStage.name = "document_enrichment";
        enrichmentStage.pluginName = "DocumentEnricherPlugin";
        enrichmentStage.operation = "enrich_document";
        enrichmentStage.required = false; // Optional stage
        enrichmentStage.executor = [this](const WorkflowContext& context) {
            return executeEnrichmentStage(context);
        };
        workflowStages_.push_back(enrichmentStage);
        
        // Stage 4: Document Storage
        WorkflowStage storageStage;
        storageStage.name = "document_storage";
        storageStage.pluginName = "DocumentStoragePlugin";
        storageStage.operation = "store_document";
        storageStage.required = true;
        storageStage.executor = [this](const WorkflowContext& context) {
            return executeStorageStage(context);
        };
        workflowStages_.push_back(storageStage);
    }
    
    qtforge::expected<StageResult, qtforge::Error> executeStage(const WorkflowStage& stage, 
                                                              const WorkflowContext& context) {
        try {
            auto startTime = std::chrono::high_resolution_clock::now();
            
            // Execute stage
            auto result = stage.executor(context);
            
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            if (result) {
                auto stageResult = result.value();
                stageResult.executionTime = duration;
                stageResult.stageName = stage.name;
                return stageResult;
            } else {
                if (stage.required) {
                    return result.error();
                } else {
                    // Optional stage failed, continue with warning
                    qtforge::Logger::warning(name(), 
                        "Optional stage failed: " + stage.name + " - " + result.error().message());
                    
                    StageResult warningResult;
                    warningResult.stageName = stage.name;
                    warningResult.success = false;
                    warningResult.warning = result.error().message();
                    warningResult.executionTime = duration;
                    return warningResult;
                }
            }
            
        } catch (const std::exception& e) {
            return qtforge::Error("Stage execution failed: " + stage.name + " - " + std::string(e.what()));
        }
    }
    
    qtforge::expected<StageResult, qtforge::Error> executeParsingStage(const WorkflowContext& context) {
        auto& messageBus = qtforge::MessageBus::instance();
        
        DocumentParseRequest request;
        request.document = context.document;
        request.options = context.options.parseOptions;
        request.requestId = context.requestId;
        
        auto response = messageBus.request<DocumentParseRequest, DocumentParseResponse>(
            "document.parse", request, std::chrono::seconds(30));
        
        if (response) {
            StageResult result;
            result.success = true;
            result.data["parsed_content"] = response.value().parsedContent;
            result.data["metadata"] = response.value().metadata;
            return result;
        } else {
            return qtforge::Error("Document parsing failed: " + response.error().message());
        }
    }
    
    qtforge::expected<StageResult, qtforge::Error> executeAnalysisStage(const WorkflowContext& context) {
        auto& messageBus = qtforge::MessageBus::instance();
        
        // Get parsed content from previous stage
        auto parseResult = context.stageResults.find("document_parsing");
        if (parseResult == context.stageResults.end()) {
            return qtforge::Error("Analysis stage requires parsing stage results");
        }
        
        ContentAnalysisRequest request;
        request.content = std::any_cast<std::string>(parseResult->second.data.at("parsed_content"));
        request.analysisType = context.options.analysisType;
        request.requestId = context.requestId;
        
        auto response = messageBus.request<ContentAnalysisRequest, ContentAnalysisResponse>(
            "content.analyze", request, std::chrono::seconds(45));
        
        if (response) {
            StageResult result;
            result.success = true;
            result.data["analysis_results"] = response.value().analysisResults;
            result.data["sentiment"] = response.value().sentiment;
            result.data["keywords"] = response.value().keywords;
            return result;
        } else {
            return qtforge::Error("Content analysis failed: " + response.error().message());
        }
    }
    
    qtforge::expected<void, qtforge::Error> compensateFailedStage(const WorkflowStage& stage, 
                                                                const WorkflowContext& context) {
        qtforge::Logger::warning(name(), "Compensating for failed stage: " + stage.name);
        
        // Implement compensation logic based on stage
        if (stage.name == "document_storage") {
            // Clean up any partial storage
            return cleanupPartialStorage(context);
        } else if (stage.name == "document_enrichment") {
            // Enrichment failure doesn't require compensation
            return {};
        }
        
        return {};
    }
    
    qtforge::expected<void, qtforge::Error> cleanupPartialStorage(const WorkflowContext& context) {
        // Implementation for cleaning up partial storage
        return {};
    }
};
```

## Parallel Processing Workflow

### Multi-Source Data Aggregation

```cpp
class DataAggregationOrchestrator : public qtforge::IPlugin {
public:
    DataAggregationOrchestrator() : currentState_(qtforge::PluginState::Unloaded) {}
    
    std::string name() const override { return "DataAggregationOrchestrator"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "Orchestrates parallel data collection from multiple sources";
    }
    
    qtforge::expected<AggregationResult, qtforge::Error> aggregateData(const AggregationRequest& request) {
        AggregationContext context;
        context.requestId = generateRequestId();
        context.sources = request.sources;
        context.aggregationType = request.aggregationType;
        context.timeout = request.timeout;
        context.startTime = std::chrono::system_clock::now();
        
        qtforge::Logger::info(name(), "Starting parallel data aggregation: " + context.requestId);
        
        // Launch parallel data collection tasks
        std::vector<std::future<DataCollectionResult>> futures;
        
        for (const auto& source : context.sources) {
            auto future = std::async(std::launch::async, [this, source, &context]() {
                return collectDataFromSource(source, context);
            });
            futures.push_back(std::move(future));
        }
        
        // Wait for all tasks to complete or timeout
        std::vector<DataCollectionResult> results;
        auto deadline = context.startTime + context.timeout;
        
        for (size_t i = 0; i < futures.size(); ++i) {
            auto remainingTime = deadline - std::chrono::system_clock::now();
            
            if (remainingTime <= std::chrono::seconds(0)) {
                qtforge::Logger::warning(name(), "Data collection timeout reached");
                break;
            }
            
            auto status = futures[i].wait_for(remainingTime);
            
            if (status == std::future_status::ready) {
                try {
                    auto result = futures[i].get();
                    results.push_back(result);
                } catch (const std::exception& e) {
                    qtforge::Logger::error(name(), 
                        "Data collection failed for source " + context.sources[i].name + 
                        ": " + std::string(e.what()));
                }
            } else {
                qtforge::Logger::warning(name(), 
                    "Data collection timeout for source: " + context.sources[i].name);
            }
        }
        
        // Aggregate collected data
        auto aggregationResult = aggregateCollectedData(results, context);
        if (!aggregationResult) {
            return aggregationResult.error();
        }
        
        AggregationResult finalResult;
        finalResult.requestId = context.requestId;
        finalResult.success = true;
        finalResult.aggregatedData = aggregationResult.value();
        finalResult.sourceResults = results;
        finalResult.processingTime = std::chrono::system_clock::now() - context.startTime;
        
        qtforge::Logger::info(name(), "Data aggregation completed: " + context.requestId);
        
        return finalResult;
    }

private:
    struct DataSource {
        std::string name;
        std::string type;
        std::string endpoint;
        std::map<std::string, std::string> parameters;
        std::chrono::seconds timeout = std::chrono::seconds(30);
    };
    
    struct AggregationContext {
        std::string requestId;
        std::vector<DataSource> sources;
        std::string aggregationType;
        std::chrono::seconds timeout;
        std::chrono::system_clock::time_point startTime;
    };
    
    struct DataCollectionResult {
        std::string sourceName;
        bool success;
        std::any data;
        std::string error;
        std::chrono::milliseconds collectionTime;
    };
    
    qtforge::PluginState currentState_;
    
    DataCollectionResult collectDataFromSource(const DataSource& source, 
                                              const AggregationContext& context) {
        DataCollectionResult result;
        result.sourceName = source.name;
        
        auto startTime = std::chrono::high_resolution_clock::now();
        
        try {
            qtforge::Logger::debug(name(), "Collecting data from source: " + source.name);
            
            // Select appropriate collector based on source type
            if (source.type == "database") {
                result.data = collectFromDatabase(source);
            } else if (source.type == "api") {
                result.data = collectFromAPI(source);
            } else if (source.type == "file") {
                result.data = collectFromFile(source);
            } else if (source.type == "stream") {
                result.data = collectFromStream(source);
            } else {
                throw std::runtime_error("Unsupported source type: " + source.type);
            }
            
            result.success = true;
            
        } catch (const std::exception& e) {
            result.success = false;
            result.error = e.what();
            qtforge::Logger::error(name(), 
                "Data collection failed for " + source.name + ": " + std::string(e.what()));
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        result.collectionTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        return result;
    }
    
    std::any collectFromDatabase(const DataSource& source) {
        auto& messageBus = qtforge::MessageBus::instance();
        
        DatabaseQueryRequest request;
        request.query = source.parameters.at("query");
        request.database = source.parameters.at("database");
        request.timeout = source.timeout;
        
        auto response = messageBus.request<DatabaseQueryRequest, DatabaseQueryResponse>(
            "database.query", request, source.timeout);
        
        if (response) {
            return response.value().results;
        } else {
            throw std::runtime_error("Database query failed: " + response.error().message());
        }
    }
    
    std::any collectFromAPI(const DataSource& source) {
        auto& messageBus = qtforge::MessageBus::instance();
        
        HttpRequestMessage request;
        request.method = source.parameters.at("method");
        request.url = source.endpoint;
        
        // Add parameters as headers or query params
        for (const auto& [key, value] : source.parameters) {
            if (key != "method") {
                request.headers[key] = value;
            }
        }
        
        auto response = messageBus.request<HttpRequestMessage, HttpResponseMessage>(
            "http.request", request, source.timeout);
        
        if (response) {
            if (response.value().statusCode == 200) {
                return response.value().body;
            } else {
                throw std::runtime_error("HTTP request failed with status: " + 
                                       std::to_string(response.value().statusCode));
            }
        } else {
            throw std::runtime_error("HTTP request failed: " + response.error().message());
        }
    }
    
    qtforge::expected<std::any, qtforge::Error> aggregateCollectedData(
        const std::vector<DataCollectionResult>& results, 
        const AggregationContext& context) {
        
        if (results.empty()) {
            return qtforge::Error("No data collected from any source");
        }
        
        // Filter successful results
        std::vector<std::any> successfulData;
        for (const auto& result : results) {
            if (result.success) {
                successfulData.push_back(result.data);
            }
        }
        
        if (successfulData.empty()) {
            return qtforge::Error("No successful data collection results");
        }
        
        // Perform aggregation based on type
        if (context.aggregationType == "merge") {
            return mergeData(successfulData);
        } else if (context.aggregationType == "union") {
            return unionData(successfulData);
        } else if (context.aggregationType == "intersect") {
            return intersectData(successfulData);
        } else if (context.aggregationType == "statistical") {
            return performStatisticalAggregation(successfulData);
        } else {
            return qtforge::Error("Unsupported aggregation type: " + context.aggregationType);
        }
    }
    
    std::any mergeData(const std::vector<std::any>& dataList) {
        // Implementation for merging data from multiple sources
        std::map<std::string, std::any> mergedData;
        
        for (size_t i = 0; i < dataList.size(); ++i) {
            mergedData["source_" + std::to_string(i)] = dataList[i];
        }
        
        return mergedData;
    }
};
```

## Conditional Workflow

### Smart Document Router

```cpp
class DocumentRoutingOrchestrator : public qtforge::IPlugin {
public:
    DocumentRoutingOrchestrator() : currentState_(qtforge::PluginState::Unloaded) {}
    
    std::string name() const override { return "DocumentRoutingOrchestrator"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "Routes documents to appropriate processing pipelines based on content analysis";
    }
    
    qtforge::expected<RoutingResult, qtforge::Error> routeDocument(const DocumentRoutingRequest& request) {
        RoutingContext context;
        context.requestId = generateRequestId();
        context.document = request.document;
        context.routingRules = loadRoutingRules();
        context.startTime = std::chrono::system_clock::now();
        
        qtforge::Logger::info(name(), "Starting document routing: " + context.requestId);
        
        // Step 1: Analyze document to determine routing
        auto analysisResult = analyzeDocumentForRouting(context);
        if (!analysisResult) {
            return analysisResult.error();
        }
        
        auto analysis = analysisResult.value();
        context.documentAnalysis = analysis;
        
        // Step 2: Apply routing rules
        auto routingDecision = applyRoutingRules(context);
        if (!routingDecision) {
            return routingDecision.error();
        }
        
        auto decision = routingDecision.value();
        context.routingDecision = decision;
        
        qtforge::Logger::info(name(), 
            "Document routed to pipeline: " + decision.targetPipeline + " for request: " + context.requestId);
        
        // Step 3: Execute selected pipeline
        auto pipelineResult = executePipeline(decision, context);
        if (!pipelineResult) {
            return pipelineResult.error();
        }
        
        RoutingResult finalResult;
        finalResult.requestId = context.requestId;
        finalResult.success = true;
        finalResult.routingDecision = decision;
        finalResult.pipelineResult = pipelineResult.value();
        finalResult.processingTime = std::chrono::system_clock::now() - context.startTime;
        
        qtforge::Logger::info(name(), "Document routing completed: " + context.requestId);
        
        return finalResult;
    }

private:
    struct RoutingRule {
        std::string name;
        std::string condition; // Expression to evaluate
        std::string targetPipeline;
        int priority;
        std::map<std::string, std::any> parameters;
    };
    
    struct DocumentAnalysis {
        std::string documentType;
        std::string language;
        std::vector<std::string> keywords;
        double confidenceScore;
        std::map<std::string, double> features;
        std::string contentCategory;
    };
    
    struct RoutingDecision {
        std::string targetPipeline;
        std::string reason;
        double confidence;
        std::vector<std::string> appliedRules;
        std::map<std::string, std::any> pipelineParameters;
    };
    
    struct RoutingContext {
        std::string requestId;
        Document document;
        std::vector<RoutingRule> routingRules;
        DocumentAnalysis documentAnalysis;
        RoutingDecision routingDecision;
        std::chrono::system_clock::time_point startTime;
    };
    
    qtforge::PluginState currentState_;
    
    std::vector<RoutingRule> loadRoutingRules() {
        std::vector<RoutingRule> rules;
        
        // Rule 1: Legal documents
        RoutingRule legalRule;
        legalRule.name = "legal_documents";
        legalRule.condition = "documentType == 'legal' || keywords.contains('contract') || keywords.contains('agreement')";
        legalRule.targetPipeline = "legal_processing_pipeline";
        legalRule.priority = 10;
        legalRule.parameters["compliance_check"] = true;
        legalRule.parameters["redaction_required"] = true;
        rules.push_back(legalRule);
        
        // Rule 2: Financial documents
        RoutingRule financialRule;
        financialRule.name = "financial_documents";
        financialRule.condition = "documentType == 'financial' || keywords.contains('invoice') || keywords.contains('receipt')";
        financialRule.targetPipeline = "financial_processing_pipeline";
        financialRule.priority = 9;
        financialRule.parameters["fraud_detection"] = true;
        financialRule.parameters["audit_trail"] = true;
        rules.push_back(financialRule);
        
        // Rule 3: Technical documents
        RoutingRule technicalRule;
        technicalRule.name = "technical_documents";
        technicalRule.condition = "documentType == 'technical' || contentCategory == 'engineering'";
        technicalRule.targetPipeline = "technical_processing_pipeline";
        technicalRule.priority = 8;
        technicalRule.parameters["code_analysis"] = true;
        technicalRule.parameters["diagram_extraction"] = true;
        rules.push_back(technicalRule);
        
        // Rule 4: Default processing
        RoutingRule defaultRule;
        defaultRule.name = "default_processing";
        defaultRule.condition = "true"; // Always matches
        defaultRule.targetPipeline = "general_processing_pipeline";
        defaultRule.priority = 1;
        rules.push_back(defaultRule);
        
        // Sort by priority (highest first)
        std::sort(rules.begin(), rules.end(), 
                 [](const RoutingRule& a, const RoutingRule& b) {
                     return a.priority > b.priority;
                 });
        
        return rules;
    }
    
    qtforge::expected<DocumentAnalysis, qtforge::Error> analyzeDocumentForRouting(const RoutingContext& context) {
        auto& messageBus = qtforge::MessageBus::instance();
        
        DocumentAnalysisRequest request;
        request.document = context.document;
        request.analysisType = "routing";
        request.requestId = context.requestId;
        
        auto response = messageBus.request<DocumentAnalysisRequest, DocumentAnalysisResponse>(
            "document.analyze", request, std::chrono::seconds(30));
        
        if (response) {
            DocumentAnalysis analysis;
            analysis.documentType = response.value().documentType;
            analysis.language = response.value().language;
            analysis.keywords = response.value().keywords;
            analysis.confidenceScore = response.value().confidence;
            analysis.features = response.value().features;
            analysis.contentCategory = response.value().category;
            
            return analysis;
        } else {
            return qtforge::Error("Document analysis failed: " + response.error().message());
        }
    }
    
    qtforge::expected<RoutingDecision, qtforge::Error> applyRoutingRules(const RoutingContext& context) {
        const auto& analysis = context.documentAnalysis;
        
        // Evaluate routing rules in priority order
        for (const auto& rule : context.routingRules) {
            if (evaluateCondition(rule.condition, analysis)) {
                RoutingDecision decision;
                decision.targetPipeline = rule.targetPipeline;
                decision.reason = "Matched rule: " + rule.name;
                decision.confidence = analysis.confidenceScore;
                decision.appliedRules.push_back(rule.name);
                decision.pipelineParameters = rule.parameters;
                
                qtforge::Logger::debug(name(), 
                    "Applied routing rule: " + rule.name + " -> " + rule.targetPipeline);
                
                return decision;
            }
        }
        
        return qtforge::Error("No routing rule matched the document");
    }
    
    bool evaluateCondition(const std::string& condition, const DocumentAnalysis& analysis) {
        // Simple condition evaluation (in practice, use a proper expression evaluator)
        if (condition == "true") {
            return true;
        }
        
        if (condition.find("documentType == 'legal'") != std::string::npos) {
            return analysis.documentType == "legal";
        }
        
        if (condition.find("documentType == 'financial'") != std::string::npos) {
            return analysis.documentType == "financial";
        }
        
        if (condition.find("documentType == 'technical'") != std::string::npos) {
            return analysis.documentType == "technical";
        }
        
        if (condition.find("keywords.contains('contract')") != std::string::npos) {
            return std::find(analysis.keywords.begin(), analysis.keywords.end(), "contract") != analysis.keywords.end();
        }
        
        if (condition.find("keywords.contains('invoice')") != std::string::npos) {
            return std::find(analysis.keywords.begin(), analysis.keywords.end(), "invoice") != analysis.keywords.end();
        }
        
        if (condition.find("contentCategory == 'engineering'") != std::string::npos) {
            return analysis.contentCategory == "engineering";
        }
        
        return false;
    }
    
    qtforge::expected<PipelineResult, qtforge::Error> executePipeline(const RoutingDecision& decision, 
                                                                    const RoutingContext& context) {
        auto& messageBus = qtforge::MessageBus::instance();
        
        PipelineExecutionRequest request;
        request.pipelineName = decision.targetPipeline;
        request.document = context.document;
        request.parameters = decision.pipelineParameters;
        request.requestId = context.requestId;
        
        auto response = messageBus.request<PipelineExecutionRequest, PipelineExecutionResponse>(
            "pipeline.execute", request, std::chrono::minutes(5));
        
        if (response) {
            return response.value().result;
        } else {
            return qtforge::Error("Pipeline execution failed: " + response.error().message());
        }
    }
};
```

## Event-Driven Orchestration

### Reactive Workflow Engine

```cpp
class ReactiveWorkflowEngine : public qtforge::IPlugin {
public:
    ReactiveWorkflowEngine() : currentState_(qtforge::PluginState::Unloaded) {}
    
    std::string name() const override { return "ReactiveWorkflowEngine"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "Event-driven workflow orchestration engine";
    }
    
    qtforge::expected<void, qtforge::Error> initialize() override {
        try {
            // Setup event handlers
            setupEventHandlers();
            
            // Load workflow definitions
            loadWorkflowDefinitions();
            
            currentState_ = qtforge::PluginState::Initialized;
            return {};
            
        } catch (const std::exception& e) {
            currentState_ = qtforge::PluginState::Error;
            return qtforge::Error("Reactive workflow engine initialization failed: " + std::string(e.what()));
        }
    }
    
    qtforge::expected<std::string, qtforge::Error> startWorkflow(const std::string& workflowName, 
                                                               const std::map<std::string, std::any>& initialData) {
        auto workflowIt = workflowDefinitions_.find(workflowName);
        if (workflowIt == workflowDefinitions_.end()) {
            return qtforge::Error("Workflow not found: " + workflowName);
        }
        
        std::string instanceId = generateInstanceId();
        
        WorkflowInstance instance;
        instance.instanceId = instanceId;
        instance.workflowName = workflowName;
        instance.definition = workflowIt->second;
        instance.currentState = "started";
        instance.data = initialData;
        instance.startTime = std::chrono::system_clock::now();
        
        {
            std::lock_guard<std::mutex> lock(instancesMutex_);
            activeInstances_[instanceId] = instance;
        }
        
        qtforge::Logger::info(name(), "Started workflow: " + workflowName + " instance: " + instanceId);
        
        // Trigger initial event
        WorkflowEvent startEvent;
        startEvent.type = "workflow.started";
        startEvent.instanceId = instanceId;
        startEvent.data = initialData;
        
        processEvent(startEvent);
        
        return instanceId;
    }

private:
    struct WorkflowDefinition {
        std::string name;
        std::map<std::string, WorkflowState> states;
        std::vector<WorkflowTransition> transitions;
        std::map<std::string, WorkflowAction> actions;
        std::string initialState = "started";
    };
    
    struct WorkflowState {
        std::string name;
        std::vector<std::string> allowedTransitions;
        std::vector<std::string> entryActions;
        std::vector<std::string> exitActions;
        std::chrono::seconds timeout = std::chrono::seconds(0);
    };
    
    struct WorkflowTransition {
        std::string name;
        std::string fromState;
        std::string toState;
        std::string triggerEvent;
        std::string condition;
        std::vector<std::string> actions;
    };
    
    struct WorkflowAction {
        std::string name;
        std::string type; // "message", "plugin_call", "script", etc.
        std::map<std::string, std::any> parameters;
    };
    
    struct WorkflowInstance {
        std::string instanceId;
        std::string workflowName;
        WorkflowDefinition definition;
        std::string currentState;
        std::map<std::string, std::any> data;
        std::chrono::system_clock::time_point startTime;
        std::chrono::system_clock::time_point lastActivity;
        std::vector<std::string> executionLog;
    };
    
    struct WorkflowEvent {
        std::string type;
        std::string instanceId;
        std::map<std::string, std::any> data;
        std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
    };
    
    qtforge::PluginState currentState_;
    std::map<std::string, WorkflowDefinition> workflowDefinitions_;
    std::map<std::string, WorkflowInstance> activeInstances_;
    std::mutex instancesMutex_;
    std::vector<qtforge::SubscriptionHandle> subscriptions_;
    
    void setupEventHandlers() {
        auto& messageBus = qtforge::MessageBus::instance();
        
        // Subscribe to various event types
        subscriptions_.emplace_back(
            messageBus.subscribe<DataProcessedEvent>("data.processed",
                [this](const DataProcessedEvent& event) {
                    WorkflowEvent workflowEvent;
                    workflowEvent.type = "data.processed";
                    workflowEvent.instanceId = event.workflowInstanceId;
                    workflowEvent.data["result"] = event.result;
                    workflowEvent.data["success"] = event.success;
                    processEvent(workflowEvent);
                })
        );
        
        subscriptions_.emplace_back(
            messageBus.subscribe<UserActionEvent>("user.action",
                [this](const UserActionEvent& event) {
                    WorkflowEvent workflowEvent;
                    workflowEvent.type = "user.action";
                    workflowEvent.instanceId = event.workflowInstanceId;
                    workflowEvent.data["action"] = event.action;
                    workflowEvent.data["userId"] = event.userId;
                    processEvent(workflowEvent);
                })
        );
        
        subscriptions_.emplace_back(
            messageBus.subscribe<TimerEvent>("timer.expired",
                [this](const TimerEvent& event) {
                    WorkflowEvent workflowEvent;
                    workflowEvent.type = "timer.expired";
                    workflowEvent.instanceId = event.workflowInstanceId;
                    workflowEvent.data["timerId"] = event.timerId;
                    processEvent(workflowEvent);
                })
        );
    }
    
    void loadWorkflowDefinitions() {
        // Load workflow definitions from configuration
        // This is a simplified example - in practice, load from files or database
        
        WorkflowDefinition documentApprovalWorkflow;
        documentApprovalWorkflow.name = "document_approval";
        
        // Define states
        WorkflowState startedState;
        startedState.name = "started";
        startedState.allowedTransitions = {"submit_for_review"};
        startedState.entryActions = {"notify_submitter"};
        documentApprovalWorkflow.states["started"] = startedState;
        
        WorkflowState reviewState;
        reviewState.name = "under_review";
        reviewState.allowedTransitions = {"approve", "reject", "request_changes"};
        reviewState.entryActions = {"assign_reviewer", "set_review_timer"};
        reviewState.timeout = std::chrono::hours(24);
        documentApprovalWorkflow.states["under_review"] = reviewState;
        
        WorkflowState approvedState;
        approvedState.name = "approved";
        approvedState.entryActions = {"notify_approval", "archive_document"};
        documentApprovalWorkflow.states["approved"] = approvedState;
        
        // Define transitions
        WorkflowTransition submitTransition;
        submitTransition.name = "submit_for_review";
        submitTransition.fromState = "started";
        submitTransition.toState = "under_review";
        submitTransition.triggerEvent = "document.submitted";
        submitTransition.actions = {"validate_document", "assign_reviewer"};
        documentApprovalWorkflow.transitions.push_back(submitTransition);
        
        WorkflowTransition approveTransition;
        approveTransition.name = "approve";
        approveTransition.fromState = "under_review";
        approveTransition.toState = "approved";
        approveTransition.triggerEvent = "user.action";
        approveTransition.condition = "data.action == 'approve'";
        approveTransition.actions = {"finalize_approval"};
        documentApprovalWorkflow.transitions.push_back(approveTransition);
        
        workflowDefinitions_["document_approval"] = documentApprovalWorkflow;
    }
    
    void processEvent(const WorkflowEvent& event) {
        std::lock_guard<std::mutex> lock(instancesMutex_);
        
        auto instanceIt = activeInstances_.find(event.instanceId);
        if (instanceIt == activeInstances_.end()) {
            qtforge::Logger::warning(name(), "Event for unknown workflow instance: " + event.instanceId);
            return;
        }
        
        auto& instance = instanceIt->second;
        
        qtforge::Logger::debug(name(), 
            "Processing event: " + event.type + " for instance: " + event.instanceId);
        
        // Find applicable transitions
        for (const auto& transition : instance.definition.transitions) {
            if (transition.fromState == instance.currentState && 
                transition.triggerEvent == event.type) {
                
                // Check condition if specified
                if (!transition.condition.empty() && !evaluateCondition(transition.condition, event, instance)) {
                    continue;
                }
                
                // Execute transition
                executeTransition(transition, event, instance);
                break;
            }
        }
        
        instance.lastActivity = std::chrono::system_clock::now();
    }
    
    void executeTransition(const WorkflowTransition& transition, 
                          const WorkflowEvent& event, 
                          WorkflowInstance& instance) {
        
        qtforge::Logger::info(name(), 
            "Executing transition: " + transition.name + " for instance: " + instance.instanceId);
        
        // Execute exit actions for current state
        auto currentStateIt = instance.definition.states.find(instance.currentState);
        if (currentStateIt != instance.definition.states.end()) {
            for (const auto& actionName : currentStateIt->second.exitActions) {
                executeAction(actionName, event, instance);
            }
        }
        
        // Execute transition actions
        for (const auto& actionName : transition.actions) {
            executeAction(actionName, event, instance);
        }
        
        // Change state
        instance.currentState = transition.toState;
        instance.executionLog.push_back("Transitioned to: " + transition.toState);
        
        // Execute entry actions for new state
        auto newStateIt = instance.definition.states.find(instance.currentState);
        if (newStateIt != instance.definition.states.end()) {
            for (const auto& actionName : newStateIt->second.entryActions) {
                executeAction(actionName, event, instance);
            }
        }
        
        qtforge::Logger::info(name(), 
            "Instance " + instance.instanceId + " transitioned to state: " + instance.currentState);
    }
    
    void executeAction(const std::string& actionName, 
                      const WorkflowEvent& event, 
                      WorkflowInstance& instance) {
        
        auto actionIt = instance.definition.actions.find(actionName);
        if (actionIt == instance.definition.actions.end()) {
            qtforge::Logger::warning(name(), "Action not found: " + actionName);
            return;
        }
        
        const auto& action = actionIt->second;
        
        qtforge::Logger::debug(name(), 
            "Executing action: " + actionName + " for instance: " + instance.instanceId);
        
        if (action.type == "message") {
            executeMessageAction(action, event, instance);
        } else if (action.type == "plugin_call") {
            executePluginCallAction(action, event, instance);
        } else if (action.type == "timer") {
            executeTimerAction(action, event, instance);
        } else {
            qtforge::Logger::warning(name(), "Unknown action type: " + action.type);
        }
    }
    
    void executeMessageAction(const WorkflowAction& action, 
                             const WorkflowEvent& event, 
                             WorkflowInstance& instance) {
        auto& messageBus = qtforge::MessageBus::instance();
        
        std::string topic = std::any_cast<std::string>(action.parameters.at("topic"));
        
        WorkflowActionMessage message;
        message.instanceId = instance.instanceId;
        message.actionName = action.name;
        message.data = event.data;
        
        messageBus.publish(topic, message);
    }
};
```

## Best Practices

### 1. Orchestration Design Principles

- **Loose Coupling**: Keep orchestrated components loosely coupled
- **Error Handling**: Implement comprehensive error handling and compensation
- **Monitoring**: Add extensive logging and monitoring
- **Scalability**: Design for horizontal scaling
- **Testability**: Make orchestrations easily testable

### 2. Performance Considerations

- **Parallel Execution**: Use parallel processing where possible
- **Resource Management**: Manage resources efficiently
- **Timeout Handling**: Implement appropriate timeouts
- **Caching**: Cache intermediate results when beneficial
- **Load Balancing**: Distribute load across available resources

### 3. Reliability Patterns

- **Circuit Breakers**: Prevent cascading failures
- **Retry Logic**: Implement intelligent retry mechanisms
- **Compensation**: Provide compensation for failed operations
- **Health Checks**: Monitor component health
- **Graceful Degradation**: Handle partial failures gracefully

## See Also

- **[Workflow Orchestration](../user-guide/workflow-orchestration.md)**: Orchestration concepts and patterns
- **[Advanced Plugin Tutorial](../tutorials/orchestration-tutorial.md)**: Hands-on orchestration tutorial
- **[Message Bus](../api/communication/message-bus.md)**: Inter-plugin communication
- **[Error Handling](../api/utils/error-handling.md)**: Error handling patterns
