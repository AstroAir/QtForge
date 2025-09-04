# Plugin Composition

This guide covers how to create composite plugins and manage complex plugin relationships in QtForge.

## Overview

Plugin composition enables:
- **Composite Plugins**: Combine multiple plugins into a single unit
- **Plugin Dependencies**: Manage complex dependency relationships
- **Service Orchestration**: Coordinate multiple services
- **Modular Architecture**: Build applications from reusable components
- **Dynamic Assembly**: Runtime plugin composition

## Composite Plugins

### Creating Composite Plugins

A composite plugin aggregates functionality from multiple sub-plugins:

```cpp
class CompositePlugin : public PluginInterface {
private:
    QList<PluginInterface*> m_subPlugins;
    
public:
    void addSubPlugin(PluginInterface* plugin) {
        m_subPlugins.append(plugin);
    }
    
    bool initialize(PluginContext* context) override {
        // Initialize all sub-plugins
        for (auto* plugin : m_subPlugins) {
            if (!plugin->initialize(context)) {
                // Rollback on failure
                shutdown();
                return false;
            }
        }
        return true;
    }
    
    bool shutdown() override {
        // Shutdown in reverse order
        for (int i = m_subPlugins.size() - 1; i >= 0; --i) {
            m_subPlugins[i]->shutdown();
        }
        return true;
    }
};
```

### Composite Plugin Factory

Create a factory for building composite plugins:

```cpp
class CompositePluginFactory {
public:
    static std::unique_ptr<CompositePlugin> createDataProcessingComposite() {
        auto composite = std::make_unique<CompositePlugin>();
        
        // Add data input plugin
        composite->addSubPlugin(new DataInputPlugin());
        
        // Add data transformation plugin
        composite->addSubPlugin(new DataTransformPlugin());
        
        // Add data output plugin
        composite->addSubPlugin(new DataOutputPlugin());
        
        return composite;
    }
    
    static std::unique_ptr<CompositePlugin> createUIComposite() {
        auto composite = std::make_unique<CompositePlugin>();
        
        // Add UI components
        composite->addSubPlugin(new MenuPlugin());
        composite->addSubPlugin(new ToolbarPlugin());
        composite->addSubPlugin(new StatusBarPlugin());
        
        return composite;
    }
};
```

## Plugin Dependencies

### Dependency Declaration

Declare plugin dependencies in metadata:

```cpp
PluginMetadata getMetadata() const override {
    PluginMetadata metadata;
    metadata.id = "com.example.dependent";
    metadata.name = "Dependent Plugin";
    metadata.version = "1.0.0";
    
    // Declare dependencies
    metadata.dependencies = {
        "com.example.core",
        "com.example.utils",
        "com.example.network"
    };
    
    return metadata;
}
```

### Dependency Resolution

Implement automatic dependency resolution:

```cpp
class DependencyResolver {
public:
    QList<QString> resolveDependencies(const QString& pluginId) {
        QList<QString> resolved;
        QSet<QString> visited;
        
        resolveDependenciesRecursive(pluginId, resolved, visited);
        return resolved;
    }
    
private:
    void resolveDependenciesRecursive(const QString& pluginId, 
                                    QList<QString>& resolved, 
                                    QSet<QString>& visited) {
        if (visited.contains(pluginId)) {
            return; // Avoid circular dependencies
        }
        
        visited.insert(pluginId);
        
        PluginMetadata metadata = getPluginMetadata(pluginId);
        for (const QString& dep : metadata.dependencies) {
            resolveDependenciesRecursive(dep, resolved, visited);
        }
        
        if (!resolved.contains(pluginId)) {
            resolved.append(pluginId);
        }
    }
};
```

### Circular Dependency Detection

Detect and handle circular dependencies:

