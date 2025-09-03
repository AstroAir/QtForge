# Network Plugin Example

This example demonstrates how to create network-enabled plugins that can communicate over HTTP, WebSockets, and other network protocols.

## Overview

Network plugins in QtForge enable:
- HTTP client and server functionality
- WebSocket communication
- REST API integration
- Real-time data streaming
- Network service discovery
- Secure network communication

## HTTP Client Plugin

### REST API Client

```cpp
// include/http_client_plugin.hpp
#pragma once

#include <qtforge/core/plugin_interface.hpp>
#include <qtforge/communication/message_bus.hpp>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>

class HttpClientPlugin : public QObject, public qtforge::IPlugin {
    Q_OBJECT

public:
    HttpClientPlugin(QObject* parent = nullptr);
    ~HttpClientPlugin() override;

    // Plugin interface
    std::string name() const override { return "HttpClientPlugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "HTTP client plugin for REST API communication";
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

    // HTTP operations
    qtforge::expected<std::string, qtforge::Error> get(
        const std::string& url,
        const std::map<std::string, std::string>& headers = {});
    
    qtforge::expected<std::string, qtforge::Error> post(
        const std::string& url,
        const std::string& data,
        const std::map<std::string, std::string>& headers = {});
    
    qtforge::expected<std::string, qtforge::Error> put(
        const std::string& url,
        const std::string& data,
        const std::map<std::string, std::string>& headers = {});
    
    qtforge::expected<std::string, qtforge::Error> deleteRequest(
        const std::string& url,
        const std::map<std::string, std::string>& headers = {});

private slots:
    void onNetworkReplyFinished();
    void onSslErrors(const QList<QSslError>& errors);

private:
    void setupMessageHandlers();
    void handleHttpRequest(const qtforge::HttpRequestMessage& message);
    QNetworkRequest createRequest(const std::string& url, 
                                 const std::map<std::string, std::string>& headers);

    qtforge::PluginState currentState_;
    std::unique_ptr<QNetworkAccessManager> networkManager_;
    std::map<QNetworkReply*, std::string> pendingRequests_;
    std::vector<qtforge::SubscriptionHandle> subscriptions_;
};
```

### HTTP Client Implementation

