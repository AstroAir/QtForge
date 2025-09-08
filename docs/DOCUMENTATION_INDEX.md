# QtForge v3.2.0 Documentation Index

This comprehensive index provides easy navigation to all QtForge v3.2.0 documentation, organized by category and user type.

## 🚀 Quick Start

### New to QtForge?

1. **[Overview](index.md)** - What is QtForge and why use it?
2. **[Installation Guide](installation.md)** - Get QtForge installed and running
3. **[Quick Start Guide](getting-started/quick-start.md)** - Your first QtForge application
4. **[Getting Started Tutorial](tutorials/getting-started-tutorial.md)** - Complete beginner tutorial

### Upgrading from v3.0.0?

1. **[Migration Guide](MIGRATION_v3_2.md)** - Complete migration instructions
2. **[What's New in v3.2.0](#whats-new-in-v320)** - New features overview
3. **[Breaking Changes](#breaking-changes)** - Important changes to be aware of

## 📚 Core Documentation

### Main Documentation

- **[README](../README.md)** - Project overview and quick start
- **[CHANGELOG](../CHANGELOG.md)** - Complete version history and changes
- **[Installation Guide](installation.md)** - Detailed installation instructions
- **[Migration Guide](MIGRATION_v3_2.md)** - v3.0.0 to v3.2.0 migration

### Architecture and Design

- **[System Design](architecture/system-design.md)** - Complete architecture overview
- **[Plugin Architecture](user-guide/plugin-architecture.md)** - Plugin system design
- **[Security Architecture](user-guide/advanced-security.md)** - Security system design

## 🔌 Plugin Development

### Getting Started

- **[Plugin Development Guide](user-guide/plugin-development.md)** - Basic plugin development
- **[First Plugin Tutorial](getting-started/first-plugin.md)** - Create your first plugin
- **[Plugin Development Best Practices](developer-guide/best-practices.md)** - Best practices

### Advanced Development

- **[Advanced Plugin Development](user-guide/advanced-plugin-development.md)** - Advanced techniques
- **[Advanced Patterns](developer-guide/advanced-patterns.md)** - Architectural patterns
- **[Plugin Composition](user-guide/plugin-composition.md)** - Combining plugins

### Multilingual Plugins (v3.2.0)

- **[Python Plugin Development](api/python/overview.md)** - Python plugin guide
- **[Lua Plugin Development](api/lua/overview.md)** - Lua plugin guide
- **[Multilingual Examples](examples/multilingual-examples.md)** - Cross-language examples

## 🔧 API Reference

### Core API

- **[API Overview](api/overview.md)** - Complete API overview
- **[Core Module](api/core/)** - Core plugin system API
- **[Plugin Interfaces](api/core/plugin-interface.md)** - IPlugin, IAdvancedPlugin, IDynamicPlugin

### Language Bindings

- **[Python API](api/python/overview.md)** - Complete Python binding reference
- **[Lua API](api/lua/overview.md)** - Complete Lua binding reference
- **[Binding Consistency](bindings/binding-consistency-report.md)** - Cross-language consistency

### Specialized Modules

- **[Security API](api/security/)** - Security and validation API
- **[Communication API](api/communication/)** - Inter-plugin communication
- **[Orchestration API](api/orchestration/)** - Workflow management
- **[Monitoring API](api/monitoring/)** - Performance monitoring

## 🛡️ Security

### Security Configuration

- **[Security Configuration](user-guide/security-configuration.md)** - Complete security setup
- **[Advanced Security](user-guide/advanced-security.md)** - Advanced security features
- **[Security Policy](../security.md)** - Project security policy

### v3.2.0 Security Features

- **Enhanced Plugin Sandbox** - Advanced sandboxing capabilities
- **Security Policy Validator** - Policy integrity validation
- **Resource Monitor Utils** - Resource monitoring and thresholds
- **Trust Level System** - Comprehensive trust management

## 📖 User Guides

### Basic Usage

- **[Configuration](user-guide/configuration.md)** - System configuration
- **[Plugin Management](user-guide/plugin-management.md)** - Managing plugins
- **[Troubleshooting](user-guide/troubleshooting.md)** - Common issues and solutions

### Advanced Features

- **[Workflow Orchestration](user-guide/workflow-orchestration.md)** - Advanced workflows
- **[Performance Optimization](user-guide/performance-optimization.md)** - Performance tuning
- **[Monitoring](user-guide/monitoring.md)** - System monitoring
- **[Threading](user-guide/threading.md)** - Multi-threading support

### Integration

- **[Python Integration](user-guide/python-integration.md)** - Python integration guide
- **[Marketplace Integration](user-guide/marketplace-integration.md)** - Plugin marketplace

## 🎯 Examples

### Basic Examples

- **[Examples Overview](examples/index.md)** - All examples overview
- **[Basic Plugin](examples/basic-plugin.md)** - Simple plugin example
- **[Service Plugin](examples/service-plugin.md)** - Background service plugin

### Advanced Examples

- **[Network Plugin](examples/network-plugin.md)** - Network-enabled plugin
- **[UI Plugin](examples/ui-plugin.md)** - User interface plugin
- **[Advanced Examples](examples/advanced.md)** - Complex plugin scenarios

### v3.2.0 Examples

- **[Python Examples](examples/python-examples.md)** - Python plugin examples
- **[Multilingual Examples](examples/multilingual-examples.md)** - Cross-language examples
- **[Advanced Plugin Interfaces](../examples/advanced_plugin_interfaces.py)** - New interface examples

## 🏗️ Development

### Contributing

- **[Contributing Guide](../CONTRIBUTING.md)** - How to contribute
- **[Code of Conduct](../CODE_OF_CONDUCT.md)** - Community guidelines
- **[Testing Guide](contributing/testing.md)** - Testing procedures

### Developer Resources

- **[Best Practices](developer-guide/best-practices.md)** - Development best practices
- **[CI/CD Optimization](developer-guide/ci-cd-optimization.md)** - Build optimization
- **[Plugin Distribution](developer-guide/plugin-distribution.md)** - Distribution guide

## 🆕 What's New in v3.2.0

### Major Features

1. **Enhanced Multilingual Support**

   - Complete Lua Plugin Bridge with sol2 integration
   - Enhanced Python bindings with type stubs
   - Cross-language communication capabilities

2. **Advanced Plugin Interfaces**

   - `IAdvancedPlugin` with service contracts
   - `IDynamicPlugin` with runtime adaptation
   - Dynamic capability negotiation

3. **Enhanced Security**

   - Advanced plugin sandboxing
   - Security policy validation
   - Resource monitoring and thresholds
   - Enhanced trust level system

4. **Service Contract System**

   - Service discovery and registration
   - Contract versioning and compatibility
   - Enhanced inter-plugin communication

5. **Configuration Management**
   - Scoped configuration access
   - Enhanced configuration API
   - Hot reload capabilities

### Breaking Changes

- **Configuration API**: Updated to use scoped access
- **Factory Functions**: Updated patterns for consistency
- **Security Policies**: Enhanced policy structure

### Migration Resources

- **[Complete Migration Guide](MIGRATION_v3_2.md)** - Step-by-step migration
- **[API Changes Summary](MIGRATION_v3_2.md#breaking-changes)** - All API changes
- **[Code Examples](MIGRATION_v3_2.md#migration-steps)** - Before/after examples

## 📋 Quick References

### API Quick Reference

- **[Core Classes](api/core/)** - Essential classes and interfaces
- **[Python Bindings](api/python/)** - Python API quick reference
- **[Lua Bindings](api/lua/)** - Lua API quick reference

### Configuration Quick Reference

- **[Security Levels](user-guide/security-configuration.md#security-levels)** - Security configuration
- **[Plugin Types](api/core/plugin-types.md)** - Supported plugin types
- **[Capabilities](api/core/plugin-interface.md#capabilities)** - Plugin capabilities

## 🔍 Search and Navigation

### By User Type

- **[New Users](#new-to-qtforge)** - Getting started resources
- **[Existing Users](#upgrading-from-v300)** - Migration and upgrade info
- **[Plugin Developers](#plugin-development)** - Development resources
- **[System Integrators](#integration)** - Integration guides

### By Topic

- **[Security](#security)** - All security-related documentation
- **[Performance](#advanced-features)** - Performance and optimization
- **[Examples](#examples)** - All code examples
- **[API Reference](#api-reference)** - Complete API documentation

### By Language

- **[C++](#core-api)** - Native C++ development
- **[Python](#language-bindings)** - Python plugin development
- **[Lua](#language-bindings)** - Lua plugin development

## 📞 Support and Community

### Getting Help

- **[FAQ](appendix/faq.md)** - Frequently asked questions
- **[Troubleshooting](user-guide/troubleshooting.md)** - Common issues
- **[GitHub Issues](https://github.com/AstroAir/QtForge/issues)** - Report bugs
- **[GitHub Discussions](https://github.com/AstroAir/QtForge/discussions)** - Community support

### Documentation Reports

- **[Documentation Update Report](documentation-update-report-v3-2.md)** - Update summary
- **[Documentation Complete Report](DOCUMENTATION_COMPLETE_REPORT_v3_2.md)** - Completion status
- **[Binding Updates Report](bindings/multilingual-bindings-update-report.md)** - Binding updates

---

_Documentation Index for QtForge v3.2.0 | Last Updated: September 2024_
