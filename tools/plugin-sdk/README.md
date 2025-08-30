# QtForge Plugin SDK

A comprehensive software development kit for creating, validating, and managing QtForge plugins.

## Overview

The QtForge Plugin SDK provides a complete set of tools for plugin development:

- **Plugin Generator**: Create new plugin projects from templates
- **Plugin Validator**: Validate plugin structure, metadata, and code
- **Templates**: Pre-built templates for different plugin types
- **Documentation**: Comprehensive guides and examples

## Features

### 🚀 Plugin Generator

Generate plugin projects with a single command:

```bash
python plugin_generator.py generate native "My Awesome Plugin" --author "Your Name"
```

**Supported Templates:**
- **Native**: C++ plugins with full QtForge integration
- **Python**: Python script plugins with bridge support
- **JavaScript**: (Coming soon) JavaScript plugins with V8 engine
- **Lua**: (Coming soon) Lua script plugins

### 🔍 Plugin Validator

Validate your plugin projects:

```bash
python plugin_validator.py /path/to/your/plugin
```

**Validation Checks:**
- Directory structure
- Metadata format and completeness
- Source code requirements
- CMake configuration (for native plugins)
- Interface definitions
- Coding standards compliance

### 📋 Templates

#### Native C++ Template

Creates a complete C++ plugin project with:
- Modern C++20 code structure
- Qt6 integration
- CMake build system
- Unit tests with Qt Test framework
- Comprehensive documentation
- Plugin metadata

#### Python Template

Creates a Python plugin project with:
- Plugin interface implementation
- Metadata configuration
- Requirements management
- Unit tests with unittest
- Code execution sandbox
- Dynamic method invocation

## Quick Start

### 1. Generate a New Plugin

```bash
# Generate a native C++ plugin
python plugin_generator.py generate native "My Plugin" \
    --author "Your Name" \
    --description "A sample plugin" \
    --version "1.0.0"

# Generate a Python plugin
python plugin_generator.py generate python "My Python Plugin" \
    --author "Your Name" \
    --description "A Python-based plugin"
```

### 2. Build and Test (Native Plugin)

```bash
cd my_plugin
mkdir build && cd build
cmake ..
cmake --build .
ctest  # Run tests
```

### 3. Test Python Plugin

```bash
cd my_python_plugin
python -m pytest test_*.py
```

### 4. Validate Your Plugin

```bash
python ../plugin-sdk/plugin_validator.py my_plugin
```

## Plugin Templates

### Native C++ Plugin Structure

```
my_plugin/
├── CMakeLists.txt          # Build configuration
├── metadata.json           # Plugin metadata
├── include/
│   └── my_plugin.hpp      # Plugin header
├── src/
│   └── my_plugin.cpp      # Plugin implementation
└── tests/
    └── test_my_plugin.cpp # Unit tests
```

### Python Plugin Structure

```
my_python_plugin/
├── metadata.json           # Plugin metadata
├── requirements.txt        # Python dependencies
├── my_python_plugin.py    # Plugin implementation
└── test_my_python_plugin.py # Unit tests
```

## Plugin Metadata Format

All plugins require a `metadata.json` file:

```json
{
  "id": "com.example.myplugin",
  "name": "My Plugin",
  "version": "1.0.0",
  "description": "A sample plugin",
  "author": "Your Name",
  "license": "MIT",
  "category": "General",
  "type": "native",
  "capabilities": ["Service"],
  "dependencies": [],
  "interfaces": [
    {
      "id": "com.example.myplugin.main",
      "version": "1.0.0",
      "description": "Main plugin interface"
    }
  ]
}
```

## Advanced Features

### Dynamic Interface System

The advanced plugin system supports:

- **Runtime Interface Discovery**: Plugins can discover and adapt to interfaces at runtime
- **Interface Versioning**: Semantic versioning for interface compatibility
- **Capability Negotiation**: Plugins can negotiate capabilities with each other
- **Multi-language Support**: Native, Python, JavaScript, and Lua plugins

### Plugin Composition

Create complex plugins by combining simpler ones:

```cpp
// Example: Composite plugin
auto composite = PluginComposer()
    .add_component("data_processor", data_plugin)
    .add_component("ui_renderer", ui_plugin)
    .connect("data_processor.output", "ui_renderer.input")
    .build();
```

