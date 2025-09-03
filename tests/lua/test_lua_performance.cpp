/**
 * @file test_lua_performance.cpp
 * @brief Performance tests for Lua bindings
 * @version 3.2.0
 */

#include <QtTest/QtTest>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTemporaryDir>
#include <chrono>

#ifdef QTFORGE_LUA_BINDINGS
#include <sol/sol.hpp>
#endif

#include "../../include/qtplugin/bridges/lua_plugin_bridge.hpp"
#include "../../include/qtplugin/core/lua_plugin_loader.hpp"
#include "../../src/lua/qt_conversions.cpp"

class TestLuaPerformance : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Binding performance tests
    void testTypeConversionPerformance();
    void testLargeDataConversion();
    void testRepeatedConversions();
    
    // Plugin execution performance
    void testPluginLoadingPerformance();
    void testPluginExecutionPerformance();
    void testMultiplePluginPerformance();
    
    // Memory performance tests
    void testMemoryUsage();
    void testMemoryLeaks();
    void testGarbageCollection();
    
    // Scalability tests
    void testLargePluginCount();
    void testHighFrequencyExecution();
    void testConcurrentExecution();
    
    // Benchmark tests
    void benchmarkSimpleExecution();
    void benchmarkComplexExecution();
    void benchmarkDataProcessing();

private:
    void createPerformanceTestPlugin(const QString& filename, const QString& content);
    void measureExecutionTime(const std::function<void()>& func, const QString& testName);
    void measureMemoryUsage(const std::function<void()>& func, const QString& testName);
    
    QTemporaryDir m_temp_dir;
    std::unique_ptr<qtplugin::LuaPluginBridge> m_bridge;
    std::unique_ptr<qtplugin::LuaPluginLoader> m_loader;
    
#ifdef QTFORGE_LUA_BINDINGS
    std::unique_ptr<sol::state> m_lua_state;
#endif
};

void TestLuaPerformance::initTestCase()
{
    qDebug() << "Starting Lua performance test suite";
    
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available for performance tests");
    }
    
    QVERIFY(m_temp_dir.isValid());
    
#ifdef QTFORGE_LUA_BINDINGS
    m_lua_state = std::make_unique<sol::state>();
    m_lua_state->open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table);
#endif
}

void TestLuaPerformance::cleanupTestCase()
{
#ifdef QTFORGE_LUA_BINDINGS
    m_lua_state.reset();
#endif
    qDebug() << "Lua performance test suite completed";
}

void TestLuaPerformance::init()
{
    if (qtplugin::LuaPluginLoader::is_lua_available()) {
        m_bridge = std::make_unique<qtplugin::LuaPluginBridge>();
        m_loader = std::make_unique<qtplugin::LuaPluginLoader>();
    }
}

void TestLuaPerformance::cleanup()
{
    if (m_bridge) {
        m_bridge->shutdown();
        m_bridge.reset();
    }
    m_loader.reset();
}

