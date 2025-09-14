# Plugin Composition Examples

This document provides practical examples of plugin composition patterns and techniques in QtForge.

## Basic Composition Examples

### Simple Composite Plugin

A basic example of combining multiple plugins into a single composite:

```cpp
// DataProcessingComposite.h
#include <QtForge/CompositePlugin>

class DataProcessingComposite : public CompositePlugin {
    Q_OBJECT

public:
    DataProcessingComposite(QObject* parent = nullptr);

    PluginMetadata getMetadata() const override;
    bool initialize(PluginContext* context) override;
    bool shutdown() override;

private:
    void setupPipeline();
};

// DataProcessingComposite.cpp
DataProcessingComposite::DataProcessingComposite(QObject* parent)
    : CompositePlugin(parent) {
    setupPipeline();
}

void DataProcessingComposite::setupPipeline() {
    // Add input plugin
    addSubPlugin(std::make_unique<DataInputPlugin>());

    // Add transformation plugins
    addSubPlugin(std::make_unique<DataValidationPlugin>());
    addSubPlugin(std::make_unique<DataTransformPlugin>());
    addSubPlugin(std::make_unique<DataEnrichmentPlugin>());

    // Add output plugin
    addSubPlugin(std::make_unique<DataOutputPlugin>());
}

PluginMetadata DataProcessingComposite::getMetadata() const {
    PluginMetadata metadata;
    metadata.id = "com.example.dataprocessing.composite";
    metadata.name = "Data Processing Composite";
    metadata.version = "1.0.0";
    metadata.description = "Complete data processing pipeline";
    metadata.author = "Example Corp";
    return metadata;
}
```

### Service Composition

Combining multiple services into a unified interface:

```cpp
class UnifiedServiceComposite : public ServicePlugin {
    Q_OBJECT

public:
    class UnifiedService : public QObject {
        Q_OBJECT

    public:
        // Authentication service methods
        bool authenticate(const QString& username, const QString& password);
        void logout();

        // Data service methods
        QVariant getData(const QString& key);
        bool setData(const QString& key, const QVariant& value);

        // Notification service methods
        void sendNotification(const QString& message);
        void subscribeToNotifications(QObject* receiver);

    private:
        AuthenticationService* m_authService;
        DataService* m_dataService;
        NotificationService* m_notificationService;
    };

    QObject* getService() override {
        return &m_unifiedService;
    }

private:
    UnifiedService m_unifiedService;
};
```

## Advanced Composition Patterns

### Conditional Composition

Compose plugins based on runtime conditions:

```cpp
class AdaptiveComposite : public CompositePlugin {
public:
    bool initialize(PluginContext* context) override {
        // Get system capabilities
        SystemInfo info = context->getSystemInfo();

        // Add core plugins
        addSubPlugin(std::make_unique<CorePlugin>());

        // Add optional plugins based on system capabilities
        if (info.hasGPU()) {
            addSubPlugin(std::make_unique<GPUAcceleratedPlugin>());
        } else {
            addSubPlugin(std::make_unique<CPUOptimizedPlugin>());
        }

        if (info.hasNetworkAccess()) {
            addSubPlugin(std::make_unique<NetworkSyncPlugin>());
        } else {
            addSubPlugin(std::make_unique<OfflineStoragePlugin>());
        }

        // Add plugins based on user preferences
        UserPreferences prefs = context->getUserPreferences();
        if (prefs.enableAdvancedFeatures()) {
            addSubPlugin(std::make_unique<AdvancedFeaturesPlugin>());
        }

        return CompositePlugin::initialize(context);
    }
};
```

### Hierarchical Composition

Create nested composite structures:

```cpp
class ApplicationComposite : public CompositePlugin {
public:
    ApplicationComposite() {
        // Create UI composite
        auto uiComposite = std::make_unique<UIComposite>();
        uiComposite->addSubPlugin(std::make_unique<MenuPlugin>());
        uiComposite->addSubPlugin(std::make_unique<ToolbarPlugin>());
        uiComposite->addSubPlugin(std::make_unique<StatusBarPlugin>());
        addSubPlugin(std::move(uiComposite));

        // Create data composite
        auto dataComposite = std::make_unique<DataComposite>();
        dataComposite->addSubPlugin(std::make_unique<DatabasePlugin>());
        dataComposite->addSubPlugin(std::make_unique<CachePlugin>());
        dataComposite->addSubPlugin(std::make_unique<SyncPlugin>());
        addSubPlugin(std::move(dataComposite));

        // Create service composite
        auto serviceComposite = std::make_unique<ServiceComposite>();
        serviceComposite->addSubPlugin(std::make_unique<AuthServicePlugin>());
        serviceComposite->addSubPlugin(std::make_unique<LoggingServicePlugin>());
        serviceComposite->addSubPlugin(std::make_unique<ConfigServicePlugin>());
        addSubPlugin(std::move(serviceComposite));
    }
};
```

## Pipeline Composition

### Data Processing Pipeline

Create a configurable data processing pipeline:

