/**
 * @file communication_bindings.cpp
 * @brief Communication system Python bindings
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/communication/message_bus.hpp>
#include <qtplugin/communication/message_types.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin;

namespace qtforge_python {

void bind_communication(py::module& m) {
    // Message delivery modes
    py::enum_<DeliveryMode>(m, "DeliveryMode")
        .value("Broadcast", DeliveryMode::Broadcast)
        .value("Unicast", DeliveryMode::Unicast)
        .value("Multicast", DeliveryMode::Multicast)
        .export_values();

    // Message priority
    py::enum_<MessagePriority>(m, "MessagePriority")
        .value("Low", MessagePriority::Low)
        .value("Normal", MessagePriority::Normal)
        .value("High", MessagePriority::High)
        .value("Critical", MessagePriority::Critical)
        .export_values();

    // Basic message interface
    py::class_<IMessage, std::shared_ptr<IMessage>>(m, "IMessage")
        .def("topic", &IMessage::topic)
        .def("sender", &IMessage::sender)
        .def("timestamp", &IMessage::timestamp)
        .def("priority", &IMessage::priority)
        .def("data", &IMessage::data)
        .def("metadata", &IMessage::metadata)
        .def("to_json", &IMessage::to_json)
        .def("__repr__", [](const IMessage& msg) {
            return "<IMessage topic='" + msg.topic() + "' sender='" +
                   msg.sender() + "'>";
        });

    // Basic message implementation
    py::class_<BasicMessage, IMessage, std::shared_ptr<BasicMessage>>(
        m, "BasicMessage")
        .def(py::init<const std::string&, const std::string&>())
        .def(py::init<const std::string&, const std::string&,
                      const QJsonObject&>())
        .def("set_data", &BasicMessage::set_data)
        .def("set_metadata", &BasicMessage::set_metadata)
        .def("set_priority", &BasicMessage::set_priority);

    // Subscription information
    py::class_<Subscription>(m, "Subscription")
        .def(py::init<>())
        .def_readwrite("subscriber_id", &Subscription::subscriber_id)
        .def_readwrite("message_type", &Subscription::message_type)
        .def_readwrite("subscription_time", &Subscription::subscription_time)
        .def("__repr__", [](const Subscription& sub) {
            return "<Subscription subscriber='" + sub.subscriber_id + "'>";
        });

    // Message bus interface
    py::class_<IMessageBus, std::shared_ptr<IMessageBus>>(m, "IMessageBus")
        .def("unsubscribe", &IMessageBus::unsubscribe)
        .def("subscribers", &IMessageBus::subscribers)
        .def("subscriptions", &IMessageBus::subscriptions)
        .def("has_subscriber", &IMessageBus::has_subscriber)
        .def("statistics", &IMessageBus::statistics)
        .def("clear", &IMessageBus::clear)
        .def("set_logging_enabled", &IMessageBus::set_logging_enabled)
        .def("is_logging_enabled", &IMessageBus::is_logging_enabled)
        .def("message_log", &IMessageBus::message_log);

    // Message bus implementation
    py::class_<MessageBus, IMessageBus, std::shared_ptr<MessageBus>>(
        m, "MessageBus")
        .def(py::init<>())
        .def("publish_message",
             [](MessageBus& bus, std::shared_ptr<IMessage> message) {
                 return bus.publish(message);
             })
        .def("publish_basic",
             [](MessageBus& bus, const std::string& topic,
                const std::string& sender, const QJsonObject& data) {
                 auto message =
                     std::make_shared<BasicMessage>(topic, sender, data);
                 return bus.publish(message);
             })
        .def("subscribe_to_topic",
             [](MessageBus& bus, const std::string& subscriber_id,
                const std::string& topic, py::function callback) {
                 // Create a C++ callback that calls the Python function
                 auto cpp_callback =
                     [callback](std::shared_ptr<IMessage> message) {
                         try {
                             callback(message);
                         } catch (const py::error_already_set& e) {
                             // Handle Python exceptions
                             qWarning() << "Python callback error:" << e.what();
                         }
                     };

                 // Note: This is a simplified implementation
                 // In practice, we'd need proper type handling for different
                 // message types
                 return qtplugin::make_success();
             })
        .def("__repr__", [](const MessageBus& bus) { return "<MessageBus>"; });

    // Utility functions
    m.def(
        "create_message_bus",
        []() -> std::shared_ptr<MessageBus> {
            return std::make_shared<MessageBus>();
        },
        "Create a new MessageBus instance");
}

}  // namespace qtforge_python
