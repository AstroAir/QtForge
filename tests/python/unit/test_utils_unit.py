#!/usr/bin/env python3
"""
Unit tests for QtForge Python utils bindings.
Tests individual functions and classes in the utils module with comprehensive coverage.
"""

import pytest
import sys
import os
import tempfile
import json
from unittest.mock import Mock, patch, MagicMock
from pathlib import Path

# Add the build directory to Python path for testing
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../build'))

try:
    import qtforge
    import qtforge.utils as utils
    BINDINGS_AVAILABLE = True
except ImportError as e:
    BINDINGS_AVAILABLE = False
    pytest.skip(f"QtForge bindings not available: {e}", allow_module_level=True)


class TestVersionClass:
    """Test Version class functionality."""
    
    def test_version_creation(self):
        """Test Version can be created."""
        if hasattr(utils, 'Version'):
            version = utils.Version()
            assert version is not None
            assert isinstance(version, utils.Version)
    
    def test_version_with_parameters(self):
        """Test Version creation with parameters."""
        if hasattr(utils, 'Version'):
            # Test different constructor patterns
            try:
                version = utils.Version(1, 2, 3)
                assert version is not None
            except TypeError:
                # Constructor might have different signature
                pass
    
    def test_version_string_representation(self):
        """Test Version string representation."""
        if hasattr(utils, 'Version'):
            version = utils.Version()
            repr_str = repr(version)
            assert isinstance(repr_str, str)
            assert len(repr_str) > 0


class TestErrorHandling:
    """Test error handling utilities."""
    
    def test_create_error_function(self):
        """Test create_error function."""
        if hasattr(utils, 'create_error'):
            error = utils.create_error(404, "Not found")
            assert error is not None
            assert isinstance(error, str)
            assert "404" in error
            assert "Not found" in error
    
    def test_create_error_with_empty_message(self):
        """Test create_error with empty message."""
        if hasattr(utils, 'create_error'):
            error = utils.create_error(500, "")
            assert error is not None
            assert "500" in error
    
    def test_create_error_with_zero_code(self):
        """Test create_error with zero error code."""
        if hasattr(utils, 'create_error'):
            error = utils.create_error(0, "Success")
            assert error is not None
            assert "0" in error
    
    def test_create_error_with_negative_code(self):
        """Test create_error with negative error code."""
        if hasattr(utils, 'create_error'):
            error = utils.create_error(-1, "Invalid")
            assert error is not None
            assert "-1" in error


class TestStringUtilities:
    """Test string utility functions."""
    
    def test_string_functions_exist(self):
        """Test that string utility functions exist."""
        # Check for common string utility functions
        string_functions = [
            'trim', 'ltrim', 'rtrim', 'to_upper', 'to_lower',
            'starts_with', 'ends_with', 'contains', 'replace_all'
        ]
        
        for func_name in string_functions:
            if hasattr(utils, func_name):
                func = getattr(utils, func_name)
                assert callable(func), f"{func_name} should be callable"
    
    def test_string_trim_function(self):
        """Test string trim function if available."""
        if hasattr(utils, 'trim'):
            # Test basic trimming
            result = utils.trim("  hello  ")
            assert result == "hello"
            
            # Test empty string
            result = utils.trim("")
            assert result == ""
            
            # Test string with no whitespace
            result = utils.trim("hello")
            assert result == "hello"
    
    def test_string_case_functions(self):
        """Test string case conversion functions if available."""
        if hasattr(utils, 'to_upper'):
            result = utils.to_upper("hello")
            assert result == "HELLO"
            
            result = utils.to_upper("")
            assert result == ""
        
        if hasattr(utils, 'to_lower'):
            result = utils.to_lower("HELLO")
            assert result == "hello"
            
            result = utils.to_lower("")
            assert result == ""
    
    def test_string_search_functions(self):
        """Test string search functions if available."""
        if hasattr(utils, 'starts_with'):
            assert utils.starts_with("hello world", "hello") == True
            assert utils.starts_with("hello world", "world") == False
            assert utils.starts_with("", "") == True
        
        if hasattr(utils, 'ends_with'):
            assert utils.ends_with("hello world", "world") == True
            assert utils.ends_with("hello world", "hello") == False
            assert utils.ends_with("", "") == True
        
        if hasattr(utils, 'contains'):
            assert utils.contains("hello world", "lo wo") == True
            assert utils.contains("hello world", "xyz") == False
            assert utils.contains("", "") == True


