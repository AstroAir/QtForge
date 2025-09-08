/**
 * @file communication_bindings.cpp
 * @brief Communication system Python bindings
 * @version 3.2.0
 * @author QtForge Development Team
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/functional.h>

#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/communication/message_types.hpp>
#include <qtplugin/communication/plugin_service_contracts.hpp>
#include <qtplugin/communication/request_response_system.hpp>

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_communication(py::module& m) {
    // === Message Priority Enum ===
    py::enum_<MessagePriority>(m, "MessagePriority", "Message priority levels")
        .value("Low", MessagePriority::Low, "Low priority message")
        .value("Normal", MessagePriority::Normal, "Normal priority message")
        .value("High", MessagePriority::High, "High priority message")
        .value("Critical", MessagePriority::Critical, "Critical priority message")
        .export_values();

    // === Delivery Mode Enum ===
    py::enum_<DeliveryMode>(m, "DeliveryMode", "Message delivery modes")
        .value("Immediate", DeliveryMode::Immediate, "Deliver immediately (synchronous)")
        .value("Queued", DeliveryMode::Queued, "Queue for later delivery (asynchronous)")
        .value("Broadcast", DeliveryMode::Broadcast, "Broadcast to all subscribers")
        .value("Unicast", DeliveryMode::Unicast, "Send to specific recipient")
        .value("Multicast", DeliveryMode::Multicast, "Send to multiple recipients")
        .export_values();

    // === Basic Message Interface ===
    py::class_<IMessage, std::shared_ptr<IMessage>>(m, "IMessage", "Base message interface")
        .def("type", &IMessage::type, "Get message type identifier")
        .def("sender", &IMessage::sender, "Get message sender")
        .def("timestamp", &IMessage::timestamp, "Get message timestamp")
        .def("priority", &IMessage::priority, "Get message priority")
        .def("to_json", &IMessage::to_json, "Serialize message to JSON")
        .def("id", &IMessage::id, "Get message ID")
        .def("__repr__", [](const IMessage& msg) {
            return "<IMessage type='" + std::string(msg.type()) + "' sender='" +
                   std::string(msg.sender()) + "'>";
        });

    // Subscription information (corrected to match actual API)
    py::class_<Subscription>(m, "Subscription")
        .def_readwrite("subscriber_id", &Subscription::subscriber_id)
        .def_readwrite("message_type", &Subscription::message_type)
        .def_readwrite("handler", &Subscription::handler)
        .def_readwrite("filter", &Subscription::filter)
        .def_readwrite("is_active", &Subscription::is_active)
        .def_readwrite("created_at", &Subscription::created_at)
        .def_readwrite("message_count", &Subscription::message_count)
        .def("__repr__", [](const Subscription& sub) {
            return "<Subscription subscriber='" + sub.subscriber_id + "'>";
        });

    // Message bus interface (corrected to match actual API)
    py::class_<IMessageBus, std::shared_ptr<IMessageBus>>(m, "IMessageBus")
        .def("unsubscribe", py::overload_cast<std::string_view, std::optional<std::type_index>>(&IMessageBus::unsubscribe),
             "Unsubscribe from messages", py::arg("subscriber_id"), py::arg("message_type") = std::nullopt)
        .def("subscribers", &IMessageBus::subscribers, "Get list of subscribers for a message type",
             py::arg("message_type"))
        .def("subscriptions", &IMessageBus::subscriptions, "Get subscription information",
             py::arg("subscriber_id"))
        .def("has_subscriber", &IMessageBus::has_subscriber, "Check if subscriber exists",
             py::arg("subscriber_id"))
        .def("statistics", &IMessageBus::statistics, "Get message bus statistics")
        .def("clear", &IMessageBus::clear, "Clear all subscriptions")
        .def("set_logging_enabled", &IMessageBus::set_logging_enabled, "Enable or disable message logging",
             py::arg("enabled"))
        .def("is_logging_enabled", &IMessageBus::is_logging_enabled, "Check if message logging is enabled")
        .def("message_log", &IMessageBus::message_log, "Get message log",
             py::arg("limit") = 100);

    // === Message Bus Implementation ===
    py::class_<MessageBus, IMessageBus, std::shared_ptr<MessageBus>>(m, "MessageBus", "Message bus implementation")
        .def(py::init<QObject*>(), "Create message bus", py::arg("parent") = nullptr)
        .def("__repr__", [](const MessageBus& bus) { return "<MessageBus>"; });

    // Service capability enum
    py::enum_<contracts::ServiceCapability>(m, "ServiceCapability")
        .value("None", contracts::ServiceCapability::None)
        .value("Synchronous", contracts::ServiceCapability::Synchronous)
        .value("Asynchronous", contracts::ServiceCapability::Asynchronous)
        .value("Streaming", contracts::ServiceCapability::Streaming)
        .value("Transactional", contracts::ServiceCapability::Transactional)
        .value("Cacheable", contracts::ServiceCapability::Cacheable)
        .value("Idempotent", contracts::ServiceCapability::Idempotent)
        .value("ThreadSafe", contracts::ServiceCapability::ThreadSafe)
        .value("Stateful", contracts::ServiceCapability::Stateful)
        .value("Discoverable", contracts::ServiceCapability::Discoverable)
        .value("Versioned", contracts::ServiceCapability::Versioned)
        .value("Authenticated", contracts::ServiceCapability::Authenticated)
        .value("Encrypted", contracts::ServiceCapability::Encrypted)
        .export_values();

    // Service version structure
    py::class_<contracts::ServiceVersion>(m, "ServiceVersion")
        .def(py::init<>())
        .def(py::init<int, int, int>())
        .def_readwrite("major", &contracts::ServiceVersion::major)
        .def_readwrite("minor", &contracts::ServiceVersion::minor)
        .def_readwrite("patch", &contracts::ServiceVersion::patch)
        .def("to_string", &contracts::ServiceVersion::to_string)
        .def("is_compatible_with", &contracts::ServiceVersion::is_compatible_with)
        .def("__repr__", [](const contracts::ServiceVersion& version) {
            return "<ServiceVersion " + version.to_string() + ">";
        });

    // Service method descriptor
    py::class_<contracts::ServiceMethodDescriptor>(m, "ServiceMethodDescriptor")
        .def(py::init<>())
        .def_readwrite("method_name", &contracts::ServiceMethodDescriptor::method_name)
        .def_readwrite("description", &contracts::ServiceMethodDescriptor::description)
        .def_readwrite("input_schema", &contracts::ServiceMethodDescriptor::input_schema)
        .def_readwrite("output_schema", &contracts::ServiceMethodDescriptor::output_schema)
        .def_readwrite("capabilities", &contracts::ServiceMethodDescriptor::capabilities)
        .def_readwrite("timeout_ms", &contracts::ServiceMethodDescriptor::timeout_ms)
        .def("to_json", &contracts::ServiceMethodDescriptor::to_json)
        .def_static("from_json", &contracts::ServiceMethodDescriptor::from_json)
        .def("__repr__", [](const contracts::ServiceMethodDescriptor& method) {
            return "<ServiceMethodDescriptor " + method.method_name.toStdString() + ">";
        });

    // Service contract
    py::class_<contracts::ServiceContract>(m, "ServiceContract")
        .def(py::init<const QString&>())
        .def_readwrite("service_name", &contracts::ServiceContract::service_name)
        .def_readwrite("version", &contracts::ServiceContract::version)
        .def_readwrite("description", &contracts::ServiceContract::description)
        .def_readwrite("methods", &contracts::ServiceContract::methods)
        .def_readwrite("dependencies", &contracts::ServiceContract::dependencies)
        .def_readwrite("capabilities", &contracts::ServiceContract::capabilities)
        .def_readwrite("metadata", &contracts::ServiceContract::metadata)
        .def("add_method", &contracts::ServiceContract::add_method)
        .def("has_method", &contracts::ServiceContract::has_method)
        .def("get_method", &contracts::ServiceContract::get_method)
        .def("is_compatible_with", &contracts::ServiceContract::is_compatible_with)
        .def("to_json", &contracts::ServiceContract::to_json)
        .def_static("from_json", &contracts::ServiceContract::from_json)
        .def("__repr__", [](const contracts::ServiceContract& contract) {
            return "<ServiceContract " + contract.service_name.toStdString() + ">";
        });

    // === Utility Functions ===
    m.def("create_message_bus", []() -> std::shared_ptr<MessageBus> {
        return std::make_shared<MessageBus>();
    }, "Create a new MessageBus instance");

    m.def("test_communication", []() -> std::string {
        return "Communication module working!";
    }, "Test function for communication module");

    m.def("create_service_contract", [](const std::string& service_name) -> contracts::ServiceContract {
        return contracts::ServiceContract(QString::fromStdString(service_name));
    }, "Create a new service contract", py::arg("service_name"));

    m.def("get_available_features", []() -> py::list {
        py::list features;
        features.append("message_bus");
        features.append("service_contracts");
        features.append("message_types");
        return features;
    }, "Get list of available communication features");
}

}  // namespace qtforge_python
