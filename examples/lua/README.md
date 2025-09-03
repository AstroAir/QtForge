# QtForge Lua Bindings Examples

This directory contains comprehensive examples demonstrating the QtForge Lua bindings. Each example focuses on specific aspects of the QtForge plugin system and provides practical, runnable Lua code with detailed explanations.

## Prerequisites

Before running these examples, ensure that:

1. **QtForge is built with Lua bindings enabled:**
   ```bash
   cmake -DQTFORGE_BUILD_LUA_BINDINGS=ON ..
   make
   ```

2. **Lua 5.1+ is installed** (LuaJIT is also supported)

3. **The QtForge Lua modules are accessible** (the build process should make them available to Lua)

## Examples Overview

### 1. Basic Plugin Management (`01_basic_plugin_management.lua`)

**What it demonstrates:**
- Creating and configuring plugin managers
- Loading and unloading plugins
- Plugin registry operations
- Plugin lifecycle management
- Dependency resolution
- Error handling in Lua

**Key concepts:**
- Plugin states and transitions
- Plugin enumeration and discovery
- Dependency graphs
- Plugin metadata handling
- Lua-specific error handling patterns

**Run it:**
```bash
lua examples/lua/01_basic_plugin_management.lua
```

### 2. Communication and Messaging (`02_communication_and_messaging.lua`)

**What it demonstrates:**
- Message bus creation and configuration
- Publish/subscribe messaging patterns
- Service contracts and discovery
- Request-response communication
- Message priorities and delivery modes
- Lua callback functions for message handling

**Key concepts:**
- Inter-plugin communication
- Message routing and filtering
- Service-oriented architecture
- Event-driven programming in Lua
- Callback management and cleanup

**Run it:**
```bash
lua examples/lua/02_communication_and_messaging.lua
```

### 3. Comprehensive Features (`03_comprehensive_features.lua`)

**What it demonstrates:**
- Security and validation systems
- Configuration and resource management
- Orchestration and workflow management
- Monitoring and hot reload capabilities
- Utility functions and helpers
- Integration patterns

**Key concepts:**
- Advanced plugin coordination
- Security policy enforcement
- Resource lifecycle management
- Workflow state management
- Development productivity tools

**Run it:**
```bash
lua examples/lua/03_comprehensive_features.lua
```

## Running All Examples

To run all examples in sequence:

```bash
# From the QtForge root directory
for example in examples/lua/[0-9]*.lua; do
    echo "Running $example..."
    lua "$example"
    echo "---"
done
```

## Example Output

Each example produces detailed output showing:
- ✅ Successful operations
- ⚠️  Warnings and expected failures
- ❌ Unexpected errors
- 📊 Status information and results
- 📋 Configuration details
- 🔧 Operation descriptions

## Understanding the Examples

### Code Structure

Each example follows a consistent Lua structure:
1. **Module documentation** - Comprehensive header comments
2. **Helper functions** - Consistent output formatting
3. **Availability checks** - Verifying QtForge bindings are loaded
4. **Demonstration functions** - Each focusing on specific functionality
5. **Error handling** - Lua-specific pcall patterns
6. **Main function** - Orchestrating all demonstrations
7. **Return values** - Exit codes for shell integration

### Lua-Specific Patterns

The examples demonstrate Lua best practices:
- **pcall for error handling** - Safe function calls with error capture
- **Table-based configuration** - Lua's natural data structure
- **Callback functions** - Event handling and message processing
- **String formatting** - Lua string manipulation techniques
- **Module organization** - Clean separation of concerns

### Error Handling Philosophy

The examples show that:
- Many operations may fail in a development environment
- Lua's pcall provides elegant error handling
- Error messages should be informative and actionable
- Graceful degradation is preferred over hard failures
- Real applications should implement comprehensive error logging

### Extensibility

Each example can be extended by:
- Adding custom plugin implementations
- Implementing real service endpoints
- Creating production-ready error handling
- Adding persistence and configuration management
- Integrating with Lua libraries and frameworks

## Integration with Tests

These examples complement the comprehensive test suite in `tests/lua/`. While tests focus on correctness and edge cases, examples focus on practical usage patterns and real-world scenarios.

## Lua-Specific Considerations

### Memory Management

