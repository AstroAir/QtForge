#!/usr/bin/env python3
"""
QtForge Python Bindings Example 2: Communication and Messaging

This example demonstrates inter-plugin communication using the QtForge
message bus system, including:
- Creating and configuring message buses
- Publishing and subscribing to messages
- Service contracts and discovery
- Request-response patterns
- Message priorities and delivery modes
"""

import sys
import time
import threading
from pathlib import Path
from typing import Any

# Add the build directory to Python path
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "build"))

try:
    import qtforge
    import qtforge.communication as comm
    print("✅ QtForge Communication bindings loaded successfully")
except ImportError as e:
    print(f"❌ Failed to import QtForge Communication: {e}")
    print("Make sure QtForge is built with Python bindings enabled")
    sys.exit(1)


def demonstrate_message_bus_creation() -> Any:
    """Demonstrate creating and configuring a message bus."""
    print("\n" + "="*50)
    print("📡 Creating Message Bus")
    print("="*50)
    
    try:
        # Create a message bus
        bus = comm.create_message_bus()
        print("✅ Message bus created successfully")
        
        # Check available methods
        methods = ['publish', 'subscribe', 'unsubscribe']
        available_methods = []
        
        for method in methods:
            if hasattr(bus, method):
                available_methods.append(method)
        
        print(f"📋 Available methods: {', '.join(available_methods)}")
        
        return bus
        
    except Exception as e:
        print(f"❌ Failed to create message bus: {e}")
        return None


def demonstrate_basic_messaging(bus) -> None:
    """Demonstrate basic publish/subscribe messaging."""
    print("\n" + "="*50)
    print("📨 Basic Publish/Subscribe Messaging")
    print("="*50)
    
    if not bus:
        print("❌ No message bus available")
        return
    
    # Storage for received messages
    received_messages = []
    
    def message_callback(message) -> None:
        """Callback function for received messages."""
        print(f"📥 Received message: {message}")
        received_messages.append(message)
    
    try:
        # Subscribe to a topic
        topic = "example.notifications"
        print(f"🔔 Subscribing to topic: {topic}")
        
        if hasattr(bus, 'subscribe'):
            try:
                bus.subscribe(topic, message_callback)
                print("✅ Successfully subscribed to topic")
            except Exception as e:
                print(f"⚠️  Subscription failed: {e}")
                return
        
        # Create and publish messages
        if hasattr(comm, 'BasicMessage'):
            print(f"\n📤 Publishing messages to topic: {topic}")
            
            messages = [
                "Hello, World!",
                "This is a test message",
                {"type": "notification", "data": "Important update"},
                42  # Test with different data types
            ]
            
            for i, msg_data in enumerate(messages, 1):
                try:
                    message = comm.BasicMessage(topic, str(msg_data))
                    publish_func = getattr(bus, 'publish', None)
                    if publish_func:
                        publish_func(message)
                        print(f"  ✅ Published message {i}: {msg_data}")
                    else:
                        print(f"  ⚠️  Publish method not available for message {i}")
                    time.sleep(0.1)  # Small delay for processing
                except Exception as e:
                    print(f"  ❌ Failed to publish message {i}: {e}")
        
        # Give some time for message processing
        time.sleep(0.5)
        
        print(f"\n📊 Total messages received: {len(received_messages)}")
        
        # Unsubscribe
        if hasattr(bus, 'unsubscribe'):
            try:
                bus.unsubscribe(topic, message_callback)
                print("✅ Successfully unsubscribed from topic")
            except Exception as e:
                print(f"⚠️  Unsubscription failed: {e}")
        
    except Exception as e:
        print(f"❌ Basic messaging demonstration failed: {e}")


