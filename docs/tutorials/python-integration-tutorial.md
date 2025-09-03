# Python Integration Tutorial

This tutorial demonstrates how to create powerful Python plugins and integrate them seamlessly with C++ QtForge applications.

## Prerequisites

- Completed [Basic Plugin Tutorial](../getting-started/first-plugin.md)
- Python 3.8+ installed
- Basic Python programming knowledge
- Understanding of QtForge plugin architecture

## Tutorial Overview

You'll build a **Machine Learning Data Processor** that demonstrates:

- Python plugin development
- C++ to Python integration
- Data exchange between languages
- Async Python operations
- Package management
- Error handling across language boundaries

## Step 1: Environment Setup

### Python Environment

```bash
# Create virtual environment
python -m venv qtforge_python_env
source qtforge_python_env/bin/activate  # Linux/macOS
# or
qtforge_python_env\Scripts\activate     # Windows

# Install required packages
pip install numpy pandas scikit-learn matplotlib qtforge-python
```

### Project Structure

```
ml-data-processor/
├── python/
│   ├── ml_processor.py
│   ├── data_analyzer.py
│   ├── model_trainer.py
│   └── requirements.txt
├── cpp/
│   ├── python_bridge.hpp
│   ├── python_bridge.cpp
│   └── ml_plugin.cpp
├── config/
│   └── plugin.json
└── CMakeLists.txt
```

## Step 2: Python Plugin Implementation

### Core Python Plugin (python/ml_processor.py)

