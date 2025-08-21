/**
 * @file test_service_contracts.cpp
 * @brief Tests for plugin service contracts system
 * @version 3.1.0
 */

#include <QtTest/QtTest>
#include <QJsonObject>
#include <QJsonArray>
#include "qtplugin/communication/plugin_service_contracts.hpp"

using namespace qtplugin::contracts;

class TestServiceContracts : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    
    // ServiceVersion tests
    void testServiceVersionCompatibility();
    void testServiceVersionString();
    
    // ServiceContract tests
    void testServiceContractCreation();
    void testServiceContractValidation();
    void testServiceContractSerialization();
    void testServiceContractMethodValidation();
    
    // ServiceContractRegistry tests
    void testRegistryRegistration();
    void testRegistryRetrieval();
    void testRegistryCapabilitySearch();
    void testRegistryDependencyValidation();
    void testRegistryProviderDiscovery();
    
    // Error handling tests
    void testInvalidContractValidation();
    void testDuplicateRegistration();
    void testMissingDependencies();
    
private:
    ServiceContract createTestContract();
    ServiceContract createDependentContract();
};

void TestServiceContracts::initTestCase() {
    qDebug() << "Starting service contracts tests";
}

void TestServiceContracts::cleanupTestCase() {
    qDebug() << "Service contracts tests completed";
}

void TestServiceContracts::testServiceVersionCompatibility() {
    ServiceVersion v1_0_0(1, 0, 0);
    ServiceVersion v1_1_0(1, 1, 0);
    ServiceVersion v1_2_0(1, 2, 0);
    ServiceVersion v2_0_0(2, 0, 0);
    
    // Same major version, higher minor should be compatible
    QVERIFY(v1_1_0.is_compatible_with(v1_0_0));
    QVERIFY(v1_2_0.is_compatible_with(v1_0_0));
    QVERIFY(v1_2_0.is_compatible_with(v1_1_0));
    
    // Lower minor version should not be compatible
    QVERIFY(!v1_0_0.is_compatible_with(v1_1_0));
    
    // Different major version should not be compatible
    QVERIFY(!v2_0_0.is_compatible_with(v1_0_0));
    QVERIFY(!v1_0_0.is_compatible_with(v2_0_0));
}

void TestServiceContracts::testServiceVersionString() {
    ServiceVersion version(1, 2, 3);
    QCOMPARE(version.to_string(), std::string("1.2.3"));
}

void TestServiceContracts::testServiceContractCreation() {
    ServiceContract contract("com.example.testservice", ServiceVersion(1, 0, 0));
    
    QCOMPARE(contract.service_name(), QString("com.example.testservice"));
    QCOMPARE(contract.version().major, 1u);
    QCOMPARE(contract.version().minor, 0u);
    QCOMPARE(contract.version().patch, 0u);
    
    // Test method chaining
    contract.set_description("Test service")
           .set_provider("test_plugin")
           .set_capabilities(static_cast<uint32_t>(ServiceCapability::Synchronous | ServiceCapability::ThreadSafe));
    
    QCOMPARE(contract.description(), QString("Test service"));
    QCOMPARE(contract.provider(), QString("test_plugin"));
    QVERIFY(contract.capabilities() & static_cast<uint32_t>(ServiceCapability::Synchronous));
    QVERIFY(contract.capabilities() & static_cast<uint32_t>(ServiceCapability::ThreadSafe));
}

void TestServiceContracts::testServiceContractValidation() {
    // Valid contract
    ServiceContract valid_contract = createTestContract();
    auto validation_result = valid_contract.validate();
    QVERIFY(validation_result.has_value());
    
    // Invalid contract - empty name
    ServiceContract invalid_contract("", ServiceVersion(1, 0, 0));
    auto invalid_result = invalid_contract.validate();
    QVERIFY(!invalid_result.has_value());
    QCOMPARE(invalid_result.error().code, qtplugin::PluginErrorCode::InvalidConfiguration);
    
    // Invalid contract - no methods
    ServiceContract no_methods_contract("com.example.empty");
    auto no_methods_result = no_methods_contract.validate();
    QVERIFY(!no_methods_result.has_value());
}

void TestServiceContracts::testServiceContractSerialization() {
    ServiceContract original = createTestContract();
    
    // Serialize to JSON
    QJsonObject json = original.to_json();
    QVERIFY(json.contains("service_name"));
    QVERIFY(json.contains("version"));
    QVERIFY(json.contains("methods"));
    
    // Deserialize from JSON
    auto deserialized_result = ServiceContract::from_json(json);
    QVERIFY(deserialized_result.has_value());
    
    const ServiceContract& deserialized = deserialized_result.value();
    QCOMPARE(deserialized.service_name(), original.service_name());
    QCOMPARE(deserialized.version().major, original.version().major);
    QCOMPARE(deserialized.methods().size(), original.methods().size());
}

