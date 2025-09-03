#!/usr/bin/env python3
"""
QtForge Python Bindings Example 3: Security and Validation

This example demonstrates security features and plugin validation
using the QtForge security system, including:
- Security managers and validators
- Permission management and trust levels
- Plugin signature verification
- Security policy engines
- Sandboxing and access control
"""

import sys
import os
import tempfile
import hashlib
from pathlib import Path

# Add the build directory to Python path
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "build"))

try:
    import qtforge
    import qtforge.security as security
    print("✅ QtForge Security bindings loaded successfully")
except ImportError as e:
    print(f"❌ Failed to import QtForge Security: {e}")
    print("Make sure QtForge is built with Python bindings enabled")
    sys.exit(1)


def demonstrate_security_manager():
    """Demonstrate security manager creation and basic operations."""
    print("\n" + "="*50)
    print("🔒 Security Manager Operations")
    print("="*50)
    
    try:
        # Create a security manager
        manager = security.create_security_manager()
        print("✅ Security manager created successfully")
        
        # Check available methods
        methods = ['validate_plugin', 'check_permissions', 'set_security_level']
        available_methods = []
        
        for method in methods:
            if hasattr(manager, method):
                available_methods.append(method)
        
        print(f"📋 Available methods: {', '.join(available_methods)}")
        
        # Test plugin validation with non-existent plugin
        if hasattr(manager, 'validate_plugin'):
            print("\n🔍 Testing plugin validation...")
            
            test_plugins = [
                "/path/to/nonexistent/plugin.so",
                "invalid_plugin.dll",
                ""  # Empty path
            ]
            
            for plugin_path in test_plugins:
                try:
                    result = manager.validate_plugin(plugin_path)
                    print(f"  📊 Validation result for '{plugin_path}': {result}")
                except Exception as e:
                    print(f"  ⚠️  Validation failed for '{plugin_path}': {e}")
        
        # Test permission checking
        if hasattr(manager, 'check_permissions') and hasattr(security, 'PluginPermission'):
            print("\n🔐 Testing permission checking...")
            
            permissions_to_test = ['FileSystemRead', 'NetworkAccess', 'ProcessCreation']
            
            for perm_name in permissions_to_test:
                if hasattr(security.PluginPermission, perm_name):
                    permission = getattr(security.PluginPermission, perm_name)
                    
                    try:
                        has_permission = manager.check_permissions("test_plugin", permission)
                        print(f"  📊 Permission {perm_name}: {has_permission}")
                    except Exception as e:
                        print(f"  ⚠️  Permission check failed for {perm_name}: {e}")
        
        # Test security level setting
        if hasattr(manager, 'set_security_level') and hasattr(security, 'SecurityLevel'):
            print("\n⚡ Testing security level configuration...")
            
            levels_to_test = ['Low', 'Medium', 'High', 'Maximum']
            
            for level_name in levels_to_test:
                if hasattr(security.SecurityLevel, level_name):
                    level = getattr(security.SecurityLevel, level_name)
                    
                    try:
                        manager.set_security_level(level)
                        print(f"  ✅ Set security level to {level_name}")
                    except Exception as e:
                        print(f"  ⚠️  Failed to set security level {level_name}: {e}")
        
        return manager
        
    except Exception as e:
        print(f"❌ Failed to create security manager: {e}")
        return None


def demonstrate_security_validator():
    """Demonstrate security validator operations."""
    print("\n" + "="*50)
    print("🔍 Security Validator Operations")
    print("="*50)
    
    try:
        # Create a security validator
        validator = security.create_security_validator()
        print("✅ Security validator created successfully")
        
        # Create temporary files for testing
        test_files = []
        
        # Create a simple text file
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.txt') as f:
            f.write("This is a test file for security validation.")
            test_files.append(f.name)
            print(f"📄 Created test file: {f.name}")
        
        # Create a binary file
        with tempfile.NamedTemporaryFile(mode='wb', delete=False, suffix='.bin') as f:
            f.write(b'\x7fELF\x02\x01\x01\x00')  # ELF header-like content
            test_files.append(f.name)
            print(f"📄 Created binary test file: {f.name}")
        
        # Test validation on created files
        if hasattr(validator, 'validate'):
            print("\n🔍 Testing file validation...")
            
            for test_file in test_files:
                try:
                    result = validator.validate(test_file)
                    print(f"  📊 Validation result for {os.path.basename(test_file)}: {result}")
                except Exception as e:
                    print(f"  ⚠️  Validation failed for {os.path.basename(test_file)}: {e}")
        
        # Test validation with non-existent file
        try:
            result = validator.validate("/definitely/does/not/exist.so")
            print(f"  📊 Validation result for non-existent file: {result}")
        except Exception as e:
            print(f"  ✅ Correctly caught error for non-existent file: {e}")
        
        # Clean up test files
        for test_file in test_files:
            try:
                os.unlink(test_file)
            except OSError:
                pass
        
        return validator
        
    except Exception as e:
        print(f"❌ Failed to create security validator: {e}")
        return None


