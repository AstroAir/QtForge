#include <QCoreApplication>
#include <QDebug>
#include <QJsonDocument>
#include <qtplugin/workflow/composition.hpp>

using namespace qtplugin::workflow::composition;

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    // Define a simple composition config (no actual plugin loading)
    PluginComposition comp{"comp.examples.basic", "Composition Basics"};
    comp.set_description("Demonstrates composition data model and validation")
        .set_strategy(CompositionStrategy::Aggregation)
        .add_plugin("com.examples.primary", PluginRole::Primary)
        .add_plugin("com.examples.secondary", PluginRole::Secondary)
        .add_binding(CompositionBinding{"com.examples.primary", "produce",
                                        "com.examples.secondary", "consume"});

    auto valid = comp.validate();
    if (!valid) {
        qWarning() << "Composition validation failed:"
                   << static_cast<int>(valid.error().code);
    }

    auto json = comp.to_json();
    qInfo() << "Composition JSON:"
            << QJsonDocument(json).toJson(QJsonDocument::Compact);

    return 0;
}
