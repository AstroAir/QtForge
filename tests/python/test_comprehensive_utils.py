#!/usr/bin/env python3
"""
Comprehensive test suite for QtForge Python Utils bindings.
Tests all utility functionality including edge cases and error handling.
"""

import pytest
import sys
import os
import json
import tempfile
from pathlib import Path
from unittest.mock import Mock, patch

# Add the build directory to Python path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../build'))

try:
    import qtforge
    import qtforge.utils as utils
    BINDINGS_AVAILABLE = True
except ImportError as e:
    BINDINGS_AVAILABLE = False
    print(f"QtForge bindings not available: {e}")

pytestmark = pytest.mark.skipif(not BINDINGS_AVAILABLE, reason="QtForge bindings not available")


class TestVersion:
    """Test Version utility functionality."""
    
    def test_version_creation(self) -> None:
        """Test Version can be created with different parameters."""
        if hasattr(utils, 'Version'):
            # Test with major, minor, patch
            version = utils.Version(1, 2, 3)
            assert version is not None
            
            # Test with string
            version_str = utils.Version("1.2.3")
            assert version_str is not None
    
    def test_version_comparison(self) -> None:
        """Test version comparison operations."""
        if hasattr(utils, 'Version'):
            v1 = utils.Version(1, 0, 0)
            v2 = utils.Version(1, 0, 1)
            v3 = utils.Version(1, 0, 0)
            
            # Test comparison operators if available
            if hasattr(v1, '__lt__'):
                assert v1 < v2
                assert not v2 < v1
            
            if hasattr(v1, '__eq__'):
                assert v1 == v3
                assert not v1 == v2
    
    def test_version_string_representation(self) -> None:
        """Test version string representation."""
        if hasattr(utils, 'Version'):
            version = utils.Version(1, 2, 3)
            version_str = str(version)
            assert isinstance(version_str, str)
            assert "1" in version_str
            assert "2" in version_str
            assert "3" in version_str
    
    def test_version_invalid_input(self) -> None:
        """Test version creation with invalid input."""
        if hasattr(utils, 'Version'):
            # Test with negative numbers
            with pytest.raises((ValueError, RuntimeError)):
                utils.Version(-1, 0, 0)
            
            # Test with invalid string format
            with pytest.raises((ValueError, RuntimeError)):
                utils.Version("invalid.version.string")
    
    def test_version_properties(self) -> None:
        """Test version property access."""
        if hasattr(utils, 'Version'):
            version = utils.Version(1, 2, 3)
            
            # Test property access if available
            if hasattr(version, 'major'):
                assert version.major == 1
            if hasattr(version, 'minor'):
                assert version.minor == 2
            if hasattr(version, 'patch'):
                assert version.patch == 3


class TestJsonUtilities:
    """Test JSON utility functions."""
    
    def test_json_parse_valid(self) -> None:
        """Test parsing valid JSON."""
        if hasattr(utils, 'parse_json'):
            valid_json = '{"key": "value", "number": 42}'
            result = utils.parse_json(valid_json)
            assert result is not None
            
            # Should be able to access parsed data
            if isinstance(result, dict):
                assert "key" in result
                assert result["key"] == "value"
                assert result["number"] == 42
    
    def test_json_parse_invalid(self) -> None:
        """Test parsing invalid JSON."""
        if hasattr(utils, 'parse_json'):
            invalid_json = '{"key": "value", "invalid": }'
            
            with pytest.raises((ValueError, RuntimeError)):
                utils.parse_json(invalid_json)
    
    def test_json_stringify(self) -> None:
        """Test JSON stringification."""
        if hasattr(utils, 'stringify_json'):
            data = {"key": "value", "number": 42, "array": [1, 2, 3]}
            result = utils.stringify_json(data)
            
            assert isinstance(result, str)
            # Should be valid JSON
            parsed_back = json.loads(result)
            assert parsed_back["key"] == "value"
            assert parsed_back["number"] == 42
    
    def test_json_stringify_invalid_data(self) -> None:
        """Test stringifying invalid data."""
        if hasattr(utils, 'stringify_json'):
            # Test with non-serializable object
            class NonSerializable:
                pass
            
            with pytest.raises((TypeError, ValueError, RuntimeError)):
                utils.stringify_json(NonSerializable())


class TestStringUtilities:
    """Test string utility functions."""
    
    def test_string_trim(self) -> None:
        """Test string trimming functionality."""
        if hasattr(utils, 'trim_string'):
            # Test with whitespace
            result = utils.trim_string("  hello world  ")
            assert result == "hello world"
            
            # Test with empty string
            result = utils.trim_string("")
            assert result == ""
            
            # Test with only whitespace
            result = utils.trim_string("   ")
            assert result == ""
    
    def test_string_split(self) -> None:
        """Test string splitting functionality."""
        if hasattr(utils, 'split_string'):
            result = utils.split_string("a,b,c", ",")
            assert isinstance(result, (list, tuple))
            assert len(result) == 3
            assert "a" in result
            assert "b" in result
            assert "c" in result
    
    def test_string_join(self) -> None:
        """Test string joining functionality."""
        if hasattr(utils, 'join_strings'):
            strings = ["a", "b", "c"]
            result = utils.join_strings(strings, ",")
            assert result == "a,b,c"
    
    def test_string_case_conversion(self) -> None:
        """Test string case conversion."""
        test_string = "Hello World"
        
        if hasattr(utils, 'to_lower'):
            result = utils.to_lower(test_string)
            assert result == "hello world"
        
        if hasattr(utils, 'to_upper'):
            result = utils.to_upper(test_string)
            assert result == "HELLO WORLD"


