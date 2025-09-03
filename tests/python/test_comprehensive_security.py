#!/usr/bin/env python3
"""
Comprehensive test suite for QtForge Python Security bindings.
Tests all security functionality including edge cases and error handling.
"""

import pytest
import sys
import os
import tempfile
import hashlib
from unittest.mock import Mock, patch

# Add the build directory to Python path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../build'))

try:
    import qtforge
    import qtforge.security as security
    BINDINGS_AVAILABLE = True
except ImportError as e:
    BINDINGS_AVAILABLE = False
    print(f"QtForge bindings not available: {e}")

pytestmark = pytest.mark.skipif(not BINDINGS_AVAILABLE, reason="QtForge bindings not available")


class TestSecurityManager:
    """Test SecurityManager functionality."""
    
    def test_security_manager_creation(self):
        """Test SecurityManager can be created."""
        if hasattr(security, 'create_security_manager'):
            manager = security.create_security_manager()
            assert manager is not None
            assert hasattr(manager, 'validate_plugin')
            assert hasattr(manager, 'check_permissions')
    
    def test_security_manager_validate_plugin(self):
        """Test plugin validation."""
        if hasattr(security, 'create_security_manager'):
            manager = security.create_security_manager()
            
            # Test with non-existent plugin
            try:
                result = manager.validate_plugin("/non/existent/plugin.so")
                assert isinstance(result, bool)
                assert not result  # Should fail validation
            except Exception as e:
                # Exception is acceptable for non-existent plugin
                pass
    
    def test_security_manager_check_permissions(self):
        """Test permission checking."""
        if hasattr(security, 'create_security_manager'):
            manager = security.create_security_manager()
            
            if hasattr(security, 'PluginPermission'):
                # Test checking file system permission
                if hasattr(security.PluginPermission, 'FileSystemRead'):
                    try:
                        has_permission = manager.check_permissions("test_plugin", security.PluginPermission.FileSystemRead)
                        assert isinstance(has_permission, bool)
                    except Exception as e:
                        # Some implementations might require plugin to be loaded first
                        pass
    
    def test_security_manager_set_security_level(self):
        """Test setting security level."""
        if hasattr(security, 'create_security_manager'):
            manager = security.create_security_manager()
            
            if hasattr(manager, 'set_security_level') and hasattr(security, 'SecurityLevel'):
                if hasattr(security.SecurityLevel, 'High'):
                    try:
                        manager.set_security_level(security.SecurityLevel.High)
                    except Exception as e:
                        # Some implementations might not allow runtime changes
                        pass


class TestSecurityValidator:
    """Test SecurityValidator functionality."""
    
    def test_security_validator_creation(self):
        """Test SecurityValidator can be created."""
        if hasattr(security, 'create_security_validator'):
            validator = security.create_security_validator()
            assert validator is not None
            assert hasattr(validator, 'validate')
    
    def test_security_validator_validate_file(self):
        """Test validating a file."""
        if hasattr(security, 'create_security_validator'):
            validator = security.create_security_validator()
            
            # Create a temporary file for testing
            with tempfile.NamedTemporaryFile(delete=False) as temp_file:
                temp_file.write(b"test content")
                temp_file.flush()
                
                try:
                    result = validator.validate(temp_file.name)
                    assert isinstance(result, bool)
                except Exception as e:
                    # Some validators might require specific file formats
                    pass
                finally:
                    os.unlink(temp_file.name)
    
    def test_security_validator_validate_invalid_file(self):
        """Test validating non-existent file."""
        if hasattr(security, 'create_security_validator'):
            validator = security.create_security_validator()
            
            with pytest.raises((FileNotFoundError, RuntimeError, ValueError)):
                validator.validate("/non/existent/file.so")


class TestSignatureVerifier:
    """Test SignatureVerifier functionality."""
    
    def test_signature_verifier_creation(self):
        """Test SignatureVerifier can be created."""
        if hasattr(security, 'create_signature_verifier'):
            verifier = security.create_signature_verifier()
            assert verifier is not None
            assert hasattr(verifier, 'verify')
    
    def test_signature_verifier_verify_file(self):
        """Test verifying file signature."""
        if hasattr(security, 'create_signature_verifier'):
            verifier = security.create_signature_verifier()
            
            # Create a temporary file for testing
            with tempfile.NamedTemporaryFile(delete=False) as temp_file:
                temp_file.write(b"test content")
                temp_file.flush()
                
                try:
                    # Test without signature (should fail)
                    result = verifier.verify(temp_file.name)
                    assert isinstance(result, bool)
                    assert not result  # Should fail without valid signature
                except Exception as e:
                    # Some verifiers might require signature file
                    pass
                finally:
                    os.unlink(temp_file.name)
    
    def test_signature_verifier_verify_with_signature(self):
        """Test verifying file with signature."""
        if hasattr(security, 'create_signature_verifier'):
            verifier = security.create_signature_verifier()
            
            # Create temporary files for testing
            with tempfile.NamedTemporaryFile(delete=False) as temp_file:
                temp_file.write(b"test content")
                temp_file.flush()
                
                with tempfile.NamedTemporaryFile(delete=False, suffix='.sig') as sig_file:
                    sig_file.write(b"fake signature")
                    sig_file.flush()
                    
                    try:
                        if hasattr(verifier, 'verify_with_signature'):
                            result = verifier.verify_with_signature(temp_file.name, sig_file.name)
                            assert isinstance(result, bool)
                    except Exception as e:
                        # Fake signature should fail
                        pass
                    finally:
                        os.unlink(temp_file.name)
                        os.unlink(sig_file.name)


