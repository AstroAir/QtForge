# Metrics Examples

This document provides comprehensive examples of metrics collection, monitoring, and analysis in QtForge plugins.

## Overview

Metrics in QtForge enable:
- **Performance Monitoring**: Track plugin performance and resource usage
- **Health Monitoring**: Monitor plugin health and availability
- **Business Metrics**: Collect application-specific metrics
- **Real-time Analytics**: Analyze metrics in real-time
- **Alerting**: Set up alerts based on metric thresholds

## Basic Metrics Collection

### Simple Metrics Plugin

```cpp
#include <QtForge/PluginMetricsCollector>
#include <QTimer>

class BasicMetricsPlugin : public PluginInterface {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.example.BasicMetrics" FILE "metadata.json")
    Q_INTERFACES(QtForge::PluginInterface)

public:
    BasicMetricsPlugin(QObject* parent = nullptr) : PluginInterface(parent) {
        setupMetrics();
    }

    bool initialize(PluginContext* context) override {
        m_context = context;
        m_metricsCollector = context->getMetricsCollector();
        
        // Register custom metrics
        registerMetrics();
        
        // Start metrics collection
        startMetricsCollection();
        
        return true;
    }

private slots:
    void collectMetrics() {
        // Collect performance metrics
        collectPerformanceMetrics();
        
        // Collect business metrics
        collectBusinessMetrics();
        
        // Collect health metrics
        collectHealthMetrics();
    }

private:
    void setupMetrics() {
        m_metricsTimer = new QTimer(this);
        connect(m_metricsTimer, &QTimer::timeout, this, &BasicMetricsPlugin::collectMetrics);
    }
    
    void registerMetrics() {
        // Register counter metrics
        m_metricsCollector->registerCounter("requests_total", 
            "Total number of requests processed");
        m_metricsCollector->registerCounter("errors_total", 
            "Total number of errors encountered");
        
        // Register gauge metrics
        m_metricsCollector->registerGauge("active_connections", 
            "Number of active connections");
        m_metricsCollector->registerGauge("memory_usage_bytes", 
            "Memory usage in bytes");
        
        // Register histogram metrics
        m_metricsCollector->registerHistogram("request_duration_seconds", 
            "Request duration in seconds", {0.1, 0.5, 1.0, 2.0, 5.0});
    }
    
    void startMetricsCollection() {
        m_metricsTimer->start(5000); // Collect every 5 seconds
    }
    
    void collectPerformanceMetrics() {
        // CPU usage
        double cpuUsage = getCurrentCpuUsage();
        m_metricsCollector->setGauge("cpu_usage_percent", cpuUsage);
        
        // Memory usage
        qint64 memoryUsage = getCurrentMemoryUsage();
        m_metricsCollector->setGauge("memory_usage_bytes", memoryUsage);
        
        // Thread count
        int threadCount = QThread::idealThreadCount();
        m_metricsCollector->setGauge("thread_count", threadCount);
    }
    
    void collectBusinessMetrics() {
        // Example: Track processed items
        int processedItems = getProcessedItemsCount();
        m_metricsCollector->incrementCounter("items_processed_total", processedItems);
        
        // Example: Track active users
        int activeUsers = getActiveUsersCount();
        m_metricsCollector->setGauge("active_users", activeUsers);
    }
    
    void collectHealthMetrics() {
        // Plugin health status
        bool isHealthy = checkPluginHealth();
        m_metricsCollector->setGauge("plugin_healthy", isHealthy ? 1 : 0);
        
        // Last successful operation timestamp
        qint64 lastSuccess = getLastSuccessfulOperationTime();
        m_metricsCollector->setGauge("last_success_timestamp", lastSuccess);
    }
    
    double getCurrentCpuUsage();
    qint64 getCurrentMemoryUsage();
    int getProcessedItemsCount();
    int getActiveUsersCount();
    bool checkPluginHealth();
    qint64 getLastSuccessfulOperationTime();
    
    PluginContext* m_context;
    PluginMetricsCollector* m_metricsCollector;
    QTimer* m_metricsTimer;
};
```

### Custom Metrics with Labels

