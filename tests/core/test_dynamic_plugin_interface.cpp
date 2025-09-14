/**
 * @file test_dynamic_plugin_interface.cpp
 * @brief Tests for dynamic plugin interface implementation
 */

#include <QtTest/QtTest>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <memory>

#include <qtplugin/core/dynamic_plugin_interface.hpp>
#include <qtplugin/utils/error_handling.hpp>

#include "../test_helpers.hpp"
#include "../test_config_templates.hpp"

using namespace qtplugin;
using namespace QtForgeTest;

/**
 * @brief Mock implementation of IDynamicPlugin for testing
 */
class MockDynamicPlugin : public QObject, public IDynamicPlugin {
    Q_OBJECT

public:
    explicit MockDynamicPlugin(QObject* parent = nullptr)
        : QObject(parent), m_initialized(false) {
        setupMockInterfaces();
    }

    // IPlugin interface implementation
    std::string id() const override { return "mock_dynamic_plugin"; }
    std::string name() const override { return "Mock Dynamic Plugin"; }
    std::string version() const override { return "1.0.0"; }
    std::string description() const override { return "Mock plugin for testing dynamic interfaces"; }

    PluginMetadata metadata() const override {
        PluginMetadata meta;
        meta.id = QString::fromStdString(id());
        meta.name = QString::fromStdString(name());
        meta.version = Version(1, 0, 0);
        meta.description = QString::fromStdString(description());
        return meta;
    }

    qtplugin::expected<void, PluginError> initialize() override {
        m_initialized = true;
        return make_success();
    }

    void shutdown() noexcept override {
        m_initialized = false;
    }

    bool is_initialized() const override { return m_initialized; }

    qtplugin::expected<void, PluginError> configure(const QJsonObject& config) override {
        Q_UNUSED(config)
        return make_success();
    }

    qtplugin::expected<QJsonObject, PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override {
        
        QString cmd = QString::fromUtf8(command.data(), command.size());
        
        QJsonObject result;
        result["command"] = cmd;
        result["status"] = "success";
        result["params"] = params;
        
        if (cmd == "fail") {
            return make_error<QJsonObject>(PluginErrorCode::ExecutionFailed, "Simulated failure");
        }
        
        return result;
    }

    std::vector<std::string> available_commands() const override {
        return {"test", "status", "adapt", "negotiate"};
    }

    // IDynamicPlugin interface implementation
    std::vector<InterfaceDescriptor> get_interface_descriptors() const override {
        return m_interface_descriptors;
    }

    bool supports_interface(const QString& interface_id,
                           const Version& min_version = Version{}) const override {
        auto it = std::find_if(m_interface_descriptors.begin(), m_interface_descriptors.end(),
                              [&interface_id, &min_version](const InterfaceDescriptor& desc) {
                                  return desc.interface_id == interface_id && desc.version >= min_version;
                              });
        return it != m_interface_descriptors.end();
    }

    std::optional<InterfaceDescriptor> get_interface_descriptor(
        const QString& interface_id) const override {
        auto it = std::find_if(m_interface_descriptors.begin(), m_interface_descriptors.end(),
                              [&interface_id](const InterfaceDescriptor& desc) {
                                  return desc.interface_id == interface_id;
                              });
        
        if (it != m_interface_descriptors.end()) {
            return *it;
        }
        return std::nullopt;
    }

    qtplugin::expected<void, PluginError> adapt_to_interface(
        const QString& interface_id, const Version& target_version) override {
        
        if (!supports_interface(interface_id, target_version)) {
            return make_error<void>(PluginErrorCode::InterfaceNotSupported,
                                  "Interface not supported or version too old");
        }
        
        // Mock adaptation logic
        m_adapted_interfaces[interface_id] = target_version;
        return make_success();
    }

    std::vector<InterfaceCapability> negotiate_capabilities(
        const std::vector<InterfaceCapability>& requested) const override {
        
        std::vector<InterfaceCapability> negotiated;
        
        for (const auto& requested_cap : requested) {
            // Check if we support this capability
            for (const auto& desc : m_interface_descriptors) {
                auto it = std::find_if(desc.capabilities.begin(), desc.capabilities.end(),
                                      [&requested_cap](const InterfaceCapability& cap) {
                                          return cap.name == requested_cap.name;
                                      });
                
                if (it != desc.capabilities.end()) {
                    negotiated.push_back(*it);
                    break;
                }
            }
        }
        
        return negotiated;
    }

    // Test helper methods
    bool isAdaptedTo(const QString& interface_id) const {
        return m_adapted_interfaces.contains(interface_id);
    }

