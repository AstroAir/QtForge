# Security Plugin Example

This comprehensive security plugin demonstrates QtForge's advanced security features including plugin validation, signature verification, permission management, security policies, and audit logging.

## Overview

The Security Plugin showcases **comprehensive security architecture** patterns including:

- ✅ **Plugin Validation** with multiple security levels
- ✅ **Signature Verification** and trust management
- ✅ **Permission Management** with context-aware checks
- ✅ **Security Policy Engine** with configurable policies
- ✅ **Security Audit Logging** with event tracking
- ✅ **Real-time Security Monitoring** with periodic checks
- ✅ **Thread-Safe Security Operations** with proper synchronization
- ✅ **Comprehensive Error Handling** and security violation tracking

## Features

### 🔒 Security Validation

- **Multi-level validation**: None, Basic, Standard, Strict, Maximum
- **File integrity checks**: Hash verification and corruption detection
- **Signature verification**: Digital signature validation with trust chains
- **Permission validation**: Access control and capability verification
- **Malware scanning**: Optional integration with security scanners

### 🛡️ Permission Management

- **Operation-based permissions**: Fine-grained access control
- **Context-aware checks**: Dynamic permission evaluation
- **Policy enforcement**: Configurable security policies
- **Audit trail**: Complete permission check logging

### 📋 Security Policies

- **Default policies**: Standard security configurations
- **Custom policies**: User-defined security rules
- **Policy inheritance**: Hierarchical policy management
- **Runtime updates**: Dynamic policy modification

### 📊 Security Monitoring

- **Real-time monitoring**: Continuous security status tracking
- **Violation detection**: Automatic security breach identification
- **Performance metrics**: Security operation statistics
- **Audit logging**: Comprehensive security event recording

## Commands

### `validate` - Plugin Validation

Validate a plugin file using the security manager:

```bash
# Basic validation
./test_security_plugin validation

# Command parameters
{
    "file_path": "/path/to/plugin.dll",
    "security_level": 1  // 0=None, 1=Basic, 2=Standard, 3=Strict, 4=Maximum
}
```

**Response:**

```json
{
  "success": true,
  "validated_level": 2,
  "errors": [],
  "warnings": ["Plugin not in standard directory"],
  "file_path": "/path/to/plugin.dll",
  "timestamp": "2024-01-15T10:30:00Z"
}
```

### `permission` - Permission Check

Check permissions for specific operations:

```bash
# Permission check
{
    "operation": "file_read",
    "context": {
        "resource": "/secure/data.txt",
        "user": "current_user"
    }
}
```

**Response:**

```json
{
    "operation": "file_read",
    "granted": true,
    "context": {...},
    "timestamp": "2024-01-15T10:30:00Z",
    "success": true
}
```

### `policy` - Security Policy Management

Manage security policies:

```bash
# List policies
{"action": "list"}

# Set policy
{
    "action": "set",
    "policy_name": "strict_validation",
    "policy_config": {
        "allow_unsigned": false,
        "require_trusted_publisher": true,
        "require_code_signing": true
    }
}
```

### `audit` - Audit Log Management

Manage security audit logs:

```bash
# Get recent audit events
{"action": "get", "limit": 50}

# Clear audit log
{"action": "clear"}
```

### `status` - Security Status

Get comprehensive security status:

```bash
# Get status
{}
```

**Response includes:**

- Current security level and configuration
- Component initialization status
- Security metrics and violation counts
- Recent audit events
- Performance statistics

### `security_test` - Security Tests

Run security functionality tests:

```bash
# Basic functionality test
{"test_type": "basic"}

# Validation system test
{"test_type": "validation"}

# Permission system test
{"test_type": "permission"}
```

## Configuration

### Security Levels

- **None (0)**: No security validation
- **Basic (1)**: File existence and basic integrity
- **Standard (2)**: File integrity + basic signature check
- **Strict (3)**: Full signature validation + permissions
- **Maximum (4)**: All checks + malware scanning

### Default Configuration

```json
{
  "security_level": 1,
  "audit_enabled": true,
  "strict_validation": false,
  "security_check_interval": 30000,
  "max_audit_log_size": 1000,
  "allowed_operations": ["validate", "check_permission", "audit"],
  "trust_store_path": "trust_store.json",
  "signature_algorithms": ["SHA256", "RSA"],
  "permission_policies": {
    "default_deny": false,
    "require_signature": true,
    "allow_self_signed": false
  }
}
```

## Building

### Prerequisites

- QtForge library v3.0.0+ with Security module
- Qt6 with Core module
- CMake 3.21 or later
- C++20 compatible compiler

### Build Commands

```bash
# Configure and build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# Run tests
ctest --output-on-failure

# Install
cmake --install . --prefix /usr/local
```

### Build Options

- `QTFORGE_BUILD_DOCS=ON`: Generate documentation
- `QTFORGE_ENABLE_STATIC_ANALYSIS=ON`: Enable static analysis
- `QTFORGE_ENABLE_COVERAGE=ON`: Enable code coverage

## Testing

### Test Types

```bash
# Basic functionality test
./test_security_plugin basic

# Validation functionality test
./test_security_plugin validation

# Permission functionality test
./test_security_plugin permission

# All tests
./test_security_plugin all
```

### Test Coverage

- ✅ Plugin initialization and shutdown
- ✅ Configuration management
- ✅ Security validation with different levels
- ✅ Permission checking with various operations
- ✅ Policy management (set, list, update)
- ✅ Audit logging (get, clear, events)
- ✅ Security monitoring and metrics
- ✅ Error handling and recovery

## Security Considerations

### Trust Management

- **Certificate validation**: X.509 certificate chain verification
- **Trust store**: Centralized trusted publisher management
- **Revocation checking**: Certificate revocation list (CRL) support
- **Time-based validation**: Certificate validity period checking

### Threat Mitigation

- **Code injection**: Signature verification prevents tampering
- **Privilege escalation**: Permission system limits access
- **Data exfiltration**: Audit logging tracks all operations
- **Denial of service**: Resource limits and monitoring

### Best Practices

1. **Always validate plugins** before loading
2. **Use appropriate security levels** for your environment
3. **Monitor audit logs** for security violations
4. **Update trust stores** regularly
5. **Configure policies** based on security requirements

## Dependencies

- **Required**: QtForge::Security, QtForge::Core
- **Optional**: QtForge::MessageBus, QtForge::ConfigurationManager

## License

MIT License - Same as QtForge library