```cpp
class LabeledMetricsPlugin : public PluginInterface {
    Q_OBJECT

public:
    void trackRequestWithLabels(const QString& method, const QString& endpoint, 
                               int statusCode, double duration) {
        // Create labels for the metric
        QVariantMap labels;
        labels["method"] = method;
        labels["endpoint"] = endpoint;
        labels["status_code"] = statusCode;
        
        // Increment counter with labels
        m_metricsCollector->incrementCounter("http_requests_total", 1, labels);
        
        // Record duration histogram with labels
        m_metricsCollector->observeHistogram("http_request_duration_seconds", 
                                           duration, labels);
        
        // Track error rate
        if (statusCode >= 400) {
            m_metricsCollector->incrementCounter("http_errors_total", 1, labels);
        }
    }
    
    void trackDatabaseOperation(const QString& operation, const QString& table, 
                              double duration, bool success) {
        QVariantMap labels;
        labels["operation"] = operation;
        labels["table"] = table;
        labels["success"] = success;
        
        // Track database operations
        m_metricsCollector->incrementCounter("db_operations_total", 1, labels);
        m_metricsCollector->observeHistogram("db_operation_duration_seconds", 
                                           duration, labels);
        
        if (!success) {
            m_metricsCollector->incrementCounter("db_errors_total", 1, labels);
        }
    }
    
    void trackCacheMetrics(const QString& cacheType) {
        QVariantMap labels;
        labels["cache_type"] = cacheType;
        
        // Cache hit/miss tracking
        int hits = getCacheHits(cacheType);
        int misses = getCacheMisses(cacheType);
        
        m_metricsCollector->setGauge("cache_hits_total", hits, labels);
        m_metricsCollector->setGauge("cache_misses_total", misses, labels);
        
        // Cache hit ratio
        double hitRatio = static_cast<double>(hits) / (hits + misses);
        m_metricsCollector->setGauge("cache_hit_ratio", hitRatio, labels);
        
        // Cache size
        qint64 cacheSize = getCacheSize(cacheType);
        m_metricsCollector->setGauge("cache_size_bytes", cacheSize, labels);
    }

private:
    int getCacheHits(const QString& cacheType);
    int getCacheMisses(const QString& cacheType);
    qint64 getCacheSize(const QString& cacheType);
    
    PluginMetricsCollector* m_metricsCollector;
};
```

## Advanced Metrics Collection

### Time-Series Metrics

```cpp
class TimeSeriesMetricsPlugin : public PluginInterface {
    Q_OBJECT

public:
    class MetricTimeSeries {
    public:
        struct DataPoint {
            QDateTime timestamp;
            double value;
            QVariantMap labels;
        };
        
        void addDataPoint(double value, const QVariantMap& labels = {}) {
            DataPoint point;
            point.timestamp = QDateTime::currentDateTime();
            point.value = value;
            point.labels = labels;
            
            m_dataPoints.append(point);
            
            // Keep only last 1000 points
            if (m_dataPoints.size() > 1000) {
                m_dataPoints.removeFirst();
            }
        }
        
        QList<DataPoint> getDataPoints(const QDateTime& start, 
                                     const QDateTime& end) const {
            QList<DataPoint> filtered;
            for (const auto& point : m_dataPoints) {
                if (point.timestamp >= start && point.timestamp <= end) {
                    filtered.append(point);
                }
            }
            return filtered;
        }
        
        double getAverage(const QDateTime& start, const QDateTime& end) const {
            auto points = getDataPoints(start, end);
            if (points.isEmpty()) return 0.0;
            
            double sum = 0.0;
            for (const auto& point : points) {
                sum += point.value;
            }
            return sum / points.size();
        }
        
        double getMax(const QDateTime& start, const QDateTime& end) const {
            auto points = getDataPoints(start, end);
            if (points.isEmpty()) return 0.0;
            
            double max = points.first().value;
            for (const auto& point : points) {
                if (point.value > max) {
                    max = point.value;
                }
            }
            return max;
        }
        
    private:
        QList<DataPoint> m_dataPoints;
    };
    
    void trackResponseTime(const QString& endpoint, double responseTime) {
        m_responseTimeSeries[endpoint].addDataPoint(responseTime);
        
        // Also send to metrics collector
        QVariantMap labels;
        labels["endpoint"] = endpoint;
        m_metricsCollector->observeHistogram("response_time_seconds", 
                                           responseTime, labels);
    }
    
    void generatePerformanceReport() {
        QDateTime now = QDateTime::currentDateTime();
        QDateTime oneHourAgo = now.addSecs(-3600);
        
        qDebug() << "=== Performance Report (Last Hour) ===";
        
        for (auto it = m_responseTimeSeries.begin(); 
             it != m_responseTimeSeries.end(); ++it) {
            
            QString endpoint = it.key();
            const MetricTimeSeries& series = it.value();
            
            double avgResponseTime = series.getAverage(oneHourAgo, now);
            double maxResponseTime = series.getMax(oneHourAgo, now);
            
            qDebug() << "Endpoint:" << endpoint;
            qDebug() << "  Average Response Time:" << avgResponseTime << "seconds";
            qDebug() << "  Max Response Time:" << maxResponseTime << "seconds";
        }
    }

private:
    QMap<QString, MetricTimeSeries> m_responseTimeSeries;
    PluginMetricsCollector* m_metricsCollector;
};
```