def demonstrate_signature_verifier():
    """Demonstrate signature verification operations."""
    print("\n" + "="*50)
    print("✍️  Signature Verification Operations")
    print("="*50)
    
    try:
        # Create a signature verifier
        verifier = security.create_signature_verifier()
        print("✅ Signature verifier created successfully")
        
        # Create test files for signature verification
        test_content = b"This is test content for signature verification."
        
        with tempfile.NamedTemporaryFile(mode='wb', delete=False) as content_file:
            content_file.write(test_content)
            content_file_path = content_file.name
        
        # Create a fake signature file
        fake_signature = hashlib.sha256(test_content).hexdigest().encode()
        
        with tempfile.NamedTemporaryFile(mode='wb', delete=False, suffix='.sig') as sig_file:
            sig_file.write(fake_signature)
            sig_file_path = sig_file.name
        
        print(f"📄 Created test content file: {os.path.basename(content_file_path)}")
        print(f"📄 Created fake signature file: {os.path.basename(sig_file_path)}")
        
        # Test signature verification
        if hasattr(verifier, 'verify'):
            print("\n✍️  Testing signature verification...")
            
            try:
                # Test verification without signature (should fail)
                result = verifier.verify(content_file_path)
                print(f"  📊 Verification without signature: {result}")
            except Exception as e:
                print(f"  ✅ Correctly failed verification without signature: {e}")
        
        # Test verification with signature file
        if hasattr(verifier, 'verify_with_signature'):
            try:
                result = verifier.verify_with_signature(content_file_path, sig_file_path)
                print(f"  📊 Verification with fake signature: {result}")
            except Exception as e:
                print(f"  ✅ Correctly rejected fake signature: {e}")
        
        # Clean up test files
        try:
            os.unlink(content_file_path)
            os.unlink(sig_file_path)
        except OSError:
            pass
        
        return verifier
        
    except Exception as e:
        print(f"❌ Failed to create signature verifier: {e}")
        return None


def demonstrate_permission_manager():
    """Demonstrate permission management operations."""
    print("\n" + "="*50)
    print("🔐 Permission Management Operations")
    print("="*50)
    
    try:
        # Create a permission manager
        perm_manager = security.create_permission_manager()
        print("✅ Permission manager created successfully")
        
        # Check available methods
        methods = ['grant_permission', 'revoke_permission', 'has_permission']
        available_methods = []
        
        for method in methods:
            if hasattr(perm_manager, method):
                available_methods.append(method)
        
        print(f"📋 Available methods: {', '.join(available_methods)}")
        
        # Test permission operations
        if hasattr(security, 'PluginPermission'):
            print("\n🔐 Testing permission operations...")
            
            test_plugin = "example_plugin"
            permissions_to_test = [
                'FileSystemRead',
                'FileSystemWrite', 
                'NetworkAccess',
                'ProcessCreation'
            ]
            
            for perm_name in permissions_to_test:
                if hasattr(security.PluginPermission, perm_name):
                    permission = getattr(security.PluginPermission, perm_name)
                    
                    print(f"\n  🔑 Testing {perm_name} permission:")
                    
                    # Grant permission
                    if hasattr(perm_manager, 'grant_permission'):
                        try:
                            perm_manager.grant_permission(test_plugin, permission)
                            print(f"    ✅ Granted {perm_name}")
                        except Exception as e:
                            print(f"    ⚠️  Failed to grant {perm_name}: {e}")
                    
                    # Check permission
                    if hasattr(perm_manager, 'has_permission'):
                        try:
                            has_perm = perm_manager.has_permission(test_plugin, permission)
                            print(f"    📊 Has {perm_name}: {has_perm}")
                        except Exception as e:
                            print(f"    ⚠️  Failed to check {perm_name}: {e}")
                    
                    # Revoke permission
                    if hasattr(perm_manager, 'revoke_permission'):
                        try:
                            perm_manager.revoke_permission(test_plugin, permission)
                            print(f"    ✅ Revoked {perm_name}")
                        except Exception as e:
                            print(f"    ⚠️  Failed to revoke {perm_name}: {e}")
                    
                    # Check permission after revocation
                    if hasattr(perm_manager, 'has_permission'):
                        try:
                            has_perm = perm_manager.has_permission(test_plugin, permission)
                            print(f"    📊 Has {perm_name} after revocation: {has_perm}")
                        except Exception as e:
                            print(f"    ⚠️  Failed to check {perm_name} after revocation: {e}")
        
        return perm_manager
        
    except Exception as e:
        print(f"❌ Failed to create permission manager: {e}")
        return None