def demonstrate_message_properties() -> Any:
    """Demonstrate message properties and metadata."""
    print("\n" + "="*50)
    print("🏷️  Message Properties and Metadata")
    print("="*50)
    
    if not hasattr(comm, 'BasicMessage'):
        print("❌ BasicMessage class not available")
        return
    
    try:
        # Create a message with basic properties
        topic = "example.properties"
        data = {"user": "alice", "action": "login", "timestamp": time.time()}
        
        message = comm.BasicMessage(topic, str(data))
        print("✅ Created message with basic properties")
        
        # Test property access
        if hasattr(message, 'topic'):
            print(f"📋 Message topic: {message.topic}")
        
        if hasattr(message, 'data'):
            print(f"📋 Message data: {message.data}")
        
        # Test metadata operations
        if hasattr(message, 'set_metadata'):
            print("\n🏷️  Setting message metadata...")
            
            metadata_items = [
                ("priority", "high"),
                ("sender", "user_service"),
                ("correlation_id", "12345"),
                ("timestamp", str(int(time.time())))
            ]
            
            for key, value in metadata_items:
                try:
                    set_metadata_func = getattr(message, 'set_metadata', None)
                    if set_metadata_func:
                        # Try different call signatures
                        try:
                            set_metadata_func({key: value})  # Dict format
                        except:
                            try:
                                set_metadata_func(key, value)  # Key-value format
                            except:
                                print(f"  ⚠️  Could not set metadata {key}: incompatible signature")
                                continue
                        print(f"  ✅ Set {key}: {value}")
                    else:
                        print(f"  ⚠️  set_metadata method not available")
                except Exception as e:
                    print(f"  ❌ Failed to set {key}: {e}")
        
        # Test metadata retrieval
        if hasattr(message, 'get_metadata'):
            print("\n📋 Retrieving message metadata...")
            
            for key, expected_value in metadata_items:
                try:
                    value = message.get_metadata(key)
                    if value == expected_value:
                        print(f"  ✅ {key}: {value}")
                    else:
                        print(f"  ⚠️  {key}: {value} (expected: {expected_value})")
                except Exception as e:
                    print(f"  ❌ Failed to get {key}: {e}")
        
        return message
        
    except Exception as e:
        print(f"❌ Message properties demonstration failed: {e}")
        return None


def demonstrate_message_priorities() -> None:
    """Demonstrate message priorities and delivery modes."""
    print("\n" + "="*50)
    print("⚡ Message Priorities and Delivery Modes")
    print("="*50)
    
    # Test MessagePriority enum
    if hasattr(comm, 'MessagePriority'):
        print("📊 Available message priorities:")
        priorities = ['Low', 'Normal', 'High', 'Critical']
        
        for priority in priorities:
            if hasattr(comm.MessagePriority, priority):
                value = getattr(comm.MessagePriority, priority)
                print(f"  • {priority}: {value}")
    
    # Test DeliveryMode enum
    if hasattr(comm, 'DeliveryMode'):
        print("\n🚚 Available delivery modes:")
        modes = ['Immediate', 'Queued', 'Persistent']
        
        for mode in modes:
            if hasattr(comm.DeliveryMode, mode):
                value = getattr(comm.DeliveryMode, mode)
                print(f"  • {mode}: {value}")
    
    # Demonstrate setting priorities and delivery modes
    if hasattr(comm, 'BasicMessage'):
        try:
            message = comm.BasicMessage("priority.test", "High priority message")
            
            # Set priority if supported
            if (hasattr(message, 'set_priority') and 
                hasattr(comm, 'MessagePriority') and 
                hasattr(comm.MessagePriority, 'High')):
                
                message.set_priority(comm.MessagePriority.High)
                print("✅ Set message priority to High")
                
                if hasattr(message, 'get_priority'):
                    priority = message.get_priority()
                    print(f"📊 Current priority: {priority}")
            
            # Set delivery mode if supported
            if (hasattr(message, 'set_delivery_mode') and 
                hasattr(comm, 'DeliveryMode') and 
                hasattr(comm.DeliveryMode, 'Persistent')):
                
                message.set_delivery_mode(comm.DeliveryMode.Persistent)
                print("✅ Set delivery mode to Persistent")
                
                if hasattr(message, 'get_delivery_mode'):
                    mode = message.get_delivery_mode()
                    print(f"📊 Current delivery mode: {mode}")
            
        except Exception as e:
            print(f"❌ Priority/delivery mode demonstration failed: {e}")