    Version getAdaptedVersion(const QString& interface_id) const {
        return m_adapted_interfaces.value(interface_id, Version{});
    }

private:
    void setupMockInterfaces() {
        // Create mock interface descriptors
        InterfaceDescriptor desc1;
        desc1.interface_id = "IDataProcessor";
        desc1.version = Version(1, 2, 0);
        desc1.description = "Data processing interface";
        
        InterfaceCapability cap1;
        cap1.name = "batch_processing";
        cap1.version = Version(1, 0, 0);
        cap1.required = true;
        desc1.capabilities.push_back(cap1);
        
        InterfaceCapability cap2;
        cap2.name = "stream_processing";
        cap2.version = Version(1, 1, 0);
        cap2.required = false;
        desc1.capabilities.push_back(cap2);
        
        m_interface_descriptors.push_back(desc1);
        
        // Second interface
        InterfaceDescriptor desc2;
        desc2.interface_id = "IValidator";
        desc2.version = Version(2, 0, 0);
        desc2.description = "Data validation interface";
        
        InterfaceCapability cap3;
        cap3.name = "schema_validation";
        cap3.version = Version(2, 0, 0);
        cap3.required = true;
        desc2.capabilities.push_back(cap3);
        
        m_interface_descriptors.push_back(desc2);
    }

    bool m_initialized;
    std::vector<InterfaceDescriptor> m_interface_descriptors;
    QHash<QString, Version> m_adapted_interfaces;
};

/**
 * @brief Test class for dynamic plugin interface
 */
class TestDynamicPluginInterface : public TestFixtureBase {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // InterfaceCapability tests
    void testInterfaceCapabilityCreation();
    void testInterfaceCapabilityJsonSerialization();
    void testInterfaceCapabilityJsonDeserialization();

    // InterfaceDescriptor tests
    void testInterfaceDescriptorCreation();
    void testInterfaceDescriptorCompatibility();
    void testInterfaceDescriptorJsonSerialization();

    // IDynamicPlugin tests
    void testDynamicPluginCreation();
    void testInterfaceSupport();
    void testInterfaceAdaptation();
    void testCapabilityNegotiation();

    // InterfaceRegistry tests
    void testInterfaceRegistration();
    void testInterfaceDiscovery();
    void testInterfaceVersioning();

    // Error handling tests
    void testInvalidInterfaceHandling();
    void testAdaptationErrors();

private:
    std::unique_ptr<MockDynamicPlugin> m_plugin;
};

void TestDynamicPluginInterface::initTestCase() {
    TestFixtureBase::initTestCase();
}

void TestDynamicPluginInterface::cleanupTestCase() {
    TestFixtureBase::cleanupTestCase();
}

void TestDynamicPluginInterface::init() {
    TestFixtureBase::init();
    m_plugin = std::make_unique<MockDynamicPlugin>();
}

void TestDynamicPluginInterface::cleanup() {
    if (m_plugin) {
        m_plugin->shutdown();
        m_plugin.reset();
    }
    TestFixtureBase::cleanup();
}

void TestDynamicPluginInterface::testInterfaceCapabilityCreation() {
    InterfaceCapability capability;
    capability.name = "test_capability";
    capability.version = Version(1, 2, 3);
    capability.required = true;
    
    QJsonObject metadata;
    metadata["description"] = "Test capability";
    capability.metadata = metadata;
    
    QCOMPARE(capability.name, QString("test_capability"));
    QVERIFY(capability.version == Version(1, 2, 3));
    QVERIFY(capability.required);
    QVERIFY(capability.metadata.contains("description"));
}

void TestDynamicPluginInterface::testInterfaceCapabilityJsonSerialization() {
    InterfaceCapability capability;
    capability.name = "serialization_test";
    capability.version = Version(2, 1, 0);
    capability.required = false;
    
    QJsonObject metadata;
    metadata["type"] = "processing";
    capability.metadata = metadata;
    
    QJsonObject json = capability.to_json();
    
    QCOMPARE(json["name"].toString(), QString("serialization_test"));
    QCOMPARE(json["version"].toString(), QString("2.1.0"));
    QCOMPARE(json["required"].toBool(), false);
    QVERIFY(json["metadata"].isObject());
    QCOMPARE(json["metadata"].toObject()["type"].toString(), QString("processing"));
}

void TestDynamicPluginInterface::testInterfaceCapabilityJsonDeserialization() {
    QJsonObject json;
    json["name"] = "deserialization_test";
    json["version"] = "1.5.2";
    json["required"] = true;
    
    QJsonObject metadata;
    metadata["category"] = "validation";
    json["metadata"] = metadata;
    
    auto result = InterfaceCapability::from_json(json);
    QTFORGE_VERIFY_SUCCESS(result);
    
    if (result.has_value()) {
        const auto& capability = result.value();
        QCOMPARE(capability.name, QString("deserialization_test"));
        QVERIFY(capability.version == Version(1, 5, 2));
        QVERIFY(capability.required);
        QCOMPARE(capability.metadata["category"].toString(), QString("validation"));
    }
}