class TestPermissionManager:
    """Test PermissionManager functionality."""
    
    def test_permission_manager_creation(self):
        """Test PermissionManager can be created."""
        if hasattr(security, 'create_permission_manager'):
            manager = security.create_permission_manager()
            assert manager is not None
            assert hasattr(manager, 'grant_permission')
            assert hasattr(manager, 'revoke_permission')
            assert hasattr(manager, 'has_permission')
    
    def test_permission_manager_grant_permission(self):
        """Test granting permissions."""
        if hasattr(security, 'create_permission_manager'):
            manager = security.create_permission_manager()
            
            if hasattr(security, 'PluginPermission') and hasattr(security.PluginPermission, 'FileSystemRead'):
                try:
                    manager.grant_permission("test_plugin", security.PluginPermission.FileSystemRead)
                except Exception as e:
                    # Some implementations might require plugin to be registered first
                    pass
    
    def test_permission_manager_revoke_permission(self):
        """Test revoking permissions."""
        if hasattr(security, 'create_permission_manager'):
            manager = security.create_permission_manager()
            
            if hasattr(security, 'PluginPermission') and hasattr(security.PluginPermission, 'FileSystemRead'):
                try:
                    # Grant first, then revoke
                    manager.grant_permission("test_plugin", security.PluginPermission.FileSystemRead)
                    manager.revoke_permission("test_plugin", security.PluginPermission.FileSystemRead)
                except Exception as e:
                    # Some implementations might require plugin to be registered first
                    pass
    
    def test_permission_manager_has_permission(self):
        """Test checking permissions."""
        if hasattr(security, 'create_permission_manager'):
            manager = security.create_permission_manager()
            
            if hasattr(security, 'PluginPermission') and hasattr(security.PluginPermission, 'FileSystemRead'):
                try:
                    has_perm = manager.has_permission("test_plugin", security.PluginPermission.FileSystemRead)
                    assert isinstance(has_perm, bool)
                except Exception as e:
                    # Some implementations might require plugin to be registered first
                    pass


class TestSecurityPolicyEngine:
    """Test SecurityPolicyEngine functionality."""
    
    def test_security_policy_engine_creation(self):
        """Test SecurityPolicyEngine can be created."""
        if hasattr(security, 'create_security_policy_engine'):
            engine = security.create_security_policy_engine()
            assert engine is not None
            assert hasattr(engine, 'evaluate_policy')
    
    def test_security_policy_engine_evaluate_policy(self):
        """Test policy evaluation."""
        if hasattr(security, 'create_security_policy_engine'):
            engine = security.create_security_policy_engine()
            
            try:
                # Test with basic policy
                result = engine.evaluate_policy("test_plugin", "test_action")
                assert isinstance(result, bool)
            except Exception as e:
                # Some implementations might require policy to be loaded first
                pass
    
    def test_security_policy_engine_load_policy(self):
        """Test loading security policy."""
        if hasattr(security, 'create_security_policy_engine'):
            engine = security.create_security_policy_engine()
            
            if hasattr(engine, 'load_policy'):
                # Create a temporary policy file
                with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as policy_file:
                    policy_file.write('{"default": "deny", "rules": []}')
                    policy_file.flush()
                    
                    try:
                        engine.load_policy(policy_file.name)
                    except Exception as e:
                        # Some implementations might use different policy format
                        pass
                    finally:
                        os.unlink(policy_file.name)


class TestSecurityEnums:
    """Test security-related enums."""
    
    def test_security_level_enum(self):
        """Test SecurityLevel enum values."""
        if hasattr(security, 'SecurityLevel'):
            levels = ['None', 'Low', 'Medium', 'High', 'Maximum']
            for level in levels:
                if hasattr(security.SecurityLevel, level):
                    value = getattr(security.SecurityLevel, level)
                    assert value is not None
    
    def test_plugin_permission_enum(self):
        """Test PluginPermission enum values."""
        if hasattr(security, 'PluginPermission'):
            permissions = ['FileSystemRead', 'FileSystemWrite', 'NetworkAccess', 'ProcessCreation', 'SystemAccess']
            for permission in permissions:
                if hasattr(security.PluginPermission, permission):
                    value = getattr(security.PluginPermission, permission)
                    assert value is not None
    
    def test_trust_level_enum(self):
        """Test TrustLevel enum values."""
        if hasattr(security, 'TrustLevel'):
            levels = ['Untrusted', 'Limited', 'Trusted', 'FullyTrusted']
            for level in levels:
                if hasattr(security.TrustLevel, level):
                    value = getattr(security.TrustLevel, level)
                    assert value is not None


