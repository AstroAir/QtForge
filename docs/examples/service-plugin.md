# Service Plugin Example

This example demonstrates how to create a service plugin that provides functionality to other plugins through a well-defined service interface.

## Overview

Service plugins in QtForge provide reusable functionality that can be consumed by other plugins. This example shows:

- Service interface definition
- Service registration and discovery
- Inter-plugin communication through services
- Service lifecycle management
- Dependency injection patterns

## Service Interface Definition

### Database Service Interface

```cpp
// include/database_service.hpp
#pragma once

#include <qtforge/core/service_interface.hpp>
#include <qtforge/utils/expected.hpp>
#include <string>
#include <vector>
#include <unordered_map>

namespace qtforge::services {

struct DatabaseRecord {
    std::string id;
    std::unordered_map<std::string, std::any> fields;
    std::chrono::system_clock::time_point created;
    std::chrono::system_clock::time_point modified;
};

struct QueryResult {
    std::vector<DatabaseRecord> records;
    size_t totalCount;
    bool hasMore;
    std::string cursor;
};

struct QueryOptions {
    size_t limit = 100;
    size_t offset = 0;
    std::string orderBy;
    bool ascending = true;
    std::unordered_map<std::string, std::any> filters;
};

class IDatabaseService : public IService {
public:
    virtual ~IDatabaseService() = default;

    // Service identification
    std::string serviceId() const override { return "database.service"; }
    std::string serviceVersion() const override { return "1.0.0"; }

    // Database operations
    virtual expected<std::string, Error> createRecord(
        const std::string& table,
        const std::unordered_map<std::string, std::any>& data) = 0;

    virtual expected<DatabaseRecord, Error> getRecord(
        const std::string& table,
        const std::string& id) = 0;

    virtual expected<void, Error> updateRecord(
        const std::string& table,
        const std::string& id,
        const std::unordered_map<std::string, std::any>& data) = 0;

    virtual expected<void, Error> deleteRecord(
        const std::string& table,
        const std::string& id) = 0;

    virtual expected<QueryResult, Error> query(
        const std::string& table,
        const QueryOptions& options = {}) = 0;

    // Transaction support
    virtual expected<std::string, Error> beginTransaction() = 0;
    virtual expected<void, Error> commitTransaction(const std::string& transactionId) = 0;
    virtual expected<void, Error> rollbackTransaction(const std::string& transactionId) = 0;

    // Schema management
    virtual expected<void, Error> createTable(
        const std::string& table,
        const std::unordered_map<std::string, std::string>& schema) = 0;

    virtual expected<void, Error> dropTable(const std::string& table) = 0;

    virtual expected<std::vector<std::string>, Error> listTables() = 0;
};

} // namespace qtforge::services
```

## Service Implementation

### SQLite Database Service