def demonstrate_security_policy_engine():
    """Demonstrate security policy engine operations."""
    print("\n" + "="*50)
    print("📋 Security Policy Engine Operations")
    print("="*50)
    
    try:
        # Create a security policy engine
        policy_engine = security.create_security_policy_engine()
        print("✅ Security policy engine created successfully")
        
        # Test policy evaluation
        if hasattr(policy_engine, 'evaluate_policy'):
            print("\n📋 Testing policy evaluation...")
            
            test_scenarios = [
                ("trusted_plugin", "file_read"),
                ("untrusted_plugin", "network_access"),
                ("system_plugin", "process_create"),
                ("user_plugin", "file_write")
            ]
            
            for plugin_name, action in test_scenarios:
                try:
                    result = policy_engine.evaluate_policy(plugin_name, action)
                    print(f"  📊 Policy evaluation for {plugin_name}.{action}: {result}")
                except Exception as e:
                    print(f"  ⚠️  Policy evaluation failed for {plugin_name}.{action}: {e}")
        
        # Test policy loading
        if hasattr(policy_engine, 'load_policy'):
            print("\n📄 Testing policy loading...")
            
            # Create a simple policy file
            policy_content = {
                "version": "1.0",
                "default_action": "deny",
                "rules": [
                    {
                        "plugin_pattern": "trusted_*",
                        "action": "allow",
                        "permissions": ["file_read", "file_write"]
                    },
                    {
                        "plugin_pattern": "system_*",
                        "action": "allow",
                        "permissions": ["*"]
                    }
                ]
            }
            
            with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as policy_file:
                import json
                json.dump(policy_content, policy_file, indent=2)
                policy_file_path = policy_file.name
            
            try:
                policy_engine.load_policy(policy_file_path)
                print(f"  ✅ Successfully loaded policy from {os.path.basename(policy_file_path)}")
            except Exception as e:
                print(f"  ⚠️  Failed to load policy: {e}")
            
            # Clean up policy file
            try:
                os.unlink(policy_file_path)
            except OSError:
                pass
        
        return policy_engine
        
    except Exception as e:
        print(f"❌ Failed to create security policy engine: {e}")
        return None


def demonstrate_security_enums():
    """Demonstrate security-related enumerations."""
    print("\n" + "="*50)
    print("📊 Security Enumerations")
    print("="*50)
    
    # SecurityLevel enum
    if hasattr(security, 'SecurityLevel'):
        print("🔒 Security Levels:")
        levels = ['None', 'Low', 'Medium', 'High', 'Maximum']
        
        for level in levels:
            if hasattr(security.SecurityLevel, level):
                value = getattr(security.SecurityLevel, level)
                print(f"  • {level}: {value}")
    
    # PluginPermission enum
    if hasattr(security, 'PluginPermission'):
        print("\n🔑 Plugin Permissions:")
        permissions = [
            'FileSystemRead', 'FileSystemWrite', 'NetworkAccess', 
            'ProcessCreation', 'SystemAccess'
        ]
        
        for permission in permissions:
            if hasattr(security.PluginPermission, permission):
                value = getattr(security.PluginPermission, permission)
                print(f"  • {permission}: {value}")
    
    # TrustLevel enum
    if hasattr(security, 'TrustLevel'):
        print("\n🛡️  Trust Levels:")
        levels = ['Untrusted', 'Limited', 'Trusted', 'FullyTrusted']
        
        for level in levels:
            if hasattr(security.TrustLevel, level):
                value = getattr(security.TrustLevel, level)
                print(f"  • {level}: {value}")