```cpp
// src/http_client_plugin.cpp
#include "http_client_plugin.hpp"
#include <qtforge/communication/message_bus.hpp>
#include <qtforge/utils/logger.hpp>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QEventLoop>
#include <QTimer>

HttpClientPlugin::HttpClientPlugin(QObject* parent)
    : QObject(parent), currentState_(qtforge::PluginState::Unloaded) {
}

HttpClientPlugin::~HttpClientPlugin() {
    cleanup();
}

qtforge::expected<void, qtforge::Error> HttpClientPlugin::initialize() {
    try {
        qtforge::Logger::info(name(), "Initializing HTTP client plugin...");
        
        // Create network access manager
        networkManager_ = std::make_unique<QNetworkAccessManager>(this);
        
        // Configure SSL
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
        QSslConfiguration::setDefaultConfiguration(sslConfig);
        
        // Setup message handlers
        setupMessageHandlers();
        
        currentState_ = qtforge::PluginState::Initialized;
        qtforge::Logger::info(name(), "HTTP client plugin initialized successfully");
        
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        return qtforge::Error("HTTP client plugin initialization failed: " + std::string(e.what()));
    }
}

qtforge::expected<std::string, qtforge::Error> HttpClientPlugin::get(
    const std::string& url,
    const std::map<std::string, std::string>& headers) {
    
    try {
        QNetworkRequest request = createRequest(url, headers);
        
        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        timeoutTimer.setInterval(30000); // 30 second timeout
        
        QNetworkReply* reply = networkManager_->get(request);
        
        // Connect signals
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
        connect(reply, QOverload<const QList<QSslError>&>::of(&QNetworkReply::sslErrors),
                this, &HttpClientPlugin::onSslErrors);
        
        timeoutTimer.start();
        loop.exec();
        
        if (!timeoutTimer.isActive()) {
            reply->deleteLater();
            return qtforge::Error("Request timeout");
        }
        
        timeoutTimer.stop();
        
        if (reply->error() != QNetworkReply::NoError) {
            QString errorString = reply->errorString();
            reply->deleteLater();
            return qtforge::Error("Network error: " + errorString.toStdString());
        }
        
        QByteArray responseData = reply->readAll();
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        reply->deleteLater();
        
        if (statusCode >= 400) {
            return qtforge::Error("HTTP error " + std::to_string(statusCode) + ": " + 
                                responseData.toStdString());
        }
        
        qtforge::Logger::debug(name(), "GET request successful: " + url);
        return responseData.toStdString();
        
    } catch (const std::exception& e) {
        return qtforge::Error("GET request failed: " + std::string(e.what()));
    }
}

qtforge::expected<std::string, qtforge::Error> HttpClientPlugin::post(
    const std::string& url,
    const std::string& data,
    const std::map<std::string, std::string>& headers) {
    
    try {
        QNetworkRequest request = createRequest(url, headers);
        
        // Set content type if not specified
        if (headers.find("Content-Type") == headers.end()) {
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        }
        
        QEventLoop loop;
        QTimer timeoutTimer;
        timeoutTimer.setSingleShot(true);
        timeoutTimer.setInterval(30000);
        
        QNetworkReply* reply = networkManager_->post(request, QByteArray::fromStdString(data));
        
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        connect(&timeoutTimer, &QTimer::timeout, &loop, &QEventLoop::quit);
        connect(reply, QOverload<const QList<QSslError>&>::of(&QNetworkReply::sslErrors),
                this, &HttpClientPlugin::onSslErrors);
        
        timeoutTimer.start();
        loop.exec();
        
        if (!timeoutTimer.isActive()) {
            reply->deleteLater();
            return qtforge::Error("Request timeout");
        }
        
        timeoutTimer.stop();
        
        if (reply->error() != QNetworkReply::NoError) {
            QString errorString = reply->errorString();
            reply->deleteLater();
            return qtforge::Error("Network error: " + errorString.toStdString());
        }
        
        QByteArray responseData = reply->readAll();
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        
        reply->deleteLater();
        
        if (statusCode >= 400) {
            return qtforge::Error("HTTP error " + std::to_string(statusCode) + ": " + 
                                responseData.toStdString());
        }
        
        qtforge::Logger::debug(name(), "POST request successful: " + url);
        return responseData.toStdString();
        
    } catch (const std::exception& e) {
        return qtforge::Error("POST request failed: " + std::string(e.what()));
    }
}

void HttpClientPlugin::setupMessageHandlers() {
    auto& messageBus = qtforge::MessageBus::instance();
    
    // Handle HTTP request messages
    subscriptions_.emplace_back(
        messageBus.subscribe<qtforge::HttpRequestMessage>("http.request",
            [this](const qtforge::HttpRequestMessage& msg) {
                handleHttpRequest(msg);
            })
    );
}

void HttpClientPlugin::handleHttpRequest(const qtforge::HttpRequestMessage& message) {
    try {
        std::string response;
        qtforge::Error error;
        
        if (message.method == "GET") {
            auto result = get(message.url, message.headers);
            if (result) {
                response = result.value();
            } else {
                error = result.error();
            }
        } else if (message.method == "POST") {
            auto result = post(message.url, message.body, message.headers);
            if (result) {
                response = result.value();
            } else {
                error = result.error();
            }
        } else {
            error = qtforge::Error("Unsupported HTTP method: " + message.method);
        }
        
        // Publish response
        auto& messageBus = qtforge::MessageBus::instance();
        qtforge::HttpResponseMessage responseMsg;
        responseMsg.requestId = message.requestId;
        responseMsg.success = error.message().empty();
        responseMsg.response = response;
        responseMsg.error = error.message();
        
        messageBus.publish("http.response", responseMsg);
        
    } catch (const std::exception& e) {
        qtforge::Logger::error(name(), "HTTP request handling failed: " + std::string(e.what()));
    }
}

QNetworkRequest HttpClientPlugin::createRequest(
    const std::string& url,
    const std::map<std::string, std::string>& headers) {
    
    QNetworkRequest request(QUrl(QString::fromStdString(url)));
    
    // Set default headers
    request.setHeader(QNetworkRequest::UserAgentHeader, "QtForge-HttpClient/1.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, 
                        QNetworkRequest::NoLessSafeRedirectPolicy);
    
    // Set custom headers
    for (const auto& header : headers) {
        request.setRawHeader(QByteArray::fromStdString(header.first),
                           QByteArray::fromStdString(header.second));
    }
    
    return request;
}

void HttpClientPlugin::onSslErrors(const QList<QSslError>& errors) {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    for (const auto& error : errors) {
        qtforge::Logger::warning(name(), "SSL Error: " + error.errorString().toStdString());
    }
    
    // In production, you should validate SSL certificates properly
    // For development, you might want to ignore certain SSL errors
    // reply->ignoreSslErrors();
}
```