```cpp
// src/sqlite_database_service.cpp
#include "sqlite_database_service.hpp"
#include <qtforge/utils/logger.hpp>
#include <sqlite3.h>
#include <json/json.h>

namespace qtforge::services {

class SQLiteDatabaseService : public IDatabaseService {
public:
    SQLiteDatabaseService() : db_(nullptr) {}

    ~SQLiteDatabaseService() override {
        cleanup();
    }

    // Service interface
    std::string serviceName() const override { return "SQLite Database Service"; }
    std::string serviceDescription() const override {
        return "SQLite-based database service implementation";
    }

    expected<void, Error> initialize(const ServiceConfig& config) override {
        try {
            std::string dbPath = config.get<std::string>("database_path", "qtforge.db");

            int result = sqlite3_open(dbPath.c_str(), &db_);
            if (result != SQLITE_OK) {
                return Error("Failed to open database: " + std::string(sqlite3_errmsg(db_)));
            }

            // Enable foreign keys
            executeSQL("PRAGMA foreign_keys = ON");

            // Create metadata table
            auto createResult = createMetadataTable();
            if (!createResult) {
                return createResult.error();
            }

            Logger::info("SQLiteDatabaseService", "Database service initialized: " + dbPath);
            return {};

        } catch (const std::exception& e) {
            return Error("Database initialization failed: " + std::string(e.what()));
        }
    }

    void cleanup() override {
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
    }

    // Database operations
    expected<std::string, Error> createRecord(
        const std::string& table,
        const std::unordered_map<std::string, std::any>& data) override {

        try {
            std::string id = generateId();

            // Build INSERT statement
            std::string sql = "INSERT INTO " + table + " (id, data, created, modified) VALUES (?, ?, ?, ?)";

            sqlite3_stmt* stmt;
            int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
            if (result != SQLITE_OK) {
                return Error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
            }

            // Bind parameters
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();

            sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, serializeData(data).c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int64(stmt, 3, timestamp);
            sqlite3_bind_int64(stmt, 4, timestamp);

            // Execute statement
            result = sqlite3_step(stmt);
            sqlite3_finalize(stmt);

            if (result != SQLITE_DONE) {
                return Error("Failed to insert record: " + std::string(sqlite3_errmsg(db_)));
            }

            Logger::debug("SQLiteDatabaseService", "Created record: " + id + " in table: " + table);
            return id;

        } catch (const std::exception& e) {
            return Error("Create record failed: " + std::string(e.what()));
        }
    }

    expected<DatabaseRecord, Error> getRecord(
        const std::string& table,
        const std::string& id) override {

        try {
            std::string sql = "SELECT id, data, created, modified FROM " + table + " WHERE id = ?";

            sqlite3_stmt* stmt;
            int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
            if (result != SQLITE_OK) {
                return Error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db_)));
            }

            sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_STATIC);

            result = sqlite3_step(stmt);
            if (result == SQLITE_ROW) {
                DatabaseRecord record;
                record.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

                std::string dataJson = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                record.fields = deserializeData(dataJson);

                auto created = sqlite3_column_int64(stmt, 2);
                auto modified = sqlite3_column_int64(stmt, 3);

                record.created = std::chrono::system_clock::from_time_t(created);
                record.modified = std::chrono::system_clock::from_time_t(modified);

                sqlite3_finalize(stmt);
                return record;

            } else if (result == SQLITE_DONE) {
                sqlite3_finalize(stmt);
                return Error("Record not found: " + id);

            } else {
                sqlite3_finalize(stmt);
                return Error("Query failed: " + std::string(sqlite3_errmsg(db_)));
            }

        } catch (const std::exception& e) {
            return Error("Get record failed: " + std::string(e.what()));
        }
    }

    expected<QueryResult, Error> query(
        const std::string& table,
        const QueryOptions& options) override {

        try {
            // Build query
            std::string sql = "SELECT id, data, created, modified FROM " + table;

            // Add WHERE clause for filters
            std::vector<std::string> whereConditions;
            for (const auto& filter : options.filters) {
                whereConditions.push_back("JSON_EXTRACT(data, '$." + filter.first + "') = ?");
            }

            if (!whereConditions.empty()) {
                sql += " WHERE " + joinStrings(whereConditions, " AND ");
            }

            // Add ORDER BY
            if (!options.orderBy.empty()) {
                sql += " ORDER BY " + options.orderBy;
                sql += options.ascending ? " ASC" : " DESC";
            }

            // Add LIMIT and OFFSET
            sql += " LIMIT ? OFFSET ?";

            sqlite3_stmt* stmt;
            int result = sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
            if (result != SQLITE_OK) {
                return Error("Failed to prepare query: " + std::string(sqlite3_errmsg(db_)));
            }

            // Bind filter parameters
            int paramIndex = 1;
            for (const auto& filter : options.filters) {
                bindAnyValue(stmt, paramIndex++, filter.second);
            }

            // Bind limit and offset
            sqlite3_bind_int64(stmt, paramIndex++, options.limit);
            sqlite3_bind_int64(stmt, paramIndex++, options.offset);

            // Execute query
            QueryResult queryResult;
            while ((result = sqlite3_step(stmt)) == SQLITE_ROW) {
                DatabaseRecord record;
                record.id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

                std::string dataJson = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                record.fields = deserializeData(dataJson);

                auto created = sqlite3_column_int64(stmt, 2);
                auto modified = sqlite3_column_int64(stmt, 3);

                record.created = std::chrono::system_clock::from_time_t(created);
                record.modified = std::chrono::system_clock::from_time_t(modified);

                queryResult.records.push_back(std::move(record));
            }

            sqlite3_finalize(stmt);

            if (result != SQLITE_DONE) {
                return Error("Query execution failed: " + std::string(sqlite3_errmsg(db_)));
            }

            // Get total count
            auto countResult = getRecordCount(table, options.filters);
            if (countResult) {
                queryResult.totalCount = countResult.value();
                queryResult.hasMore = (options.offset + queryResult.records.size()) < queryResult.totalCount;
            }

            return queryResult;

        } catch (const std::exception& e) {
            return Error("Query failed: " + std::string(e.what()));
        }
    }

private:
    sqlite3* db_;

    expected<void, Error> createMetadataTable() {
        std::string sql = R"(
            CREATE TABLE IF NOT EXISTS _qtforge_metadata (
                table_name TEXT PRIMARY KEY,
                schema TEXT NOT NULL,
                created INTEGER NOT NULL,
                modified INTEGER NOT NULL
            )
        )";

        return executeSQL(sql);
    }

    expected<void, Error> executeSQL(const std::string& sql) {
        char* errorMsg = nullptr;
        int result = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errorMsg);

        if (result != SQLITE_OK) {
            std::string error = errorMsg ? errorMsg : "Unknown error";
            sqlite3_free(errorMsg);
            return Error("SQL execution failed: " + error);
        }

        return {};
    }

    std::string generateId() {
        // Simple UUID generation (in production, use proper UUID library)
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);

        std::string uuid = "xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx";
        for (char& c : uuid) {
            if (c == 'x') {
                c = "0123456789abcdef"[dis(gen)];
            } else if (c == 'y') {
                c = "89ab"[dis(gen) & 3];
            }
        }

        return uuid;
    }

    std::string serializeData(const std::unordered_map<std::string, std::any>& data) {
        Json::Value json;
        for (const auto& pair : data) {
            // Convert std::any to JSON (simplified implementation)
            json[pair.first] = convertAnyToJson(pair.second);
        }

        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, json);
    }

    std::unordered_map<std::string, std::any> deserializeData(const std::string& jsonStr) {
        Json::Value json;
        Json::Reader reader;

        if (!reader.parse(jsonStr, json)) {
            throw std::runtime_error("Failed to parse JSON data");
        }

        std::unordered_map<std::string, std::any> data;
        for (const auto& key : json.getMemberNames()) {
            data[key] = convertJsonToAny(json[key]);
        }

        return data;
    }
};

} // namespace qtforge::services
```