def demonstrate_service_contracts() -> None:
    """Demonstrate service contracts and discovery."""
    print("\n" + "="*50)
    print("📋 Service Contracts and Discovery")
    print("="*50)
    
    # Test ServiceVersion
    if hasattr(comm, 'ServiceVersion'):
        try:
            version = comm.ServiceVersion(1, 2, 3)
            print("✅ Created service version 1.2.3")
            
            if hasattr(version, 'major'):
                print(f"📊 Major version: {version.major}")
            if hasattr(version, 'minor'):
                print(f"📊 Minor version: {version.minor}")
            if hasattr(version, 'patch'):
                print(f"📊 Patch version: {version.patch}")
                
        except Exception as e:
            print(f"❌ Failed to create service version: {e}")
    
    # Test ServiceMethodDescriptor
    if hasattr(comm, 'ServiceMethodDescriptor'):
        try:
            descriptor = comm.ServiceMethodDescriptor()
            print("✅ Created service method descriptor")
            
            if hasattr(descriptor, 'name'):
                descriptor.name = "calculate"
                print(f"📋 Method name: {descriptor.name}")
                
        except Exception as e:
            print(f"❌ Failed to create method descriptor: {e}")
    
    # Test ServiceContract
    if hasattr(comm, 'ServiceContract'):
        try:
            contract = comm.ServiceContract()
            print("✅ Created service contract")
            
            # Test adding methods if supported
            if (hasattr(contract, 'add_method') and 
                hasattr(comm, 'ServiceMethodDescriptor')):
                
                method = comm.ServiceMethodDescriptor()
                if hasattr(method, 'name'):
                    method.name = "process_data"
                
                try:
                    contract.add_method(method)
                    print("✅ Added method to service contract")
                except Exception as e:
                    print(f"⚠️  Failed to add method: {e}")
            
            # Test contract validation
            if hasattr(contract, 'validate'):
                try:
                    is_valid = contract.validate()
                    print(f"📊 Contract validation result: {is_valid}")
                except Exception as e:
                    print(f"⚠️  Contract validation failed: {e}")
                    
        except Exception as e:
            print(f"❌ Failed to create service contract: {e}")
    
    # Test ServiceCapability enum
    if hasattr(comm, 'ServiceCapability'):
        print("\n🛠️  Available service capabilities:")
        capabilities = ['Synchronous', 'Asynchronous', 'Streaming', 'Transactional']
        
        for capability in capabilities:
            if hasattr(comm.ServiceCapability, capability):
                value = getattr(comm.ServiceCapability, capability)
                print(f"  • {capability}: {value}")


def demonstrate_request_response() -> None:
    """Demonstrate request-response communication patterns."""
    print("\n" + "="*50)
    print("🔄 Request-Response Communication")
    print("="*50)
    
    # Test Request creation
    if hasattr(comm, 'Request'):
        try:
            request = comm.Request("calculator_service", "add")
            print("✅ Created request for calculator_service.add")
            
            if hasattr(request, 'service'):
                print(f"📋 Service: {request.service}")
            if hasattr(request, 'method'):
                print(f"📋 Method: {request.method}")
                
        except Exception as e:
            print(f"❌ Failed to create request: {e}")
    
    # Test Response creation
    if hasattr(comm, 'Response'):
        try:
            response = comm.Response()
            print("✅ Created response object")
            
            # Test setting result
            if hasattr(response, 'set_result'):
                response.set_result({"sum": 42, "status": "success"})
                print("✅ Set response result")
                
                if hasattr(response, 'get_result'):
                    result = response.get_result()
                    print(f"📊 Response result: {result}")
                    
        except Exception as e:
            print(f"❌ Failed to create response: {e}")
    
    # Demonstrate request-response with message bus
    bus = comm.create_message_bus() if hasattr(comm, 'create_message_bus') else None
    
    if bus and hasattr(bus, 'send_request') and hasattr(comm, 'Request'):
        print("\n🔄 Testing request-response with message bus...")
        
        try:
            request = comm.Request("echo_service", "echo")
            
            # Test with timeout
            response = bus.send_request(request, timeout=1.0)
            
            if response:
                print("✅ Received response from service")
                if hasattr(response, 'get_result'):
                    result = response.get_result()
                    print(f"📊 Response data: {result}")
            else:
                print("⚠️  No response received (service may not be available)")
                
        except Exception as e:
            print(f"⚠️  Request-response failed: {e} (this is expected if no service is running)")