- QtForge objects are managed by the C++ layer
- Lua garbage collection handles callback cleanup
- Be mindful of circular references in callbacks
- Use weak references for long-lived callback storage

### Performance

- Lua bindings add minimal overhead
- Callback functions should be lightweight
- Consider LuaJIT for performance-critical applications
- Profile callback-heavy scenarios

### Debugging

- Use Lua's debug library for stack traces
- Print statements are effective for development
- Consider lua-inspect for object introspection
- Use pcall consistently for error isolation

## Troubleshooting

### Common Issues

1. **"attempt to index global 'qtforge' (a nil value)"**
   - Ensure QtForge is built with Lua bindings enabled
   - Check that the Lua modules are in the correct path
   - Verify Lua version compatibility

2. **"QtForge Lua bindings not available"**
   - Rebuild QtForge with `-DQTFORGE_BUILD_LUA_BINDINGS=ON`
   - Check for build errors in the Lua binding compilation
   - Ensure Lua development headers are installed

3. **Callback functions not working**
   - Verify callback function signatures match expectations
   - Check for proper error handling in callbacks
   - Ensure callbacks don't create circular references

4. **Many operations show warnings**
   - This is expected in a development environment
   - Examples are designed to work without actual plugins
   - Focus on successful creation of managers and basic operations

### Getting Help

- Check the comprehensive test suite for additional usage patterns
- Review the QtForge documentation for detailed API reference
- Examine the C++ source code for implementation details
- Use Lua's type() and pairs() functions for runtime inspection
- Consult Lua documentation for language-specific questions

## Lua Version Compatibility

The examples are designed to work with:
- **Lua 5.1** - Minimum supported version
- **Lua 5.2** - Full compatibility
- **Lua 5.3** - Full compatibility
- **Lua 5.4** - Full compatibility
- **LuaJIT** - Recommended for performance

### Version-Specific Notes

- **Lua 5.1**: Uses older table.getn() patterns where needed
- **Lua 5.2+**: Takes advantage of newer language features
- **LuaJIT**: Optimized for FFI integration where applicable

## Performance Considerations

### Best Practices

1. **Minimize callback overhead** - Keep callback functions simple
2. **Batch operations** - Group related operations together
3. **Use local variables** - Faster access than globals
4. **Avoid string concatenation in loops** - Use table.concat instead
5. **Profile critical paths** - Use Lua profiling tools

### Memory Usage

- QtForge objects are reference-counted in C++
- Lua holds references through the binding layer
- Callbacks may extend object lifetimes
- Use weak references for observer patterns

## Next Steps

After running these examples:

1. **Explore the test suite** in `tests/lua/` for comprehensive API coverage
2. **Create custom plugins** using the patterns demonstrated
3. **Build real applications** incorporating QtForge plugin architecture
4. **Integrate with Lua frameworks** like OpenResty, Lapis, or Torch
5. **Contribute examples** for additional use cases and patterns

## Contributing

To contribute new examples:

1. Follow the existing naming convention (`NN_descriptive_name.lua`)
2. Include comprehensive error handling using pcall patterns
3. Add corresponding entries to this README
4. Ensure examples work in development environments
5. Test with multiple Lua versions where possible
6. Follow Lua style guidelines and best practices

## Integration Examples

### Web Applications

```lua
-- Example: Integrating with OpenResty
local qtforge = require('qtforge')
local manager = qtforge.core.create_plugin_manager()

-- Load web-specific plugins
manager:load_plugin('web_auth_plugin.so')
manager:load_plugin('session_manager_plugin.so')
```

### Game Development

```lua
-- Example: Integrating with LÖVE 2D
local qtforge = require('qtforge')
local orchestrator = qtforge.orchestration.create_plugin_orchestrator()

-- Load game plugins
orchestrator:register_plugin('physics_engine', 'physics.so')
orchestrator:register_plugin('audio_manager', 'audio.so')
```

### Scientific Computing

```lua
-- Example: Integrating with Torch
local qtforge = require('qtforge')
local bus = qtforge.communication.create_message_bus()

-- Set up data processing pipeline
bus:subscribe('data.processed', function(message)
    -- Process with Torch tensors
    local tensor = torch.Tensor(message.data)
    -- ... processing logic
end)
```

These examples demonstrate the flexibility and power of QtForge's Lua bindings across different application domains.