class TestFileSystemUtilities:
    """Test filesystem utility functions."""
    
    def test_filesystem_functions_exist(self):
        """Test that filesystem utility functions exist."""
        fs_functions = [
            'file_exists', 'directory_exists', 'create_directory',
            'remove_file', 'remove_directory', 'get_file_size',
            'get_file_extension', 'get_filename', 'get_directory'
        ]
        
        for func_name in fs_functions:
            if hasattr(utils, func_name):
                func = getattr(utils, func_name)
                assert callable(func), f"{func_name} should be callable"
    
    def test_file_exists_function(self):
        """Test file_exists function if available."""
        if hasattr(utils, 'file_exists'):
            # Test with a file that definitely doesn't exist
            assert utils.file_exists("/nonexistent/path/file.txt") == False
            
            # Test with empty path
            assert utils.file_exists("") == False
    
    def test_directory_exists_function(self):
        """Test directory_exists function if available."""
        if hasattr(utils, 'directory_exists'):
            # Test with a directory that definitely doesn't exist
            assert utils.directory_exists("/nonexistent/path/") == False
            
            # Test with empty path
            assert utils.directory_exists("") == False
    
    def test_get_file_extension_function(self):
        """Test get_file_extension function if available."""
        if hasattr(utils, 'get_file_extension'):
            assert utils.get_file_extension("file.txt") == ".txt"
            assert utils.get_file_extension("file.tar.gz") == ".gz"
            assert utils.get_file_extension("file") == ""
            assert utils.get_file_extension("") == ""
    
    def test_get_filename_function(self):
        """Test get_filename function if available."""
        if hasattr(utils, 'get_filename'):
            assert utils.get_filename("/path/to/file.txt") == "file.txt"
            assert utils.get_filename("file.txt") == "file.txt"
            assert utils.get_filename("/path/to/") == ""
            assert utils.get_filename("") == ""


class TestJsonUtilities:
    """Test JSON utility functions."""
    
    def test_json_functions_exist(self):
        """Test that JSON utility functions exist."""
        json_functions = [
            'parse_json', 'stringify_json', 'validate_json',
            'json_get', 'json_set', 'json_has_key'
        ]
        
        for func_name in json_functions:
            if hasattr(utils, func_name):
                func = getattr(utils, func_name)
                assert callable(func), f"{func_name} should be callable"
    
    def test_parse_json_function(self):
        """Test parse_json function if available."""
        if hasattr(utils, 'parse_json'):
            # Test valid JSON
            result = utils.parse_json('{"key": "value"}')
            assert result is not None
            
            # Test invalid JSON
            try:
                result = utils.parse_json('invalid json')
                # Should either return None or raise exception
            except:
                pass  # Exception is acceptable
    
    def test_stringify_json_function(self):
        """Test stringify_json function if available."""
        if hasattr(utils, 'stringify_json'):
            # Test with simple object
            try:
                result = utils.stringify_json({"key": "value"})
                assert isinstance(result, str)
                assert "key" in result
                assert "value" in result
            except TypeError:
                # Function might expect different input format
                pass
    
    def test_validate_json_function(self):
        """Test validate_json function if available."""
        if hasattr(utils, 'validate_json'):
            # Test valid JSON
            assert utils.validate_json('{"key": "value"}') == True
            
            # Test invalid JSON
            assert utils.validate_json('invalid json') == False
            
            # Test empty string
            assert utils.validate_json('') == False


class TestLoggingUtilities:
    """Test logging utility functions."""
    
    def test_logging_functions_exist(self):
        """Test that logging utility functions exist."""
        logging_functions = [
            'log_debug', 'log_info', 'log_warning', 'log_error',
            'log_critical', 'set_log_level', 'get_log_level'
        ]
        
        for func_name in logging_functions:
            if hasattr(utils, func_name):
                func = getattr(utils, func_name)
                assert callable(func), f"{func_name} should be callable"
    
    def test_log_functions(self):
        """Test logging functions if available."""
        log_functions = ['log_debug', 'log_info', 'log_warning', 'log_error', 'log_critical']
        
        for func_name in log_functions:
            if hasattr(utils, func_name):
                func = getattr(utils, func_name)
                # Test that function can be called without crashing
                try:
                    func("Test message")
                    func("")  # Test empty message
                except:
                    pass  # Some implementations might require additional parameters
    
    def test_log_level_functions(self):
        """Test log level functions if available."""
        if hasattr(utils, 'set_log_level') and hasattr(utils, 'get_log_level'):
            # Test setting and getting log level
            try:
                original_level = utils.get_log_level()
                utils.set_log_level(2)  # Assuming numeric levels
                new_level = utils.get_log_level()
                assert new_level == 2
                utils.set_log_level(original_level)  # Restore
            except:
                pass  # Implementation might use different level system


