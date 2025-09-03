# Basic Plugin Examples

!!! info "Example Collection"
**Difficulty**: Beginner to Intermediate  
 **Language**: C++  
 **QtForge Version**: v3.0+  
 **Complete Source**: Available in `examples/plugins/` directory

## Overview

This collection provides complete, working examples of QtForge plugins ranging from simple "Hello World" implementations to more sophisticated plugins with advanced features. Each example includes full source code, build instructions, and detailed explanations.

### Examples in This Collection

- [Hello World Plugin](#hello-world-plugin) - Basic plugin structure
- [Data Processing Plugin](#data-processing-plugin) - File I/O and data manipulation
- [Service Provider Plugin](#service-provider-plugin) - Service contracts and inter-plugin communication
- [Configurable Plugin](#configurable-plugin) - Configuration management and persistence
- [Async Plugin](#async-plugin) - Asynchronous operations and threading
- [Transaction-Aware Plugin](#transaction-aware-plugin) - ACID transaction support

## Hello World Plugin

A minimal plugin that demonstrates the basic QtForge plugin structure.

### Source Code

**hello_world_plugin.hpp**

```cpp
#pragma once

#include <qtplugin/core/plugin_interface.hpp>
#include <QObject>

class HelloWorldPlugin : public QObject, public IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qtforge.examples.HelloWorldPlugin" FILE "hello_world_plugin.json")
    Q_INTERFACES(IPlugin)

public:
    explicit HelloWorldPlugin(QObject* parent = nullptr);
    ~HelloWorldPlugin() override = default;

    // IPlugin interface
    qtplugin::expected<void, PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override;
    PluginMetadata metadata() const override;
    PluginState state() const override;

private:
    PluginState m_state{PluginState::Unloaded};
    QString m_greeting{"Hello, World!"};
};
```

**hello_world_plugin.cpp**

```cpp
#include "hello_world_plugin.hpp"
#include <QDebug>
#include <QDateTime>

HelloWorldPlugin::HelloWorldPlugin(QObject* parent)
    : QObject(parent), m_state(PluginState::Unloaded) {
}

qtplugin::expected<void, PluginError> HelloWorldPlugin::initialize() {
    qDebug() << "HelloWorldPlugin: Initializing...";

    m_state = PluginState::Running;

    qDebug() << "HelloWorldPlugin: Initialized successfully";
    return make_success();
}

void HelloWorldPlugin::shutdown() noexcept {
    qDebug() << "HelloWorldPlugin: Shutting down...";
    m_state = PluginState::Unloaded;
}

qtplugin::expected<QJsonObject, PluginError> HelloWorldPlugin::execute_command(
    std::string_view command,
    const QJsonObject& params) {

    if (m_state != PluginState::Running) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState,
                                     "Plugin not initialized");
    }

    if (command == "hello") {
        QString name = params.value("name").toString("World");
        QString message = QString("Hello, %1!").arg(name);

        return QJsonObject{
            {"message", message},
            {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    }
    else if (command == "set_greeting") {
        m_greeting = params.value("greeting").toString("Hello, World!");
        return QJsonObject{
            {"status", "success"},
            {"new_greeting", m_greeting}
        };
    }
    else if (command == "get_greeting") {
        return QJsonObject{
            {"greeting", m_greeting}
        };
    }

    return make_error<QJsonObject>(PluginErrorCode::CommandNotFound,
                                 QString("Unknown command: %1").arg(QString::fromStdString(std::string(command))));
}

std::vector<std::string> HelloWorldPlugin::available_commands() const {
    return {"hello", "set_greeting", "get_greeting"};
}

PluginMetadata HelloWorldPlugin::metadata() const {
    PluginMetadata meta;
    meta.id = "hello_world_plugin";
    meta.name = "Hello World Plugin";
    meta.version = Version(1, 0, 0);
    meta.description = "A simple example plugin that demonstrates basic QtForge functionality";
    meta.author = "QtForge Examples";
    meta.license = "MIT";
    meta.website = "https://github.com/QtForge/QtForge";
    return meta;
}

PluginState HelloWorldPlugin::state() const {
    return m_state;
}
```

**hello_world_plugin.json**

```json
{
  "id": "hello_world_plugin",
  "name": "Hello World Plugin",
  "version": "1.0.0",
  "description": "A simple example plugin",
  "author": "QtForge Examples",
  "license": "MIT",
  "interfaces": ["IPlugin"],
  "dependencies": []
}
```

### Usage Example

```cpp
#include <qtplugin/core/plugin_manager.hpp>
#include <QDebug>

int main() {
    auto manager = PluginManager::create();

    // Load the plugin
    auto load_result = manager->load_plugin("hello_world_plugin.so");
    if (!load_result) {
        qWarning() << "Failed to load plugin:" << load_result.error().message();
        return 1;
    }

    QString plugin_id = load_result.value();
    auto plugin = manager->get_plugin(plugin_id);

    if (plugin) {
        // Execute hello command
        auto result = plugin->execute_command("hello", QJsonObject{{"name", "QtForge"}});
        if (result) {
            qDebug() << "Result:" << result.value();
        }

        // Set custom greeting
        plugin->execute_command("set_greeting", QJsonObject{{"greeting", "Greetings"}});

        // Get greeting
        auto greeting_result = plugin->execute_command("get_greeting");
        if (greeting_result) {
            qDebug() << "Current greeting:" << greeting_result.value()["greeting"].toString();
        }
    }

    return 0;
}
```

## Data Processing Plugin

A more sophisticated plugin that processes CSV files and demonstrates file I/O operations.

### Source Code

**data_processor_plugin.hpp**

```cpp
#pragma once

#include <qtplugin/core/plugin_interface.hpp>
#include <QObject>
#include <QStringList>

class DataProcessorPlugin : public QObject, public IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qtforge.examples.DataProcessorPlugin" FILE "data_processor_plugin.json")
    Q_INTERFACES(IPlugin)

public:
    explicit DataProcessorPlugin(QObject* parent = nullptr);
    ~DataProcessorPlugin() override = default;

    // IPlugin interface
    qtplugin::expected<void, PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override;
    PluginMetadata metadata() const override;
    PluginState state() const override;

private:
    PluginState m_state{PluginState::Unloaded};

    // Helper methods
    qtplugin::expected<QJsonObject, PluginError> load_csv(const QString& file_path);
    qtplugin::expected<QJsonObject, PluginError> save_csv(const QString& file_path, const QJsonArray& data);
    qtplugin::expected<QJsonObject, PluginError> process_data(const QJsonArray& data, const QString& operation);

    QJsonArray filter_data(const QJsonArray& data, const QString& column, const QJsonValue& value);
    QJsonArray sort_data(const QJsonArray& data, const QString& column, bool ascending = true);
    QJsonObject calculate_statistics(const QJsonArray& data, const QString& column);
};
```

**data_processor_plugin.cpp**

```cpp
#include "data_processor_plugin.hpp"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <algorithm>
#include <numeric>

DataProcessorPlugin::DataProcessorPlugin(QObject* parent)
    : QObject(parent), m_state(PluginState::Unloaded) {
}

qtplugin::expected<void, PluginError> DataProcessorPlugin::initialize() {
    qDebug() << "DataProcessorPlugin: Initializing...";

    m_state = PluginState::Running;

    qDebug() << "DataProcessorPlugin: Initialized successfully";
    return make_success();
}

void DataProcessorPlugin::shutdown() noexcept {
    qDebug() << "DataProcessorPlugin: Shutting down...";
    m_state = PluginState::Unloaded;
}

qtplugin::expected<QJsonObject, PluginError> DataProcessorPlugin::execute_command(
    std::string_view command,
    const QJsonObject& params) {

    if (m_state != PluginState::Running) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidState,
                                     "Plugin not initialized");
    }

    if (command == "load_csv") {
        QString file_path = params.value("file_path").toString();
        if (file_path.isEmpty()) {
            return make_error<QJsonObject>(PluginErrorCode::InvalidParameter,
                                         "file_path parameter is required");
        }
        return load_csv(file_path);
    }
    else if (command == "save_csv") {
        QString file_path = params.value("file_path").toString();
        QJsonArray data = params.value("data").toArray();

        if (file_path.isEmpty() || data.isEmpty()) {
            return make_error<QJsonObject>(PluginErrorCode::InvalidParameter,
                                         "file_path and data parameters are required");
        }
        return save_csv(file_path, data);
    }
    else if (command == "process_data") {
        QJsonArray data = params.value("data").toArray();
        QString operation = params.value("operation").toString();

        if (data.isEmpty() || operation.isEmpty()) {
            return make_error<QJsonObject>(PluginErrorCode::InvalidParameter,
                                         "data and operation parameters are required");
        }
        return process_data(data, operation);
    }

    return make_error<QJsonObject>(PluginErrorCode::CommandNotFound,
                                 QString("Unknown command: %1").arg(QString::fromStdString(std::string(command))));
}

std::vector<std::string> DataProcessorPlugin::available_commands() const {
    return {"load_csv", "save_csv", "process_data"};
}

PluginMetadata DataProcessorPlugin::metadata() const {
    PluginMetadata meta;
    meta.id = "data_processor_plugin";
    meta.name = "Data Processor Plugin";
    meta.version = Version(1, 0, 0);
    meta.description = "Plugin for processing CSV data files";
    meta.author = "QtForge Examples";
    meta.license = "MIT";
    return meta;
}

PluginState DataProcessorPlugin::state() const {
    return m_state;
}

qtplugin::expected<QJsonObject, PluginError> DataProcessorPlugin::load_csv(const QString& file_path) {
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return make_error<QJsonObject>(PluginErrorCode::FileNotFound,
                                     QString("Cannot open file: %1").arg(file_path));
    }

    QTextStream in(&file);
    QJsonArray data;
    QStringList headers;
    bool first_line = true;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(',');

        if (first_line) {
            headers = fields;
            first_line = false;
            continue;
        }

        QJsonObject row;
        for (int i = 0; i < fields.size() && i < headers.size(); ++i) {
            row[headers[i]] = fields[i].trimmed();
        }
        data.append(row);
    }

    return QJsonObject{
        {"status", "success"},
        {"data", data},
        {"headers", QJsonArray::fromStringList(headers)},
        {"row_count", data.size()}
    };
}

qtplugin::expected<QJsonObject, PluginError> DataProcessorPlugin::save_csv(
    const QString& file_path,
    const QJsonArray& data) {

    if (data.isEmpty()) {
        return make_error<QJsonObject>(PluginErrorCode::InvalidParameter,
                                     "No data to save");
    }

    // Ensure directory exists
    QDir dir = QFileInfo(file_path).absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFile file(file_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return make_error<QJsonObject>(PluginErrorCode::FileAccessError,
                                     QString("Cannot create file: %1").arg(file_path));
    }

    QTextStream out(&file);

    // Write headers (from first row keys)
    if (!data.isEmpty()) {
        QJsonObject first_row = data[0].toObject();
        QStringList headers = first_row.keys();
        out << headers.join(',') << '\n';

        // Write data rows
        for (const auto& value : data) {
            QJsonObject row = value.toObject();
            QStringList row_data;
            for (const QString& header : headers) {
                row_data << row.value(header).toString();
            }
            out << row_data.join(',') << '\n';
        }
    }

    return QJsonObject{
        {"status", "success"},
        {"file_path", file_path},
        {"rows_written", data.size()}
    };
}

qtplugin::expected<QJsonObject, PluginError> DataProcessorPlugin::process_data(
    const QJsonArray& data,
    const QString& operation) {

    // This is a simplified example - real implementation would be more robust
    if (operation == "count") {
        return QJsonObject{
            {"operation", "count"},
            {"result", data.size()}
        };
    }
    else if (operation == "summary") {
        QJsonObject summary;
        summary["total_rows"] = data.size();

        if (!data.isEmpty()) {
            QJsonObject first_row = data[0].toObject();
            summary["columns"] = QJsonArray::fromStringList(first_row.keys());
            summary["column_count"] = first_row.keys().size();
        }

        return QJsonObject{
            {"operation", "summary"},
            {"result", summary}
        };
    }

    return make_error<QJsonObject>(PluginErrorCode::InvalidParameter,
                                 QString("Unknown operation: %1").arg(operation));
}
```

### Usage Example

```cpp
#include <qtplugin/core/plugin_manager.hpp>
#include <QDebug>

int main() {
    auto manager = PluginManager::create();

    // Load the data processor plugin
    auto load_result = manager->load_plugin("data_processor_plugin.so");
    if (!load_result) {
        qWarning() << "Failed to load plugin:" << load_result.error().message();
        return 1;
    }

    QString plugin_id = load_result.value();
    auto plugin = manager->get_plugin(plugin_id);

    if (plugin) {
        // Load CSV file
        auto load_result = plugin->execute_command("load_csv",
            QJsonObject{{"file_path", "data/sample.csv"}});

        if (load_result) {
            QJsonObject result = load_result.value();
            QJsonArray data = result["data"].toArray();

            qDebug() << "Loaded" << data.size() << "rows";

            // Process the data
            auto process_result = plugin->execute_command("process_data",
                QJsonObject{
                    {"data", data},
                    {"operation", "summary"}
                });

            if (process_result) {
                qDebug() << "Processing result:" << process_result.value();
            }

            // Save processed data
            plugin->execute_command("save_csv",
                QJsonObject{
                    {"file_path", "output/processed.csv"},
                    {"data", data}
                });
        }
    }

    return 0;
}
```

## Service Provider Plugin

Demonstrates service contracts and inter-plugin communication.

### Source Code

**service_provider_plugin.hpp**

```cpp
#pragma once

#include <qtplugin/core/plugin_interface.hpp>
#include <qtplugin/contracts/service_contracts.hpp>
#include <QObject>
#include <QTimer>

class ServiceProviderPlugin : public QObject, public IAdvancedPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qtforge.examples.ServiceProviderPlugin" FILE "service_provider_plugin.json")
    Q_INTERFACES(IAdvancedPlugin)

public:
    explicit ServiceProviderPlugin(QObject* parent = nullptr);
    ~ServiceProviderPlugin() override = default;

    // IPlugin interface
    qtplugin::expected<void, PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override;
    PluginMetadata metadata() const override;
    PluginState state() const override;

    // IAdvancedPlugin interface
    std::vector<contracts::ServiceContract> get_service_contracts() const override;
    qtplugin::expected<QJsonObject, PluginError> call_service(
        const QString& service_name,
        const QString& method_name,
        const QJsonObject& parameters = {},
        std::chrono::milliseconds timeout = std::chrono::milliseconds{30000}) override;

signals:
    void service_event(const QString& event_type, const QJsonObject& data);

private slots:
    void on_heartbeat_timer();

private:
    PluginState m_state{PluginState::Unloaded};
    QTimer* m_heartbeat_timer{nullptr};
    int m_request_count{0};
    QDateTime m_start_time;

    // Service implementations
    qtplugin::expected<QJsonObject, PluginError> calculate_service(const QJsonObject& params);
    qtplugin::expected<QJsonObject, PluginError> status_service(const QJsonObject& params);
    qtplugin::expected<QJsonObject, PluginError> config_service(const QJsonObject& params);
};
```

**service_provider_plugin.cpp**

```cpp
#include "service_provider_plugin.hpp"
#include <QDebug>
#include <QDateTime>
#include <cmath>

ServiceProviderPlugin::ServiceProviderPlugin(QObject* parent)
    : QObject(parent), m_state(PluginState::Unloaded) {

    m_heartbeat_timer = new QTimer(this);
    connect(m_heartbeat_timer, &QTimer::timeout, this, &ServiceProviderPlugin::on_heartbeat_timer);
}

qtplugin::expected<void, PluginError> ServiceProviderPlugin::initialize() {
    qDebug() << "ServiceProviderPlugin: Initializing...";

    m_state = PluginState::Running;
    m_start_time = QDateTime::currentDateTime();

    // Start heartbeat timer (every 30 seconds)
    m_heartbeat_timer->start(30000);

    qDebug() << "ServiceProviderPlugin: Initialized successfully";
    return make_success();
}

void ServiceProviderPlugin::shutdown() noexcept {
    qDebug() << "ServiceProviderPlugin: Shutting down...";

    if (m_heartbeat_timer) {
        m_heartbeat_timer->stop();
    }

    m_state = PluginState::Unloaded;
}

std::vector<contracts::ServiceContract> ServiceProviderPlugin::get_service_contracts() const {
    return {
        contracts::ServiceContract{
            .service_name = "calculation_service",
            .version = "1.0",
            .methods = {
                {"add", "Add two numbers"},
                {"multiply", "Multiply two numbers"},
                {"power", "Calculate power of a number"}
            },
            .dependencies = {}
        },
        contracts::ServiceContract{
            .service_name = "status_service",
            .version = "1.0",
            .methods = {
                {"get_status", "Get service status"},
                {"get_statistics", "Get service statistics"}
            },
            .dependencies = {}
        }
    };
}

qtplugin::expected<QJsonObject, PluginError> ServiceProviderPlugin::call_service(
    const QString& service_name,
    const QString& method_name,
    const QJsonObject& parameters,
    std::chrono::milliseconds timeout) {

    Q_UNUSED(timeout) // For this example, we ignore timeout

    m_request_count++;

    if (service_name == "calculation_service") {
        return calculate_service(QJsonObject{
            {"method", method_name},
            {"params", parameters}
        });
    }
    else if (service_name == "status_service") {
        return status_service(QJsonObject{
            {"method", method_name},
            {"params", parameters}
        });
    }

    return make_error<QJsonObject>(PluginErrorCode::ServiceNotFound,
                                 QString("Service not found: %1").arg(service_name));
}

qtplugin::expected<QJsonObject, PluginError> ServiceProviderPlugin::calculate_service(const QJsonObject& params) {
    QString method = params["method"].toString();
    QJsonObject method_params = params["params"].toObject();

    if (method == "add") {
        double a = method_params["a"].toDouble();
        double b = method_params["b"].toDouble();
        return QJsonObject{
            {"result", a + b},
            {"operation", "addition"}
        };
    }
    else if (method == "multiply") {
        double a = method_params["a"].toDouble();
        double b = method_params["b"].toDouble();
        return QJsonObject{
            {"result", a * b},
            {"operation", "multiplication"}
        };
    }
    else if (method == "power") {
        double base = method_params["base"].toDouble();
        double exponent = method_params["exponent"].toDouble();
        return QJsonObject{
            {"result", std::pow(base, exponent)},
            {"operation", "power"}
        };
    }

    return make_error<QJsonObject>(PluginErrorCode::MethodNotFound,
                                 QString("Method not found: %1").arg(method));
}

qtplugin::expected<QJsonObject, PluginError> ServiceProviderPlugin::status_service(const QJsonObject& params) {
    QString method = params["method"].toString();

    if (method == "get_status") {
        return QJsonObject{
            {"status", "running"},
            {"uptime_seconds", m_start_time.secsTo(QDateTime::currentDateTime())},
            {"state", static_cast<int>(m_state)}
        };
    }
    else if (method == "get_statistics") {
        return QJsonObject{
            {"request_count", m_request_count},
            {"start_time", m_start_time.toString(Qt::ISODate)},
            {"current_time", QDateTime::currentDateTime().toString(Qt::ISODate)}
        };
    }

    return make_error<QJsonObject>(PluginErrorCode::MethodNotFound,
                                 QString("Method not found: %1").arg(method));
}

void ServiceProviderPlugin::on_heartbeat_timer() {
    emit service_event("heartbeat", QJsonObject{
        {"timestamp", QDateTime::currentDateTime().toString(Qt::ISODate)},
        {"uptime", m_start_time.secsTo(QDateTime::currentDateTime())},
        {"request_count", m_request_count}
    });
}

// Implement remaining IPlugin methods...
qtplugin::expected<QJsonObject, PluginError> ServiceProviderPlugin::execute_command(
    std::string_view command,
    const QJsonObject& params) {

    if (command == "get_contracts") {
        QJsonArray contracts;
        for (const auto& contract : get_service_contracts()) {
            QJsonObject contract_obj;
            contract_obj["service_name"] = contract.service_name;
            contract_obj["version"] = contract.version;

            QJsonArray methods;
            for (const auto& [method_name, description] : contract.methods) {
                methods.append(QJsonObject{
                    {"name", method_name},
                    {"description", description}
                });
            }
            contract_obj["methods"] = methods;
            contracts.append(contract_obj);
        }

        return QJsonObject{{"contracts", contracts}};
    }

    return make_error<QJsonObject>(PluginErrorCode::CommandNotFound,
                                 QString("Unknown command: %1").arg(QString::fromStdString(std::string(command))));
}

std::vector<std::string> ServiceProviderPlugin::available_commands() const {
    return {"get_contracts"};
}

PluginMetadata ServiceProviderPlugin::metadata() const {
    PluginMetadata meta;
    meta.id = "service_provider_plugin";
    meta.name = "Service Provider Plugin";
    meta.version = Version(1, 0, 0);
    meta.description = "Example plugin demonstrating service contracts";
    meta.author = "QtForge Examples";
    meta.license = "MIT";
    return meta;
}

PluginState ServiceProviderPlugin::state() const {
    return m_state;
}
```

## Configurable Plugin {#configurable-plugin}

A plugin that demonstrates configuration management and persistence.

### Features

- Configuration loading and saving
- Runtime configuration updates
- Configuration validation
- Default value handling

### Source Code

**configurable_plugin.hpp**

```cpp
#pragma once

#include <qtplugin/core/plugin_interface.hpp>
#include <QObject>
#include <QJsonObject>
#include <QSettings>

class ConfigurablePlugin : public QObject, public IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qtforge.examples.ConfigurablePlugin" FILE "configurable_plugin.json")
    Q_INTERFACES(IPlugin)

public:
    explicit ConfigurablePlugin(QObject* parent = nullptr);
    ~ConfigurablePlugin() override = default;

    // IPlugin interface
    qtplugin::expected<void, PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}) override;

    qtplugin::expected<void, PluginError> configure(const QJsonObject& config) override;
    QJsonObject current_configuration() const override;
    std::vector<std::string> supported_commands() const override;
    PluginMetadata metadata() const override;
    PluginState state() const override;

private:
    void loadConfiguration();
    void saveConfiguration();
    bool validateConfiguration(const QJsonObject& config);

    PluginState m_state = PluginState::Unloaded;
    QJsonObject m_config;
    std::unique_ptr<QSettings> m_settings;
};
```

## Async Plugin {#async-plugin}

A plugin that demonstrates asynchronous operations and threading.

### Features

- Asynchronous task execution
- Thread pool management
- Progress reporting
- Cancellation support

### Source Code

**async_plugin.hpp**

```cpp
#pragma once

#include <qtplugin/core/plugin_interface.hpp>
#include <QObject>
#include <QThreadPool>
#include <QFuture>
#include <QFutureWatcher>

class AsyncPlugin : public QObject, public IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qtforge.examples.AsyncPlugin" FILE "async_plugin.json")
    Q_INTERFACES(IPlugin)

public:
    explicit AsyncPlugin(QObject* parent = nullptr);
    ~AsyncPlugin() override = default;

    // IPlugin interface
    qtplugin::expected<void, PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}) override;

    std::vector<std::string> supported_commands() const override;
    PluginMetadata metadata() const override;
    PluginState state() const override;

private slots:
    void onTaskFinished();
    void onTaskProgress(int progress);

private:
    void startAsyncTask(const QJsonObject& params);
    void cancelAllTasks();

    PluginState m_state = PluginState::Unloaded;
    QThreadPool* m_threadPool;
    QList<QFutureWatcher<QJsonObject>*> m_watchers;
};
```

## Transaction-Aware Plugin {#transaction-aware-plugin}

A plugin that demonstrates ACID transaction support.

### Features

- Transaction management
- Rollback capabilities
- Nested transactions
- Data consistency

### Source Code

**transaction_plugin.hpp**

```cpp
#pragma once

#include <qtplugin/core/plugin_interface.hpp>
#include <qtplugin/transactions/transaction_manager.hpp>
#include <QObject>
#include <QJsonObject>

class TransactionAwarePlugin : public QObject, public IPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qtforge.examples.TransactionAwarePlugin" FILE "transaction_plugin.json")
    Q_INTERFACES(IPlugin)

public:
    explicit TransactionAwarePlugin(QObject* parent = nullptr);
    ~TransactionAwarePlugin() override = default;

    // IPlugin interface
    qtplugin::expected<void, PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command,
        const QJsonObject& params = {}) override;

    std::vector<std::string> supported_commands() const override;
    PluginMetadata metadata() const override;
    PluginState state() const override;

private:
    qtplugin::expected<QJsonObject, PluginError> beginTransaction(const QJsonObject& params);
    qtplugin::expected<QJsonObject, PluginError> commitTransaction(const QJsonObject& params);
    qtplugin::expected<QJsonObject, PluginError> rollbackTransaction(const QJsonObject& params);

    PluginState m_state = PluginState::Unloaded;
    std::shared_ptr<qtplugin::TransactionManager> m_transactionManager;
    std::map<std::string, std::any> m_transactionData;
};
```

## Building the Examples

All examples can be built using the provided CMake configuration:

```bash
mkdir build && cd build
cmake .. -DBUILD_EXAMPLES=ON
cmake --build .
```

## Next Steps

- **[Advanced Examples](advanced.md)**: More complex plugin patterns
- **[Plugin Development Guide](../user-guide/plugin-development.md)**: Comprehensive development guide
- **[API Reference](../api/index.md)**: Detailed API documentation
