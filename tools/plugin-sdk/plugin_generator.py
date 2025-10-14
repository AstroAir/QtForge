#!/usr/bin/env python3
"""QtForge Plugin Generator
A comprehensive tool for generating plugin projects with templates and scaffolding.
"""

from __future__ import annotations

import argparse
import json
import sys
from datetime import datetime
from pathlib import Path
from typing import Any


class PluginTemplate:
    """Base class for plugin templates"""

    def __init__(self, name: str, description: str) -> None:
        self.name = name
        self.description = description
        self.variables: dict[str, Any] = {}

    def set_variables(self, variables: dict[str, Any]) -> None:
        """Set template variables"""
        self.variables.update(variables)

    def render_template(self, template_content: str) -> str:
        """Render template with variables"""
        for key, value in self.variables.items():
            placeholder = f"{{{{{key}}}}}"
            template_content = template_content.replace(placeholder, str(value))
        return template_content

    def generate_files(self, output_dir: Path) -> list[Path]:
        """Generate files for this template"""
        raise NotImplementedError


class NativePluginTemplate(PluginTemplate):
    """Template for native C++ plugins"""

    def __init__(self) -> None:
        super().__init__("native", "Native C++ plugin with full QtForge integration")

    def generate_files(self, output_dir: Path) -> list[Path]:
        """Generate native plugin files"""
        generated_files = []

        # Create directory structure
        src_dir = output_dir / "src"
        include_dir = output_dir / "include"
        tests_dir = output_dir / "tests"

        for dir_path in [src_dir, include_dir, tests_dir]:
            dir_path.mkdir(parents=True, exist_ok=True)

        # Generate header file
        header_content = self._generate_header()
        header_file = include_dir / f"{self.variables['class_name'].lower()}.hpp"
        header_file.write_text(header_content)
        generated_files.append(header_file)

        # Generate source file
        source_content = self._generate_source()
        source_file = src_dir / f"{self.variables['class_name'].lower()}.cpp"
        source_file.write_text(source_content)
        generated_files.append(source_file)

        # Generate CMakeLists.txt
        cmake_content = self._generate_cmake()
        cmake_file = output_dir / "CMakeLists.txt"
        cmake_file.write_text(cmake_content)
        generated_files.append(cmake_file)

        # Generate metadata.json
        metadata_content = self._generate_metadata()
        metadata_file = output_dir / "metadata.json"
        metadata_file.write_text(metadata_content)
        generated_files.append(metadata_file)

        # Generate test file
        test_content = self._generate_test()
        test_file = tests_dir / f"test_{self.variables['class_name'].lower()}.cpp"
        test_file.write_text(test_content)
        generated_files.append(test_file)

        return generated_files

    def _generate_header(self) -> str:
        """Generate plugin header file"""
        return self.render_template("""/**
 * @file {{class_name_lower}}.hpp
 * @brief {{description}}
 * @version {{version}}
 * @author {{author}}
 */

#pragma once

#include <qtplugin/interfaces/core/dynamic_plugin_interface.hpp>
#include <QObject>
#include <QString>
#include <QJsonObject>
#include <memory>

class {{class_name}} : public QObject, public qtplugin::IDynamicPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtplugin.IDynamicPlugin/{{version}}" FILE "metadata.json")
    Q_INTERFACES(qtplugin::IDynamicPlugin)

public:
    explicit {{class_name}}(QObject* parent = nullptr);
    ~{{class_name}}() override;

    // === IPlugin Implementation ===
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    qtplugin::Version version() const noexcept override;
    std::string_view author() const noexcept override;
    std::string id() const noexcept override;

    qtplugin::expected<void, qtplugin::PluginError> initialize() override;
    void shutdown() noexcept override;
    qtplugin::PluginState state() const noexcept override;

    qtplugin::PluginCapabilities capabilities() const noexcept override;

    qtplugin::expected<void, qtplugin::PluginError> configure(
        const QJsonObject& config) override;
    QJsonObject current_configuration() const override;

    qtplugin::expected<QJsonObject, qtplugin::PluginError> execute_command(
        std::string_view command, const QJsonObject& params = {}) override;
    std::vector<std::string> available_commands() const override;

    // === IDynamicPlugin Implementation ===
    std::vector<qtplugin::InterfaceDescriptor> get_interface_descriptors() const override;
    bool supports_interface(const QString& interface_id,
                           const qtplugin::Version& min_version = qtplugin::Version{}) const override;
    std::optional<qtplugin::InterfaceDescriptor> get_interface_descriptor(
        const QString& interface_id) const override;

    qtplugin::expected<void, qtplugin::PluginError> adapt_to_interface(
        const QString& interface_id, const qtplugin::Version& target_version) override;

    qtplugin::PluginType get_plugin_type() const override;
    qtplugin::PluginExecutionContext get_execution_context() const override;

    qtplugin::expected<QVariant, qtplugin::PluginError> execute_code(
        const QString& code, const QJsonObject& context = {}) override;

    qtplugin::expected<QVariant, qtplugin::PluginError> invoke_method(
        const QString& method_name, const QVariantList& parameters = {},
        const QString& interface_id = {}) override;

private:
    qtplugin::PluginState m_state{qtplugin::PluginState::Unloaded};
    QJsonObject m_configuration;
    std::vector<qtplugin::InterfaceDescriptor> m_interfaces;

    void setup_interfaces();
};
""")

    def _generate_source(self) -> str:
        """Generate plugin source file"""
        return self.render_template("""/**
 * @file {{class_name_lower}}.cpp
 * @brief Implementation of {{class_name}}
 * @version {{version}}
 */

#include "../include/{{class_name_lower}}.hpp"
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY({{class_name_lower}}Log, "{{plugin_id}}")

{{class_name}}::{{class_name}}(QObject* parent)
    : QObject(parent) {
    setup_interfaces();
}

{{class_name}}::~{{class_name}}() {
    if (m_state != qtplugin::PluginState::Unloaded) {
        shutdown();
    }
}

std::string_view {{class_name}}::name() const noexcept {
    return "{{name}}";
}

std::string_view {{class_name}}::description() const noexcept {
    return "{{description}}";
}

qtplugin::Version {{class_name}}::version() const noexcept {
    return qtplugin::Version::from_string("{{version}}").value_or(qtplugin::Version{});
}

std::string_view {{class_name}}::author() const noexcept {
    return "{{author}}";
}

std::string {{class_name}}::id() const noexcept {
    return "{{plugin_id}}";
}

qtplugin::expected<void, qtplugin::PluginError> {{class_name}}::initialize() {
    if (m_state != qtplugin::PluginState::Unloaded) {
        return qtplugin::make_error<void>(qtplugin::PluginErrorCode::InvalidState,
                                         "Plugin already initialized");
    }

    m_state = qtplugin::PluginState::Loading;

    // TODO: Add your initialization code here

    m_state = qtplugin::PluginState::Running;
    qCDebug({{class_name_lower}}Log) << "Plugin initialized successfully";

    return qtplugin::make_success();
}

void {{class_name}}::shutdown() noexcept {
    if (m_state == qtplugin::PluginState::Unloaded) {
        return;
    }

    m_state = qtplugin::PluginState::Stopping;

    // TODO: Add your cleanup code here

    m_state = qtplugin::PluginState::Unloaded;
    qCDebug({{class_name_lower}}Log) << "Plugin shutdown completed";
}

qtplugin::PluginState {{class_name}}::state() const noexcept {
    return m_state;
}

qtplugin::PluginCapabilities {{class_name}}::capabilities() const noexcept {
    return qtplugin::PluginCapability::{{capability}};
}

qtplugin::expected<void, qtplugin::PluginError> {{class_name}}::configure(
    const QJsonObject& config) {

    m_configuration = config;

    // TODO: Process configuration

    return qtplugin::make_success();
}

QJsonObject {{class_name}}::current_configuration() const {
    return m_configuration;
}

qtplugin::expected<QJsonObject, qtplugin::PluginError> {{class_name}}::execute_command(
    std::string_view command, const QJsonObject& params) {

    QString cmd = QString::fromUtf8(command.data(), command.size());

    if (cmd == "status") {
        QJsonObject result;
        result["state"] = static_cast<int>(m_state);
        result["name"] = QString::fromStdString(std::string(name()));
        return result;
    }

    // TODO: Add your commands here

    return qtplugin::make_error<QJsonObject>(qtplugin::PluginErrorCode::CommandNotFound,
                                            "Unknown command: " + std::string(command));
}

std::vector<std::string> {{class_name}}::available_commands() const {
    return {"status"};
}

// === IDynamicPlugin Implementation ===

std::vector<qtplugin::InterfaceDescriptor> {{class_name}}::get_interface_descriptors() const {
    return m_interfaces;
}

bool {{class_name}}::supports_interface(const QString& interface_id,
                                       const qtplugin::Version& min_version) const {
    for (const auto& interface : m_interfaces) {
        if (interface.interface_id == interface_id && interface.version >= min_version) {
            return true;
        }
    }
    return false;
}

std::optional<qtplugin::InterfaceDescriptor> {{class_name}}::get_interface_descriptor(
    const QString& interface_id) const {

    for (const auto& interface : m_interfaces) {
        if (interface.interface_id == interface_id) {
            return interface;
        }
    }
    return std::nullopt;
}

qtplugin::expected<void, qtplugin::PluginError> {{class_name}}::adapt_to_interface(
    const QString& interface_id, const qtplugin::Version& target_version) {

    // TODO: Implement interface adaptation logic
    return qtplugin::make_success();
}

qtplugin::PluginType {{class_name}}::get_plugin_type() const {
    return qtplugin::PluginType::Native;
}

qtplugin::PluginExecutionContext {{class_name}}::get_execution_context() const {
    return qtplugin::PluginTypeUtils::get_default_context(qtplugin::PluginType::Native);
}

qtplugin::expected<QVariant, qtplugin::PluginError> {{class_name}}::execute_code(
    const QString& code, const QJsonObject& context) {

    Q_UNUSED(code)
    Q_UNUSED(context)

    return qtplugin::make_error<QVariant>(qtplugin::PluginErrorCode::NotSupported,
                                         "Code execution not supported for native plugins");
}

qtplugin::expected<QVariant, qtplugin::PluginError> {{class_name}}::invoke_method(
    const QString& method_name, const QVariantList& parameters,
    const QString& interface_id) {

    Q_UNUSED(method_name)
    Q_UNUSED(parameters)
    Q_UNUSED(interface_id)

    // TODO: Implement dynamic method invocation
    return qtplugin::make_error<QVariant>(qtplugin::PluginErrorCode::CommandNotFound,
                                         "Method not found: " + method_name.toStdString());
}

void {{class_name}}::setup_interfaces() {
    // TODO: Define your plugin interfaces here
    qtplugin::InterfaceDescriptor interface;
    interface.interface_id = "{{plugin_id}}.main";
    interface.version = version();
    interface.description = QString::fromStdString(std::string(description()));

    m_interfaces.push_back(interface);
}

#include "{{class_name_lower}}.moc"
""")

    def _generate_cmake(self) -> str:
        """Generate CMakeLists.txt"""
        return self.render_template("""cmake_minimum_required(VERSION 3.21)
project({{project_name}} VERSION {{version}} LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(QtForge REQUIRED COMPONENTS Core)

# Create the plugin
add_library({{target_name}} MODULE
    src/{{class_name_lower}}.cpp
    include/{{class_name_lower}}.hpp
)

# Set plugin properties
set_target_properties({{target_name}} PROPERTIES
    PREFIX ""
    SUFFIX ".qtplugin"
    AUTOMOC ON
    OUTPUT_NAME {{plugin_name}}
)

# Link libraries
target_link_libraries({{target_name}}
    PRIVATE
        Qt6::Core
        QtForge::Core
)

# Include directories
target_include_directories({{target_name}}
    PRIVATE
        include
)

# Install plugin
install(TARGETS {{target_name}}
    LIBRARY DESTINATION plugins
)

# Install metadata
install(FILES metadata.json
    DESTINATION plugins
    RENAME {{plugin_name}}.json
)

# Tests (optional)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)

    add_executable({{target_name}}_test
        tests/test_{{class_name_lower}}.cpp
        src/{{class_name_lower}}.cpp
    )

    target_link_libraries({{target_name}}_test
        PRIVATE
            Qt6::Core
            Qt6::Test
            QtForge::Core
    )

    target_include_directories({{target_name}}_test
        PRIVATE
            include
    )

    add_test(NAME {{target_name}}_test COMMAND {{target_name}}_test)
endif()
""")

    def _generate_metadata(self) -> str:
        """Generate metadata.json"""
        metadata = {
            "id": self.variables["plugin_id"],
            "name": self.variables["name"],
            "version": self.variables["version"],
            "description": self.variables["description"],
            "author": self.variables["author"],
            "license": self.variables.get("license", "MIT"),
            "category": self.variables.get("category", "General"),
            "capabilities": [self.variables.get("capability", "Service")],
            "dependencies": [],
            "interfaces": [
                {
                    "id": f"{self.variables['plugin_id']}.main",
                    "version": self.variables["version"],
                    "description": self.variables["description"],
                }
            ],
            "generated": {
                "timestamp": datetime.now().isoformat(),
                "generator": "QtForge Plugin Generator v1.0",
                "template": "native",
            },
        }
        return json.dumps(metadata, indent=2)

    def _generate_test(self) -> str:
        """Generate test file"""
        return self.render_template("""/**
 * @file test_{{class_name_lower}}.cpp
 * @brief Unit tests for {{class_name}}
 */

#include <QtTest/QtTest>
#include <QObject>
#include "../include/{{class_name_lower}}.hpp"

class Test{{class_name}} : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testInitialization();
    void testCommands();
    void testConfiguration();

private:
    std::unique_ptr<{{class_name}}> m_plugin;
};

void Test{{class_name}}::initTestCase() {
    m_plugin = std::make_unique<{{class_name}}>();
}

void Test{{class_name}}::cleanupTestCase() {
    m_plugin.reset();
}

void Test{{class_name}}::testInitialization() {
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);

    auto result = m_plugin->initialize();
    QVERIFY(result.has_value());
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Running);

    m_plugin->shutdown();
    QCOMPARE(m_plugin->state(), qtplugin::PluginState::Unloaded);
}

void Test{{class_name}}::testCommands() {
    m_plugin->initialize();

    auto commands = m_plugin->available_commands();
    QVERIFY(!commands.empty());
    QVERIFY(std::find(commands.begin(), commands.end(), "status") != commands.end());

    auto result = m_plugin->execute_command("status");
    QVERIFY(result.has_value());

    auto status = result.value();
    QVERIFY(status.contains("state"));
    QVERIFY(status.contains("name"));
}

void Test{{class_name}}::testConfiguration() {
    QJsonObject config;
    config["test_setting"] = "test_value";

    auto result = m_plugin->configure(config);
    QVERIFY(result.has_value());

    auto current_config = m_plugin->current_configuration();
    QCOMPARE(current_config["test_setting"].toString(), QString("test_value"));
}

QTEST_MAIN(Test{{class_name}})
#include "test_{{class_name_lower}}.moc"
""")