void TestServiceContracts::testServiceContractMethodValidation() {
    ServiceContract contract = createTestContract();
    
    // Valid method call
    QJsonObject valid_params;
    valid_params["message"] = "Hello World";
    valid_params["count"] = 5;
    
    auto valid_result = contract.validate_method_call("send_message", valid_params);
    QVERIFY(valid_result.has_value());
    
    // Invalid method call - missing required parameter
    QJsonObject missing_param;
    missing_param["count"] = 5;
    
    auto missing_result = contract.validate_method_call("send_message", missing_param);
    QVERIFY(!missing_result.has_value());
    QCOMPARE(missing_result.error().code, qtplugin::PluginErrorCode::InvalidParameters);
    
    // Invalid method call - wrong parameter type
    QJsonObject wrong_type;
    wrong_type["message"] = 123; // Should be string
    wrong_type["count"] = 5;
    
    auto wrong_type_result = contract.validate_method_call("send_message", wrong_type);
    QVERIFY(!wrong_type_result.has_value());
    
    // Invalid method call - unknown method
    auto unknown_method_result = contract.validate_method_call("unknown_method", valid_params);
    QVERIFY(!unknown_method_result.has_value());
    QCOMPARE(unknown_method_result.error().code, qtplugin::PluginErrorCode::CommandNotFound);
}

void TestServiceContracts::testRegistryRegistration() {
    ServiceContractRegistry& registry = ServiceContractRegistry::instance();
    ServiceContract contract = createTestContract();
    
    // Register contract
    auto register_result = registry.register_contract("test_plugin", contract);
    QVERIFY(register_result.has_value());
    
    // Verify registration
    auto get_result = registry.get_contract("com.example.testservice");
    QVERIFY(get_result.has_value());
    QCOMPARE(get_result.value().service_name(), contract.service_name());
    
    // Cleanup
    registry.unregister_contract("test_plugin", "com.example.testservice");
}

void TestServiceContracts::testRegistryRetrieval() {
    ServiceContractRegistry& registry = ServiceContractRegistry::instance();
    ServiceContract contract = createTestContract();
    
    registry.register_contract("test_plugin", contract);
    
    // Test retrieval with version compatibility
    auto get_result = registry.get_contract("com.example.testservice", ServiceVersion(1, 0, 0));
    QVERIFY(get_result.has_value());
    
    // Test retrieval with incompatible version
    auto incompatible_result = registry.get_contract("com.example.testservice", ServiceVersion(2, 0, 0));
    QVERIFY(!incompatible_result.has_value());
    QCOMPARE(incompatible_result.error().code, qtplugin::PluginErrorCode::IncompatibleVersion);
    
    // Cleanup
    registry.unregister_contract("test_plugin", "com.example.testservice");
}

void TestServiceContracts::testRegistryCapabilitySearch() {
    ServiceContractRegistry& registry = ServiceContractRegistry::instance();
    ServiceContract contract = createTestContract();
    
    registry.register_contract("test_plugin", contract);
    
    // Search by capability
    auto contracts = registry.find_contracts_by_capability(ServiceCapability::Synchronous);
    QVERIFY(!contracts.empty());
    
    bool found = false;
    for (const auto& found_contract : contracts) {
        if (found_contract.service_name() == contract.service_name()) {
            found = true;
            break;
        }
    }
    QVERIFY(found);
    
    // Cleanup
    registry.unregister_contract("test_plugin", "com.example.testservice");
}

void TestServiceContracts::testInvalidContractValidation() {
    ServiceContractRegistry& registry = ServiceContractRegistry::instance();
    
    // Try to register invalid contract
    ServiceContract invalid_contract("", ServiceVersion(1, 0, 0));
    auto register_result = registry.register_contract("test_plugin", invalid_contract);
    QVERIFY(!register_result.has_value());
    QCOMPARE(register_result.error().code, qtplugin::PluginErrorCode::InvalidConfiguration);
}

ServiceContract TestServiceContracts::createTestContract() {
    ServiceContract contract("com.example.testservice", ServiceVersion(1, 0, 0));
    
    contract.set_description("Test service for unit tests")
           .set_provider("test_plugin")
           .set_capabilities(static_cast<uint32_t>(ServiceCapability::Synchronous | ServiceCapability::ThreadSafe));
    
    // Add a method
    ServiceMethod method("send_message", "Send a message");
    method.add_parameter(ServiceParameter("message", "string", "Message to send", true))
          .add_parameter(ServiceParameter("count", "number", "Number of times to send", false))
          .set_return_type(ServiceParameter("result", "object", "Operation result"));
    
    contract.add_method(method);
    
    return contract;
}

ServiceContract TestServiceContracts::createDependentContract() {
    ServiceContract contract("com.example.dependent", ServiceVersion(1, 0, 0));
    
    contract.set_description("Dependent service")
           .add_dependency("com.example.testservice", ServiceVersion(1, 0, 0));
    
    ServiceMethod method("process", "Process data");
    method.add_parameter(ServiceParameter("data", "object", "Data to process", true))
          .set_return_type(ServiceParameter("result", "object", "Processed result"));
    
    contract.add_method(method);
    
    return contract;
}

QTEST_MAIN(TestServiceContracts)
#include "test_service_contracts.moc"