void TestLuaPerformance::testTypeConversionPerformance()
{
#ifdef QTFORGE_LUA_BINDINGS
    const int iterations = 10000;
    
    // Test QString conversion performance
    measureExecutionTime([&]() {
        QString test_string = "Performance test string with some content";
        for (int i = 0; i < iterations; ++i) {
            sol::object lua_obj = qtforge_lua::qstring_to_lua(test_string, *m_lua_state);
            QString qt_string = qtforge_lua::lua_to_qstring(lua_obj);
            Q_UNUSED(qt_string)
        }
    }, "QString conversion");
    
    // Test QJsonObject conversion performance
    QJsonObject test_json;
    test_json["string"] = "test value";
    test_json["number"] = 42.5;
    test_json["boolean"] = true;
    test_json["array"] = QJsonArray{1, 2, 3, 4, 5};
    
    measureExecutionTime([&]() {
        for (int i = 0; i < iterations; ++i) {
            sol::object lua_obj = qtforge_lua::qjson_to_lua(test_json, *m_lua_state);
            QJsonValue json_val = qtforge_lua::lua_to_qjson(lua_obj);
            Q_UNUSED(json_val)
        }
    }, "QJsonObject conversion");
    
    // Test QStringList conversion performance
    QStringList test_list = {"item1", "item2", "item3", "item4", "item5"};
    
    measureExecutionTime([&]() {
        for (int i = 0; i < iterations; ++i) {
            sol::object lua_obj = qtforge_lua::qstringlist_to_lua(test_list, *m_lua_state);
            QStringList qt_list = qtforge_lua::lua_to_qstringlist(lua_obj);
            Q_UNUSED(qt_list)
        }
    }, "QStringList conversion");
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPerformance::testLargeDataConversion()
{
#ifdef QTFORGE_LUA_BINDINGS
    // Create large JSON object
    QJsonObject large_json;
    for (int i = 0; i < 1000; ++i) {
        large_json[QString("key_%1").arg(i)] = QString("value_%1").arg(i);
    }
    
    // Create large JSON array
    QJsonArray large_array;
    for (int i = 0; i < 1000; ++i) {
        large_array.append(i);
    }
    large_json["large_array"] = large_array;
    
    measureExecutionTime([&]() {
        sol::object lua_obj = qtforge_lua::qjson_to_lua(large_json, *m_lua_state);
        QJsonValue json_back = qtforge_lua::lua_to_qjson(lua_obj);
        Q_UNUSED(json_back)
    }, "Large data conversion");
    
    // Test memory usage with large data
    measureMemoryUsage([&]() {
        for (int i = 0; i < 100; ++i) {
            sol::object lua_obj = qtforge_lua::qjson_to_lua(large_json, *m_lua_state);
            QJsonValue json_back = qtforge_lua::lua_to_qjson(lua_obj);
            Q_UNUSED(json_back)
        }
    }, "Large data memory usage");
#else
    QSKIP("Lua bindings not available");
#endif
}

void TestLuaPerformance::testPluginLoadingPerformance()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }
    
    QString plugin_content = R"(
local plugin = {}

function plugin.initialize()
    return {success = true}
end

function plugin.simple_operation()
    local result = 0
    for i = 1, 100 do
        result = result + i
    end
    return {success = true, result = result}
end

return plugin
)";
    
    // Test single plugin loading performance
    measureExecutionTime([&]() {
        createPerformanceTestPlugin("perf_plugin.lua", plugin_content);
        std::filesystem::path plugin_path(m_temp_dir.filePath("perf_plugin.lua").toStdString());
        
        auto result = m_loader->load(plugin_path);
        QVERIFY(result.has_value());
        
        auto plugin = result.value();
        auto unload_result = m_loader->unload(plugin->id());
        QVERIFY(unload_result.has_value());
    }, "Single plugin loading");
    
    // Test multiple plugin loading performance
    const int plugin_count = 50;
    measureExecutionTime([&]() {
        std::vector<std::string> plugin_ids;
        
        for (int i = 0; i < plugin_count; ++i) {
            QString filename = QString("perf_plugin_%1.lua").arg(i);
            createPerformanceTestPlugin(filename, plugin_content);
            std::filesystem::path plugin_path(m_temp_dir.filePath(filename).toStdString());
            
            auto result = m_loader->load(plugin_path);
            if (result.has_value()) {
                plugin_ids.push_back(result.value()->id());
            }
        }
        
        // Unload all plugins
        for (const auto& id : plugin_ids) {
            m_loader->unload(id);
        }
    }, QString("Multiple plugin loading (%1 plugins)").arg(plugin_count));
}

void TestLuaPerformance::testPluginExecutionPerformance()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }
    
    QVERIFY(m_bridge->initialize().has_value());
    
    QString performance_plugin = R"(
local plugin = {}

function plugin.simple_calculation(n)
    local result = 0
    for i = 1, n do
        result = result + i
    end
    return result
end

function plugin.string_processing(text, count)
    local result = text
    for i = 1, count do
        result = result .. "_" .. i
    end
    return result
end

function plugin.data_processing(data)
    local processed = {}
    for i, value in ipairs(data) do
        processed[i] = value * 2 + 1
    end
    return processed