```python
import qtforge
from qtforge.core import IPlugin, PluginState, PluginError
from qtforge.communication import MessageBus
import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
import json
import asyncio
import logging

class MLDataProcessor(IPlugin):
    def __init__(self):
        super().__init__()
        self._state = PluginState.UNLOADED
        self._message_bus = None
        self._model = None
        self._data_cache = {}
        self._logger = logging.getLogger(__name__)
        
    def name(self) -> str:
        return "MLDataProcessor"
    
    def version(self) -> str:
        return "1.0.0"
    
    def description(self) -> str:
        return "Machine Learning data processor with scikit-learn integration"
    
    def author(self) -> str:
        return "ML Team"
    
    def dependencies(self) -> list:
        return ["CorePlugin >= 1.0.0"]
    
    def initialize(self) -> bool:
        try:
            self._logger.info("Initializing ML Data Processor...")
            
            # Initialize message bus
            self._message_bus = MessageBus.instance()
            
            # Subscribe to data processing requests
            self._message_bus.subscribe("ml.data_process", self._handle_data_process)
            self._message_bus.subscribe("ml.train_model", self._handle_train_model)
            self._message_bus.subscribe("ml.predict", self._handle_predict)
            
            # Initialize ML model
            self._model = RandomForestClassifier(n_estimators=100, random_state=42)
            
            self._state = PluginState.INITIALIZED
            self._logger.info("ML Data Processor initialized successfully")
            return True
            
        except Exception as e:
            self._logger.error(f"Initialization failed: {e}")
            self._state = PluginState.ERROR
            return False
    
    def activate(self) -> bool:
        if self._state != PluginState.INITIALIZED:
            return False
        
        try:
            self._logger.info("Activating ML Data Processor...")
            
            # Publish activation message
            self._message_bus.publish("plugin.events", {
                "type": "activation",
                "plugin": self.name(),
                "timestamp": qtforge.utils.current_timestamp(),
                "capabilities": ["data_analysis", "model_training", "prediction"]
            })
            
            self._state = PluginState.ACTIVE
            self._logger.info("ML Data Processor activated successfully")
            return True
            
        except Exception as e:
            self._logger.error(f"Activation failed: {e}")
            self._state = PluginState.ERROR
            return False
    
    def deactivate(self) -> bool:
        if self._state != PluginState.ACTIVE:
            return False
        
        try:
            self._logger.info("Deactivating ML Data Processor...")
            
            # Clear data cache
            self._data_cache.clear()
            
            # Publish deactivation message
            self._message_bus.publish("plugin.events", {
                "type": "deactivation",
                "plugin": self.name(),
                "timestamp": qtforge.utils.current_timestamp()
            })
            
            self._state = PluginState.INITIALIZED
            return True
            
        except Exception as e:
            self._logger.error(f"Deactivation failed: {e}")
            self._state = PluginState.ERROR
            return False
    
    def cleanup(self):
        self._logger.info("Cleaning up ML Data Processor...")
        
        if self._message_bus:
            self._message_bus.unsubscribe_all(self)
        
        self._data_cache.clear()
        self._model = None
        self._state = PluginState.UNLOADED
        
        self._logger.info("ML Data Processor cleaned up")
    
    def state(self) -> PluginState:
        return self._state
    
    def is_compatible(self, version: str) -> bool:
        return version >= "1.0.0"
    
    # ML-specific methods
    def _handle_data_process(self, message):
        """Handle data processing requests"""
        try:
            data_id = message.get("data_id")
            data = message.get("data")
            operation = message.get("operation", "analyze")
            
            self._logger.info(f"Processing data: {data_id}, operation: {operation}")
            
            if operation == "analyze":
                result = self._analyze_data(data)
            elif operation == "clean":
                result = self._clean_data(data)
            elif operation == "transform":
                result = self._transform_data(data, message.get("transform_params", {}))
            else:
                raise ValueError(f"Unknown operation: {operation}")
            
            # Cache result
            self._data_cache[data_id] = result
            
            # Publish result
            self._message_bus.publish("ml.data_result", {
                "data_id": data_id,
                "operation": operation,
                "result": result,
                "timestamp": qtforge.utils.current_timestamp()
            })
            
        except Exception as e:
            self._logger.error(f"Data processing failed: {e}")
            self._message_bus.publish("ml.error", {
                "error": str(e),
                "operation": "data_process",
                "timestamp": qtforge.utils.current_timestamp()
            })
    
    def _handle_train_model(self, message):
        """Handle model training requests"""
        try:
            data_id = message.get("data_id")
            target_column = message.get("target_column")
            model_params = message.get("model_params", {})
            
            self._logger.info(f"Training model with data: {data_id}")
            
            # Get data from cache
            if data_id not in self._data_cache:
                raise ValueError(f"Data not found: {data_id}")
            
            data = self._data_cache[data_id]
            df = pd.DataFrame(data)
            
            # Prepare features and target
            X = df.drop(columns=[target_column])
            y = df[target_column]
            
            # Split data
            X_train, X_test, y_train, y_test = train_test_split(
                X, y, test_size=0.2, random_state=42)
            
            # Update model parameters
            if model_params:
                self._model.set_params(**model_params)
            
            # Train model
            self._model.fit(X_train, y_train)
            
            # Evaluate model
            y_pred = self._model.predict(X_test)
            accuracy = accuracy_score(y_test, y_pred)
            
            # Publish training result
            self._message_bus.publish("ml.training_result", {
                "data_id": data_id,
                "accuracy": float(accuracy),
                "model_params": model_params,
                "training_samples": len(X_train),
                "test_samples": len(X_test),
                "timestamp": qtforge.utils.current_timestamp()
            })
            
        except Exception as e:
            self._logger.error(f"Model training failed: {e}")
            self._message_bus.publish("ml.error", {
                "error": str(e),
                "operation": "train_model",
                "timestamp": qtforge.utils.current_timestamp()
            })
    
    def _handle_predict(self, message):
        """Handle prediction requests"""
        try:
            data = message.get("data")
            
            if self._model is None:
                raise ValueError("Model not trained")
            
            # Make prediction
            df = pd.DataFrame(data)
            predictions = self._model.predict(df)
            probabilities = self._model.predict_proba(df)
            
            # Publish prediction result
            self._message_bus.publish("ml.prediction_result", {
                "predictions": predictions.tolist(),
                "probabilities": probabilities.tolist(),
                "input_samples": len(df),
                "timestamp": qtforge.utils.current_timestamp()
            })
            
        except Exception as e:
            self._logger.error(f"Prediction failed: {e}")
            self._message_bus.publish("ml.error", {
                "error": str(e),
                "operation": "predict",
                "timestamp": qtforge.utils.current_timestamp()
            })
    
    def _analyze_data(self, data):
        """Analyze data and return statistics"""
        df = pd.DataFrame(data)
        
        analysis = {
            "shape": df.shape,
            "columns": df.columns.tolist(),
            "dtypes": df.dtypes.to_dict(),
            "missing_values": df.isnull().sum().to_dict(),
            "statistics": df.describe().to_dict()
        }
        
        return analysis
    
    def _clean_data(self, data):
        """Clean data by handling missing values and outliers"""
        df = pd.DataFrame(data)
        
        # Handle missing values
        numeric_columns = df.select_dtypes(include=[np.number]).columns
        df[numeric_columns] = df[numeric_columns].fillna(df[numeric_columns].mean())
        
        categorical_columns = df.select_dtypes(include=['object']).columns
        df[categorical_columns] = df[categorical_columns].fillna(df[categorical_columns].mode().iloc[0])
        
        # Remove outliers using IQR method
        for column in numeric_columns:
            Q1 = df[column].quantile(0.25)
            Q3 = df[column].quantile(0.75)
            IQR = Q3 - Q1
            lower_bound = Q1 - 1.5 * IQR
            upper_bound = Q3 + 1.5 * IQR
            df = df[(df[column] >= lower_bound) & (df[column] <= upper_bound)]
        
        return df.to_dict('records')
    
    def _transform_data(self, data, params):
        """Transform data according to parameters"""
        df = pd.DataFrame(data)
        
        # Apply transformations based on parameters
        if params.get("normalize", False):
            numeric_columns = df.select_dtypes(include=[np.number]).columns
            df[numeric_columns] = (df[numeric_columns] - df[numeric_columns].mean()) / df[numeric_columns].std()
        
        if params.get("encode_categorical", False):
            categorical_columns = df.select_dtypes(include=['object']).columns
            df = pd.get_dummies(df, columns=categorical_columns)
        
        return df.to_dict('records')

# Plugin factory function
def create_plugin():
    return MLDataProcessor()
```