class PythonPluginTemplate(PluginTemplate):
    """Template for Python script plugins"""

    def __init__(self) -> None:
        super().__init__("python", "Python script plugin with QtForge integration")

    def generate_files(self, output_dir: Path) -> list[Path]:
        """Generate Python plugin files"""
        generated_files = []

        # Generate main plugin file
        plugin_content = self._generate_plugin()
        plugin_file = output_dir / f"{self.variables['plugin_name']}.py"
        plugin_file.write_text(plugin_content)
        generated_files.append(plugin_file)

        # Generate metadata.json
        metadata_content = self._generate_metadata()
        metadata_file = output_dir / "metadata.json"
        metadata_file.write_text(metadata_content)
        generated_files.append(metadata_file)

        # Generate requirements.txt
        requirements_content = self._generate_requirements()
        requirements_file = output_dir / "requirements.txt"
        requirements_file.write_text(requirements_content)
        generated_files.append(requirements_file)

        # Generate test file
        test_content = self._generate_test()
        test_file = output_dir / f"test_{self.variables['plugin_name']}.py"
        test_file.write_text(test_content)
        generated_files.append(test_file)

        return generated_files

    def _generate_plugin(self) -> str:
        """Generate Python plugin file"""
        return self.render_template('''#!/usr/bin/env python3
"""
{{name}} - {{description}}
Version: {{version}}
Author: {{author}}
"""

import json
import logging
from typing import Dict, List, Any, Optional
from datetime import datetime

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger("{{plugin_id}}")

class {{class_name}}:
    """{{description}}"""

    def __init__(self) -> None:
        self.state = "unloaded"
        self.configuration = {}
        self.interfaces = []
        self._setup_interfaces()

    # === Plugin Interface ===

    def name(self) -> str:
        return "{{name}}"

    def description(self) -> str:
        return "{{description}}"

    def version(self) -> str:
        return "{{version}}"

    def author(self) -> str:
        return "{{author}}"

    def id(self) -> str:
        return "{{plugin_id}}"

    def initialize(self) -> Dict[str, Any]:
        """Initialize the plugin"""
        if self.state != "unloaded":
            return {"success": False, "error": "Plugin already initialized"}

        self.state = "loading"

        try:
            # TODO: Add your initialization code here
            logger.info(f"Initializing plugin {self.name()}")

            self.state = "running"
            logger.info("Plugin initialized successfully")
            return {"success": True}

        except Exception as e:
            self.state = "error"
            logger.error(f"Failed to initialize plugin: {e}")
            return {"success": False, "error": str(e)}

    def shutdown(self) -> Dict[str, Any]:
        """Shutdown the plugin"""
        if self.state == "unloaded":
            return {"success": True}

        self.state = "stopping"

        try:
            # TODO: Add your cleanup code here
            logger.info("Shutting down plugin")

            self.state = "unloaded"
            logger.info("Plugin shutdown completed")
            return {"success": True}

        except Exception as e:
            logger.error(f"Error during shutdown: {e}")
            return {"success": False, "error": str(e)}

    def get_state(self) -> str:
        return self.state

    def capabilities(self) -> List[str]:
        return ["{{capability}}"]

    def configure(self, config: dict[str, Any]) -> dict[str, Any]:
        """Configure the plugin"""
        try:
            self.configuration.update(config)
            # TODO: Process configuration
            logger.info("Plugin configured successfully")
            return {"success": True}
        except Exception as e:
            logger.error(f"Configuration failed: {e}")
            return {"success": False, "error": str(e)}

    def current_configuration(self) -> Dict[str, Any]:
        return self.configuration.copy()

    def execute_command(self, command: str, params: dict[str, Any] | None = None) -> dict[str, Any]:
        """Execute a plugin command"""
        if params is None:
            params = {}

        if command == "status":
            return {
                "success": True,
                "result": {
                    "state": self.state,
                    "name": self.name(),
                    "version": self.version()
                }
            }

        # TODO: Add your commands here

        return {"success": False, "error": f"Unknown command: {command}"}

    def available_commands(self) -> List[str]:
        return ["status"]

    # === Dynamic Plugin Interface ===

    def get_interface_descriptors(self) -> List[Dict[str, Any]]:
        """Get supported interface descriptors"""
        return [interface for interface in self.interfaces]

    def supports_interface(self, interface_id: str, min_version: str = "0.0.0") -> bool:
        """Check if plugin supports a specific interface"""
        for interface in self.interfaces:
            if interface["interface_id"] == interface_id:
                # Simple version comparison (you might want to use a proper version library)
                return interface["version"] >= min_version
        return False

    def get_interface_descriptor(self, interface_id: str) -> Optional[Dict[str, Any]]:
        """Get interface descriptor by ID"""
        for interface in self.interfaces:
            if interface["interface_id"] == interface_id:
                return interface
        return None

    def get_plugin_type(self) -> str:
        return "python"

    def get_execution_context(self) -> Dict[str, Any]:
        return {
            "type": "python",
            "interpreter_path": "python",
            "timeout": 60000
        }

    def execute_code(self, code: str, context: dict[str, Any] | None = None) -> dict[str, Any]:
        """Execute Python code in plugin context"""
        if context is None:
            context = {}

        try:
            # Create a safe execution environment
            safe_globals = {
                "__builtins__": {
                    "print": print,
                    "len": len,
                    "str": str,
                    "int": int,
                    "float": float,
                    "bool": bool,
                    "list": list,
                    "dict": dict,
                    "tuple": tuple,
                    "set": set,
                }
            }
            safe_globals.update(context)

            # Execute the code
            result = eval(code, safe_globals)
            return {"success": True, "result": result}

        except Exception as e:
            logger.error(f"Code execution failed: {e}")
            return {"success": False, "error": str(e)}

    def invoke_method(self, method_name: str, parameters: list[Any] | None = None,
                     interface_id: str = "") -> dict[str, Any]:
        """Invoke a method dynamically"""
        if parameters is None:
            parameters = []

        # Check if method exists
        if hasattr(self, method_name):
            try:
                method = getattr(self, method_name)
                if callable(method):
                    result = method(*parameters)
                    return {"success": True, "result": result}
                else:
                    return {"success": False, "error": f"{method_name} is not callable"}
            except Exception as e:
                return {"success": False, "error": str(e)}

        return {"success": False, "error": f"Method not found: {method_name}"}

    def _setup_interfaces(self) -> None:
        """Setup plugin interfaces"""
        interface = {
            "interface_id": f"{self.id()}.main",
            "version": self.version(),
            "description": self.description(),
            "capabilities": [],
            "schema": {},
            "metadata": {}
        }
        self.interfaces.append(interface)

# Plugin entry point
def create_plugin() -> None:
    """Create and return plugin instance"""
    return {{class_name}}()

# For testing
if __name__ == "__main__":
    plugin = create_plugin()
    print(f"Plugin: {plugin.name()} v{plugin.version()}")
    print(f"Description: {plugin.description()}")

    # Test initialization
    result = plugin.initialize()
    print(f"Initialize: {result}")

    # Test command execution
    status = plugin.execute_command("status")
    print(f"Status: {status}")

    # Test shutdown
    shutdown_result = plugin.shutdown()
    print(f"Shutdown: {shutdown_result}")
''')

    def _generate_metadata(self) -> str:
        """Generate metadata.json for Python plugin"""
        metadata = {
            "id": self.variables["plugin_id"],
            "name": self.variables["name"],
            "version": self.variables["version"],
            "description": self.variables["description"],
            "author": self.variables["author"],
            "license": self.variables.get("license", "MIT"),
            "category": self.variables.get("category", "General"),
            "type": "python",
            "entry_point": f"{self.variables['plugin_name']}.py",
            "main_class": self.variables["class_name"],
            "capabilities": [self.variables.get("capability", "Service")],
            "dependencies": [],
            "python_requirements": ["qtforge-python-bridge"],
            "interfaces": [
                {
                    "id": f"{self.variables['plugin_id']}.main",
                    "version": self.variables["version"],
                    "description": self.variables["description"],
                }
            ],
            "generated": {
                "timestamp": datetime.now().isoformat(),
                "generator": "QtForge Plugin Generator v1.0",
                "template": "python",
            },
        }
        return json.dumps(metadata, indent=2)

    def _generate_requirements(self) -> str:
        """Generate requirements.txt"""
        return """# QtForge Python Plugin Requirements
qtforge-python-bridge>=1.0.0
"""

    def _generate_test(self) -> str:
        """Generate test file"""
        return self.render_template('''#!/usr/bin/env python3
"""
Unit tests for {{class_name}}
"""

import unittest
import sys
import os

# Add the plugin directory to the path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from {{plugin_name}} import {{class_name}}

class Test{{class_name}}(unittest.TestCase):
    """Test cases for {{class_name}}"""

    def setUp(self) -> None:
        """Set up test fixtures"""
        self.plugin = {{class_name}}()

    def tearDown(self) -> None:
        """Clean up after tests"""
        if self.plugin.get_state() != "unloaded":
            self.plugin.shutdown()

    def test_plugin_metadata(self) -> None:
        """Test plugin metadata"""
        self.assertEqual(self.plugin.name(), "{{name}}")
        self.assertEqual(self.plugin.description(), "{{description}}")
        self.assertEqual(self.plugin.version(), "{{version}}")
        self.assertEqual(self.plugin.author(), "{{author}}")
        self.assertEqual(self.plugin.id(), "{{plugin_id}}")

    def test_initialization(self) -> None:
        """Test plugin initialization"""
        self.assertEqual(self.plugin.get_state(), "unloaded")

        result = self.plugin.initialize()
        self.assertTrue(result["success"])
        self.assertEqual(self.plugin.get_state(), "running")

        # Test double initialization
        result = self.plugin.initialize()
        self.assertFalse(result["success"])

    def test_commands(self) -> None:
        """Test command execution"""
        self.plugin.initialize()

        commands = self.plugin.available_commands()
        self.assertIn("status", commands)

        result = self.plugin.execute_command("status")
        self.assertTrue(result["success"])
        self.assertIn("result", result)
        self.assertIn("state", result["result"])
        self.assertIn("name", result["result"])

    def test_configuration(self) -> None:
        """Test plugin configuration"""
        config = {"test_setting": "test_value"}
        result = self.plugin.configure(config)
        self.assertTrue(result["success"])

        current_config = self.plugin.current_configuration()
        self.assertEqual(current_config["test_setting"], "test_value")

    def test_interfaces(self) -> None:
        """Test interface support"""
        interfaces = self.plugin.get_interface_descriptors()
        self.assertGreater(len(interfaces), 0)

        main_interface_id = f"{self.plugin.id()}.main"
        self.assertTrue(self.plugin.supports_interface(main_interface_id))

        descriptor = self.plugin.get_interface_descriptor(main_interface_id)
        self.assertIsNotNone(descriptor)
        self.assertEqual(descriptor["interface_id"], main_interface_id)

    def test_code_execution(self) -> None:
        """Test code execution"""
        result = self.plugin.execute_code("2 + 2")
        self.assertTrue(result["success"])
        self.assertEqual(result["result"], 4)

        # Test with context
        context = {"x": 10}
        result = self.plugin.execute_code("x * 2", context)
        self.assertTrue(result["success"])
        self.assertEqual(result["result"], 20)

    def test_method_invocation(self) -> None:
        """Test dynamic method invocation"""
        result = self.plugin.invoke_method("name")
        self.assertTrue(result["success"])
        self.assertEqual(result["result"], "{{name}}")

        # Test non-existent method
        result = self.plugin.invoke_method("non_existent_method")
        self.assertFalse(result["success"])

if __name__ == "__main__":
    unittest.main()
''')