```cpp
class DataPipeline : public CompositePlugin {
public:
    struct PipelineConfig {
        QStringList inputSources;
        QStringList transformations;
        QStringList outputTargets;
        QVariantMap parameters;
    };

    void configure(const PipelineConfig& config) {
        // Clear existing pipeline
        clearSubPlugins();

        // Add input plugins
        for (const QString& source : config.inputSources) {
            auto plugin = createInputPlugin(source);
            if (plugin) {
                addSubPlugin(std::move(plugin));
            }
        }

        // Add transformation plugins
        for (const QString& transform : config.transformations) {
            auto plugin = createTransformPlugin(transform);
            if (plugin) {
                configurePlugin(plugin.get(), config.parameters);
                addSubPlugin(std::move(plugin));
            }
        }

        // Add output plugins
        for (const QString& target : config.outputTargets) {
            auto plugin = createOutputPlugin(target);
            if (plugin) {
                addSubPlugin(std::move(plugin));
            }
        }
    }

private:
    std::unique_ptr<PluginInterface> createInputPlugin(const QString& type) {
        if (type == "file") return std::make_unique<FileInputPlugin>();
        if (type == "database") return std::make_unique<DatabaseInputPlugin>();
        if (type == "network") return std::make_unique<NetworkInputPlugin>();
        return nullptr;
    }

    std::unique_ptr<PluginInterface> createTransformPlugin(const QString& type) {
        if (type == "filter") return std::make_unique<FilterPlugin>();
        if (type == "aggregate") return std::make_unique<AggregatePlugin>();
        if (type == "enrich") return std::make_unique<EnrichmentPlugin>();
        return nullptr;
    }

    std::unique_ptr<PluginInterface> createOutputPlugin(const QString& type) {
        if (type == "file") return std::make_unique<FileOutputPlugin>();
        if (type == "database") return std::make_unique<DatabaseOutputPlugin>();
        if (type == "api") return std::make_unique<APIOutputPlugin>();
        return nullptr;
    }
};

// Usage example
void setupDataPipeline() {
    DataPipeline::PipelineConfig config;
    config.inputSources = {"file", "database"};
    config.transformations = {"filter", "aggregate", "enrich"};
    config.outputTargets = {"database", "api"};
    config.parameters = {
        {"filter_criteria", "status=active"},
        {"aggregate_function", "sum"},
        {"enrich_source", "external_api"}
    };

    auto pipeline = std::make_unique<DataPipeline>();
    pipeline->configure(config);

    PluginManager::instance()->loadPlugin(std::move(pipeline));
}
```

### Workflow Composition

Create complex workflows from simple components:

```cpp
class WorkflowComposite : public CompositePlugin {
public:
    struct WorkflowStep {
        QString pluginId;
        QVariantMap configuration;
        QStringList dependencies;
        bool optional = false;
    };

    void defineWorkflow(const QList<WorkflowStep>& steps) {
        m_workflowSteps = steps;
        buildComposition();
    }

private:
    void buildComposition() {
        // Sort steps by dependencies
        auto sortedSteps = topologicalSort(m_workflowSteps);

        for (const auto& step : sortedSteps) {
            auto plugin = PluginFactory::instance()->createPlugin(step.pluginId);
            if (plugin) {
                // Configure plugin
                if (auto configurable = qobject_cast<ConfigurablePlugin*>(plugin.get())) {
                    configurable->configure(step.configuration);
                }

                addSubPlugin(std::move(plugin));
            } else if (!step.optional) {
                throw std::runtime_error("Required plugin not found: " + step.pluginId.toStdString());
            }
        }
    }

    QList<WorkflowStep> topologicalSort(const QList<WorkflowStep>& steps) {
        // Implement topological sorting based on dependencies
        // ... sorting logic ...
        return steps; // Simplified
    }

    QList<WorkflowStep> m_workflowSteps;
};
```

## Dynamic Composition

### Runtime Plugin Assembly

Assemble plugins at runtime based on configuration:

```cpp
class DynamicComposite : public CompositePlugin {
public:
    void loadFromConfiguration(const QJsonObject& config) {
        QJsonArray plugins = config["plugins"].toArray();

        for (const auto& value : plugins) {
            QJsonObject pluginConfig = value.toObject();
            QString pluginId = pluginConfig["id"].toString();
            bool enabled = pluginConfig["enabled"].toBool(true);

            if (enabled) {
                auto plugin = createAndConfigurePlugin(pluginId, pluginConfig);
                if (plugin) {
                    addSubPlugin(std::move(plugin));
                }
            }
        }
    }

private:
    std::unique_ptr<PluginInterface> createAndConfigurePlugin(
        const QString& pluginId, const QJsonObject& config) {

        auto plugin = PluginFactory::instance()->createPlugin(pluginId);
        if (!plugin) {
            return nullptr;
        }

        // Apply configuration
        if (config.contains("configuration")) {
            QJsonObject pluginConfig = config["configuration"].toObject();
            applyConfiguration(plugin.get(), pluginConfig);
        }

        return plugin;
    }

    void applyConfiguration(PluginInterface* plugin, const QJsonObject& config) {
        if (auto configurable = qobject_cast<ConfigurablePlugin*>(plugin)) {
            QVariantMap configMap;
            for (auto it = config.begin(); it != config.end(); ++it) {
                configMap[it.key()] = it.value().toVariant();
            }
            configurable->configure(configMap);
        }
    }
};

// Example configuration file
/*
{
  "plugins": [
    {
      "id": "com.example.input",
      "enabled": true,
      "configuration": {
        "source": "database",
        "connection_string": "sqlite:///data.db"
      }
    },
    {
      "id": "com.example.processor",
      "enabled": true,
      "configuration": {
        "algorithm": "advanced",
        "threads": 4
      }
    },
    {
      "id": "com.example.output",
      "enabled": false
    }
  ]
}
*/
```