## Step 3: C++ Python Bridge

### Python Bridge Header (cpp/python_bridge.hpp)

```cpp
#pragma once

#include <qtforge/core/plugin_interface.hpp>
#include <qtforge/python/python_executor.hpp>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class PythonBridge : public qtforge::IPlugin {
public:
    PythonBridge();
    ~PythonBridge() override;

    // Plugin interface
    std::string name() const override { return "PythonBridge"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "Bridge for Python plugin integration";
    }
    
    std::vector<std::string> dependencies() const override {
        return {"CorePlugin >= 1.0.0"};
    }

    // Lifecycle
    qtforge::expected<void, qtforge::Error> initialize() override;
    qtforge::expected<void, qtforge::Error> activate() override;
    qtforge::expected<void, qtforge::Error> deactivate() override;
    void cleanup() override;

    qtforge::PluginState state() const override { return currentState_; }
    bool isCompatible(const std::string& version) const override;

    // Python integration
    qtforge::expected<void, qtforge::Error> loadPythonPlugin(const std::string& pluginPath);
    qtforge::expected<std::string, qtforge::Error> callPythonFunction(
        const std::string& functionName, 
        const std::vector<std::any>& args);
    
    qtforge::expected<void, qtforge::Error> sendDataToPython(
        const std::string& dataId,
        const std::unordered_map<std::string, std::any>& data);

private:
    void setupMessageHandlers();
    void handleMLResult(const qtforge::MLResultMessage& message);
    void handleMLError(const qtforge::MLErrorMessage& message);

    qtforge::PluginState currentState_;
    std::unique_ptr<qtforge::python::PythonExecutor> pythonExecutor_;
    std::vector<qtforge::SubscriptionHandle> subscriptions_;
    std::unordered_map<std::string, std::any> dataCache_;
};
```

### Python Bridge Implementation (cpp/python_bridge.cpp)

