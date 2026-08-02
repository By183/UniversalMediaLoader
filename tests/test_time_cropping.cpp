#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include "CommandBuilder.h"
#include "ProcessRunner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    std::cout << "=== TESTING TIME CROPPING FEATURE ===" << std::endl;

    QString url = "https://www.youtube.com/watch?v=aqz-KE-bpKQ";
    QString outputDir = "d:/Universal Media Loader/video_downloader/downloads_test";
    QString customName = "TestCrop_1m_2m";
    QString timeStart = "00:01:00";
    QString timeEnd = "00:02:00";

    QStringList args = CommandBuilder::buildCommand(url, "360p", false, outputDir, customName, timeStart, timeEnd);

    std::cout << "Generated Command:" << std::endl;
    for (const QString &arg : args) {
        std::cout << arg.toStdString() << " ";
    }
    std::cout << "\n\nLaunching download..." << std::endl;

    ProcessRunner runner;

    QObject::connect(&runner, &ProcessRunner::logOutputEmitted, [](const QString &line) {
        std::cout << "[LOG] " << line.toStdString() << "\n";
    });

    QObject::connect(&runner, &ProcessRunner::finished, [&app](bool success, int exitCode) {
        std::cout << "\nFINISHED! Success: " << (success ? "YES" : "NO") << " | ExitCode: " << exitCode << "\n";
        app.quit();
    });

    runner.startDownload(args, outputDir);

    return app.exec();
}