### Aggregated Metrics

```cpp
class AggregatedMetricsPlugin : public PluginInterface {
    Q_OBJECT

public:
    class MetricAggregator {
    public:
        void addValue(const QString& metricName, double value, 
                     const QVariantMap& labels = {}) {
            QString key = createKey(metricName, labels);
            m_values[key].append(value);
            
            // Update aggregated values
            updateAggregates(key);
        }
        
        double getSum(const QString& metricName, const QVariantMap& labels = {}) const {
            QString key = createKey(metricName, labels);
            return m_aggregates[key].sum;
        }
        
        double getAverage(const QString& metricName, const QVariantMap& labels = {}) const {
            QString key = createKey(metricName, labels);
            const auto& agg = m_aggregates[key];
            return agg.count > 0 ? agg.sum / agg.count : 0.0;
        }
        
        double getMin(const QString& metricName, const QVariantMap& labels = {}) const {
            QString key = createKey(metricName, labels);
            return m_aggregates[key].min;
        }
        
        double getMax(const QString& metricName, const QVariantMap& labels = {}) const {
            QString key = createKey(metricName, labels);
            return m_aggregates[key].max;
        }
        
        int getCount(const QString& metricName, const QVariantMap& labels = {}) const {
            QString key = createKey(metricName, labels);
            return m_aggregates[key].count;
        }
        
    private:
        struct Aggregates {
            double sum = 0.0;
            double min = std::numeric_limits<double>::max();
            double max = std::numeric_limits<double>::lowest();
            int count = 0;
        };
        
        QString createKey(const QString& metricName, const QVariantMap& labels) const {
            QString key = metricName;
            for (auto it = labels.begin(); it != labels.end(); ++it) {
                key += QString("_%1_%2").arg(it.key(), it.value().toString());
            }
            return key;
        }
        
        void updateAggregates(const QString& key) {
            const auto& values = m_values[key];
            auto& agg = m_aggregates[key];
            
            agg.sum = 0.0;
            agg.min = std::numeric_limits<double>::max();
            agg.max = std::numeric_limits<double>::lowest();
            agg.count = values.size();
            
            for (double value : values) {
                agg.sum += value;
                agg.min = qMin(agg.min, value);
                agg.max = qMax(agg.max, value);
            }
        }
        
        QMap<QString, QList<double>> m_values;
        QMap<QString, Aggregates> m_aggregates;
    };
    
    void trackProcessingTime(const QString& operation, double time) {
        QVariantMap labels;
        labels["operation"] = operation;
        
        m_aggregator.addValue("processing_time", time, labels);
        
        // Report aggregated metrics periodically
        reportAggregatedMetrics();
    }
    
private:
    void reportAggregatedMetrics() {
        static QDateTime lastReport = QDateTime::currentDateTime();
        QDateTime now = QDateTime::currentDateTime();
        
        // Report every minute
        if (lastReport.secsTo(now) >= 60) {
            QVariantMap labels;
            labels["operation"] = "data_processing";
            
            double avgTime = m_aggregator.getAverage("processing_time", labels);
            double maxTime = m_aggregator.getMax("processing_time", labels);
            int count = m_aggregator.getCount("processing_time", labels);
            
            m_metricsCollector->setGauge("avg_processing_time", avgTime, labels);
            m_metricsCollector->setGauge("max_processing_time", maxTime, labels);
            m_metricsCollector->setGauge("processing_count", count, labels);
            
            lastReport = now;
        }
    }
    
    MetricAggregator m_aggregator;
    PluginMetricsCollector* m_metricsCollector;
};
```