```cpp
#include "python_bridge.hpp"
#include <qtforge/communication/message_bus.hpp>
#include <qtforge/utils/logger.hpp>
#include <qtforge/python/python_integration.hpp>

PythonBridge::PythonBridge() 
    : currentState_(qtforge::PluginState::Unloaded) {
}

PythonBridge::~PythonBridge() {
    cleanup();
}

qtforge::expected<void, qtforge::Error> PythonBridge::initialize() {
    try {
        qtforge::Logger::info(name(), "Initializing Python Bridge...");
        
        // Initialize Python integration
        qtforge::python::PythonIntegration::initialize();
        
        // Create Python executor
        pythonExecutor_ = std::make_unique<qtforge::python::PythonExecutor>();
        
        // Setup message handlers
        setupMessageHandlers();
        
        currentState_ = qtforge::PluginState::Initialized;
        qtforge::Logger::info(name(), "Python Bridge initialized successfully");
        
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        return qtforge::Error("Python Bridge initialization failed: " + std::string(e.what()));
    }
}

qtforge::expected<void, qtforge::Error> PythonBridge::activate() {
    if (currentState_ != qtforge::PluginState::Initialized) {
        return qtforge::Error("Plugin must be initialized before activation");
    }
    
    try {
        qtforge::Logger::info(name(), "Activating Python Bridge...");
        
        // Load Python ML processor
        auto loadResult = loadPythonPlugin("python/ml_processor.py");
        if (!loadResult) {
            return qtforge::Error("Failed to load Python ML processor: " + 
                                loadResult.error().message());
        }
        
        currentState_ = qtforge::PluginState::Active;
        qtforge::Logger::info(name(), "Python Bridge activated successfully");
        
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        return qtforge::Error("Python Bridge activation failed: " + std::string(e.what()));
    }
}

qtforge::expected<void, qtforge::Error> PythonBridge::loadPythonPlugin(const std::string& pluginPath) {
    try {
        // Load Python module
        std::string pythonCode = R"(
import sys
import os
sys.path.append(os.path.dirname(')" + pluginPath + R"('))

# Import the plugin module
import ml_processor

# Create plugin instance
plugin_instance = ml_processor.create_plugin()

# Initialize and activate the plugin
if plugin_instance.initialize():
    if plugin_instance.activate():
        print("Python plugin loaded and activated successfully")
    else:
        raise Exception("Failed to activate Python plugin")
else:
    raise Exception("Failed to initialize Python plugin")
)";
        
        auto result = pythonExecutor_->execute(pythonCode);
        if (!result) {
            return qtforge::Error("Failed to load Python plugin: " + result.error().message());
        }
        
        qtforge::Logger::info(name(), "Python plugin loaded successfully: " + pluginPath);
        return {};
        
    } catch (const std::exception& e) {
        return qtforge::Error("Python plugin loading failed: " + std::string(e.what()));
    }
}

void PythonBridge::setupMessageHandlers() {
    auto& messageBus = qtforge::MessageBus::instance();
    
    // Handle ML results
    subscriptions_.emplace_back(
        messageBus.subscribe<qtforge::MLResultMessage>("ml.data_result",
            [this](const qtforge::MLResultMessage& msg) {
                handleMLResult(msg);
            })
    );
    
    // Handle ML errors
    subscriptions_.emplace_back(
        messageBus.subscribe<qtforge::MLErrorMessage>("ml.error",
            [this](const qtforge::MLErrorMessage& msg) {
                handleMLError(msg);
            })
    );
}

qtforge::expected<void, qtforge::Error> PythonBridge::sendDataToPython(
    const std::string& dataId,
    const std::unordered_map<std::string, std::any>& data) {
    
    try {
        // Convert C++ data to Python-compatible format
        std::string pythonCode = "import json\n";
        pythonCode += "data = " + convertToPythonDict(data) + "\n";
        pythonCode += "message_bus.publish('ml.data_process', {\n";
        pythonCode += "    'data_id': '" + dataId + "',\n";
        pythonCode += "    'data': data,\n";
        pythonCode += "    'operation': 'analyze'\n";
        pythonCode += "})\n";
        
        auto result = pythonExecutor_->execute(pythonCode);
        if (!result) {
            return qtforge::Error("Failed to send data to Python: " + result.error().message());
        }
        
        return {};
        
    } catch (const std::exception& e) {
        return qtforge::Error("Data transmission failed: " + std::string(e.what()));
    }
}

// Plugin factory functions
extern "C" QTFORGE_EXPORT qtforge::IPlugin* createPlugin() {
    return new PythonBridge();
}

extern "C" QTFORGE_EXPORT void destroyPlugin(qtforge::IPlugin* plugin) {
    delete plugin;
}
```