## WebSocket Plugin

### WebSocket Client Implementation

```cpp
// include/websocket_plugin.hpp
#pragma once

#include <qtforge/core/plugin_interface.hpp>
#include <qtforge/communication/message_bus.hpp>
#include <QWebSocket>
#include <QTimer>
#include <memory>

class WebSocketPlugin : public QObject, public qtforge::IPlugin {
    Q_OBJECT

public:
    WebSocketPlugin(QObject* parent = nullptr);
    ~WebSocketPlugin() override;

    // Plugin interface
    std::string name() const override { return "WebSocketPlugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "WebSocket client plugin for real-time communication";
    }

    // Lifecycle
    qtforge::expected<void, qtforge::Error> initialize() override;
    qtforge::expected<void, qtforge::Error> activate() override;
    qtforge::expected<void, qtforge::Error> deactivate() override;
    void cleanup() override;

    qtforge::PluginState state() const override { return currentState_; }
    bool isCompatible(const std::string& version) const override;

    // WebSocket operations
    qtforge::expected<void, qtforge::Error> connectToServer(const std::string& url);
    qtforge::expected<void, qtforge::Error> disconnect();
    qtforge::expected<void, qtforge::Error> sendMessage(const std::string& message);
    bool isConnected() const;

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);
    void onBinaryMessageReceived(const QByteArray& message);
    void onError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError>& errors);
    void onPingTimeout();

private:
    void setupMessageHandlers();
    void handleWebSocketConnect(const qtforge::WebSocketConnectMessage& message);
    void handleWebSocketSend(const qtforge::WebSocketSendMessage& message);
    void startHeartbeat();
    void stopHeartbeat();

    qtforge::PluginState currentState_;
    std::unique_ptr<QWebSocket> webSocket_;
    std::unique_ptr<QTimer> heartbeatTimer_;
    std::vector<qtforge::SubscriptionHandle> subscriptions_;
    std::string currentUrl_;
    bool autoReconnect_ = true;
    int reconnectAttempts_ = 0;
    static constexpr int maxReconnectAttempts_ = 5;
};
```

### WebSocket Implementation

