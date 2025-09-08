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
using namespace qtplugin;
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

    // Plugin composition manager (singleton) - DISABLED: Not implemented
    // py::class_<CompositionManager>(m, "CompositionManager")
    //     .def_static("instance", &CompositionManager::instance,
    //                 py::return_value_policy::reference)
    //     .def("register_composition",
    //          &CompositionManager::register_composition)
    //     .def("unregister_composition",
    //          &CompositionManager::unregister_composition)
    //     .def("get_composition", &CompositionManager::get_composition)
    //     .def("list_compositions", &CompositionManager::list_compositions)
    //     .def("create_composite_plugin",
    //          &CompositionManager::create_composite_plugin)
    //     .def("destroy_composite_plugin",
    //          &CompositionManager::destroy_composite_plugin)
    //     .def("list_composite_plugins",
    //          &CompositionManager::list_composite_plugins)
    //     .def("get_composite_plugin",
    //          &CompositionManager::get_composite_plugin)
    //     .def("__repr__", [](const CompositionManager& manager) {
    //         auto compositions = manager.list_compositions();
    //         return "<CompositionManager compositions=" +
    //                std::to_string(compositions.size()) + ">";
    //     });

    // Utility functions
    // m.def(
    //     "get_composition_manager",
    //     []() -> CompositionManager& {
    //         return CompositionManager::instance();
    //     },
    //     py::return_value_policy::reference,
    //     "Get the CompositionManager singleton instance");

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

    // === Additional Utility Functions ===
    m.def("test_composition", []() -> std::string {
        return "Composition module working!";
    }, "Test function for composition module");

    m.def("get_available_composition_features", []() -> py::list {
        py::list features;
        features.append("plugin_composition");
        features.append("composition_strategies");
        features.append("plugin_roles");
        features.append("composition_bindings");
        return features;
    }, "Get list of available composition features");

    m.def("validate_composition_strategy", [](int strategy) -> bool {
        return strategy >= static_cast<int>(CompositionStrategy::Aggregation) &&
               strategy <= static_cast<int>(CompositionStrategy::Bridge);
    }, "Validate composition strategy value", py::arg("strategy"));

    m.def("validate_plugin_role", [](int role) -> bool {
        return role >= static_cast<int>(PluginRole::Primary) &&
               role <= static_cast<int>(PluginRole::Bridge);
    }, "Validate plugin role value", py::arg("role"));
}

}  // namespace qtforge_python