def demonstrate_security_context():
    """Demonstrate security context operations."""
    print("\n" + "="*50)
    print("🏷️  Security Context Operations")
    print("="*50)
    
    if hasattr(security, 'SecurityContext'):
        try:
            # Create a security context
            context = security.SecurityContext()
            print("✅ Security context created successfully")
            
            # Test trust level operations
            if (hasattr(context, 'set_trust_level') and 
                hasattr(security, 'TrustLevel') and 
                hasattr(security.TrustLevel, 'Trusted')):
                
                context.set_trust_level(security.TrustLevel.Trusted)
                print("✅ Set trust level to Trusted")
                
                if hasattr(context, 'get_trust_level'):
                    level = context.get_trust_level()
                    print(f"📊 Current trust level: {level}")
            
            # Test permission operations
            if (hasattr(context, 'add_permission') and 
                hasattr(security, 'PluginPermission')):
                
                permissions_to_add = ['FileSystemRead', 'NetworkAccess']
                
                for perm_name in permissions_to_add:
                    if hasattr(security.PluginPermission, perm_name):
                        permission = getattr(security.PluginPermission, perm_name)
                        
                        context.add_permission(permission)
                        print(f"✅ Added {perm_name} permission to context")
                        
                        if hasattr(context, 'has_permission'):
                            has_perm = context.has_permission(permission)
                            print(f"📊 Context has {perm_name}: {has_perm}")
            
            return context
            
        except Exception as e:
            print(f"❌ Failed to work with security context: {e}")
            return None
    else:
        print("⚠️  SecurityContext class not available")
        return None


def demonstrate_error_handling():
    """Demonstrate security error handling."""
    print("\n" + "="*50)
    print("⚠️  Security Error Handling")
    print("="*50)
    
    # Test various error conditions
    try:
        manager = security.create_security_manager()
        
        # Test with invalid parameters
        print("🧪 Testing error handling with invalid parameters...")
        
        error_tests = [
            ("validate_plugin with None", lambda: manager.validate_plugin(None)),
            ("validate_plugin with empty string", lambda: manager.validate_plugin("")),
        ]
        
        for test_name, test_func in error_tests:
            try:
                result = test_func()
                print(f"  ⚠️  {test_name}: Unexpected success ({result})")
            except Exception as e:
                print(f"  ✅ {test_name}: Correctly caught {type(e).__name__}: {e}")
        
        # Test permission manager error handling
        if hasattr(security, 'create_permission_manager'):
            perm_manager = security.create_permission_manager()
            
            permission_error_tests = [
                ("grant_permission with None plugin", 
                 lambda: perm_manager.grant_permission(None, security.PluginPermission.FileSystemRead) 
                 if hasattr(security, 'PluginPermission') and hasattr(security.PluginPermission, 'FileSystemRead') else None),
            ]
            
            for test_name, test_func in permission_error_tests:
                if test_func:
                    try:
                        result = test_func()
                        print(f"  ⚠️  {test_name}: Unexpected success ({result})")
                    except Exception as e:
                        print(f"  ✅ {test_name}: Correctly caught {type(e).__name__}: {e}")
        
    except Exception as e:
        print(f"❌ Error handling demonstration failed: {e}")


def main():
    """Main demonstration function."""
    print("QtForge Python Bindings - Security and Validation Example")
    print("=" * 60)
    
    # Demonstrate each aspect of security
    demonstrate_security_manager()
    demonstrate_security_validator()
    demonstrate_signature_verifier()
    demonstrate_permission_manager()
    demonstrate_security_policy_engine()
    demonstrate_security_enums()
    demonstrate_security_context()
    demonstrate_error_handling()
    
    print("\n" + "="*60)
    print("🎉 Security and Validation Example Complete!")
    print("="*60)
    
    print("\n📚 Key Takeaways:")
    print("• Security managers coordinate all security operations")
    print("• Validators ensure plugin integrity and safety")
    print("• Permission managers control plugin access rights")
    print("• Policy engines enable flexible security rule definition")
    print("• Trust levels and contexts provide fine-grained control")
    
    print("\n🔗 Next Steps:")
    print("• Implement custom security policies and validators")
    print("• Create plugin sandboxing and isolation mechanisms")
    print("• Add cryptographic signature verification")
    print("• Develop security audit and monitoring systems")


if __name__ == "__main__":
    main()