## Service Plugin Implementation

### Database Service Plugin

```cpp
// src/database_service_plugin.cpp
#include <qtforge/core/plugin_interface.hpp>
#include <qtforge/core/service_registry.hpp>
#include "sqlite_database_service.hpp"

class DatabaseServicePlugin : public qtforge::IPlugin {
public:
    DatabaseServicePlugin() : currentState_(qtforge::PluginState::Unloaded) {}

    ~DatabaseServicePlugin() override {
        cleanup();
    }

    // Plugin metadata
    std::string name() const override { return "DatabaseServicePlugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override {
        return "Provides database service functionality";
    }
    std::vector<std::string> dependencies() const override {
        return {"CorePlugin >= 1.0.0"};
    }

    // Plugin lifecycle
    qtforge::expected<void, qtforge::Error> initialize() override {
        try {
            qtforge::Logger::info(name(), "Initializing database service plugin...");

            // Create service instance
            databaseService_ = std::make_unique<qtforge::services::SQLiteDatabaseService>();

            // Configure service
            qtforge::ServiceConfig config;
            config.set("database_path", "./qtforge_data.db");

            auto initResult = databaseService_->initialize(config);
            if (!initResult) {
                return qtforge::Error("Failed to initialize database service: " +
                                    initResult.error().message());
            }

            currentState_ = qtforge::PluginState::Initialized;
            qtforge::Logger::info(name(), "Database service plugin initialized successfully");

            return {};

        } catch (const std::exception& e) {
            currentState_ = qtforge::PluginState::Error;
            return qtforge::Error("Database service plugin initialization failed: " +
                                std::string(e.what()));
        }
    }

    qtforge::expected<void, qtforge::Error> activate() override {
        if (currentState_ != qtforge::PluginState::Initialized) {
            return qtforge::Error("Plugin must be initialized before activation");
        }

        try {
            qtforge::Logger::info(name(), "Activating database service plugin...");

            // Register service
            auto& serviceRegistry = qtforge::ServiceRegistry::instance();
            auto registerResult = serviceRegistry.registerService(
                databaseService_->serviceId(),
                std::static_pointer_cast<qtforge::IService>(databaseService_)
            );

            if (!registerResult) {
                return qtforge::Error("Failed to register database service: " +
                                    registerResult.error().message());
            }

            currentState_ = qtforge::PluginState::Active;
            qtforge::Logger::info(name(), "Database service plugin activated successfully");

            return {};

        } catch (const std::exception& e) {
            currentState_ = qtforge::PluginState::Error;
            return qtforge::Error("Database service plugin activation failed: " +
                                std::string(e.what()));
        }
    }

    qtforge::expected<void, qtforge::Error> deactivate() override {
        if (currentState_ != qtforge::PluginState::Active) {
            return qtforge::Error("Plugin is not active");
        }

        try {
            qtforge::Logger::info(name(), "Deactivating database service plugin...");

            // Unregister service
            auto& serviceRegistry = qtforge::ServiceRegistry::instance();
            serviceRegistry.unregisterService(databaseService_->serviceId());

            currentState_ = qtforge::PluginState::Initialized;
            qtforge::Logger::info(name(), "Database service plugin deactivated successfully");

            return {};

        } catch (const std::exception& e) {
            currentState_ = qtforge::PluginState::Error;
            return qtforge::Error("Database service plugin deactivation failed: " +
                                std::string(e.what()));
        }
    }

    void cleanup() override {
        qtforge::Logger::info(name(), "Cleaning up database service plugin...");

        if (databaseService_) {
            databaseService_->cleanup();
            databaseService_.reset();
        }

        currentState_ = qtforge::PluginState::Unloaded;
        qtforge::Logger::info(name(), "Database service plugin cleaned up");
    }

    qtforge::PluginState state() const override { return currentState_; }
    bool isCompatible(const std::string& version) const override {
        return version >= "1.0.0";
    }

private:
    qtforge::PluginState currentState_;
    std::shared_ptr<qtforge::services::SQLiteDatabaseService> databaseService_;
};

// Plugin factory functions
extern "C" QTFORGE_EXPORT qtforge::IPlugin* createPlugin() {
    return new DatabaseServicePlugin();
}

extern "C" QTFORGE_EXPORT void destroyPlugin(qtforge::IPlugin* plugin) {
    delete plugin;
}
```

