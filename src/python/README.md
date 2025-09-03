# QtForge Python Bindings

This directory contains the Python bindings for QtForge using pybind11.

## Structure

- `qtforge_python.cpp` - Main Python module entry point
- `core/` - Core plugin system bindings (includes advanced and dynamic plugin interfaces)
- `communication/` - Message bus and communication bindings (includes service contracts)
- `security/` - Security manager bindings (includes all security components)
- `utils/` - Utility class bindings
- `managers/` - Manager class bindings (includes version management)
- `orchestration/` - Plugin orchestration and workflow bindings
- `monitoring/` - Hot reload and metrics collection bindings
- `transactions/` - Transaction management bindings
- `composition/` - Plugin composition bindings
- `marketplace/` - Plugin marketplace bindings
- `threading/` - Threading and concurrency bindings

## Building

Python bindings are built when `QTFORGE_BUILD_PYTHON_BINDINGS` is enabled:

```bash
cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON ..
```

## Requirements

- Python 3.8 or later
- pybind11
- Qt6 Core (and optional components)

## New Features in v3.2.0

### Enhanced Core Bindings
- **Advanced Plugin Interface**: Support for service contracts and advanced communication
- **Dynamic Plugin Interface**: Runtime interface adaptation and capability negotiation
- **Plugin Type System**: Support for Native, Python, Lua, Remote, and Composite plugins
- **Lifecycle Management**: Complete plugin lifecycle with health monitoring and auto-restart

### Service Contracts
- **Service Discovery**: Automatic service registration and discovery
- **Version Compatibility**: Service version negotiation and compatibility checking
- **Method Descriptors**: Detailed service method specifications with input/output schemas
- **Capability Flags**: Fine-grained service capability declarations

### Enhanced Security
- **Permission System**: Granular permission management (FileSystem, Network, Registry, etc.)
- **Trust Levels**: Multi-level trust system (Untrusted, Limited, Trusted, FullyTrusted)
- **Security Components**: Complete coverage of all security validation components
- **Policy Engine**: Configurable security policy evaluation and enforcement

### Manager Enhancements
- **Version Management**: Multi-version plugin support with migration and rollback
- **Configuration Scoping**: Global, Plugin, User, Session, and Runtime configuration scopes
- **Resource Management**: Advanced resource allocation and monitoring
- **Logging Integration**: Comprehensive logging with multiple levels and file output

## Building Python Bindings

### Prerequisites

1. **Python 3.8 or later**
2. **pybind11** - Install via pip: `pip install pybind11`
3. **Qt6 Core** - Required for the base library
4. **CMake 3.21 or later**
5. **C++17 compatible compiler**

### Build Steps

1. **Configure with Python bindings enabled:**

   ```bash
   mkdir build && cd build
   cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON ..
   ```

2. **Optional: Specify Python version or installation directory:**

   ```bash
   cmake -DQTFORGE_BUILD_PYTHON_BINDINGS=ON \
         -DQTFORGE_PYTHON_MIN_VERSION=3.8 \
         -DQTFORGE_PYTHON_MAX_VERSION=3.12 \
         -DQTFORGE_PYTHON_INSTALL_DIR=/custom/path \
         ..
   ```

3. **Build the project:**

   ```bash
   cmake --build .
   ```

4. **Install (optional):**
   ```bash
   cmake --install .
   ```

### Testing the Build

Run the test script to verify everything works:

```bash
cd src/python
python example_usage.py
```

## Usage

### Basic Plugin Management

```python
import qtforge

# Create plugin manager
manager = qtforge.PluginManager()
print(f"Plugin manager created with {manager.plugin_count()} plugins")

# Load a plugin
try:
    result = manager.load_plugin("path/to/plugin.so")
    if result.has_value():
        plugin_id = result.value()
        plugin = manager.get_plugin(plugin_id)

        # Initialize plugin
        init_result = plugin.initialize()
        if init_result.has_value():
            print(f"Plugin {plugin.name()} initialized successfully")
        else:
            print(f"Failed to initialize plugin: {init_result.error().message}")
    else:
        print(f"Failed to load plugin: {result.error().message}")
except Exception as e:
    print(f"Error: {e}")
```

### Working with Plugin Metadata

```python
import qtforge

# Create metadata
metadata = qtforge.PluginMetadata()
metadata.name = "My Plugin"
metadata.version = qtforge.Version(1, 0, 0)
metadata.description = "A sample plugin"
metadata.author = "Developer Name"
metadata.capabilities = qtforge.PluginCapability.Service | qtforge.PluginCapability.Network

print(f"Plugin: {metadata.name} v{metadata.version}")
print(f"Capabilities: {metadata.capabilities}")
```

### Message Bus Communication

```python
import qtforge

# Create message bus
bus = qtforge.communication.MessageBus()

# Subscribe to messages
def message_handler(message):
    print(f"Received message: {message.topic()} from {message.sender()}")

# Subscribe to a topic
bus.subscribe_to_topic("my_subscriber", "test_topic", message_handler)

# Publish a message
bus.publish_basic("test_topic", "sender_id", {"key": "value"})
```

### Security Management

```python
import qtforge

# Create security manager
security = qtforge.security.SecurityManager()

# Set security level
security.set_security_level(qtforge.SecurityLevel.High)

# Validate a plugin
result = security.validate_plugin("path/to/plugin.so")
if result.is_valid:
    print("Plugin validation passed")
else:
    print(f"Plugin validation failed: {result.error_message}")
```

### Configuration Management

```python
import qtforge

# Create configuration manager
config = qtforge.managers.ConfigurationManager()

# Set configuration values
config.set_value("database.host", "localhost")
config.set_value("database.port", 5432)

# Get configuration values
host = config.get_value("database.host", "default_host")
port = config.get_value("database.port", 3306)

print(f"Database: {host}:{port}")
```