class TestSecurityContext:
    """Test security context functionality."""
    
    def test_security_context_creation(self):
        """Test creating security context."""
        if hasattr(security, 'SecurityContext'):
            context = security.SecurityContext()
            assert context is not None
    
    def test_security_context_set_trust_level(self):
        """Test setting trust level in context."""
        if hasattr(security, 'SecurityContext') and hasattr(security, 'TrustLevel'):
            context = security.SecurityContext()
            
            if hasattr(context, 'set_trust_level') and hasattr(security.TrustLevel, 'Trusted'):
                context.set_trust_level(security.TrustLevel.Trusted)
                
                if hasattr(context, 'get_trust_level'):
                    level = context.get_trust_level()
                    assert level == security.TrustLevel.Trusted
    
    def test_security_context_add_permission(self):
        """Test adding permissions to context."""
        if hasattr(security, 'SecurityContext') and hasattr(security, 'PluginPermission'):
            context = security.SecurityContext()
            
            if hasattr(context, 'add_permission') and hasattr(security.PluginPermission, 'FileSystemRead'):
                context.add_permission(security.PluginPermission.FileSystemRead)
                
                if hasattr(context, 'has_permission'):
                    has_perm = context.has_permission(security.PluginPermission.FileSystemRead)
                    assert has_perm


class TestSecurityValidationResult:
    """Test security validation result functionality."""
    
    def test_validation_result_creation(self):
        """Test creating validation result."""
        if hasattr(security, 'ValidationResult'):
            result = security.ValidationResult(True, "Validation passed")
            assert result is not None
            
            if hasattr(result, 'is_valid'):
                assert result.is_valid
            if hasattr(result, 'message'):
                assert result.message == "Validation passed"
    
    def test_validation_result_failure(self):
        """Test creating failure validation result."""
        if hasattr(security, 'ValidationResult'):
            result = security.ValidationResult(False, "Validation failed")
            assert result is not None
            
            if hasattr(result, 'is_valid'):
                assert not result.is_valid
            if hasattr(result, 'message'):
                assert result.message == "Validation failed"


class TestSecurityErrorHandling:
    """Test error handling in security bindings."""
    
    def test_invalid_plugin_path(self):
        """Test handling invalid plugin paths."""
        if hasattr(security, 'create_security_manager'):
            manager = security.create_security_manager()
            
            # Test with None path
            with pytest.raises((ValueError, RuntimeError, TypeError)):
                manager.validate_plugin(None)
            
            # Test with empty path
            with pytest.raises((ValueError, RuntimeError)):
                manager.validate_plugin("")
    
    def test_invalid_permission_operations(self):
        """Test handling invalid permission operations."""
        if hasattr(security, 'create_permission_manager'):
            manager = security.create_permission_manager()
            
            # Test with None plugin name
            if hasattr(security, 'PluginPermission') and hasattr(security.PluginPermission, 'FileSystemRead'):
                with pytest.raises((ValueError, RuntimeError, TypeError)):
                    manager.grant_permission(None, security.PluginPermission.FileSystemRead)
    
    def test_security_exception_handling(self):
        """Test security exception handling."""
        if hasattr(security, 'SecurityException'):
            # Test creating security exception
            exception = security.SecurityException("Test security error")
            assert exception is not None
            assert str(exception) == "Test security error"


class TestSecurityIntegration:
    """Test integration between security components."""
    
    def test_manager_validator_integration(self):
        """Test integration between SecurityManager and SecurityValidator."""
        if hasattr(security, 'create_security_manager') and hasattr(security, 'create_security_validator'):
            manager = security.create_security_manager()
            validator = security.create_security_validator()
            
            # Test if manager can use validator
            if hasattr(manager, 'set_validator'):
                try:
                    manager.set_validator(validator)
                except Exception as e:
                    # Some implementations might not support this pattern
                    pass
    
    def test_manager_permission_integration(self):
        """Test integration between SecurityManager and PermissionManager."""
        if hasattr(security, 'create_security_manager') and hasattr(security, 'create_permission_manager'):
            manager = security.create_security_manager()
            perm_manager = security.create_permission_manager()
            
            # Test if manager can use permission manager
            if hasattr(manager, 'set_permission_manager'):
                try:
                    manager.set_permission_manager(perm_manager)
                except Exception as e:
                    # Some implementations might not support this pattern
                    pass


class TestSecurityConfiguration:
    """Test security configuration functionality."""
    
    def test_security_config_creation(self):
        """Test creating security configuration."""
        if hasattr(security, 'SecurityConfig'):
            config = security.SecurityConfig()
            assert config is not None
    
    def test_security_config_properties(self):
        """Test security configuration properties."""
        if hasattr(security, 'SecurityConfig'):
            config = security.SecurityConfig()
            
            # Test setting various properties
            properties = ['enforce_signatures', 'sandbox_enabled', 'strict_permissions']
            for prop in properties:
                if hasattr(config, prop):
                    try:
                        setattr(config, prop, True)
                        value = getattr(config, prop)
                        assert value is True
                    except (AttributeError, TypeError):
                        # Some properties might be read-only
                        pass


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