## Metrics Export and Visualization

### Prometheus Metrics Export

```cpp
class PrometheusMetricsExporter : public QObject {
    Q_OBJECT

public:
    PrometheusMetricsExporter(int port = 9090, QObject* parent = nullptr)
        : QObject(parent), m_port(port) {
        setupHttpServer();
    }
    
    void startServer() {
        if (m_server->listen(QHostAddress::Any, m_port)) {
            qDebug() << "Prometheus metrics server started on port:" << m_port;
        } else {
            qWarning() << "Failed to start metrics server:" << m_server->errorString();
        }
    }

private slots:
    void handleMetricsRequest() {
        QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket) return;
        
        QByteArray request = socket->readAll();
        
        // Check if this is a metrics request
        if (request.contains("GET /metrics")) {
            QByteArray response = generatePrometheusMetrics();
            
            QByteArray httpResponse = "HTTP/1.1 200 OK\r\n";
            httpResponse += "Content-Type: text/plain; version=0.0.4; charset=utf-8\r\n";
            httpResponse += QString("Content-Length: %1\r\n").arg(response.size()).toUtf8();
            httpResponse += "\r\n";
            httpResponse += response;
            
            socket->write(httpResponse);
        } else {
            // Return 404 for other requests
            QByteArray notFound = "HTTP/1.1 404 Not Found\r\n\r\n";
            socket->write(notFound);
        }
        
        socket->disconnectFromHost();
    }
    
    void onNewConnection() {
        QTcpSocket* socket = m_server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead,
                this, &PrometheusMetricsExporter::handleMetricsRequest);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }

private:
    void setupHttpServer() {
        m_server = new QTcpServer(this);
        connect(m_server, &QTcpServer::newConnection,
                this, &PrometheusMetricsExporter::onNewConnection);
    }
    
    QByteArray generatePrometheusMetrics() {
        QByteArray metrics;
        
        // Get metrics from all plugins
        auto* collector = PluginMetricsCollector::instance();
        auto allMetrics = collector->getAllMetrics();
        
        for (const auto& metric : allMetrics) {
            metrics += formatPrometheusMetric(metric);
        }
        
        return metrics;
    }
    
    QByteArray formatPrometheusMetric(const MetricData& metric) {
        QByteArray formatted;
        
        // Add help text
        formatted += QString("# HELP %1 %2\n").arg(metric.name, metric.description).toUtf8();
        
        // Add type
        QString type = "gauge";
        if (metric.type == MetricType::Counter) type = "counter";
        else if (metric.type == MetricType::Histogram) type = "histogram";
        
        formatted += QString("# TYPE %1 %2\n").arg(metric.name, type).toUtf8();
        
        // Add metric values
        for (const auto& sample : metric.samples) {
            QString line = metric.name;
            
            // Add labels
            if (!sample.labels.isEmpty()) {
                QStringList labelPairs;
                for (auto it = sample.labels.begin(); it != sample.labels.end(); ++it) {
                    labelPairs.append(QString("%1=\"%2\"").arg(it.key(), it.value().toString()));
                }
                line += QString("{%1}").arg(labelPairs.join(","));
            }
            
            line += QString(" %1 %2\n").arg(sample.value).arg(sample.timestamp);
            formatted += line.toUtf8();
        }
        
        return formatted;
    }
    
    QTcpServer* m_server;
    int m_port;
};
```

### Grafana Dashboard Integration

