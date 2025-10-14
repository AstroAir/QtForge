#include <QCoreApplication>
#include <QDebug>
#include <qtplugin/security/components/security_validator.hpp>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);

    qtplugin::security::components::SecurityValidator validator;
    validator.setStrictnessLevel(6);

    if (!validator.initialize()) {
        qWarning() << "Security validator failed to initialize";
    }

    // Demonstrate metadata validation on a non-existent file (stubbed API)
    auto res =
        validator.validate_metadata(QString::fromUtf8("./nonexistent.json"));
    qInfo() << "Validation result (enum):" << static_cast<int>(res);
    qInfo() << "Report:" << validator.getValidationReport();

    return 0;
}
