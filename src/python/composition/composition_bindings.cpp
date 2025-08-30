/**
 * @file composition_bindings.cpp
 * @brief Composition system Python bindings
 * @version 3.0.0
 * @author QtForge Development Team
 */

#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <qtplugin/composition/plugin_composition.hpp>

#include "../qt_conversions.hpp"

namespace py = pybind11;
using namespace qtplugin::composition;

namespace qtforge_python {

void bind_composition(py::module& m) {
    // Composition strategy enum
    py::enum_<CompositionStrategy>(m, "CompositionStrategy")
        .value("Aggregation", CompositionStrategy::Aggregation)
        .value("Pipeline", CompositionStrategy::Pipeline)
        .value("Facade", CompositionStrategy::Facade)
        .value("Decorator", CompositionStrategy::Decorator)
        .value("Proxy", CompositionStrategy::Proxy)
        .value("Adapter", CompositionStrategy::Adapter)
        .value("Bridge", CompositionStrategy::Bridge)
        .export_values();

    // Plugin role enum
    py::enum_<PluginRole>(m, "PluginRole")
        .value("Primary", PluginRole::Primary)
        .value("Secondary", PluginRole::Secondary)
        .value("Auxiliary", PluginRole::Auxiliary)
        .value("Decorator", PluginRole::Decorator)
        .value("Adapter", PluginRole::Adapter)
        .value("Bridge", PluginRole::Bridge)
        .export_values();

    // Composition binding
    py::class_<CompositionBinding>(m, "CompositionBinding")
        .def(py::init<>())
        .def(py::init<const QString&, const QString&, const QString&,
                      const QString&>())
        .def_readwrite("source_plugin_id",
                       &CompositionBinding::source_plugin_id)
        .def_readwrite("source_method", &CompositionBinding::source_method)
        .def_readwrite("target_plugin_id",
                       &CompositionBinding::target_plugin_id)
        .def_readwrite("target_method", &CompositionBinding::target_method)
        .def_readwrite("parameter_mapping",
                       &CompositionBinding::parameter_mapping)
        .def_readwrite("bidirectional", &CompositionBinding::bidirectional)
        .def_readwrite("priority", &CompositionBinding::priority)
        .def("__repr__", [](const CompositionBinding& binding) {
            return "<CompositionBinding " +
                   binding.source_plugin_id.toStdString() + "." +
                   binding.source_method.toStdString() + " -> " +
                   binding.target_plugin_id.toStdString() + "." +
                   binding.target_method.toStdString() + ">";
        });

    // Plugin composition
    py::class_<PluginComposition>(m, "PluginComposition")
        .def(py::init<const QString&, const QString&>(),
             py::arg("composition_id"), py::arg("name") = "")
        .def("set_description", &PluginComposition::set_description,
             py::return_value_policy::reference)
        .def("set_strategy", &PluginComposition::set_strategy,
             py::return_value_policy::reference)
        .def("add_plugin", &PluginComposition::add_plugin, py::arg("plugin_id"),
             py::arg("role") = PluginRole::Secondary,
             py::return_value_policy::reference)
        .def("set_primary_plugin", &PluginComposition::set_primary_plugin,
             py::return_value_policy::reference)
        .def("add_binding", &PluginComposition::add_binding,
             py::return_value_policy::reference)
        .def("set_configuration", &PluginComposition::set_configuration,
             py::return_value_policy::reference)
        .def("id", &PluginComposition::id)
        .def("name", &PluginComposition::name)
        .def("description", &PluginComposition::description)
        .def("strategy", &PluginComposition::strategy)
        .def("primary_plugin_id", &PluginComposition::primary_plugin_id)
        .def("plugins", &PluginComposition::plugins)
        .def("bindings", &PluginComposition::bindings)
        .def("configuration", &PluginComposition::configuration)
        .def("get_plugins_by_role", &PluginComposition::get_plugins_by_role)
        .def("validate", &PluginComposition::validate)
        .def("to_json", &PluginComposition::to_json)
        .def_static("from_json", &PluginComposition::from_json)
        .def("__repr__", [](const PluginComposition& comp) {
            return "<PluginComposition id='" + comp.id().toStdString() +
                   "' plugins=" + std::to_string(comp.plugins().size()) + ">";
        });

    // Plugin composition manager
    py::class_<PluginCompositionManager,
               std::shared_ptr<PluginCompositionManager>>(
        m, "PluginCompositionManager")
        .def(py::init<>())
        .def_static("create", &PluginCompositionManager::create)
        .def("register_composition",
             &PluginCompositionManager::register_composition)
        .def("unregister_composition",
             &PluginCompositionManager::unregister_composition)
        .def("get_composition", &PluginCompositionManager::get_composition)
        .def("has_composition", &PluginCompositionManager::has_composition)
        .def("list_compositions", &PluginCompositionManager::list_compositions)
        .def("activate_composition",
             &PluginCompositionManager::activate_composition)
        .def("deactivate_composition",
             &PluginCompositionManager::deactivate_composition)
        .def("is_composition_active",
             &PluginCompositionManager::is_composition_active)
        .def("get_active_compositions",
             &PluginCompositionManager::get_active_compositions)
        .def("execute_composition",
             &PluginCompositionManager::execute_composition)
        .def("clear_compositions",
             &PluginCompositionManager::clear_compositions)
        .def("__repr__", [](const PluginCompositionManager& manager) {
            auto compositions = manager.list_compositions();
            return "<PluginCompositionManager compositions=" +
                   std::to_string(compositions.size()) + ">";
        });

    // Utility functions
    m.def(
        "create_composition_manager",
        []() -> std::shared_ptr<PluginCompositionManager> {
            return PluginCompositionManager::create();
        },
        "Create a new PluginCompositionManager instance");

    m.def(
        "create_composition",
        [](const std::string& id,
           const std::string& name) -> PluginComposition {
            return PluginComposition(QString::fromStdString(id),
                                     QString::fromStdString(name));
        },
        py::arg("composition_id"), py::arg("name") = "",
        "Create a new PluginComposition instance");

    m.def(
        "create_composition_binding",
        [](const std::string& src_plugin, const std::string& src_method,
           const std::string& tgt_plugin,
           const std::string& tgt_method) -> CompositionBinding {
            return CompositionBinding(QString::fromStdString(src_plugin),
                                      QString::fromStdString(src_method),
                                      QString::fromStdString(tgt_plugin),
                                      QString::fromStdString(tgt_method));
        },
        py::arg("source_plugin"), py::arg("source_method"),
        py::arg("target_plugin"), py::arg("target_method"),
        "Create a new CompositionBinding instance");

    // Helper functions for common composition patterns
    m.def(
        "create_pipeline_composition",
        [](const std::string& id,
           const std::vector<std::string>& plugin_ids) -> PluginComposition {
            PluginComposition comp(QString::fromStdString(id));
            comp.set_strategy(CompositionStrategy::Pipeline);

            for (size_t i = 0; i < plugin_ids.size(); ++i) {
                QString plugin_id = QString::fromStdString(plugin_ids[i]);
                PluginRole role =
                    (i == 0) ? PluginRole::Primary : PluginRole::Secondary;
                comp.add_plugin(plugin_id, role);

                // Create pipeline bindings
                if (i > 0) {
                    QString prev_plugin =
                        QString::fromStdString(plugin_ids[i - 1]);
                    CompositionBinding binding(prev_plugin, "output", plugin_id,
                                               "input");
                    comp.add_binding(binding);
                }
            }

            return comp;
        },
        py::arg("composition_id"), py::arg("plugin_ids"),
        "Create a pipeline composition with automatic binding setup");

    m.def(
        "create_facade_composition",
        [](const std::string& id, const std::string& facade_plugin,
           const std::vector<std::string>& backend_plugins)
            -> PluginComposition {
            PluginComposition comp(QString::fromStdString(id));
            comp.set_strategy(CompositionStrategy::Facade);
            comp.set_primary_plugin(QString::fromStdString(facade_plugin));

            for (const auto& backend : backend_plugins) {
                comp.add_plugin(QString::fromStdString(backend),
                                PluginRole::Secondary);
            }

            return comp;
        },
        py::arg("composition_id"), py::arg("facade_plugin"),
        py::arg("backend_plugins"),
        "Create a facade composition with a primary facade plugin and backend "
        "plugins");
}

}  // namespace qtforge_python
