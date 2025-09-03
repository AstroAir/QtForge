#!/usr/bin/env lua
--[[
QtForge Lua Bindings Example 2: Communication and Messaging

This example demonstrates inter-plugin communication using the QtForge
message bus system, including:
- Creating and configuring message buses
- Publishing and subscribing to messages
- Service contracts and discovery
- Request-response patterns
- Message priorities and delivery modes
--]]

-- Helper functions for output formatting
local function print_header(title)
    print("\n" .. string.rep("=", 50))
    print("📡 " .. title)
    print(string.rep("=", 50))
end

local function print_success(message)
    print("✅ " .. message)
end

local function print_warning(message)
    print("⚠️  " .. message)
end

local function print_error(message)
    print("❌ " .. message)
end

local function print_info(message)
    print("📋 " .. message)
end

local function print_status(message)
    print("📊 " .. message)
end

-- Check if QtForge communication bindings are available
local function check_communication_availability()
    if not qtforge then
        print_error("QtForge Lua bindings not available")
        return false
    end
    
    if not qtforge.communication then
        print_error("QtForge Communication module not available")
        return false
    end
    
    print_success("QtForge Communication bindings loaded successfully")
    return true
end

-- Demonstrate message bus creation and configuration
local function demonstrate_message_bus_creation()
    print_header("Creating Message Bus")
    
    if not qtforge.communication.create_message_bus then
        print_error("Message bus creation not available")
        return nil
    end
    
    local success, bus = pcall(qtforge.communication.create_message_bus)
    
    if not success then
        print_error("Failed to create message bus: " .. tostring(bus))
        return nil
    end
    
    print_success("Message bus created successfully")
    
    -- Check available methods
    local methods = {"publish", "subscribe", "unsubscribe"}
    local available_methods = {}
    
    for _, method in ipairs(methods) do
        if bus[method] then
            table.insert(available_methods, method)
        end
    end
    
    print_info("Available methods: " .. table.concat(available_methods, ", "))
    
    return bus
end