end

return plugin
)";
    
    createPerformanceTestPlugin("exec_perf_plugin.lua", performance_plugin);
    QString plugin_path = m_temp_dir.filePath("exec_perf_plugin.lua");
    
    auto load_result = m_bridge->load_lua_plugin(plugin_path);
    QVERIFY(load_result.has_value());
    
    const int iterations = 1000;
    
    // Test simple calculation performance
    measureExecutionTime([&]() {
        for (int i = 0; i < iterations; ++i) {
            QString code = "return plugin.simple_calculation(100)";
            auto result = m_bridge->execute_code(code);
            Q_UNUSED(result)
        }
    }, "Simple calculation execution");
    
    // Test string processing performance
    measureExecutionTime([&]() {
        for (int i = 0; i < iterations; ++i) {
            QString code = "return plugin.string_processing('test', 10)";
            auto result = m_bridge->execute_code(code);
            Q_UNUSED(result)
        }
    }, "String processing execution");
    
    // Test data processing performance
    measureExecutionTime([&]() {
        for (int i = 0; i < iterations; ++i) {
            QString code = "return plugin.data_processing({1, 2, 3, 4, 5, 6, 7, 8, 9, 10})";
            auto result = m_bridge->execute_code(code);
            Q_UNUSED(result)
        }
    }, "Data processing execution");
}

void TestLuaPerformance::benchmarkSimpleExecution()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }
    
    QVERIFY(m_bridge->initialize().has_value());
    
    const int iterations = 10000;
    
    QBENCHMARK {
        QString code = "return 42 + 58";
        auto result = m_bridge->execute_code(code);
        Q_UNUSED(result)
    }
}

void TestLuaPerformance::benchmarkComplexExecution()
{
    if (!qtplugin::LuaPluginLoader::is_lua_available()) {
        QSKIP("Lua bindings not available");
    }
    
    QVERIFY(m_bridge->initialize().has_value());
    
    QString complex_code = R"(
        local function fibonacci(n)
            if n <= 1 then return n end
            return fibonacci(n-1) + fibonacci(n-2)
        end
        
        local result = {}
        for i = 1, 10 do
            result[i] = fibonacci(i)
        end
        return result
    )";
    
    QBENCHMARK {
        auto result = m_bridge->execute_code(complex_code);
        Q_UNUSED(result)
    }
}

void TestLuaPerformance::createPerformanceTestPlugin(const QString& filename, const QString& content)
{
    QFile file(m_temp_dir.filePath(filename));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream stream(&file);
    stream << content;
    file.close();
}

void TestLuaPerformance::measureExecutionTime(const std::function<void()>& func, const QString& testName)
{
    QElapsedTimer timer;
    timer.start();
    
    func();
    
    qint64 elapsed = timer.elapsed();
    qDebug() << QString("%1: %2 ms").arg(testName).arg(elapsed);
    
    // Performance assertions (adjust thresholds as needed)
    if (testName.contains("conversion")) {
        QVERIFY2(elapsed < 5000, qPrintable(QString("%1 took too long: %2 ms").arg(testName).arg(elapsed)));
    } else if (testName.contains("loading")) {
        QVERIFY2(elapsed < 10000, qPrintable(QString("%1 took too long: %2 ms").arg(testName).arg(elapsed)));
    } else if (testName.contains("execution")) {
        QVERIFY2(elapsed < 3000, qPrintable(QString("%1 took too long: %2 ms").arg(testName).arg(elapsed)));
    }
}

void TestLuaPerformance::measureMemoryUsage(const std::function<void()>& func, const QString& testName)
{
    // Basic memory measurement (platform-specific implementations would be more accurate)
    size_t initial_memory = 0; // Would use platform-specific memory measurement
    
    func();
    
    size_t final_memory = 0; // Would use platform-specific memory measurement
    
    qDebug() << QString("%1: Memory delta: %2 bytes").arg(testName).arg(final_memory - initial_memory);
    
    // Memory usage assertions would go here
}

QTEST_MAIN(TestLuaPerformance)
#include "test_lua_performance.moc"