### Hot-Swappable Composition

Enable runtime plugin replacement:

```cpp
class HotSwappableComposite : public CompositePlugin {
public:
    bool replacePlugin(const QString& oldPluginId, const QString& newPluginId) {
        // Find the old plugin
        auto it = std::find_if(m_subPlugins.begin(), m_subPlugins.end(),
            [&oldPluginId](const auto& plugin) {
                return plugin->getMetadata().id == oldPluginId;
            });

        if (it == m_subPlugins.end()) {
            return false; // Plugin not found
        }

        // Create new plugin
        auto newPlugin = PluginFactory::instance()->createPlugin(newPluginId);
        if (!newPlugin) {
            return false; // Failed to create new plugin
        }

        // Initialize new plugin
        if (!newPlugin->initialize(m_context)) {
            return false; // Failed to initialize
        }

        // Shutdown old plugin
        (*it)->shutdown();

        // Replace plugin
        *it = std::move(newPlugin);

        emit pluginReplaced(oldPluginId, newPluginId);
        return true;
    }

    bool addPluginAtRuntime(std::unique_ptr<PluginInterface> plugin) {
        if (!plugin->initialize(m_context)) {
            return false;
        }

        addSubPlugin(std::move(plugin));
        emit pluginAdded(plugin->getMetadata().id);
        return true;
    }

    bool removePluginAtRuntime(const QString& pluginId) {
        auto it = std::find_if(m_subPlugins.begin(), m_subPlugins.end(),
            [&pluginId](const auto& plugin) {
                return plugin->getMetadata().id == pluginId;
            });

        if (it == m_subPlugins.end()) {
            return false;
        }

        (*it)->shutdown();
        m_subPlugins.erase(it);

        emit pluginRemoved(pluginId);
        return true;
    }

signals:
    void pluginReplaced(const QString& oldId, const QString& newId);
    void pluginAdded(const QString& pluginId);
    void pluginRemoved(const QString& pluginId);
};
```

## Testing Composition

### Unit Testing Composite Plugins

```cpp
class CompositePluginTest : public QObject {
    Q_OBJECT

private slots:
    void testBasicComposition() {
        auto composite = std::make_unique<DataProcessingComposite>();

        // Verify sub-plugins are added
        QCOMPARE(composite->getSubPluginCount(), 5);

        // Test initialization
        MockPluginContext context;
        QVERIFY(composite->initialize(&context));

        // Verify all sub-plugins are initialized
        for (int i = 0; i < composite->getSubPluginCount(); ++i) {
            auto plugin = composite->getSubPlugin(i);
            QVERIFY(plugin->isInitialized());
        }
    }

    void testFailureHandling() {
        auto composite = std::make_unique<TestComposite>();

        // Add a plugin that will fail to initialize
        composite->addSubPlugin(std::make_unique<FailingPlugin>());
        composite->addSubPlugin(std::make_unique<WorkingPlugin>());

        MockPluginContext context;

        // Initialization should fail
        QVERIFY(!composite->initialize(&context));

        // Verify cleanup occurred
        QVERIFY(!composite->isInitialized());
    }
};
```

## Best Practices

### Composition Guidelines

1. **Keep compositions focused**: Each composite should have a single, well-defined purpose
2. **Handle failures gracefully**: Implement proper error handling and rollback
3. **Document dependencies**: Clearly document plugin dependencies and interactions
4. **Test thoroughly**: Test both individual plugins and the complete composition
5. **Monitor performance**: Ensure composition doesn't introduce performance bottlenecks

### Performance Considerations

```cpp
// Use lazy initialization for better performance
class LazyComposite : public CompositePlugin {
private:
    QList<std::function<std::unique_ptr<PluginInterface>()>> m_pluginFactories;

public:
    void addPluginFactory(std::function<std::unique_ptr<PluginInterface>()> factory) {
        m_pluginFactories.append(factory);
    }

    PluginInterface* getPlugin(int index) override {
        if (index >= m_initializedPlugins.size()) {
            // Lazy initialization
            auto plugin = m_pluginFactories[index]();
            plugin->initialize(m_context);
            m_initializedPlugins.append(std::move(plugin));
        }
        return m_initializedPlugins[index].get();
    }
};
```

## See Also

- [Plugin Composition Guide](../user-guide/plugin-composition.md)
- [Composite Plugin API](../api/composition/composite-plugin.md)
- [Plugin Architecture](../user-guide/plugin-architecture.md)
- [Advanced Plugin Development](../user-guide/advanced-plugin-development.md)