```cpp
// src/websocket_plugin.cpp
#include "websocket_plugin.hpp"
#include <qtforge/communication/message_bus.hpp>
#include <qtforge/utils/logger.hpp>
#include <QJsonDocument>
#include <QJsonObject>

WebSocketPlugin::WebSocketPlugin(QObject* parent)
    : QObject(parent), currentState_(qtforge::PluginState::Unloaded) {
}

WebSocketPlugin::~WebSocketPlugin() {
    cleanup();
}

qtforge::expected<void, qtforge::Error> WebSocketPlugin::initialize() {
    try {
        qtforge::Logger::info(name(), "Initializing WebSocket plugin...");
        
        // Create WebSocket
        webSocket_ = std::make_unique<QWebSocket>();
        
        // Connect signals
        connect(webSocket_.get(), &QWebSocket::connected, 
                this, &WebSocketPlugin::onConnected);
        connect(webSocket_.get(), &QWebSocket::disconnected, 
                this, &WebSocketPlugin::onDisconnected);
        connect(webSocket_.get(), &QWebSocket::textMessageReceived,
                this, &WebSocketPlugin::onTextMessageReceived);
        connect(webSocket_.get(), &QWebSocket::binaryMessageReceived,
                this, &WebSocketPlugin::onBinaryMessageReceived);
        connect(webSocket_.get(), QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
                this, &WebSocketPlugin::onError);
        connect(webSocket_.get(), &QWebSocket::sslErrors,
                this, &WebSocketPlugin::onSslErrors);
        
        // Create heartbeat timer
        heartbeatTimer_ = std::make_unique<QTimer>(this);
        heartbeatTimer_->setInterval(30000); // 30 seconds
        connect(heartbeatTimer_.get(), &QTimer::timeout,
                this, &WebSocketPlugin::onPingTimeout);
        
        // Setup message handlers
        setupMessageHandlers();
        
        currentState_ = qtforge::PluginState::Initialized;
        qtforge::Logger::info(name(), "WebSocket plugin initialized successfully");
        
        return {};
        
    } catch (const std::exception& e) {
        currentState_ = qtforge::PluginState::Error;
        return qtforge::Error("WebSocket plugin initialization failed: " + std::string(e.what()));
    }
}

qtforge::expected<void, qtforge::Error> WebSocketPlugin::connectToServer(const std::string& url) {
    try {
        if (isConnected()) {
            return qtforge::Error("Already connected to WebSocket server");
        }
        
        currentUrl_ = url;
        reconnectAttempts_ = 0;
        
        qtforge::Logger::info(name(), "Connecting to WebSocket server: " + url);
        webSocket_->open(QUrl(QString::fromStdString(url)));
        
        return {};
        
    } catch (const std::exception& e) {
        return qtforge::Error("WebSocket connection failed: " + std::string(e.what()));
    }
}

qtforge::expected<void, qtforge::Error> WebSocketPlugin::sendMessage(const std::string& message) {
    try {
        if (!isConnected()) {
            return qtforge::Error("WebSocket not connected");
        }
        
        qint64 bytesSent = webSocket_->sendTextMessage(QString::fromStdString(message));
        if (bytesSent == -1) {
            return qtforge::Error("Failed to send WebSocket message");
        }
        
        qtforge::Logger::debug(name(), "WebSocket message sent: " + std::to_string(bytesSent) + " bytes");
        return {};
        
    } catch (const std::exception& e) {
        return qtforge::Error("WebSocket send failed: " + std::string(e.what()));
    }
}

void WebSocketPlugin::onConnected() {
    qtforge::Logger::info(name(), "WebSocket connected to: " + currentUrl_);
    
    reconnectAttempts_ = 0;
    startHeartbeat();
    
    // Publish connection event
    auto& messageBus = qtforge::MessageBus::instance();
    qtforge::WebSocketEventMessage event;
    event.type = "connected";
    event.url = currentUrl_;
    event.timestamp = std::chrono::system_clock::now();
    
    messageBus.publish("websocket.event", event);
}

void WebSocketPlugin::onDisconnected() {
    qtforge::Logger::info(name(), "WebSocket disconnected from: " + currentUrl_);
    
    stopHeartbeat();
    
    // Publish disconnection event
    auto& messageBus = qtforge::MessageBus::instance();
    qtforge::WebSocketEventMessage event;
    event.type = "disconnected";
    event.url = currentUrl_;
    event.timestamp = std::chrono::system_clock::now();
    
    messageBus.publish("websocket.event", event);
    
    // Auto-reconnect if enabled
    if (autoReconnect_ && reconnectAttempts_ < maxReconnectAttempts_) {
        reconnectAttempts_++;
        
        qtforge::Logger::info(name(), "Attempting to reconnect (" + 
                            std::to_string(reconnectAttempts_) + "/" + 
                            std::to_string(maxReconnectAttempts_) + ")");
        
        QTimer::singleShot(5000, [this]() {
            connectToServer(currentUrl_);
        });
    }
}

void WebSocketPlugin::onTextMessageReceived(const QString& message) {
    qtforge::Logger::debug(name(), "WebSocket message received: " + message.toStdString());
    
    // Publish received message
    auto& messageBus = qtforge::MessageBus::instance();
    qtforge::WebSocketMessageEvent messageEvent;
    messageEvent.type = "text";
    messageEvent.data = message.toStdString();
    messageEvent.timestamp = std::chrono::system_clock::now();
    
    messageBus.publish("websocket.message", messageEvent);
}

void WebSocketPlugin::onError(QAbstractSocket::SocketError error) {
    QString errorString = webSocket_->errorString();
    qtforge::Logger::error(name(), "WebSocket error: " + errorString.toStdString());
    
    // Publish error event
    auto& messageBus = qtforge::MessageBus::instance();
    qtforge::WebSocketEventMessage event;
    event.type = "error";
    event.error = errorString.toStdString();
    event.timestamp = std::chrono::system_clock::now();
    
    messageBus.publish("websocket.event", event);
}

void WebSocketPlugin::setupMessageHandlers() {
    auto& messageBus = qtforge::MessageBus::instance();
    
    // Handle WebSocket connection requests
    subscriptions_.emplace_back(
        messageBus.subscribe<qtforge::WebSocketConnectMessage>("websocket.connect",
            [this](const qtforge::WebSocketConnectMessage& msg) {
                handleWebSocketConnect(msg);
            })
    );
    
    // Handle WebSocket send requests
    subscriptions_.emplace_back(
        messageBus.subscribe<qtforge::WebSocketSendMessage>("websocket.send",
            [this](const qtforge::WebSocketSendMessage& msg) {
                handleWebSocketSend(msg);
            })
    );
}

void WebSocketPlugin::startHeartbeat() {
    heartbeatTimer_->start();
}

void WebSocketPlugin::stopHeartbeat() {
    heartbeatTimer_->stop();
}

void WebSocketPlugin::onPingTimeout() {
    if (isConnected()) {
        // Send ping frame
        webSocket_->ping();
        qtforge::Logger::debug(name(), "WebSocket ping sent");
    }
}

bool WebSocketPlugin::isConnected() const {
    return webSocket_ && webSocket_->state() == QAbstractSocket::ConnectedState;
}
```