class PluginGenerator:
    """Main plugin generator class"""

    def __init__(self) -> None:
        self.templates = {
            "native": NativePluginTemplate(),
            "python": PythonPluginTemplate(),
        }

    def generate_plugin(
        self, template_name: str, output_dir: Path, variables: dict[str, Any]
    ) -> list[Path]:
        """Generate a plugin using the specified template"""
        if template_name not in self.templates:
            raise ValueError(f"Unknown template: {template_name}")

        template = self.templates[template_name]

        # Set default variables
        default_variables = self._get_default_variables(variables)
        template.set_variables(default_variables)

        # Create output directory
        output_dir.mkdir(parents=True, exist_ok=True)

        # Generate files
        generated_files = template.generate_files(output_dir)

        return generated_files

    def _get_default_variables(self, user_variables: dict[str, Any]) -> dict[str, Any]:
        """Get default variables with user overrides"""
        defaults = {
            "version": "1.0.0",
            "author": "Plugin Developer",
            "license": "MIT",
            "category": "General",
            "capability": "Service",
        }

        # Merge user variables
        defaults.update(user_variables)

        # Generate derived variables
        if "name" in defaults and "class_name" not in defaults:
            # Convert name to class name (e.g., "My Plugin" -> "MyPlugin")
            defaults["class_name"] = "".join(
                word.capitalize() for word in defaults["name"].split()
            )

        if "class_name" in defaults and "class_name_lower" not in defaults:
            defaults["class_name_lower"] = defaults["class_name"].lower()

        if "name" in defaults and "plugin_name" not in defaults:
            # Convert name to plugin name (e.g., "My Plugin" -> "my_plugin")
            defaults["plugin_name"] = defaults["name"].lower().replace(" ", "_")

        if "plugin_name" in defaults and "target_name" not in defaults:
            defaults["target_name"] = defaults["plugin_name"]

        if "plugin_name" in defaults and "project_name" not in defaults:
            defaults["project_name"] = defaults["plugin_name"].replace("_", "-")

        if "name" in defaults and "plugin_id" not in defaults:
            # Generate plugin ID (e.g., "My Plugin" -> "com.example.myplugin")
            plugin_name = defaults["name"].lower().replace(" ", "")
            defaults["plugin_id"] = f"com.example.{plugin_name}"

        return defaults

    def list_templates(self) -> list[str]:
        """Get list of available templates"""
        return list(self.templates.keys())

    def get_template_info(self, template_name: str) -> dict[str, str] | None:
        """Get information about a template"""
        if template_name not in self.templates:
            return None

        template = self.templates[template_name]
        return {"name": template.name, "description": template.description}


