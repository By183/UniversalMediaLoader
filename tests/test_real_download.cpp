#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include "CommandBuilder.h"
#include "ProcessRunner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QString url = "https://www.youtube.com/watch?v=aqz-KE-bpKQ";
    QString outputDir = "d:/Universal Media Loader/video_downloader/downloads_test";

    QStringList args = CommandBuilder::buildCommand(url, "360p", false, outputDir, "TestVideo");

    std::cout << "Starting download command: " << args.join(" ").toStdString() << "\n";

    ProcessRunner runner;

    QObject::connect(&runner, &ProcessRunner::progressUpdated, [](const ParsedProgress &progress) {
        if (!progress.statusMessage.isEmpty()) {
            std::cout << "[STATUS] " << progress.statusMessage.toStdString();
            if (progress.hasPercentage) {
                std::cout << " (" << progress.percentage << "%)";
            }
            std::cout << "\n";
        }
    });

    QObject::connect(&runner, &ProcessRunner::logOutputEmitted, [](const QString &line) {
        std::cout << "[RAW] " << line.toStdString() << "\n";
    });

    QObject::connect(&runner, &ProcessRunner::finished, [&app](bool success, int exitCode) {
        std::cout << "\nFINISHED! Success: " << (success ? "YES" : "NO") << ", ExitCode: " << exitCode << "\n";
        app.quit();
    });

    runner.startDownload(args);

    return app.exec();
}