## Network Service Discovery

### Service Discovery Plugin

```cpp
// include/service_discovery_plugin.hpp
#pragma once

#include <qtforge/core/plugin_interface.hpp>
#include <QUdpSocket>
#include <QTimer>
#include <QHostAddress>
#include <memory>

struct NetworkService {
    std::string name;
    std::string type;
    QHostAddress address;
    quint16 port;
    std::map<std::string, std::string> properties;
    std::chrono::system_clock::time_point lastSeen;
};

class ServiceDiscoveryPlugin : public QObject, public qtforge::IPlugin {
    Q_OBJECT

public:
    ServiceDiscoveryPlugin(QObject* parent = nullptr);
    ~ServiceDiscoveryPlugin() override;

    // Plugin interface
    std::string name() const override { return "ServiceDiscoveryPlugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "Network service discovery using UDP multicast";
    }

    // Lifecycle
    qtforge::expected<void, qtforge::Error> initialize() override;
    qtforge::expected<void, qtforge::Error> activate() override;
    qtforge::expected<void, qtforge::Error> deactivate() override;
    void cleanup() override;

    qtforge::PluginState state() const override { return currentState_; }
    bool isCompatible(const std::string& version) const override;

    // Service discovery operations
    qtforge::expected<void, qtforge::Error> announceService(const NetworkService& service);
    qtforge::expected<void, qtforge::Error> stopAnnouncing(const std::string& serviceName);
    std::vector<NetworkService> getDiscoveredServices() const;
    std::vector<NetworkService> findServices(const std::string& serviceType) const;

private slots:
    void onUdpDataReceived();
    void onAnnouncementTimer();
    void onCleanupTimer();

private:
    void setupMulticastSocket();
    void processServiceAnnouncement(const QByteArray& data, const QHostAddress& sender);
    void sendServiceAnnouncement(const NetworkService& service);
    void cleanupStaleServices();

    qtforge::PluginState currentState_;
    std::unique_ptr<QUdpSocket> multicastSocket_;
    std::unique_ptr<QTimer> announcementTimer_;
    std::unique_ptr<QTimer> cleanupTimer_;
    
    std::map<std::string, NetworkService> announcedServices_;
    std::map<std::string, NetworkService> discoveredServices_;
    
    static constexpr quint16 multicastPort_ = 45454;
    static const QHostAddress multicastAddress_;
    static constexpr int announcementInterval_ = 30000; // 30 seconds
    static constexpr int serviceTimeout_ = 90000; // 90 seconds
};
```