```cpp
class CircularDependencyDetector {
public:
    bool hasCircularDependency(const QString& pluginId) {
        QSet<QString> visited;
        QSet<QString> recursionStack;
        
        return hasCircularDependencyRecursive(pluginId, visited, recursionStack);
    }
    
private:
    bool hasCircularDependencyRecursive(const QString& pluginId,
                                       QSet<QString>& visited,
                                       QSet<QString>& recursionStack) {
        visited.insert(pluginId);
        recursionStack.insert(pluginId);
        
        PluginMetadata metadata = getPluginMetadata(pluginId);
        for (const QString& dep : metadata.dependencies) {
            if (!visited.contains(dep)) {
                if (hasCircularDependencyRecursive(dep, visited, recursionStack)) {
                    return true;
                }
            } else if (recursionStack.contains(dep)) {
                return true; // Circular dependency found
            }
        }
        
        recursionStack.remove(pluginId);
        return false;
    }
};
```

## Service Orchestration

### Service Composition

Compose services from multiple plugins:

```cpp
class ServiceComposer {
public:
    template<typename ServiceInterface>
    ServiceInterface* composeService(const QList<QString>& pluginIds) {
        auto composite = new CompositeService<ServiceInterface>();
        
        for (const QString& pluginId : pluginIds) {
            auto* plugin = PluginManager::instance()->getPlugin(pluginId);
            if (auto* servicePlugin = qobject_cast<ServicePlugin*>(plugin)) {
                auto* service = servicePlugin->getService<ServiceInterface>();
                if (service) {
                    composite->addService(service);
                }
            }
        }
        
        return composite;
    }
};

template<typename ServiceInterface>
class CompositeService : public ServiceInterface {
private:
    QList<ServiceInterface*> m_services;
    
public:
    void addService(ServiceInterface* service) {
        m_services.append(service);
    }
    
    // Implement service interface methods to delegate to sub-services
    void processRequest(const Request& request) override {
        for (auto* service : m_services) {
            service->processRequest(request);
        }
    }
};
```

### Pipeline Composition

Create processing pipelines from plugins:

```cpp
class ProcessingPipeline {
private:
    QList<ProcessorPlugin*> m_processors;
    
public:
    void addProcessor(ProcessorPlugin* processor) {
        m_processors.append(processor);
    }
    
    QVariant process(const QVariant& input) {
        QVariant result = input;
        
        for (auto* processor : m_processors) {
            result = processor->process(result);
        }
        
        return result;
    }
};

// Usage
ProcessingPipeline pipeline;
pipeline.addProcessor(new ValidationProcessor());
pipeline.addProcessor(new TransformationProcessor());
pipeline.addProcessor(new EnrichmentProcessor());

QVariant result = pipeline.process(inputData);
```

## Modular Architecture

### Module Definition

Define application modules as plugin collections:

```cpp
struct Module {
    QString name;
    QString version;
    QList<QString> pluginIds;
    QList<QString> dependencies;
    
    bool isCompatibleWith(const Module& other) const {
        // Check version compatibility
        return isVersionCompatible(version, other.version);
    }
};

class ModuleManager {
public:
    bool loadModule(const Module& module) {
        // Check dependencies
        for (const QString& dep : module.dependencies) {
            if (!isModuleLoaded(dep)) {
                qWarning() << "Module dependency not loaded:" << dep;
                return false;
            }
        }
        
        // Load all plugins in module
        for (const QString& pluginId : module.pluginIds) {
            if (!PluginManager::instance()->loadPlugin(pluginId)) {
                // Rollback on failure
                unloadModule(module);
                return false;
            }
        }
        
        m_loadedModules.insert(module.name, module);
        return true;
    }
    
private:
    QMap<QString, Module> m_loadedModules;
};
```

### Feature Composition

Compose application features from modules:

```cpp
class FeatureComposer {
public:
    bool enableFeature(const QString& featureName) {
        auto feature = m_features.value(featureName);
        if (feature.modules.isEmpty()) {
            return false;
        }
        
        // Load required modules
        for (const QString& moduleName : feature.modules) {
            if (!ModuleManager::instance()->loadModule(moduleName)) {
                return false;
            }
        }
        
        // Configure feature
        configureFeature(feature);
        
        m_enabledFeatures.insert(featureName);
        return true;
    }
    
private:
    struct Feature {
        QString name;
        QList<QString> modules;
        QVariantMap configuration;
    };
    
    QMap<QString, Feature> m_features;
    QSet<QString> m_enabledFeatures;
};
```