```cpp
class GrafanaDashboardGenerator : public QObject {
    Q_OBJECT

public:
    struct DashboardPanel {
        QString title;
        QString metricQuery;
        QString panelType; // "graph", "singlestat", "table"
        QVariantMap options;
    };
    
    struct Dashboard {
        QString title;
        QString description;
        QList<DashboardPanel> panels;
        QVariantMap settings;
    };
    
    QJsonObject generateDashboard(const Dashboard& dashboard) {
        QJsonObject dashboardJson;
        dashboardJson["title"] = dashboard.title;
        dashboardJson["description"] = dashboard.description;
        dashboardJson["editable"] = true;
        dashboardJson["hideControls"] = false;
        dashboardJson["time"] = QJsonObject{
            {"from", "now-1h"},
            {"to", "now"}
        };
        dashboardJson["refresh"] = "5s";
        
        QJsonArray panels;
        int panelId = 1;
        
        for (const auto& panel : dashboard.panels) {
            QJsonObject panelJson = generatePanel(panel, panelId++);
            panels.append(panelJson);
        }
        
        dashboardJson["panels"] = panels;
        return dashboardJson;
    }
    
    Dashboard createPluginPerformanceDashboard() {
        Dashboard dashboard;
        dashboard.title = "Plugin Performance Dashboard";
        dashboard.description = "Monitor QtForge plugin performance metrics";
        
        // CPU Usage Panel
        DashboardPanel cpuPanel;
        cpuPanel.title = "CPU Usage";
        cpuPanel.metricQuery = "cpu_usage_percent";
        cpuPanel.panelType = "graph";
        cpuPanel.options["yAxes"] = QJsonArray{
            QJsonObject{{"max", 100}, {"min", 0}, {"unit", "percent"}}
        };
        dashboard.panels.append(cpuPanel);
        
        // Memory Usage Panel
        DashboardPanel memoryPanel;
        memoryPanel.title = "Memory Usage";
        memoryPanel.metricQuery = "memory_usage_bytes";
        memoryPanel.panelType = "graph";
        memoryPanel.options["yAxes"] = QJsonArray{
            QJsonObject{{"unit", "bytes"}}
        };
        dashboard.panels.append(memoryPanel);
        
        // Request Rate Panel
        DashboardPanel requestPanel;
        requestPanel.title = "Request Rate";
        requestPanel.metricQuery = "rate(http_requests_total[5m])";
        requestPanel.panelType = "graph";
        requestPanel.options["yAxes"] = QJsonArray{
            QJsonObject{{"unit", "reqps"}}
        };
        dashboard.panels.append(requestPanel);
        
        // Error Rate Panel
        DashboardPanel errorPanel;
        errorPanel.title = "Error Rate";
        errorPanel.metricQuery = "rate(http_errors_total[5m]) / rate(http_requests_total[5m])";
        errorPanel.panelType = "singlestat";
        errorPanel.options["format"] = "percent";
        errorPanel.options["thresholds"] = "0.01,0.05";
        dashboard.panels.append(errorPanel);
        
        return dashboard;
    }

private:
    QJsonObject generatePanel(const DashboardPanel& panel, int panelId) {
        QJsonObject panelJson;
        panelJson["id"] = panelId;
        panelJson["title"] = panel.title;
        panelJson["type"] = panel.panelType;
        panelJson["span"] = 6;
        panelJson["targets"] = QJsonArray{
            QJsonObject{
                {"expr", panel.metricQuery},
                {"format", "time_series"},
                {"intervalFactor", 2}
            }
        };
        
        // Add panel-specific options
        for (auto it = panel.options.begin(); it != panel.options.end(); ++it) {
            panelJson[it.key()] = QJsonValue::fromVariant(it.value());
        }
        
        return panelJson;
    }
};
```

## Real-time Metrics Monitoring

### Metrics Alerting System

