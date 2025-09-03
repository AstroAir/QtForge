# Basic Plugin Example

This example demonstrates how to create a simple QtForge plugin that implements basic functionality.

## Overview

The basic plugin example shows:
- Plugin interface implementation
- Lifecycle management
- Configuration handling
- Simple message communication
- Error handling

## Plugin Implementation

### Header File (basic_plugin.hpp)

```cpp
#pragma once

#include <qtforge/core/plugin_interface.hpp>
#include <qtforge/communication/message_bus.hpp>
#include <memory>
#include <string>

class BasicPlugin : public qtforge::IPlugin {
public:
    BasicPlugin();
    ~BasicPlugin() override;
    
    // Plugin metadata
    std::string name() const override;
    std::string version() const override;
    std::string description() const override;
    std::string author() const override;
    std::vector<std::string> dependencies() const override;
    
    // Plugin lifecycle
    qtforge::expected<void, qtforge::Error> initialize() override;
    qtforge::expected<void, qtforge::Error> activate() override;
    qtforge::expected<void, qtforge::Error> deactivate() override;
    void cleanup() override;
    
    // Plugin state
    qtforge::PluginState state() const override;
    bool isCompatible(const std::string& version) const override;
    
    // Custom functionality
    void processMessage(const std::string& message);
    std::string getStatus() const;
    
private:
    qtforge::PluginState currentState_;
    std::string status_;
    qtforge::SubscriptionHandle messageHandle_;
    
    void handleIncomingMessage(const qtforge::TextMessage& message);
};

// Plugin factory functions
extern "C" QTFORGE_EXPORT qtforge::IPlugin* createPlugin();
extern "C" QTFORGE_EXPORT void destroyPlugin(qtforge::IPlugin* plugin);
extern "C" QTFORGE_EXPORT qtforge::PluginInfo getPluginInfo();
```

### Implementation File (basic_plugin.cpp)

