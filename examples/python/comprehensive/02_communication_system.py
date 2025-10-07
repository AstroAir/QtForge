#!/usr/bin/env python3
"""
QtForge Python Bindings Example: Communication System

This example demonstrates the communication system features using
the QtForge Python bindings. It covers:

1. Creating and configuring MessageBus
2. Publishing and subscribing to messages
3. Request/Response patterns
4. Service contracts and communication protocols
5. Message serialization and deserialization
6. Error handling in communication

Prerequisites:
- QtForge built with Python bindings enabled
- Communication module available
- Python 3.8 or later
"""

import sys
import os
import json
import time
import threading
from pathlib import Path
from typing import Any, Optional, List, Dict

# Add the build directory to Python path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '../../../build'))

try:
    import qtforge
    import qtforge.communication as comm
    print("✅ QtForge communication bindings loaded successfully")
except ImportError as e:
    print(f"❌ Failed to import QtForge communication: {e}")
    print("Make sure QtForge is built with communication bindings enabled")
    sys.exit(1)


class CommunicationExample:
    """Comprehensive example of communication system operations."""
    
    def __init__(self) -> None:
        """Initialize the communication example."""
        self.message_bus: Optional[Any] = None
        self.messages_received: List[Any] = []
        self.setup_components()
    
    def setup_components(self) -> None:
        """Set up the communication components."""
        print("\n🔧 Setting up communication components...")
        
        try:
            # Create MessageBus
            if hasattr(comm, 'MessageBus'):
                self.message_bus = comm.MessageBus()
                print("✅ MessageBus created")
            elif hasattr(comm, 'create_message_bus'):
                self.message_bus = comm.create_message_bus()
                print("✅ MessageBus created via factory function")
            else:
                raise RuntimeError("MessageBus not available")
            
        except Exception as e:
            print(f"❌ Failed to setup communication components: {e}")
            raise
    
    def demonstrate_message_creation(self) -> List[Any]:
        """Demonstrate message creation and manipulation."""
        print("\n📨 Demonstrating message creation...")
        
        # Create messages using different methods
        messages = []
        
        # Method 1: Direct Message creation
        if hasattr(comm, 'Message'):
            try:
                message1 = comm.Message()
                print("✅ Message created using Message()")
                
                # Set message properties if available
                if hasattr(message1, 'topic'):
                    message1.topic = "example.topic"
                    print(f"Message topic set: {message1.topic}")
                
                if hasattr(message1, 'payload'):
                    message1.payload = {"key": "value", "number": 42}
                    print(f"Message payload set: {message1.payload}")
                
                messages.append(message1)
                
            except Exception as e:
                print(f"Message creation error: {e}")
        
        # Method 2: Factory function
        if hasattr(comm, 'create_message'):
            try:
                message2 = comm.create_message("factory.topic", {"data": "from_factory"})
                print("✅ Message created using create_message()")
                messages.append(message2)
                
            except Exception as e:
                print(f"Factory message creation error: {e}")
        
        # Test message serialization
        for i, message in enumerate(messages):
            if hasattr(message, 'to_json'):
                try:
                    json_data = message.to_json()
                    print(f"Message {i+1} serialized: {type(json_data)}")
                except Exception as e:
                    print(f"Message {i+1} serialization error: {e}")
        
        return messages
    
    def demonstrate_message_publishing(self) -> None:
        """Demonstrate message publishing."""
        print("\n📤 Demonstrating message publishing...")
        
        if not self.message_bus:
            print("⚠️  MessageBus not available")
            return
        
        # Create test messages
        test_messages = [
            {"topic": "test.basic", "payload": {"message": "Hello World"}},
            {"topic": "test.data", "payload": {"numbers": [1, 2, 3], "text": "test"}},
            {"topic": "test.event", "payload": {"event": "user_action", "timestamp": time.time()}},
        ]
        
        for msg_data in test_messages:
            try:
                # Create message
                message = None
                if hasattr(comm, 'create_message'):
                    message = comm.create_message(msg_data["topic"], msg_data["payload"])
                elif hasattr(comm, 'Message'):
                    message = comm.Message()
                    if hasattr(message, 'topic'):
                        message.topic = msg_data["topic"]
                    if hasattr(message, 'payload'):
                        message.payload = msg_data["payload"]
                
                if message and hasattr(self.message_bus, 'publish'):
                    result = self.message_bus.publish(message)
                    print(f"Published message to '{msg_data['topic']}': {result}")
                else:
                    print(f"⚠️  Cannot publish message to '{msg_data['topic']}'")
                    
            except Exception as e:
                print(f"Publishing error for '{msg_data['topic']}': {e}")
    
    def demonstrate_message_subscription(self) -> None:
        """Demonstrate message subscription."""
        print("\n📥 Demonstrating message subscription...")
        
        if not self.message_bus:
            print("⚠️  MessageBus not available")
            return
        
        # Define callback function
        def message_callback(message) -> None:
            """Callback function for received messages."""
            try:
                topic = getattr(message, 'topic', 'unknown') if hasattr(message, 'topic') else 'unknown'
                payload = getattr(message, 'payload', {}) if hasattr(message, 'payload') else {}
                
                self.messages_received.append({
                    'topic': topic,
                    'payload': payload,
                    'timestamp': time.time()
                })
                
                print(f"📨 Received message on '{topic}': {payload}")
                
            except Exception as e:
                print(f"Callback error: {e}")
        
        # Subscribe to topics
        topics_to_subscribe = ["test.basic", "test.data", "test.event", "test.*"]
        
        for topic in topics_to_subscribe:
            try:
                if hasattr(self.message_bus, 'subscribe'):
                    result = self.message_bus.subscribe(topic, message_callback)
                    print(f"Subscribed to '{topic}': {result}")
                else:
                    print(f"⚠️  Cannot subscribe to '{topic}' - method not available")
                    
            except Exception as e:
                print(f"Subscription error for '{topic}': {e}")
    
    def demonstrate_request_response(self) -> None:
        """Demonstrate request/response patterns."""
        print("\n🔄 Demonstrating request/response patterns...")
        
        # Create requests
        requests = []
        
        # Method 1: Direct Request creation
        if hasattr(comm, 'Request'):
            try:
                request1 = comm.Request()
                print("✅ Request created using Request()")
                
                if hasattr(request1, 'method'):
                    request1.method = "get_user_info"
                if hasattr(request1, 'params'):
                    request1.params = {"user_id": 123}
                
                requests.append(request1)
                
            except Exception as e:
                print(f"Request creation error: {e}")
        
        # Method 2: Factory function
        if hasattr(comm, 'create_request'):
            try:
                request2 = comm.create_request("calculate_sum", {"numbers": [1, 2, 3, 4, 5]})
                print("✅ Request created using create_request()")
                requests.append(request2)
                
            except Exception as e:
                print(f"Factory request creation error: {e}")
        
        # Create responses
        if hasattr(comm, 'Response'):
            try:
                response = comm.Response()
                print("✅ Response created")
                
                if hasattr(response, 'result'):
                    response.result = {"status": "success", "data": "response_data"}
                if hasattr(response, 'error'):
                    response.error = None
                
            except Exception as e:
                print(f"Response creation error: {e}")
        
        # Test request/response serialization
        for i, request in enumerate(requests):
            if hasattr(request, 'to_json'):
                try:
                    json_data = request.to_json()
                    print(f"Request {i+1} serialized: {type(json_data)}")
                except Exception as e:
                    print(f"Request {i+1} serialization error: {e}")
    
    def demonstrate_service_contracts(self) -> None:
        """Demonstrate service contracts."""
        print("\n📋 Demonstrating service contracts...")
        
        # Create service contract
        if hasattr(comm, 'ServiceContract'):
            try:
                contract = comm.ServiceContract()
                print("✅ ServiceContract created")
                
                # Configure contract properties
                if hasattr(contract, 'service_name'):
                    contract.service_name = "UserService"
                    print(f"Service name: {contract.service_name}")
                
                if hasattr(contract, 'version'):
                    contract.version = "1.0.0"  # type: ignore
                    print(f"Service version: {contract.version}")
                
                if hasattr(contract, 'description'):
                    contract.description = "User management service"
                    print(f"Service description: {contract.description}")
                
            except Exception as e:
                print(f"ServiceContract error: {e}")
        
        elif hasattr(comm, 'create_service_contract'):
            try:
                contract = comm.create_service_contract()  # type: ignore
                print("✅ ServiceContract created via factory")
                
            except Exception as e:
                print(f"ServiceContract factory error: {e}")
        
        else:
            print("⚠️  ServiceContract not available")
    
    def demonstrate_message_bus_operations(self) -> None:
        """Demonstrate advanced MessageBus operations."""
        print("\n🚌 Demonstrating MessageBus operations...")
        
        if not self.message_bus:
            print("⚠️  MessageBus not available")
            return
        
        # Get topics
        if hasattr(self.message_bus, 'get_topics'):
            try:
                topics = self.message_bus.get_topics()
                print(f"Available topics: {topics}")
            except Exception as e:
                print(f"Get topics error: {e}")
        
        # Get subscriber count
        if hasattr(self.message_bus, 'get_subscriber_count'):
            try:
                count = self.message_bus.get_subscriber_count("test.basic")
                print(f"Subscribers for 'test.basic': {count}")
            except Exception as e:
                print(f"Get subscriber count error: {e}")
        
        # Test unsubscription
        if hasattr(self.message_bus, 'unsubscribe'):
            try:
                result = self.message_bus.unsubscribe("test.basic")
                print(f"Unsubscribed from 'test.basic': {result}")
            except Exception as e:
                print(f"Unsubscribe error: {e}")
    
    def demonstrate_threading_safety(self) -> None:
        """Demonstrate thread-safe communication operations."""
        print("\n🧵 Demonstrating threading safety...")
        
        if not self.message_bus:
            print("⚠️  MessageBus not available")
            return
        
        results = []
        errors = []
        
        def worker_thread(thread_id) -> None:
            """Worker thread for testing thread safety."""
            try:
                # Create and publish messages from multiple threads
                for i in range(5):
                    if hasattr(comm, 'create_message'):
                        message = comm.create_message(
                            f"thread.{thread_id}",
                            {"thread_id": thread_id, "message_id": i, "data": f"data_{i}"}
                        )
                        
                        if hasattr(self.message_bus, 'publish'):
                            result = self.message_bus.publish(message)  # type: ignore
                            results.append(f"Thread {thread_id}, Message {i}: {result}")
                    
                    time.sleep(0.01)  # Small delay
                    
            except Exception as e:
                errors.append(f"Thread {thread_id} error: {e}")
        
        # Start multiple threads
        threads = []
        for i in range(3):
            thread = threading.Thread(target=worker_thread, args=(i,))
            threads.append(thread)
            thread.start()
        
        # Wait for all threads to complete
        for thread in threads:
            thread.join(timeout=5)
        
        print(f"Threading test results: {len(results)} successful operations")
        if errors:
            print(f"Threading errors: {errors}")
        else:
            print("✅ No threading errors detected")
    
    def demonstrate_error_handling(self) -> None:
        """Demonstrate communication error handling."""
        print("\n⚠️  Demonstrating communication error handling...")
        
        # Test various error conditions
        error_tests = [
            ("Publishing null message", lambda: self.message_bus.publish(None) if self.message_bus and hasattr(self.message_bus, 'publish') else None),
            ("Subscribing with null callback", lambda: self.message_bus.subscribe("test", None) if self.message_bus and hasattr(self.message_bus, 'subscribe') else None),
            ("Unsubscribing non-existent topic", lambda: self.message_bus.unsubscribe("non.existent") if self.message_bus and hasattr(self.message_bus, 'unsubscribe') else None),
        ]
        
        for test_name, test_func in error_tests:
            print(f"\nTesting: {test_name}")
            try:
                result = test_func()
                print(f"Result: {result}")
            except Exception as e:
                print(f"Exception caught: {type(e).__name__}: {e}")
    
    def cleanup(self) -> None:
        """Clean up communication resources."""
        print("\n🧹 Cleaning up communication resources...")
        
        # Unsubscribe from all topics if possible
        if self.message_bus and hasattr(self.message_bus, 'unsubscribe'):
            topics_to_cleanup = ["test.basic", "test.data", "test.event", "test.*"]
            for topic in topics_to_cleanup:
                try:
                    self.message_bus.unsubscribe(topic)
                except Exception:
                    pass  # Ignore cleanup errors
        
        print("✅ Communication cleanup completed")
    
    def run_complete_example(self) -> int:
        """Run the complete communication example."""
        print("🚀 QtForge Python Bindings - Communication System Example")
        print("=" * 70)
        
        try:
            messages = self.demonstrate_message_creation()
            self.demonstrate_message_subscription()
            self.demonstrate_message_publishing()
            self.demonstrate_request_response()
            self.demonstrate_service_contracts()
            self.demonstrate_message_bus_operations()
            self.demonstrate_threading_safety()
            self.demonstrate_error_handling()
            
            # Show received messages summary
            print(f"\n📊 Summary: Received {len(self.messages_received)} messages")
            for msg in self.messages_received[-3:]:  # Show last 3 messages
                print(f"  - {msg['topic']}: {msg['payload']}")
            
        except Exception as e:
            print(f"\n❌ Communication example failed with error: {e}")
            return 1
        
        finally:
            self.cleanup()
        
        print("\n✅ Communication system example completed successfully!")
        return 0


def main() -> int:
    """Main entry point for the example."""
    try:
        example = CommunicationExample()
        return example.run_complete_example()
    except Exception as e:
        print(f"❌ Failed to run communication example: {e}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