```cpp
class MetricsAlertingSystem : public QObject {
    Q_OBJECT

public:
    enum AlertSeverity {
        Info,
        Warning,
        Critical
    };
    
    struct AlertRule {
        QString name;
        QString metricName;
        QString condition; // "greater_than", "less_than", "equals"
        double threshold;
        AlertSeverity severity;
        int evaluationInterval; // seconds
        QVariantMap labels;
    };
    
    struct Alert {
        QString ruleName;
        QString message;
        AlertSeverity severity;
        QDateTime timestamp;
        QVariantMap labels;
        double value;
    };
    
    void addAlertRule(const AlertRule& rule) {
        m_alertRules.append(rule);
    }
    
    void evaluateAlerts() {
        auto* collector = PluginMetricsCollector::instance();
        
        for (const auto& rule : m_alertRules) {
            double currentValue = collector->getMetricValue(rule.metricName, rule.labels);
            
            bool alertTriggered = false;
            if (rule.condition == "greater_than" && currentValue > rule.threshold) {
                alertTriggered = true;
            } else if (rule.condition == "less_than" && currentValue < rule.threshold) {
                alertTriggered = true;
            } else if (rule.condition == "equals" && qFuzzyCompare(currentValue, rule.threshold)) {
                alertTriggered = true;
            }
            
            if (alertTriggered) {
                triggerAlert(rule, currentValue);
            }
        }
    }

signals:
    void alertTriggered(const Alert& alert);

private slots:
    void onPeriodicEvaluation() {
        evaluateAlerts();
    }

private:
    void triggerAlert(const AlertRule& rule, double value) {
        Alert alert;
        alert.ruleName = rule.name;
        alert.message = QString("Metric %1 %2 threshold %3 (current: %4)")
                       .arg(rule.metricName, rule.condition)
                       .arg(rule.threshold).arg(value);
        alert.severity = rule.severity;
        alert.timestamp = QDateTime::currentDateTime();
        alert.labels = rule.labels;
        alert.value = value;
        
        emit alertTriggered(alert);
        
        // Log alert
        QString severityStr = (alert.severity == Critical) ? "CRITICAL" :
                             (alert.severity == Warning) ? "WARNING" : "INFO";
        qWarning() << "[ALERT]" << severityStr << alert.message;
    }
    
    QList<AlertRule> m_alertRules;
    QTimer* m_evaluationTimer;
};
```

## Best Practices

### Metrics Collection Guidelines

1. **Choose Meaningful Metrics**: Collect metrics that provide actionable insights
2. **Use Appropriate Metric Types**: Use counters for cumulative values, gauges for current values
3. **Add Relevant Labels**: Use labels to add dimensions to your metrics
4. **Avoid High Cardinality**: Don't create too many unique label combinations
5. **Monitor Collection Overhead**: Ensure metrics collection doesn't impact performance

### Performance Considerations

```cpp
class EfficientMetricsCollector {
public:
    // Use sampling for high-frequency metrics
    void trackHighFrequencyMetric(const QString& name, double value) {
        static int sampleCounter = 0;
        static const int SAMPLE_RATE = 100; // Sample every 100th call
        
        if (++sampleCounter % SAMPLE_RATE == 0) {
            m_metricsCollector->observeHistogram(name, value);
        }
    }
    
    // Batch metric updates
    void batchUpdateMetrics(const QMap<QString, double>& metrics) {
        m_metricsCollector->beginBatch();
        
        for (auto it = metrics.begin(); it != metrics.end(); ++it) {
            m_metricsCollector->setGauge(it.key(), it.value());
        }
        
        m_metricsCollector->endBatch();
    }
    
    // Use metric caching for expensive calculations
    double getCachedMetric(const QString& name) {
        static QMap<QString, QPair<double, QDateTime>> cache;
        static const int CACHE_TTL = 30; // 30 seconds
        
        auto it = cache.find(name);
        QDateTime now = QDateTime::currentDateTime();
        
        if (it != cache.end() && it->second.secsTo(now) < CACHE_TTL) {
            return it->first; // Return cached value
        }
        
        // Calculate new value
        double value = calculateExpensiveMetric(name);
        cache[name] = qMakePair(value, now);
        
        return value;
    }

private:
    double calculateExpensiveMetric(const QString& name);
    PluginMetricsCollector* m_metricsCollector;
};
```

## See Also

- [Plugin Metrics Collector](../api/monitoring/plugin-metrics-collector.md)
- [Resource Monitor](../api/monitoring/resource-monitor.md)
- [Performance Optimization](../user-guide/performance-optimization.md)
- [Monitoring Guide](../user-guide/monitoring.md)