class TestFilesystemUtilities:
    """Test filesystem utility functions."""
    
    def test_file_exists(self) -> None:
        """Test file existence checking."""
        if hasattr(utils, 'file_exists'):
            # Test with this test file (should exist)
            assert utils.file_exists(__file__)
            
            # Test with non-existent file
            assert not utils.file_exists("/non/existent/file.txt")
    
    def test_directory_exists(self) -> None:
        """Test directory existence checking."""
        if hasattr(utils, 'directory_exists'):
            # Test with current directory (should exist)
            assert utils.directory_exists(os.path.dirname(__file__))
            
            # Test with non-existent directory
            assert not utils.directory_exists("/non/existent/directory")
    
    def test_create_directory(self) -> None:
        """Test directory creation."""
        if hasattr(utils, 'create_directory'):
            with tempfile.TemporaryDirectory() as temp_dir:
                test_dir = os.path.join(temp_dir, "test_subdir")
                
                # Directory should not exist initially
                assert not os.path.exists(test_dir)
                
                # Create directory
                result = utils.create_directory(test_dir)
                
                # Directory should now exist
                assert os.path.exists(test_dir)
                assert os.path.isdir(test_dir)
    
    def test_read_file(self) -> None:
        """Test file reading functionality."""
        if hasattr(utils, 'read_file'):
            with tempfile.NamedTemporaryFile(mode='w', delete=False) as temp_file:
                test_content = "Hello, World!\nThis is a test file."
                temp_file.write(test_content)
                temp_file.flush()
                
                try:
                    # Read the file
                    content = utils.read_file(temp_file.name)
                    assert content == test_content
                finally:
                    os.unlink(temp_file.name)
    
    def test_write_file(self) -> None:
        """Test file writing functionality."""
        if hasattr(utils, 'write_file'):
            with tempfile.NamedTemporaryFile(delete=False) as temp_file:
                test_content = "Hello, World!\nThis is a test file."
                
                try:
                    # Write to the file
                    utils.write_file(temp_file.name, test_content)
                    
                    # Read back and verify
                    with open(temp_file.name, 'r') as f:
                        content = f.read()
                    assert content == test_content
                finally:
                    os.unlink(temp_file.name)
    
    def test_get_file_size(self) -> None:
        """Test getting file size."""
        if hasattr(utils, 'get_file_size'):
            with tempfile.NamedTemporaryFile(mode='w', delete=False) as temp_file:
                test_content = "Hello, World!"
                temp_file.write(test_content)
                temp_file.flush()
                
                try:
                    size = utils.get_file_size(temp_file.name)
                    assert size == len(test_content)
                finally:
                    os.unlink(temp_file.name)


class TestLoggingUtilities:
    """Test logging utility functions."""
    
    def test_log_levels(self) -> None:
        """Test log level constants."""
        if hasattr(utils, 'LogLevel'):
            levels = ['Debug', 'Info', 'Warning', 'Error', 'Critical']
            for level in levels:
                if hasattr(utils.LogLevel, level):
                    value = getattr(utils.LogLevel, level)
                    assert value is not None
    
    def test_log_functions(self) -> None:
        """Test logging functions."""
        test_message = "Test log message"
        
        # Test different log levels
        log_functions = ['log_debug', 'log_info', 'log_warning', 'log_error']
        for func_name in log_functions:
            if hasattr(utils, func_name):
                func = getattr(utils, func_name)
                # Should not raise exception
                func(test_message)


class TestTimeUtilities:
    """Test time utility functions."""
    
    def test_current_timestamp(self) -> None:
        """Test getting current timestamp."""
        if hasattr(utils, 'current_timestamp'):
            timestamp = utils.current_timestamp()
            assert isinstance(timestamp, (int, float))
            assert timestamp > 0
    
    def test_format_timestamp(self) -> None:
        """Test timestamp formatting."""
        if hasattr(utils, 'format_timestamp'):
            timestamp = 1609459200  # 2021-01-01 00:00:00 UTC
            formatted = utils.format_timestamp(timestamp)
            assert isinstance(formatted, str)
            assert len(formatted) > 0
    
    def test_sleep_function(self) -> None:
        """Test sleep functionality."""
        if hasattr(utils, 'sleep'):
            import time
            start_time = time.time()
            utils.sleep(0.1)  # Sleep for 100ms
            end_time = time.time()
            
            # Should have slept for approximately 100ms
            elapsed = end_time - start_time
            assert elapsed >= 0.09  # Allow some tolerance


class TestErrorUtilities:
    """Test error handling utilities."""
    
    def test_error_creation(self) -> None:
        """Test creating errors."""
        if hasattr(utils, 'create_error'):
            error = utils.create_error("Test error message")
            assert error is not None
            assert str(error) == "Test error message"
    
    def test_error_codes(self) -> None:
        """Test error code constants."""
        if hasattr(utils, 'ErrorCode'):
            codes = ['Success', 'InvalidArgument', 'FileNotFound', 'PermissionDenied']
            for code in codes:
                if hasattr(utils.ErrorCode, code):
                    value = getattr(utils.ErrorCode, code)
                    assert value is not None


class TestMemoryUtilities:
    """Test memory management utilities."""
    
    def test_memory_usage(self) -> None:
        """Test getting memory usage information."""
        if hasattr(utils, 'get_memory_usage'):
            usage = utils.get_memory_usage()
            assert isinstance(usage, (int, float))
            assert usage >= 0
    
    def test_garbage_collection(self) -> None:
        """Test garbage collection trigger."""
        if hasattr(utils, 'trigger_gc'):
            # Should not raise exception
            utils.trigger_gc()


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