## Service Consumer Example

### User Management Plugin

```cpp
// src/user_management_plugin.cpp
#include <qtforge/core/plugin_interface.hpp>
#include <qtforge/core/service_registry.hpp>
#include "database_service.hpp"

class UserManagementPlugin : public qtforge::IPlugin {
public:
    UserManagementPlugin() : currentState_(qtforge::PluginState::Unloaded) {}

    // Plugin interface implementation...
    std::string name() const override { return "UserManagementPlugin"; }
    std::vector<std::string> dependencies() const override {
        return {"DatabaseServicePlugin >= 1.0.0"};
    }

    qtforge::expected<void, qtforge::Error> initialize() override {
        try {
            qtforge::Logger::info(name(), "Initializing user management plugin...");

            // Get database service
            auto& serviceRegistry = qtforge::ServiceRegistry::instance();
            auto serviceResult = serviceRegistry.getService<qtforge::services::IDatabaseService>(
                "database.service");

            if (!serviceResult) {
                return qtforge::Error("Database service not available: " +
                                    serviceResult.error().message());
            }

            databaseService_ = serviceResult.value();

            // Create users table
            auto createTableResult = createUsersTable();
            if (!createTableResult) {
                return qtforge::Error("Failed to create users table: " +
                                    createTableResult.error().message());
            }

            currentState_ = qtforge::PluginState::Initialized;
            return {};

        } catch (const std::exception& e) {
            currentState_ = qtforge::PluginState::Error;
            return qtforge::Error("User management plugin initialization failed: " +
                                std::string(e.what()));
        }
    }

    // User management operations
    qtforge::expected<std::string, qtforge::Error> createUser(
        const std::string& username,
        const std::string& email,
        const std::string& hashedPassword) {

        if (!databaseService_) {
            return qtforge::Error("Database service not available");
        }

        std::unordered_map<std::string, std::any> userData;
        userData["username"] = username;
        userData["email"] = email;
        userData["password_hash"] = hashedPassword;
        userData["active"] = true;
        userData["last_login"] = std::chrono::system_clock::now();

        return databaseService_->createRecord("users", userData);
    }

    qtforge::expected<User, qtforge::Error> getUser(const std::string& userId) {
        if (!databaseService_) {
            return qtforge::Error("Database service not available");
        }

        auto recordResult = databaseService_->getRecord("users", userId);
        if (!recordResult) {
            return recordResult.error();
        }

        return convertRecordToUser(recordResult.value());
    }

    qtforge::expected<std::vector<User>, qtforge::Error> searchUsers(
        const std::string& searchTerm) {

        if (!databaseService_) {
            return qtforge::Error("Database service not available");
        }

        qtforge::services::QueryOptions options;
        options.filters["username"] = searchTerm + "%";
        options.limit = 50;

        auto queryResult = databaseService_->query("users", options);
        if (!queryResult) {
            return queryResult.error();
        }

        std::vector<User> users;
        for (const auto& record : queryResult.value().records) {
            auto userResult = convertRecordToUser(record);
            if (userResult) {
                users.push_back(userResult.value());
            }
        }

        return users;
    }

private:
    qtforge::PluginState currentState_;
    std::shared_ptr<qtforge::services::IDatabaseService> databaseService_;

    qtforge::expected<void, qtforge::Error> createUsersTable() {
        std::unordered_map<std::string, std::string> schema;
        schema["username"] = "TEXT UNIQUE NOT NULL";
        schema["email"] = "TEXT UNIQUE NOT NULL";
        schema["password_hash"] = "TEXT NOT NULL";
        schema["active"] = "BOOLEAN DEFAULT TRUE";
        schema["last_login"] = "INTEGER";

        return databaseService_->createTable("users", schema);
    }

    qtforge::expected<User, qtforge::Error> convertRecordToUser(
        const qtforge::services::DatabaseRecord& record) {

        try {
            User user;
            user.id = record.id;
            user.username = std::any_cast<std::string>(record.fields.at("username"));
            user.email = std::any_cast<std::string>(record.fields.at("email"));
            user.active = std::any_cast<bool>(record.fields.at("active"));
            user.created = record.created;
            user.modified = record.modified;

            return user;

        } catch (const std::exception& e) {
            return qtforge::Error("Failed to convert record to user: " + std::string(e.what()));
        }
    }
};
```

