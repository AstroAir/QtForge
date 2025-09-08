#!/usr/bin/env python3
"""
Sample Python Plugin for QtForge
Demonstrates basic plugin functionality with the Python bridge.
"""

class SamplePlugin:
    """A sample plugin demonstrating QtForge Python plugin capabilities"""

    def __init__(self):
        self.name = "Sample Python Plugin"
        self.version = "1.0.0"
        self.description = "A demonstration plugin for QtForge Python bridge"
        self.author = "QtForge Team"
        self.license = "MIT"

        # Plugin state
        self.initialized = False
        self.data = {}
        self.counter = 0
        self.event_handlers = {}
        self.subscribed_events = []

    def initialize(self):
        """Initialize the plugin"""
        self.initialized = True
        self.data = {"startup_time": "2024-01-01T00:00:00Z"}
        return {"success": True, "message": "Plugin initialized successfully"}

    def shutdown(self):
        """Shutdown the plugin"""
        self.initialized = False
        self.data.clear()
        return {"success": True, "message": "Plugin shutdown successfully"}

    def get_info(self):
        """Get plugin information"""
        return {
            "name": self.name,
            "version": self.version,
            "description": self.description,
            "author": self.author,
            "license": self.license,
            "initialized": self.initialized
        }

    def process_data(self, input_data):
        """Process some data"""
        self.counter += 1
        result = {
            "processed": True,
            "input": input_data,
            "counter": self.counter,
            "timestamp": f"2024-01-01T00:00:{self.counter:02d}Z"
        }
        return result

    def get_counter(self):
        """Get the current counter value"""
        return self.counter

    def set_counter(self, value):
        """Set the counter value"""
        self.counter = int(value)
        return {"success": True, "new_value": self.counter}

    def add_data(self, key, value):
        """Add data to the plugin's data store"""
        self.data[key] = value
        return {"success": True, "key": key, "value": value}

    def get_data(self, key=None):
        """Get data from the plugin's data store"""
        if key is None:
            return self.data
        return self.data.get(key)

    def list_methods(self):
        """List available methods"""
        methods = []
        for attr_name in dir(self):
            if not attr_name.startswith('_') and callable(getattr(self, attr_name)):
                methods.append(attr_name)
        return methods

    def list_properties(self):
        """List available properties"""
        properties = []
        for attr_name in dir(self):
            if not attr_name.startswith('_') and not callable(getattr(self, attr_name)):
                properties.append({
                    "name": attr_name,
                    "type": type(getattr(self, attr_name)).__name__,
                    "value": str(getattr(self, attr_name))
                })
        return properties

    def subscribe_events(self, event_names):
        """Subscribe to events (called by bridge)"""
        for event_name in event_names:
            if event_name not in self.subscribed_events:
                self.subscribed_events.append(event_name)
        return {"success": True, "subscribed_events": self.subscribed_events}

    def unsubscribe_events(self, event_names):
        """Unsubscribe from events (called by bridge)"""
        for event_name in event_names:
            if event_name in self.subscribed_events:
                self.subscribed_events.remove(event_name)
        return {"success": True, "subscribed_events": self.subscribed_events}

    def emit_event(self, event_name, event_data):
        """Handle event emission (called by bridge)"""
        result = {"event_received": True, "event_name": event_name, "event_data": event_data}

        # Call specific event handler if it exists
        handler_method = f"on_{event_name}"
        if hasattr(self, handler_method):
            handler = getattr(self, handler_method)
            if callable(handler):
                handler_result = handler(event_data)
                result["handler_result"] = handler_result

        return result

    def handle_event(self, event_name, event_data):
        """Generic event handler"""
        self.data[f"last_event_{event_name}"] = {
            "data": event_data,
            "timestamp": f"2024-01-01T00:00:{self.counter:02d}Z"
        }
        return {"handled": True, "event_name": event_name}

    def on_test_event(self, event_data):
        """Specific handler for 'test_event'"""
        self.counter += 1
        return {
            "test_event_handled": True,
            "counter": self.counter,
            "received_data": event_data
        }

    def trigger_custom_event(self, event_name="custom_event", data=None):
        """Trigger a custom event (for testing)"""
        if data is None:
            data = {"triggered_by": "plugin", "counter": self.counter}

        # Store event for later retrieval
        self.data[f"triggered_{event_name}"] = data

        return {
            "event_triggered": True,
            "event_name": event_name,
            "event_data": data
        }

# Factory function for creating plugin instances
def create_plugin():
    """Create and return a plugin instance"""
    return SamplePlugin()

# For direct instantiation
if __name__ == "__main__":
    plugin = create_plugin()
    print(f"Created plugin: {plugin.get_info()}")