void TestDynamicPluginInterface::testInterfaceDescriptorCreation() {
    InterfaceDescriptor descriptor;
    descriptor.interface_id = "ITestInterface";
    descriptor.version = Version(3, 0, 0);
    descriptor.description = "Test interface descriptor";
    
    InterfaceCapability cap;
    cap.name = "test_capability";
    cap.version = Version(1, 0, 0);
    descriptor.capabilities.push_back(cap);
    
    QCOMPARE(descriptor.interface_id, QString("ITestInterface"));
    QVERIFY(descriptor.version == Version(3, 0, 0));
    QCOMPARE(descriptor.capabilities.size(), 1);
}

void TestDynamicPluginInterface::testInterfaceDescriptorCompatibility() {
    InterfaceDescriptor desc1;
    desc1.interface_id = "ICompatibilityTest";
    desc1.version = Version(2, 1, 0);
    
    InterfaceCapability required_cap;
    required_cap.name = "required_feature";
    required_cap.version = Version(1, 0, 0);
    required_cap.required = true;
    desc1.capabilities.push_back(required_cap);
    
    InterfaceDescriptor desc2;
    desc2.interface_id = "ICompatibilityTest";
    desc2.version = Version(2, 0, 0);
    
    InterfaceCapability provided_cap;
    provided_cap.name = "required_feature";
    provided_cap.version = Version(1, 1, 0);
    provided_cap.required = false;
    desc2.capabilities.push_back(provided_cap);
    
    // desc2 should be compatible with desc1 (provides required capability)
    QVERIFY(desc2.is_compatible_with(desc1));
    
    // Test incompatible version
    InterfaceDescriptor desc3;
    desc3.interface_id = "ICompatibilityTest";
    desc3.version = Version(3, 0, 0); // Major version difference
    QVERIFY(!desc3.is_compatible_with(desc1));
}

void TestDynamicPluginInterface::testInterfaceDescriptorJsonSerialization() {
    InterfaceDescriptor descriptor;
    descriptor.interface_id = "IJsonTest";
    descriptor.version = Version(1, 0, 0);
    descriptor.description = "JSON serialization test";
    
    InterfaceCapability cap;
    cap.name = "json_capability";
    cap.version = Version(1, 0, 0);
    descriptor.capabilities.push_back(cap);
    
    QJsonObject json = descriptor.to_json();
    
    QCOMPARE(json["interface_id"].toString(), QString("IJsonTest"));
    QCOMPARE(json["version"].toString(), QString("1.0.0"));
    QCOMPARE(json["description"].toString(), QString("JSON serialization test"));
    QVERIFY(json["capabilities"].isArray());
    
    auto result = InterfaceDescriptor::from_json(json);
    QTFORGE_VERIFY_SUCCESS(result);
    
    if (result.has_value()) {
        const auto& deserialized = result.value();
        QCOMPARE(deserialized.interface_id, descriptor.interface_id);
        QVERIFY(deserialized.version == descriptor.version);
        QCOMPARE(deserialized.capabilities.size(), 1);
    }
}

void TestDynamicPluginInterface::testDynamicPluginCreation() {
    QVERIFY(m_plugin != nullptr);
    QCOMPARE(m_plugin->id(), std::string("mock_dynamic_plugin"));
    QCOMPARE(m_plugin->name(), std::string("Mock Dynamic Plugin"));
    
    auto descriptors = m_plugin->get_interface_descriptors();
    QVERIFY(!descriptors.empty());
    QCOMPARE(descriptors.size(), 2);
}

void TestDynamicPluginInterface::testInterfaceSupport() {
    // Test supported interface
    QVERIFY(m_plugin->supports_interface("IDataProcessor"));
    QVERIFY(m_plugin->supports_interface("IValidator"));
    
    // Test unsupported interface
    QVERIFY(!m_plugin->supports_interface("IUnsupportedInterface"));
    
    // Test version requirements
    QVERIFY(m_plugin->supports_interface("IDataProcessor", Version(1, 0, 0)));
    QVERIFY(!m_plugin->supports_interface("IDataProcessor", Version(2, 0, 0)));
    
    // Test getting interface descriptor
    auto descriptor = m_plugin->get_interface_descriptor("IDataProcessor");
    QVERIFY(descriptor.has_value());
    QCOMPARE(descriptor->interface_id, QString("IDataProcessor"));
    
    auto invalid_descriptor = m_plugin->get_interface_descriptor("IInvalid");
    QVERIFY(!invalid_descriptor.has_value());
}