## Step 4: Async Python Operations

### Async Data Processing

```python
import asyncio
import aiohttp
import aiofiles

class AsyncMLProcessor:
    def __init__(self):
        self.session = None
        
    async def __aenter__(self):
        self.session = aiohttp.ClientSession()
        return self
        
    async def __aexit__(self, exc_type, exc_val, exc_tb):
        if self.session:
            await self.session.close()
    
    async def fetch_external_data(self, url: str) -> dict:
        """Fetch data from external API asynchronously"""
        async with self.session.get(url) as response:
            if response.status == 200:
                return await response.json()
            else:
                raise Exception(f"HTTP {response.status}: {await response.text()}")
    
    async def process_large_dataset(self, file_path: str) -> dict:
        """Process large dataset asynchronously"""
        async with aiofiles.open(file_path, 'r') as file:
            data = await file.read()
            
        # Process data in chunks to avoid blocking
        chunks = [data[i:i+1000] for i in range(0, len(data), 1000)]
        
        tasks = []
        for chunk in chunks:
            task = asyncio.create_task(self.process_chunk(chunk))
            tasks.append(task)
        
        results = await asyncio.gather(*tasks)
        
        # Combine results
        combined_result = {
            "total_chunks": len(chunks),
            "processed_items": sum(len(result) for result in results),
            "results": results
        }
        
        return combined_result
    
    async def process_chunk(self, chunk: str) -> list:
        """Process a single chunk of data"""
        # Simulate async processing
        await asyncio.sleep(0.1)
        
        # Process chunk (example: split into lines)
        lines = chunk.split('\n')
        processed_lines = [line.strip().upper() for line in lines if line.strip()]
        
        return processed_lines

# Integration with main plugin
class AsyncMLDataProcessor(MLDataProcessor):
    def __init__(self):
        super().__init__()
        self.async_processor = AsyncMLProcessor()
        self.loop = None
    
    def initialize(self) -> bool:
        if not super().initialize():
            return False
        
        try:
            # Create event loop for async operations
            self.loop = asyncio.new_event_loop()
            asyncio.set_event_loop(self.loop)
            
            # Subscribe to async processing requests
            self._message_bus.subscribe("ml.async_process", self._handle_async_process)
            
            return True
            
        except Exception as e:
            self._logger.error(f"Async initialization failed: {e}")
            return False
    
    def _handle_async_process(self, message):
        """Handle asynchronous processing requests"""
        try:
            operation = message.get("operation")
            
            if operation == "fetch_external":
                url = message.get("url")
                task = self.loop.create_task(self._async_fetch_external(url))
                
            elif operation == "process_large_file":
                file_path = message.get("file_path")
                task = self.loop.create_task(self._async_process_file(file_path))
            
            else:
                raise ValueError(f"Unknown async operation: {operation}")
            
            # Schedule task completion handler
            task.add_done_callback(self._async_task_completed)
            
        except Exception as e:
            self._logger.error(f"Async processing failed: {e}")
            self._message_bus.publish("ml.error", {
                "error": str(e),
                "operation": "async_process",
                "timestamp": qtforge.utils.current_timestamp()
            })
    
    async def _async_fetch_external(self, url: str):
        """Fetch external data asynchronously"""
        async with self.async_processor as processor:
            result = await processor.fetch_external_data(url)
            return {"operation": "fetch_external", "url": url, "result": result}
    
    async def _async_process_file(self, file_path: str):
        """Process large file asynchronously"""
        async with self.async_processor as processor:
            result = await processor.process_large_dataset(file_path)
            return {"operation": "process_large_file", "file_path": file_path, "result": result}
    
    def _async_task_completed(self, task):
        """Handle completion of async tasks"""
        try:
            result = task.result()
            
            # Publish async result
            self._message_bus.publish("ml.async_result", {
                "result": result,
                "timestamp": qtforge.utils.current_timestamp()
            })
            
        except Exception as e:
            self._logger.error(f"Async task failed: {e}")
            self._message_bus.publish("ml.error", {
                "error": str(e),
                "operation": "async_task",
                "timestamp": qtforge.utils.current_timestamp()
            })
```