## Testing Network Plugins

### Network Integration Tests

```cpp
#include <gtest/gtest.h>
#include "http_client_plugin.hpp"
#include "websocket_plugin.hpp"
#include <QCoreApplication>
#include <QEventLoop>
#include <QTimer>

class NetworkPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char** argv = nullptr;
            app_ = std::make_unique<QCoreApplication>(argc, argv);
        }
        
        httpPlugin_ = std::make_unique<HttpClientPlugin>();
        httpPlugin_->initialize();
        httpPlugin_->activate();
        
        wsPlugin_ = std::make_unique<WebSocketPlugin>();
        wsPlugin_->initialize();
        wsPlugin_->activate();
    }
    
    void TearDown() override {
        httpPlugin_->cleanup();
        wsPlugin_->cleanup();
    }
    
    std::unique_ptr<QCoreApplication> app_;
    std::unique_ptr<HttpClientPlugin> httpPlugin_;
    std::unique_ptr<WebSocketPlugin> wsPlugin_;
};

TEST_F(NetworkPluginTest, HttpGetRequest) {
    // Test HTTP GET request
    auto result = httpPlugin_->get("https://httpbin.org/get");
    EXPECT_TRUE(result.has_value());
    
    if (result.has_value()) {
        std::string response = result.value();
        EXPECT_FALSE(response.empty());
        EXPECT_NE(response.find("httpbin.org"), std::string::npos);
    }
}

TEST_F(NetworkPluginTest, HttpPostRequest) {
    // Test HTTP POST request
    std::string jsonData = R"({"test": "data", "number": 42})";
    std::map<std::string, std::string> headers;
    headers["Content-Type"] = "application/json";
    
    auto result = httpPlugin_->post("https://httpbin.org/post", jsonData, headers);
    EXPECT_TRUE(result.has_value());
    
    if (result.has_value()) {
        std::string response = result.value();
        EXPECT_FALSE(response.empty());
        EXPECT_NE(response.find("test"), std::string::npos);
        EXPECT_NE(response.find("42"), std::string::npos);
    }
}

TEST_F(NetworkPluginTest, WebSocketConnection) {
    // Test WebSocket connection (using a test WebSocket server)
    auto connectResult = wsPlugin_->connectToServer("wss://echo.websocket.org");
    EXPECT_TRUE(connectResult.has_value());
    
    // Wait for connection
    QEventLoop loop;
    QTimer::singleShot(5000, &loop, &QEventLoop::quit);
    loop.exec();
    
    EXPECT_TRUE(wsPlugin_->isConnected());
    
    // Test sending message
    auto sendResult = wsPlugin_->sendMessage("Hello WebSocket!");
    EXPECT_TRUE(sendResult.has_value());
    
    // Disconnect
    auto disconnectResult = wsPlugin_->disconnect();
    EXPECT_TRUE(disconnectResult.has_value());
}
```

## Key Features Demonstrated

1. **HTTP Client**: RESTful API communication with proper error handling
2. **WebSocket Client**: Real-time bidirectional communication
3. **SSL/TLS Support**: Secure network communication
4. **Service Discovery**: Network service announcement and discovery
5. **Error Handling**: Comprehensive network error handling
6. **Async Operations**: Non-blocking network operations
7. **Message Integration**: Integration with QtForge message bus

## Best Practices

1. **Error Handling**: Handle network errors gracefully
2. **Timeouts**: Implement appropriate timeouts for network operations
3. **SSL/TLS**: Use secure connections for sensitive data
4. **Retry Logic**: Implement retry mechanisms for transient failures
5. **Resource Management**: Properly clean up network resources
6. **Threading**: Handle network operations on appropriate threads

## Next Steps

- **[UI Plugin Example](ui-plugin.md)**: User interface plugins
- **[Python Examples](python-examples.md)**: Python network plugins
- **[Advanced Examples](advanced.md)**: Complex network patterns
- **[Security Configuration](../user-guide/security-configuration.md)**: Network security best practices
