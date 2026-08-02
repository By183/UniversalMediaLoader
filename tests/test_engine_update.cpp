#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <iostream>
#include "EngineUpdater.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== TESTING ENGINE UPDATER ===" << std::endl;

    EngineUpdater updater;

    QObject::connect(&updater, &EngineUpdater::statusEmitted, [](const QString &status) {
        std::cout << "[STATUS] " << status.toStdString() << "\n";
    });

    QObject::connect(&updater, &EngineUpdater::logEmitted, [](const QString &msg) {
        std::cout << "[LOG] " << msg.toStdString() << "\n";
    });

    QObject::connect(&updater, &EngineUpdater::finished, [&app](bool success, const QString &msg) {
        std::cout << "\nFINISHED! Success: " << (success ? "YES" : "NO") << " | Message: " << msg.toStdString() << "\n";
        app.quit();
    });

    updater.startUpdate();

    return app.exec();
}