-- Demonstrate basic publish/subscribe messaging
local function demonstrate_basic_messaging(bus)
    print_header("Basic Publish/Subscribe Messaging")
    
    if not bus then
        print_error("No message bus available")
        return
    end
    
    -- Storage for received messages
    local received_messages = {}
    
    -- Callback function for received messages
    local function message_callback(message)
        print("📥 Received message: " .. tostring(message))
        table.insert(received_messages, message)
    end
    
    -- Subscribe to a topic
    local topic = "example.notifications"
    print("🔔 Subscribing to topic: " .. topic)
    
    if bus.subscribe then
        local sub_success, sub_result = pcall(function()
            return bus:subscribe(topic, message_callback)
        end)
        
        if sub_success then
            print_success("Successfully subscribed to topic")
        else
            print_warning("Subscription failed: " .. tostring(sub_result))
            return
        end
    else
        print_warning("Subscribe method not available")
        return
    end
    
    -- Create and publish messages
    if qtforge.communication.BasicMessage then
        print("\n📤 Publishing messages to topic: " .. topic)
        
        local messages = {
            "Hello, World!",
            "This is a test message",
            {type = "notification", data = "Important update"},
            42  -- Test with different data types
        }
        
        for i, msg_data in ipairs(messages) do
            local msg_success, message = pcall(function()
                return qtforge.communication.BasicMessage(topic, msg_data)
            end)
            
            if msg_success then
                local pub_success, pub_result = pcall(function()
                    return bus:publish(message)
                end)
                
                if pub_success then
                    print("  ✅ Published message " .. i .. ": " .. tostring(msg_data))
                else
                    print("  ❌ Failed to publish message " .. i .. ": " .. tostring(pub_result))
                end
            else
                print("  ❌ Failed to create message " .. i .. ": " .. tostring(message))
            end
            
            -- Small delay for processing
            os.execute("sleep 0.1")
        end
    else
        print_warning("BasicMessage class not available")
    end
    
    -- Give some time for message processing
    os.execute("sleep 0.5")
    
    print_status("Total messages received: " .. #received_messages)
    
    -- Unsubscribe
    if bus.unsubscribe then
        local unsub_success, unsub_result = pcall(function()
            return bus:unsubscribe(topic, message_callback)
        end)
        
        if unsub_success then
            print_success("Successfully unsubscribed from topic")
        else
            print_warning("Unsubscription failed: " .. tostring(unsub_result))
        end
    end
end

-- Demonstrate message properties and metadata
local function demonstrate_message_properties()
    print_header("Message Properties and Metadata")
    
    if not qtforge.communication.BasicMessage then
        print_error("BasicMessage class not available")
        return
    end
    
    local topic = "example.properties"
    local data = {user = "alice", action = "login", timestamp = os.time()}
    
    local success, message = pcall(function()
        return qtforge.communication.BasicMessage(topic, data)
    end)
    
    if not success then
        print_error("Failed to create message: " .. tostring(message))
        return
    end
    
    print_success("Created message with basic properties")
    
    -- Test property access
    if message.topic then
        print_info("Message topic: " .. tostring(message.topic))
    end
    
    if message.data then
        print_info("Message data: " .. tostring(message.data))
    end
    
    -- Test metadata operations
    if message.set_metadata then
        print("\n🏷️  Setting message metadata...")
        
        local metadata_items = {
            {"priority", "high"},
            {"sender", "user_service"},
            {"correlation_id", "12345"},
            {"timestamp", tostring(os.time())}
        }
        
        for _, item in ipairs(metadata_items) do
            local key, value = item[1], item[2]
            
            local meta_success, meta_result = pcall(function()
                return message:set_metadata(key, value)
            end)
            
            if meta_success then
                print("  ✅ Set " .. key .. ": " .. value)
            else
                print("  ❌ Failed to set " .. key .. ": " .. tostring(meta_result))
            end
        end
    end
    
    -- Test metadata retrieval
    if message.get_metadata then
        print("\n📋 Retrieving message metadata...")
        
        local metadata_keys = {"priority", "sender", "correlation_id", "timestamp"}
        
        for _, key in ipairs(metadata_keys) do
            local get_success, value = pcall(function()
                return message:get_metadata(key)
            end)
            
            if get_success then
                if value then
                    print("  ✅ " .. key .. ": " .. tostring(value))
                else
                    print("  ⚠️  " .. key .. ": not set")
                end
            else
                print("  ❌ Failed to get " .. key .. ": " .. tostring(value))
            end
        end
    end
    
    return message
end

-- Demonstrate message priorities and delivery modes
local function demonstrate_message_priorities()
    print_header("Message Priorities and Delivery Modes")
    
    -- Test MessagePriority enum
    if MessagePriority then
        print("📊 Available message priorities:")
        local priorities = {"Low", "Normal", "High", "Critical"}
        
        for _, priority in ipairs(priorities) do
            if MessagePriority[priority] then
                local value = MessagePriority[priority]
                print("  • " .. priority .. ": " .. tostring(value))
            end
        end
    end
    
    -- Test DeliveryMode enum
    if DeliveryMode then
        print("\n🚚 Available delivery modes:")
        local modes = {"Immediate", "Queued", "Persistent"}
        
        for _, mode in ipairs(modes) do
            if DeliveryMode[mode] then
                local value = DeliveryMode[mode]
                print("  • " .. mode .. ": " .. tostring(value))
            end
        end
    end
    
    -- Demonstrate setting priorities and delivery modes
    if qtforge.communication.BasicMessage then
        local success, message = pcall(function()
            return qtforge.communication.BasicMessage("priority.test", "High priority message")
        end)
        
        if success then
            -- Set priority if supported
            if message.set_priority and MessagePriority and MessagePriority.High then
                local priority_success, priority_result = pcall(function()
                    return message:set_priority(MessagePriority.High)
                end)
                
                if priority_success then
                    print_success("Set message priority to High")
                    
                    if message.get_priority then
                        local get_success, priority = pcall(function()
                            return message:get_priority()
                        end)
                        
                        if get_success then
                            print_status("Current priority: " .. tostring(priority))
                        end
                    end
                else
                    print_warning("Failed to set priority: " .. tostring(priority_result))
                end
            end
            
            -- Set delivery mode if supported
            if message.set_delivery_mode and DeliveryMode and DeliveryMode.Persistent then
                local mode_success, mode_result = pcall(function()
                    return message:set_delivery_mode(DeliveryMode.Persistent)
                end)
                
                if mode_success then
                    print_success("Set delivery mode to Persistent")
                    
                    if message.get_delivery_mode then
                        local get_success, mode = pcall(function()
                            return message:get_delivery_mode()
                        end)
                        
                        if get_success then
                            print_status("Current delivery mode: " .. tostring(mode))
                        end
                    end
                else
                    print_warning("Failed to set delivery mode: " .. tostring(mode_result))
                end
            end
        else
            print_error("Failed to create message for priority/delivery mode demo: " .. tostring(message))
        end
    end
end

-- Demonstrate service contracts and discovery
local function demonstrate_service_contracts()
    print_header("Service Contracts and Discovery")
    
    -- Test ServiceVersion
    if qtforge.communication.ServiceVersion then
        local version_success, version = pcall(function()
            return qtforge.communication.ServiceVersion(1, 2, 3)
        end)
        
        if version_success then
            print_success("Created service version 1.2.3")
            
            if version.major then
                print_status("Major version: " .. tostring(version.major))
            end
            if version.minor then
                print_status("Minor version: " .. tostring(version.minor))
            end
            if version.patch then
                print_status("Patch version: " .. tostring(version.patch))
            end
        else
            print_error("Failed to create service version: " .. tostring(version))
        end
    end
    
    -- Test ServiceMethodDescriptor
    if qtforge.communication.ServiceMethodDescriptor then
        local desc_success, descriptor = pcall(qtforge.communication.ServiceMethodDescriptor)
        
        if desc_success then
            print_success("Created service method descriptor")
            
            if descriptor.name then
                descriptor.name = "calculate"
                print_info("Method name: " .. tostring(descriptor.name))
            end
        else
            print_error("Failed to create method descriptor: " .. tostring(descriptor))
        end
    end
    
    -- Test ServiceContract
    if qtforge.communication.ServiceContract then
        local contract_success, contract = pcall(qtforge.communication.ServiceContract)
        
        if contract_success then
            print_success("Created service contract")
            
            -- Test adding methods if supported
            if contract.add_method and qtforge.communication.ServiceMethodDescriptor then
                local method_success, method = pcall(qtforge.communication.ServiceMethodDescriptor)
                
                if method_success and method.name then
                    method.name = "process_data"
                    
                    local add_success, add_result = pcall(function()
                        return contract:add_method(method)
                    end)
                    
                    if add_success then
                        print_success("Added method to service contract")
                    else
                        print_warning("Failed to add method: " .. tostring(add_result))
                    end
                end
            end
            
            -- Test contract validation
            if contract.validate then
                local validate_success, is_valid = pcall(function()
                    return contract:validate()
                end)
                
                if validate_success then
                    print_status("Contract validation result: " .. tostring(is_valid))
                else
                    print_warning("Contract validation failed: " .. tostring(is_valid))
                end
            end
        else
            print_error("Failed to create service contract: " .. tostring(contract))
        end
    end
    
    -- Test ServiceCapability enum
    if ServiceCapability then
        print("\n🛠️  Available service capabilities:")
        local capabilities = {"Synchronous", "Asynchronous", "Streaming", "Transactional"}
        
        for _, capability in ipairs(capabilities) do
            if ServiceCapability[capability] then
                local value = ServiceCapability[capability]
                print("  • " .. capability .. ": " .. tostring(value))
            end
        end
    end
end

-- Demonstrate request-response communication patterns
local function demonstrate_request_response()
    print_header("Request-Response Communication")
    
    -- Test Request creation
    if qtforge.communication.Request then
        local req_success, request = pcall(function()
            return qtforge.communication.Request("calculator_service", "add")
        end)
        
        if req_success then
            print_success("Created request for calculator_service.add")
            
            if request.service then
                print_info("Service: " .. tostring(request.service))
            end
            if request.method then
                print_info("Method: " .. tostring(request.method))
            end
        else
            print_error("Failed to create request: " .. tostring(request))
        end
    end
    
    -- Test Response creation
    if qtforge.communication.Response then
        local resp_success, response = pcall(qtforge.communication.Response)
        
        if resp_success then
            print_success("Created response object")
            
            -- Test setting result
            if response.set_result then
                local result_data = {sum = 42, status = "success"}
                
                local set_success, set_result = pcall(function()
                    return response:set_result(result_data)
                end)
                
                if set_success then
                    print_success("Set response result")
                    
                    if response.get_result then
                        local get_success, result = pcall(function()
                            return response:get_result()
                        end)
                        
                        if get_success then
                            print_status("Response result: " .. tostring(result))
                        end
                    end
                else
                    print_warning("Failed to set result: " .. tostring(set_result))
                end
            end
        else
            print_error("Failed to create response: " .. tostring(response))
        end
    end
    
    -- Demonstrate request-response with message bus
    local bus = nil
    if qtforge.communication.create_message_bus then
        local bus_success, bus_result = pcall(qtforge.communication.create_message_bus)
        if bus_success then
            bus = bus_result
        end
    end
    
    if bus and bus.send_request and qtforge.communication.Request then
        print("\n🔄 Testing request-response with message bus...")
        
        local req_success, request = pcall(function()
            return qtforge.communication.Request("echo_service", "echo")
        end)
        
        if req_success then
            local send_success, response = pcall(function()
                return bus:send_request(request, 1.0)  -- 1 second timeout
            end)
            
            if send_success then
                if response then
                    print_success("Received response from service")
                    if response.get_result then
                        local result_success, result = pcall(function()
                            return response:get_result()
                        end)
                        
                        if result_success then
                            print_status("Response data: " .. tostring(result))
                        end
                    end
                else
                    print_warning("No response received (service may not be available)")
                end
            else
                print_warning("Request-response failed: " .. tostring(response) .. " (this is expected if no service is running)")
            end
        end
    end
end

-- Demonstrate communication enumerations
local function demonstrate_communication_enums()
    print_header("Communication Enumerations")
    
    -- DeliveryMode enum
    if DeliveryMode then
        print("🚚 Delivery Modes:")
        local modes = {"Immediate", "Queued", "Persistent"}
        
        for _, mode in ipairs(modes) do
            if DeliveryMode[mode] then
                local value = DeliveryMode[mode]
                print("  • " .. mode .. ": " .. tostring(value))
            end
        end
    end
    
    -- MessagePriority enum
    if MessagePriority then
        print("\n⚡ Message Priorities:")
        local priorities = {"Low", "Normal", "High", "Critical"}
        
        for _, priority in ipairs(priorities) do
            if MessagePriority[priority] then
                local value = MessagePriority[priority]
                print("  • " .. priority .. ": " .. tostring(value))
            end
        end
    end
    
    -- ServiceCapability enum
    if ServiceCapability then
        print("\n🛠️  Service Capabilities:")
        local capabilities = {"Synchronous", "Asynchronous", "Streaming", "Transactional"}
        
        for _, capability in ipairs(capabilities) do
            if ServiceCapability[capability] then
                local value = ServiceCapability[capability]
                print("  • " .. capability .. ": " .. tostring(value))
            end
        end
    end
end

-- Demonstrate error handling in communication operations
local function demonstrate_error_handling()
    print_header("Communication Error Handling")
    
    -- Test various error conditions
    if qtforge.communication.create_message_bus then
        local success, bus = pcall(qtforge.communication.create_message_bus)
        
        if success then
            -- Test publishing None message
            print("🧪 Testing error handling with invalid parameters...")
            
            local pub_success, pub_result = pcall(function()
                return bus:publish(nil)
            end)
            
            if pub_success then
                print_warning("Unexpected success with nil message: " .. tostring(pub_result))
            else
                print_success("Correctly caught error for nil message: " .. tostring(pub_result))
            end
            
            -- Test subscribing with invalid topic
            local sub_success, sub_result = pcall(function()
                return bus:subscribe("", function() end)
            end)
            
            if sub_success then
                print_warning("Unexpected success with empty topic: " .. tostring(sub_result))
            else
                print_success("Correctly caught error for empty topic: " .. tostring(sub_result))
            end
        end
    end
    
    -- Test service not found error
    if qtforge.communication.create_message_bus and qtforge.communication.Request then
        local bus_success, bus = pcall(qtforge.communication.create_message_bus)
        
        if bus_success and bus.send_request then
            print("\n🧪 Testing service not found error...")
            
            local req_success, request = pcall(function()
                return qtforge.communication.Request("non_existent_service", "test_method")
            end)
            
            if req_success then
                local send_success, response = pcall(function()
                    return bus:send_request(request, 0.1)  -- Very short timeout
                end)
                
                if send_success then
                    if response and response.is_error then
                        local error_success, is_error = pcall(function()
                            return response:is_error()
                        end)
                        
                        if error_success and is_error then
                            print_success("Correctly received error response")
                        end
                    else
                        print_warning("No response for non-existent service (expected)")
                    end
                else
                    print_success("Correctly caught exception for non-existent service: " .. tostring(response))
                end
            end
        end
    end
end

-- Main demonstration function
local function main()
    print("QtForge Lua Bindings - Communication and Messaging Example")
    print(string.rep("=", 65))
    
    -- Check if communication bindings are available
    if not check_communication_availability() then
        return 1
    end
    
    -- Demonstrate each aspect of communication
    local bus = demonstrate_message_bus_creation()
    demonstrate_basic_messaging(bus)
    demonstrate_message_properties()
    demonstrate_message_priorities()
    demonstrate_service_contracts()
    demonstrate_request_response()
    demonstrate_communication_enums()
    demonstrate_error_handling()
    
    print("\n" .. string.rep("=", 65))
    print("🎉 Communication and Messaging Example Complete!")
    print(string.rep("=", 65))
    
    print("\n📚 Key Takeaways:")
    print("• Message buses enable decoupled inter-plugin communication")
    print("• Service contracts define clear interfaces between components")
    print("• Request-response patterns support synchronous interactions")
    print("• Message priorities and delivery modes control message handling")
    print("• Proper error handling ensures robust communication")
    
    print("\n🔗 Next Steps:")
    print("• Implement custom message handlers and filters")
    print("• Create service discovery and registration mechanisms")
    print("• Add message persistence and reliability features")
    print("• Explore advanced routing and transformation patterns")
    
    return 0
end

-- Run the main function
return main()