def main() -> int:
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="QtForge Plugin Generator - Create plugin projects with templates and scaffolding"
    )

    parser.add_argument(
        "--version", action="version", version="QtForge Plugin Generator v1.0"
    )

    subparsers = parser.add_subparsers(dest="command", help="Available commands")

    # Generate command
    generate_parser = subparsers.add_parser("generate", help="Generate a new plugin")
    generate_parser.add_argument(
        "template", choices=["native", "python"], help="Plugin template to use"
    )
    generate_parser.add_argument("name", help="Plugin name")
    generate_parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=Path(),
        help="Output directory (default: current directory)",
    )
    generate_parser.add_argument(
        "--author", default="Plugin Developer", help="Plugin author name"
    )
    generate_parser.add_argument("--description", help="Plugin description")
    generate_parser.add_argument("--version", default="1.0.0", help="Plugin version")
    generate_parser.add_argument("--license", default="MIT", help="Plugin license")
    generate_parser.add_argument(
        "--category", default="General", help="Plugin category"
    )
    generate_parser.add_argument(
        "--capability", default="Service", help="Plugin capability"
    )
    generate_parser.add_argument(
        "--plugin-id", help="Plugin ID (default: auto-generated)"
    )
    generate_parser.add_argument(
        "--class-name", help="Plugin class name (default: auto-generated)"
    )

    # List command
    list_parser = subparsers.add_parser("list", help="List available templates")

    # Info command
    info_parser = subparsers.add_parser("info", help="Get information about a template")
    info_parser.add_argument("template", help="Template name")

    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        return 1

    generator = PluginGenerator()

    if args.command == "generate":
        # Prepare variables
        variables = {
            "name": args.name,
            "author": args.author,
            "version": args.version,
            "license": args.license,
            "category": args.category,
            "capability": args.capability,
        }

        if args.description:
            variables["description"] = args.description
        else:
            variables["description"] = (
                f"A {args.template} plugin generated by QtForge Plugin Generator"
            )

        if args.plugin_id:
            variables["plugin_id"] = args.plugin_id

        if args.class_name:
            variables["class_name"] = args.class_name

        try:
            # Generate plugin
            plugin_name = variables.get("plugin_name")
            if plugin_name is None:
                plugin_name = args.name.lower().replace(" ", "_")
            output_dir = args.output / plugin_name
            generated_files = generator.generate_plugin(
                args.template, output_dir, variables
            )

            print(f"✅ Successfully generated {args.template} plugin '{args.name}'")
            print(f"📁 Output directory: {output_dir.absolute()}")
            print(f"📄 Generated {len(generated_files)} files:")

            for file_path in generated_files:
                relative_path = file_path.relative_to(output_dir)
                print(f"   - {relative_path}")

            print("\n🚀 Next steps:")
            print(f"   1. cd {output_dir.name}")
            if args.template == "native":
                print("   2. mkdir build && cd build")
                print("   3. cmake ..")
                print("   4. cmake --build .")
            elif args.template == "python":
                print("   2. pip install -r requirements.txt")
                print("   3. python -m pytest test_*.py")

            return 0

        except Exception as e:
            print(f"❌ Error generating plugin: {e}")
            return 1

    elif args.command == "list":
        print("Available plugin templates:")
        for template_name in generator.list_templates():
            info = generator.get_template_info(template_name)
            if info:
                print(f"  {template_name:10} - {info['description']}")
        return 0

    elif args.command == "info":
        info = generator.get_template_info(args.template)
        if info:
            print(f"Template: {info['name']}")
            print(f"Description: {info['description']}")
            return 0
        print(f"❌ Unknown template: {args.template}")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