## Dynamic Assembly

### Runtime Composition

Compose plugins at runtime based on configuration:

```cpp
class DynamicComposer {
public:
    std::unique_ptr<CompositePlugin> composeFromConfig(const QJsonObject& config) {
        auto composite = std::make_unique<CompositePlugin>();
        
        QJsonArray plugins = config["plugins"].toArray();
        for (const auto& value : plugins) {
            QJsonObject pluginConfig = value.toObject();
            QString pluginId = pluginConfig["id"].toString();
            
            auto* plugin = createPluginFromConfig(pluginId, pluginConfig);
            if (plugin) {
                composite->addSubPlugin(plugin);
            }
        }
        
        return composite;
    }
    
private:
    PluginInterface* createPluginFromConfig(const QString& pluginId, 
                                          const QJsonObject& config) {
        // Create plugin instance
        auto* plugin = PluginFactory::instance()->createPlugin(pluginId);
        if (!plugin) {
            return nullptr;
        }
        
        // Configure plugin
        if (auto* configurable = qobject_cast<ConfigurablePlugin*>(plugin)) {
            configurable->configure(config);
        }
        
        return plugin;
    }
};
```

### Adaptive Composition

Adapt plugin composition based on runtime conditions:

```cpp
class AdaptiveComposer {
public:
    void adaptComposition() {
        SystemMetrics metrics = SystemMonitor::instance()->getMetrics();
        
        if (metrics.cpuUsage > 80) {
            // High CPU usage - use lightweight plugins
            replacePlugin("com.heavy.processor", "com.light.processor");
        }
        
        if (metrics.memoryUsage > 90) {
            // High memory usage - unload optional plugins
            unloadOptionalPlugins();
        }
        
        if (metrics.networkLatency > 1000) {
            // High latency - enable caching plugins
            loadPlugin("com.cache.plugin");
        }
    }
    
private:
    void replacePlugin(const QString& oldPluginId, const QString& newPluginId) {
        PluginManager* manager = PluginManager::instance();
        
        // Unload old plugin
        manager->unloadPlugin(oldPluginId);
        
        // Load new plugin
        manager->loadPlugin(newPluginId);
    }
};
```

## Best Practices

### Composition Guidelines

1. **Loose Coupling**: Minimize dependencies between composed plugins
2. **Clear Interfaces**: Define clear service interfaces
3. **Error Handling**: Handle composition failures gracefully
4. **Resource Management**: Properly manage resources in composite plugins
5. **Testing**: Test compositions thoroughly

### Performance Considerations

```cpp
// Lazy initialization for better performance
class LazyCompositePlugin : public CompositePlugin {
private:
    QList<std::function<PluginInterface*()>> m_pluginFactories;
    QList<PluginInterface*> m_initializedPlugins;
    
public:
    void addPluginFactory(std::function<PluginInterface*()> factory) {
        m_pluginFactories.append(factory);
    }
    
    PluginInterface* getPlugin(int index) {
        if (index >= m_initializedPlugins.size()) {
            // Lazy initialization
            auto* plugin = m_pluginFactories[index]();
            m_initializedPlugins.append(plugin);
        }
        return m_initializedPlugins[index];
    }
};
```

## Troubleshooting

### Common Issues

1. **Circular Dependencies**: Use dependency analysis tools
2. **Initialization Order**: Ensure proper initialization sequence
3. **Resource Conflicts**: Avoid resource conflicts between plugins
4. **Version Incompatibilities**: Implement version checking

### Debugging Composition

```cpp
// Enable composition debugging
CompositionDebugger::instance()->setEnabled(true);

// Log composition events
connect(CompositePlugin::instance(), &CompositePlugin::pluginAdded,
        [](const QString& pluginId) {
    qDebug() << "Plugin added to composition:" << pluginId;
});
```

## See Also

- [Plugin Architecture](plugin-architecture.md)
- [Plugin Development](plugin-development.md)
- [Composite Plugin API](../api/composition/composite-plugin.md)
- [Advanced Plugin Development](advanced-plugin-development.md)