## Testing Service Plugins

### Service Integration Tests

```cpp
#include <gtest/gtest.h>
#include "database_service_plugin.hpp"
#include "user_management_plugin.hpp"
#include <qtforge/core/plugin_manager.hpp>
#include <qtforge/core/service_registry.hpp>

class ServicePluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        pluginManager_ = std::make_unique<qtforge::PluginManager>();

        // Load and initialize database service plugin
        auto dbPluginResult = pluginManager_->loadPlugin("DatabaseServicePlugin.qtplugin");
        ASSERT_TRUE(dbPluginResult.has_value());

        dbPlugin_ = dbPluginResult.value();
        ASSERT_TRUE(dbPlugin_->initialize().has_value());
        ASSERT_TRUE(dbPlugin_->activate().has_value());

        // Load and initialize user management plugin
        auto userPluginResult = pluginManager_->loadPlugin("UserManagementPlugin.qtplugin");
        ASSERT_TRUE(userPluginResult.has_value());

        userPlugin_ = userPluginResult.value();
        ASSERT_TRUE(userPlugin_->initialize().has_value());
        ASSERT_TRUE(userPlugin_->activate().has_value());
    }

    void TearDown() override {
        if (userPlugin_) {
            userPlugin_->deactivate();
            userPlugin_->cleanup();
        }

        if (dbPlugin_) {
            dbPlugin_->deactivate();
            dbPlugin_->cleanup();
        }
    }

    std::unique_ptr<qtforge::PluginManager> pluginManager_;
    std::shared_ptr<qtforge::IPlugin> dbPlugin_;
    std::shared_ptr<qtforge::IPlugin> userPlugin_;
};

TEST_F(ServicePluginTest, ServiceRegistration) {
    auto& serviceRegistry = qtforge::ServiceRegistry::instance();

    // Check that database service is registered
    auto dbService = serviceRegistry.getService<qtforge::services::IDatabaseService>("database.service");
    EXPECT_TRUE(dbService.has_value());

    // Verify service functionality
    std::unordered_map<std::string, std::any> testData;
    testData["name"] = std::string("Test Record");
    testData["value"] = 42;

    auto createResult = dbService.value()->createRecord("test_table", testData);
    EXPECT_TRUE(createResult.has_value());

    std::string recordId = createResult.value();
    EXPECT_FALSE(recordId.empty());

    // Retrieve the record
    auto getResult = dbService.value()->getRecord("test_table", recordId);
    EXPECT_TRUE(getResult.has_value());

    auto record = getResult.value();
    EXPECT_EQ(record.id, recordId);
    EXPECT_EQ(std::any_cast<std::string>(record.fields.at("name")), "Test Record");
    EXPECT_EQ(std::any_cast<int>(record.fields.at("value")), 42);
}

TEST_F(ServicePluginTest, UserManagement) {
    auto userMgmtPlugin = std::dynamic_pointer_cast<UserManagementPlugin>(userPlugin_);
    ASSERT_TRUE(userMgmtPlugin != nullptr);

    // Create a user
    auto createResult = userMgmtPlugin->createUser("testuser", "test@example.com", "hashed_password");
    EXPECT_TRUE(createResult.has_value());

    std::string userId = createResult.value();
    EXPECT_FALSE(userId.empty());

    // Retrieve the user
    auto getUserResult = userMgmtPlugin->getUser(userId);
    EXPECT_TRUE(getUserResult.has_value());

    auto user = getUserResult.value();
    EXPECT_EQ(user.id, userId);
    EXPECT_EQ(user.username, "testuser");
    EXPECT_EQ(user.email, "test@example.com");
    EXPECT_TRUE(user.active);
}
```

## Key Features Demonstrated

1. **Service Interface Design**: Clean, well-defined service interfaces
2. **Service Implementation**: Concrete implementation with proper error handling
3. **Service Registration**: Automatic service registration and discovery
4. **Dependency Injection**: Services injected into dependent plugins
5. **Inter-Plugin Communication**: Plugins communicating through services
6. **Resource Management**: Proper cleanup and resource management
7. **Testing**: Comprehensive testing of service interactions

## Best Practices

1. **Interface Stability**: Keep service interfaces stable across versions
2. **Error Handling**: Comprehensive error handling with meaningful messages
3. **Resource Management**: Proper cleanup of service resources
4. **Thread Safety**: Ensure services are thread-safe when needed
5. **Documentation**: Document service interfaces and usage patterns

## Next Steps

- **[Network Plugin Example](network-plugin.md)**: Network-enabled plugins
- **[UI Plugin Example](ui-plugin.md)**: User interface plugins
- **[Advanced Examples](advanced.md)**: More complex plugin patterns
- **[Service Architecture Guide](../user-guide/plugin-architecture.md)**: Service architecture patterns