def demonstrate_async_communication() -> None:
    """Demonstrate asynchronous communication patterns."""
    print("\n" + "="*50)
    print("🔀 Asynchronous Communication")
    print("="*50)
    
    bus = comm.create_message_bus() if hasattr(comm, 'create_message_bus') else None
    
    if not bus:
        print("❌ No message bus available for async demo")
        return
    
    # Test async request-response
    if hasattr(bus, 'send_request_async') and hasattr(comm, 'Request'):
        print("🔄 Testing asynchronous request-response...")
        
        try:
            request = comm.Request("async_service", "process")
            future = bus.send_request_async(request)
            
            if future:
                print("✅ Async request sent, future received")
                
                # Test waiting for result
                if hasattr(future, 'wait'):
                    try:
                        result = future.wait(timeout=1.0)
                        print(f"📊 Async result: {result}")
                    except Exception as e:
                        print(f"⚠️  Async wait failed: {e}")
                        
        except Exception as e:
            print(f"⚠️  Async request failed: {e}")
    
    # Demonstrate threaded message handling
    print("\n🧵 Testing threaded message handling...")
    
    messages_received = []
    
    def threaded_subscriber(topic_suffix) -> None:
        """Function to run in separate thread for message handling."""
        topic = f"async.test.{topic_suffix}"
        
        def callback(message) -> None:
            messages_received.append(f"Thread-{topic_suffix}: {message}")
        
        try:
            if hasattr(bus, 'subscribe'):
                bus.subscribe(topic, callback)
                time.sleep(0.5)  # Wait for messages
        except Exception as e:
            print(f"⚠️  Thread {topic_suffix} subscription failed: {e}")
    
    # Start multiple subscriber threads
    threads = []
    for i in range(3):
        thread = threading.Thread(target=threaded_subscriber, args=(i,))
        threads.append(thread)
        thread.start()
    
    # Give threads time to subscribe
    time.sleep(0.2)
    
    # Publish messages to different topics
    if hasattr(comm, 'BasicMessage'):
        for i in range(3):
            topic = f"async.test.{i}"
            try:
                message = comm.BasicMessage(topic, f"Message for thread {i}")
                publish_func = getattr(bus, 'publish', None)
                if publish_func:
                    publish_func(message)
                    print(f"📤 Published to {topic}")
                else:
                    print(f"⚠️  Publish method not available for {topic}")
            except Exception as e:
                print(f"❌ Failed to publish to {topic}: {e}")
    
    # Wait for threads to complete
    for thread in threads:
        thread.join(timeout=2.0)
    
    print(f"📊 Total messages received by threads: {len(messages_received)}")
    for msg in messages_received:
        print(f"  📥 {msg}")


def main() -> None:
    """Main demonstration function."""
    print("QtForge Python Bindings - Communication and Messaging Example")
    print("=" * 65)
    
    # Demonstrate each aspect of communication
    bus = demonstrate_message_bus_creation()
    demonstrate_basic_messaging(bus)
    demonstrate_message_properties()
    demonstrate_message_priorities()
    demonstrate_service_contracts()
    demonstrate_request_response()
    demonstrate_async_communication()
    
    print("\n" + "="*65)
    print("🎉 Communication and Messaging Example Complete!")
    print("="*65)
    
    print("\n📚 Key Takeaways:")
    print("• Message buses enable decoupled inter-plugin communication")
    print("• Service contracts define clear interfaces between components")
    print("• Request-response patterns support synchronous interactions")
    print("• Message priorities and delivery modes control message handling")
    print("• Asynchronous patterns improve application responsiveness")
    
    print("\n🔗 Next Steps:")
    print("• Implement custom message handlers and filters")
    print("• Create service discovery and registration mechanisms")
    print("• Add message persistence and reliability features")
    print("• Explore advanced routing and transformation patterns")


if __name__ == "__main__":
    main()