## Step 5: Testing Python Integration

### Integration Tests

```cpp
#include <gtest/gtest.h>
#include "python_bridge.hpp"
#include <qtforge/communication/message_bus.hpp>

class PythonIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        bridge_ = std::make_unique<PythonBridge>();
        bridge_->initialize();
        bridge_->activate();
    }
    
    void TearDown() override {
        bridge_->cleanup();
    }
    
    std::unique_ptr<PythonBridge> bridge_;
};

TEST_F(PythonIntegrationTest, DataProcessing) {
    // Prepare test data
    std::unordered_map<std::string, std::any> testData;
    testData["values"] = std::vector<double>{1.0, 2.0, 3.0, 4.0, 5.0};
    testData["labels"] = std::vector<std::string>{"A", "B", "C", "D", "E"};
    
    // Send data to Python
    auto result = bridge_->sendDataToPython("test_data_1", testData);
    EXPECT_TRUE(result.has_value());
    
    // Wait for processing result
    auto& messageBus = qtforge::MessageBus::instance();
    bool resultReceived = false;
    
    auto handle = messageBus.subscribe<qtforge::MLResultMessage>("ml.data_result",
        [&resultReceived](const qtforge::MLResultMessage& msg) {
            resultReceived = true;
            EXPECT_EQ(msg.dataId, "test_data_1");
            EXPECT_EQ(msg.operation, "analyze");
        });
    
    // Wait for result
    auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(5);
    while (!resultReceived && std::chrono::steady_clock::now() < timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    EXPECT_TRUE(resultReceived);
}

TEST_F(PythonIntegrationTest, ModelTraining) {
    // Prepare training data
    std::unordered_map<std::string, std::any> trainingData;
    // ... populate with training data
    
    auto result = bridge_->sendDataToPython("training_data", trainingData);
    EXPECT_TRUE(result.has_value());
    
    // Send training request
    auto& messageBus = qtforge::MessageBus::instance();
    qtforge::MLTrainingMessage trainingMsg;
    trainingMsg.dataId = "training_data";
    trainingMsg.targetColumn = "target";
    trainingMsg.modelParams = {{"n_estimators", 50}};
    
    messageBus.publish("ml.train_model", trainingMsg);
    
    // Wait for training result
    bool trainingCompleted = false;
    auto handle = messageBus.subscribe<qtforge::MLTrainingResultMessage>("ml.training_result",
        [&trainingCompleted](const qtforge::MLTrainingResultMessage& msg) {
            trainingCompleted = true;
            EXPECT_GT(msg.accuracy, 0.0);
            EXPECT_LE(msg.accuracy, 1.0);
        });
    
    // Wait for training completion
    auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(30);
    while (!trainingCompleted && std::chrono::steady_clock::now() < timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    EXPECT_TRUE(trainingCompleted);
}
```

## Key Learning Points

1. **Language Integration**: Seamless integration between C++ and Python
2. **Data Exchange**: Efficient data transfer between language boundaries
3. **Async Operations**: Handling asynchronous Python operations
4. **Error Handling**: Cross-language error handling strategies
5. **Package Management**: Managing Python dependencies
6. **Performance**: Optimizing cross-language communication

## Best Practices

1. **Data Serialization**: Use efficient serialization formats (JSON, MessagePack)
2. **Error Propagation**: Ensure errors are properly propagated across languages
3. **Resource Management**: Proper cleanup of Python resources
4. **Threading**: Handle threading differences between C++ and Python
5. **Testing**: Comprehensive integration testing

## Next Steps

- **[Advanced Python Patterns](../user-guide/python-integration.md)**: Advanced Python integration patterns
- **[Performance Optimization](../user-guide/performance-optimization.md)**: Optimizing Python integration performance
- **[Deployment Guide](../deployment/python-deployment.md)**: Deploying Python-integrated applications
- **[Troubleshooting](../troubleshooting/python-issues.md)**: Common Python integration issues