### Security and Sandboxing

- **Process Isolation**: Script plugins run in separate processes
- **Resource Limiting**: CPU, memory, and I/O limits
- **Permission System**: Fine-grained access control
- **Code Signing**: Plugin signature verification

## Command Reference

### Plugin Generator

```bash
python plugin_generator.py <command> [options]

Commands:
  generate <template> <name>  Generate a new plugin
  list                        List available templates
  info <template>            Get template information

Options:
  -o, --output DIR           Output directory
  --author NAME              Plugin author
  --description TEXT         Plugin description
  --version VERSION          Plugin version
  --license LICENSE          Plugin license
  --category CATEGORY        Plugin category
  --capability CAPABILITY    Plugin capability
  --plugin-id ID             Plugin identifier
  --class-name NAME          Plugin class name
```

### Plugin Validator

```bash
python plugin_validator.py <plugin_path> [options]

Options:
  --format {text,json}       Output format
  --severity {error,warning,info}  Minimum severity level
  --quiet, -q                Only show failed checks
```

## Integration with QtForge

### Plugin Manager Integration

```cpp
#include <qtplugin/core/plugin_manager.hpp>
#include <qtplugin/core/dynamic_plugin_interface.hpp>

// Load and use plugins
auto& manager = PluginManager::instance();
auto plugin = manager.load_plugin("com.example.myplugin");

if (plugin) {
    // Use dynamic interface
    auto dynamic_plugin = std::dynamic_pointer_cast<IDynamicPlugin>(plugin);
    if (dynamic_plugin) {
        auto result = dynamic_plugin->invoke_method("process_data", {data});
    }
}
```

### Plugin Development Workflow

1. **Generate**: Use the plugin generator to create a new project
2. **Develop**: Implement your plugin logic
3. **Validate**: Use the validator to check your plugin
4. **Test**: Run unit tests and integration tests
5. **Package**: Build and package your plugin
6. **Deploy**: Install in QtForge plugin directory

## Best Practices

### Plugin Design

- **Single Responsibility**: Each plugin should have a clear, focused purpose
- **Loose Coupling**: Minimize dependencies between plugins
- **Interface Stability**: Design stable interfaces that won't break compatibility
- **Error Handling**: Implement comprehensive error handling and recovery
- **Documentation**: Provide clear documentation and examples

### Performance

- **Lazy Loading**: Load resources only when needed
- **Caching**: Cache expensive computations
- **Async Operations**: Use asynchronous operations for I/O
- **Resource Management**: Properly manage memory and system resources

### Security

- **Input Validation**: Validate all external inputs
- **Principle of Least Privilege**: Request only necessary permissions
- **Secure Communication**: Use encrypted channels for network communication
- **Code Review**: Review code for security vulnerabilities

## Troubleshooting

### Common Issues

**Plugin Not Loading**
- Check metadata.json format
- Verify plugin ID uniqueness
- Ensure all dependencies are available

**Build Errors (Native Plugins)**
- Check CMake configuration
- Verify Qt6 and QtForge installation
- Check include paths and library links

**Python Plugin Errors**
- Check Python version compatibility
- Install required dependencies
- Verify plugin class implementation

**Validation Failures**
- Run validator with `--severity info` for detailed feedback
- Check plugin structure against templates
- Verify metadata completeness

### Getting Help

- Check the [QtForge Documentation](../docs/)
- Review example plugins in [examples/](../examples/)
- Submit issues on the project repository
- Join the QtForge community discussions

## Contributing

We welcome contributions to the Plugin SDK:

1. Fork the repository
2. Create a feature branch
3. Add your improvements
4. Include tests and documentation
5. Submit a pull request

### Adding New Templates

To add a new plugin template:

1. Create a new template class inheriting from `PluginTemplate`
2. Implement the `generate_files()` method
3. Add the template to the generator's template registry
4. Update documentation and tests

## License

The QtForge Plugin SDK is licensed under the MIT License. See [LICENSE](../../LICENSE) for details.

## Changelog

### Version 1.0.0
- Initial release
- Native C++ and Python templates
- Plugin generator and validator tools
- Dynamic interface system
- Comprehensive documentation