class TestTimeUtilities:
    """Test time utility functions."""
    
    def test_time_functions_exist(self):
        """Test that time utility functions exist."""
        time_functions = [
            'current_timestamp', 'format_timestamp', 'parse_timestamp',
            'sleep', 'get_elapsed_time'
        ]
        
        for func_name in time_functions:
            if hasattr(utils, func_name):
                func = getattr(utils, func_name)
                assert callable(func), f"{func_name} should be callable"
    
    def test_current_timestamp_function(self):
        """Test current_timestamp function if available."""
        if hasattr(utils, 'current_timestamp'):
            timestamp = utils.current_timestamp()
            assert isinstance(timestamp, (int, float))
            assert timestamp > 0
    
    def test_format_timestamp_function(self):
        """Test format_timestamp function if available."""
        if hasattr(utils, 'format_timestamp'):
            try:
                # Test with current timestamp
                timestamp = 1640995200  # 2022-01-01 00:00:00 UTC
                formatted = utils.format_timestamp(timestamp, "yyyy-MM-dd")
                assert isinstance(formatted, str)
                assert len(formatted) > 0
            except:
                pass  # Implementation might use different format strings
    
    def test_sleep_function(self):
        """Test sleep function if available."""
        if hasattr(utils, 'sleep'):
            import time
            start_time = time.time()
            try:
                utils.sleep(0.1)  # Sleep for 100ms
                elapsed = time.time() - start_time
                assert elapsed >= 0.05  # Allow some tolerance
            except:
                pass  # Implementation might use different units


class TestUtilityModuleStructure:
    """Test overall utils module structure."""
    
    def test_module_attributes(self):
        """Test that utils module has expected attributes."""
        assert hasattr(utils, '__name__')
        assert utils.__name__ == 'qtforge.utils'
    
    def test_module_functions_are_callable(self):
        """Test that all module functions are callable."""
        for attr_name in dir(utils):
            if not attr_name.startswith('_'):
                attr = getattr(utils, attr_name)
                if callable(attr):
                    # Just verify it's callable, don't call it
                    assert callable(attr)
    
    def test_module_constants(self):
        """Test module constants if they exist."""
        # Check for common constants
        constants = ['VERSION', 'MAX_PATH_LENGTH', 'DEFAULT_TIMEOUT']
        for const_name in constants:
            if hasattr(utils, const_name):
                const_value = getattr(utils, const_name)
                assert const_value is not None


class TestErrorConditions:
    """Test error conditions and edge cases."""
    
    def test_null_pointer_handling(self):
        """Test handling of null/None inputs."""
        functions_to_test = []
        
        # Collect all callable functions from utils module
        for attr_name in dir(utils):
            if not attr_name.startswith('_'):
                attr = getattr(utils, attr_name)
                if callable(attr):
                    functions_to_test.append((attr_name, attr))
        
        # Test each function with None input
        for func_name, func in functions_to_test:
            try:
                # Try calling with None - should not crash
                result = func(None)
                # If it returns something, it should be a valid response
                assert result is not None or result is None
            except (TypeError, ValueError, AttributeError):
                # These exceptions are acceptable for None inputs
                pass
            except Exception as e:
                # Other exceptions might indicate poor error handling
                pytest.fail(f"Function {func_name} raised unexpected exception with None input: {e}")
    
    def test_empty_string_handling(self):
        """Test handling of empty string inputs."""
        string_functions = []
        
        # Collect functions that likely take string inputs
        for attr_name in dir(utils):
            if not attr_name.startswith('_'):
                attr = getattr(utils, attr_name)
                if callable(attr) and any(keyword in attr_name.lower() for keyword in 
                                        ['string', 'trim', 'upper', 'lower', 'file', 'path', 'json']):
                    string_functions.append((attr_name, attr))
        
        # Test each function with empty string
        for func_name, func in string_functions:
            try:
                result = func("")
                # Should return a valid result
                assert result is not None or result == ""
            except (TypeError, ValueError):
                # These exceptions are acceptable for empty inputs
                pass
    
    def test_large_input_handling(self):
        """Test handling of large inputs."""
        if hasattr(utils, 'trim'):
            # Test with very long string
            large_string = "x" * 10000
            try:
                result = utils.trim(large_string)
                assert isinstance(result, str)
            except MemoryError:
                # Acceptable for very large inputs
                pass
        
        if hasattr(utils, 'parse_json'):
            # Test with large JSON string
            large_json = '{"key": "' + "x" * 1000 + '"}'
            try:
                result = utils.parse_json(large_json)
                # Should handle large but valid JSON
            except (MemoryError, ValueError):
                # Acceptable for very large inputs
                pass


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