```cpp
#include "basic_plugin.hpp"
#include <qtforge/core/plugin_manager.hpp>
#include <qtforge/communication/message_bus.hpp>
#include <qtforge/utils/logger.hpp>
#include <iostream>

BasicPlugin::BasicPlugin() 
    : currentState_(qtforge::PluginState::Unloaded)
    , status_("Created") {
}

BasicPlugin::~BasicPlugin() {
    cleanup();
}

// Plugin metadata
std::string BasicPlugin::name() const {
    return "BasicPlugin";
}

std::string BasicPlugin::version() const {
    return "1.0.0";
}

std::string BasicPlugin::description() const {
    return "A basic example plugin demonstrating QtForge functionality";
}

std::string BasicPlugin::author() const {
    return "QtForge Team";
}

std::vector<std::string> BasicPlugin::dependencies() const {
    return {}; // No dependencies
}

// Plugin lifecycle
qtforge::expected<void, qtforge::Error> BasicPlugin::initialize() {
    try {
        qtforge::Logger::info("BasicPlugin", "Initializing plugin...");
        
        // Subscribe to messages
        auto& messageBus = qtforge::MessageBus::instance();
        messageHandle_ = messageBus.subscribe<qtforge::TextMessage>("basic.messages",
            [this](const qtforge::TextMessage& msg) {
                handleIncomingMessage(msg);
            });
        
        currentState_ = qtforge::PluginState::Initialized;
        status_ = "Initialized";
        
        qtforge::Logger::info("BasicPlugin", "Plugin initialized successfully");
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        status_ = "Initialization failed: " + std::string(e.what());
        return qtforge::Error("Initialization failed: " + std::string(e.what()));
    }
}

qtforge::expected<void, qtforge::Error> BasicPlugin::activate() {
    if (currentState_ != qtforge::PluginState::Initialized) {
        return qtforge::Error("Plugin must be initialized before activation");
    }
    
    try {
        qtforge::Logger::info("BasicPlugin", "Activating plugin...");
        
        // Publish activation message
        auto& messageBus = qtforge::MessageBus::instance();
        qtforge::TextMessage activationMsg;
        activationMsg.content = "BasicPlugin activated";
        activationMsg.sender = name();
        activationMsg.timestamp = std::chrono::system_clock::now();
        
        messageBus.publish("plugin.events", activationMsg);
        
        currentState_ = qtforge::PluginState::Active;
        status_ = "Active";
        
        qtforge::Logger::info("BasicPlugin", "Plugin activated successfully");
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        status_ = "Activation failed: " + std::string(e.what());
        return qtforge::Error("Activation failed: " + std::string(e.what()));
    }
}

qtforge::expected<void, qtforge::Error> BasicPlugin::deactivate() {
    if (currentState_ != qtforge::PluginState::Active) {
        return qtforge::Error("Plugin is not active");
    }
    
    try {
        qtforge::Logger::info("BasicPlugin", "Deactivating plugin...");
        
        // Publish deactivation message
        auto& messageBus = qtforge::MessageBus::instance();
        qtforge::TextMessage deactivationMsg;
        deactivationMsg.content = "BasicPlugin deactivated";
        deactivationMsg.sender = name();
        deactivationMsg.timestamp = std::chrono::system_clock::now();
        
        messageBus.publish("plugin.events", deactivationMsg);
        
        currentState_ = qtforge::PluginState::Initialized;
        status_ = "Deactivated";
        
        qtforge::Logger::info("BasicPlugin", "Plugin deactivated successfully");
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        status_ = "Deactivation failed: " + std::string(e.what());
        return qtforge::Error("Deactivation failed: " + std::string(e.what()));
    }
}

void BasicPlugin::cleanup() {
    qtforge::Logger::info("BasicPlugin", "Cleaning up plugin...");
    
    // Unsubscribe from messages
    if (messageHandle_.isValid()) {
        messageHandle_.unsubscribe();
    }
    
    currentState_ = qtforge::PluginState::Unloaded;
    status_ = "Cleaned up";
    
    qtforge::Logger::info("BasicPlugin", "Plugin cleaned up");
}

// Plugin state
qtforge::PluginState BasicPlugin::state() const {
    return currentState_;
}

bool BasicPlugin::isCompatible(const std::string& version) const {
    // Simple version compatibility check
    return version >= "1.0.0";
}

// Custom functionality
void BasicPlugin::processMessage(const std::string& message) {
    qtforge::Logger::info("BasicPlugin", "Processing message: " + message);
    
    // Simple message processing
    if (message == "status") {
        qtforge::Logger::info("BasicPlugin", "Status: " + status_);
    } else if (message == "ping") {
        qtforge::Logger::info("BasicPlugin", "Pong!");
    } else {
        qtforge::Logger::info("BasicPlugin", "Unknown message: " + message);
    }
}

std::string BasicPlugin::getStatus() const {
    return status_;
}

void BasicPlugin::handleIncomingMessage(const qtforge::TextMessage& message) {
    qtforge::Logger::info("BasicPlugin", 
        "Received message from " + message.sender + ": " + message.content);
    
    processMessage(message.content);
}

// Plugin factory functions
extern "C" QTFORGE_EXPORT qtforge::IPlugin* createPlugin() {
    return new BasicPlugin();
}

extern "C" QTFORGE_EXPORT void destroyPlugin(qtforge::IPlugin* plugin) {
    delete plugin;
}

extern "C" QTFORGE_EXPORT qtforge::PluginInfo getPluginInfo() {
    qtforge::PluginInfo info;
    info.name = "BasicPlugin";
    info.version = "1.0.0";
    info.description = "A basic example plugin";
    info.author = "QtForge Team";
    return info;
}
```

## Build Configuration

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(BasicPlugin)

# Find QtForge
find_package(QtForge REQUIRED)

# Create plugin library
add_library(BasicPlugin SHARED
    basic_plugin.cpp
    basic_plugin.hpp
)

# Link QtForge libraries
target_link_libraries(BasicPlugin
    QtForge::Core
    QtForge::Communication
    QtForge::Utils
)