void TestDynamicPluginInterface::testInterfaceAdaptation() {
    // Test successful adaptation
    auto result = m_plugin->adapt_to_interface("IDataProcessor", Version(1, 2, 0));
    QTFORGE_VERIFY_SUCCESS(result);
    QVERIFY(m_plugin->isAdaptedTo("IDataProcessor"));
    
    // Test adaptation to unsupported interface
    auto invalid_result = m_plugin->adapt_to_interface("IUnsupported", Version(1, 0, 0));
    QTFORGE_VERIFY_ERROR(invalid_result, PluginErrorCode::InterfaceNotSupported);
    
    // Test adaptation to version that's too new
    auto version_result = m_plugin->adapt_to_interface("IDataProcessor", Version(3, 0, 0));
    QTFORGE_VERIFY_ERROR(version_result, PluginErrorCode::InterfaceNotSupported);
}

void TestDynamicPluginInterface::testCapabilityNegotiation() {
    std::vector<InterfaceCapability> requested;
    
    InterfaceCapability req1;
    req1.name = "batch_processing";
    req1.version = Version(1, 0, 0);
    requested.push_back(req1);
    
    InterfaceCapability req2;
    req2.name = "unsupported_capability";
    req2.version = Version(1, 0, 0);
    requested.push_back(req2);
    
    auto negotiated = m_plugin->negotiate_capabilities(requested);
    
    // Should only get the supported capability
    QCOMPARE(negotiated.size(), 1);
    QCOMPARE(negotiated[0].name, QString("batch_processing"));
}

void TestDynamicPluginInterface::testInterfaceRegistration() {
    auto& registry = InterfaceRegistry::instance();
    
    InterfaceDescriptor descriptor;
    descriptor.interface_id = "ITestRegistration";
    descriptor.version = Version(1, 0, 0);
    descriptor.description = "Test registration";
    
    auto result = registry.register_interface(descriptor);
    QTFORGE_VERIFY_SUCCESS(result);
    
    // Test duplicate registration
    auto duplicate_result = registry.register_interface(descriptor);
    QTFORGE_VERIFY_ERROR(duplicate_result, PluginErrorCode::AlreadyExists);
    
    // Test newer version registration
    descriptor.version = Version(1, 1, 0);
    auto newer_result = registry.register_interface(descriptor);
    QTFORGE_VERIFY_SUCCESS(newer_result);
}

void TestDynamicPluginInterface::testInterfaceDiscovery() {
    auto& registry = InterfaceRegistry::instance();
    
    InterfaceDescriptor descriptor;
    descriptor.interface_id = "ITestDiscovery";
    descriptor.version = Version(2, 0, 0);
    
    auto register_result = registry.register_interface(descriptor);
    QTFORGE_VERIFY_SUCCESS(register_result);
    
    auto found_descriptor = registry.get_interface("ITestDiscovery");
    QVERIFY(found_descriptor.has_value());
    QCOMPARE(found_descriptor->interface_id, QString("ITestDiscovery"));
    
    auto not_found = registry.get_interface("INotFound");
    QVERIFY(!not_found.has_value());
    
    auto all_interfaces = registry.get_all_interfaces();
    QVERIFY(!all_interfaces.empty());
}

void TestDynamicPluginInterface::testInterfaceVersioning() {
    auto& registry = InterfaceRegistry::instance();
    
    InterfaceDescriptor v1;
    v1.interface_id = "IVersionTest";
    v1.version = Version(1, 0, 0);
    
    InterfaceDescriptor v2;
    v2.interface_id = "IVersionTest";
    v2.version = Version(2, 0, 0);
    
    auto result1 = registry.register_interface(v1);
    QTFORGE_VERIFY_SUCCESS(result1);
    
    auto result2 = registry.register_interface(v2);
    QTFORGE_VERIFY_SUCCESS(result2);
    
    // Should get the latest version
    auto found = registry.get_interface("IVersionTest");
    QVERIFY(found.has_value());
    QVERIFY(found->version == Version(2, 0, 0));
}

void TestDynamicPluginInterface::testInvalidInterfaceHandling() {
    // Test invalid JSON deserialization
    QJsonObject invalid_json;
    invalid_json["invalid_field"] = "value";
    
    auto capability_result = InterfaceCapability::from_json(invalid_json);
    QVERIFY(!capability_result.has_value());
    
    auto descriptor_result = InterfaceDescriptor::from_json(invalid_json);
    QVERIFY(!descriptor_result.has_value());
}

void TestDynamicPluginInterface::testAdaptationErrors() {
    // Test adaptation to non-existent interface
    auto result = m_plugin->adapt_to_interface("INonExistent", Version(1, 0, 0));
    QTFORGE_VERIFY_ERROR(result, PluginErrorCode::InterfaceNotSupported);
    
    // Test adaptation with incompatible version
    auto version_result = m_plugin->adapt_to_interface("IDataProcessor", Version(10, 0, 0));
    QTFORGE_VERIFY_ERROR(version_result, PluginErrorCode::InterfaceNotSupported);
}

QTEST_MAIN(TestDynamicPluginInterface)
#include "test_dynamic_plugin_interface.moc"