# Set plugin properties
set_target_properties(BasicPlugin PROPERTIES
    PREFIX ""
    SUFFIX ".qtplugin"
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

# Compiler-specific settings
if(MSVC)
    target_compile_definitions(BasicPlugin PRIVATE
        QTFORGE_EXPORT=__declspec(dllexport)
    )
else()
    target_compile_definitions(BasicPlugin PRIVATE
        QTFORGE_EXPORT=__attribute__((visibility("default")))
    )
endif()

# Install plugin
install(TARGETS BasicPlugin
    DESTINATION plugins/
)

# Install plugin manifest
install(FILES plugin.json
    DESTINATION plugins/
)
```

### Plugin Manifest (plugin.json)

```json
{
    "name": "BasicPlugin",
    "version": "1.0.0",
    "description": "A basic example plugin demonstrating QtForge functionality",
    "author": "QtForge Team",
    "license": "MIT",
    "qtforge_version": ">=1.0.0",
    "dependencies": [],
    "capabilities": [
        "message-processing",
        "status-reporting"
    ],
    "configuration": {
        "enabled": {
            "type": "boolean",
            "default": true,
            "description": "Enable the plugin"
        },
        "log_level": {
            "type": "string",
            "default": "info",
            "enum": ["debug", "info", "warning", "error"],
            "description": "Logging level"
        }
    },
    "resources": {
        "memory_limit": "10MB",
        "cpu_limit": "5%"
    }
}
```

## Usage Example

### Loading and Using the Plugin

```cpp
#include <qtforge/core/plugin_manager.hpp>
#include <qtforge/communication/message_bus.hpp>
#include <iostream>

int main() {
    try {
        // Create plugin manager
        qtforge::PluginManager manager;
        
        // Load the basic plugin
        auto result = manager.loadPlugin("BasicPlugin.qtplugin");
        if (!result) {
            std::cerr << "Failed to load plugin: " << result.error().message() << std::endl;
            return 1;
        }
        
        auto plugin = result.value();
        std::cout << "Loaded plugin: " << plugin->name() << " v" << plugin->version() << std::endl;
        
        // Initialize the plugin
        auto initResult = plugin->initialize();
        if (!initResult) {
            std::cerr << "Failed to initialize plugin: " << initResult.error().message() << std::endl;
            return 1;
        }
        
        // Activate the plugin
        auto activateResult = plugin->activate();
        if (!activateResult) {
            std::cerr << "Failed to activate plugin: " << activateResult.error().message() << std::endl;
            return 1;
        }
        
        // Send messages to the plugin
        auto& messageBus = qtforge::MessageBus::instance();
        
        qtforge::TextMessage msg1;
        msg1.content = "ping";
        msg1.sender = "main";
        msg1.timestamp = std::chrono::system_clock::now();
        messageBus.publish("basic.messages", msg1);
        
        qtforge::TextMessage msg2;
        msg2.content = "status";
        msg2.sender = "main";
        msg2.timestamp = std::chrono::system_clock::now();
        messageBus.publish("basic.messages", msg2);
        
        // Wait a bit for message processing
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Deactivate and cleanup
        plugin->deactivate();
        plugin->cleanup();
        
        std::cout << "Plugin example completed successfully" << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
```

## Configuration

The plugin can be configured through the plugin manifest or at runtime:

### Runtime Configuration

```cpp
// Get plugin configuration
auto config = plugin->getConfiguration();

// Update configuration
config.set("log_level", "debug");
config.set("enabled", true);

// Apply configuration
plugin->updateConfiguration(config);
```

## Testing

### Unit Tests

```cpp
#include <gtest/gtest.h>
#include "basic_plugin.hpp"

class BasicPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin_ = std::make_unique<BasicPlugin>();
    }
    
    void TearDown() override {
        plugin_->cleanup();
    }
    
    std::unique_ptr<BasicPlugin> plugin_;
};

TEST_F(BasicPluginTest, PluginMetadata) {
    EXPECT_EQ(plugin_->name(), "BasicPlugin");
    EXPECT_EQ(plugin_->version(), "1.0.0");
    EXPECT_FALSE(plugin_->description().empty());
}

TEST_F(BasicPluginTest, PluginLifecycle) {
    // Test initialization
    auto initResult = plugin_->initialize();
    EXPECT_TRUE(initResult.has_value());
    EXPECT_EQ(plugin_->state(), qtforge::PluginState::Initialized);
    
    // Test activation
    auto activateResult = plugin_->activate();
    EXPECT_TRUE(activateResult.has_value());
    EXPECT_EQ(plugin_->state(), qtforge::PluginState::Active);
    
    // Test deactivation
    auto deactivateResult = plugin_->deactivate();
    EXPECT_TRUE(deactivateResult.has_value());
    EXPECT_EQ(plugin_->state(), qtforge::PluginState::Initialized);
}

TEST_F(BasicPluginTest, MessageProcessing) {
    plugin_->initialize();
    plugin_->activate();
    
    // Test message processing
    plugin_->processMessage("ping");
    plugin_->processMessage("status");
    plugin_->processMessage("unknown");
    
    // Verify status
    EXPECT_FALSE(plugin_->getStatus().empty());
}
```

## Key Features Demonstrated

1. **Plugin Interface Implementation**: Complete implementation of IPlugin interface
2. **Lifecycle Management**: Proper initialization, activation, deactivation, and cleanup
3. **Message Communication**: Publishing and subscribing to messages
4. **Error Handling**: Comprehensive error handling with expected<T, Error>
5. **Configuration**: Plugin manifest and runtime configuration
6. **Logging**: Integrated logging support
7. **Testing**: Unit tests for plugin functionality

## Next Steps

- **[Service Plugin Example](service-plugin.md)**: Learn about service plugins
- **[Network Plugin Example](network-plugin.md)**: Network-enabled plugins
- **[Advanced Examples](advanced.md)**: More complex plugin patterns
- **[Plugin Development Guide](../user-guide/plugin-development.md)**: Comprehensive development guide
