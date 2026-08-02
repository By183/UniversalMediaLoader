#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <iostream>
#include "CommandBuilder.h"
#include "ProcessRunner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    bool withPause = false;
    if (argc > 1 && QString(argv[1]) == "--with-pause") {
        withPause = true;
    }

    std::cout << "=== RUNNING DIAGNOSTIC TEST (WithPause: " << (withPause ? "YES" : "NO") << ") ===" << std::endl;

    QString url = "https://www.twitch.tv/videos/2834308857";
    QString outputDir = "d:/Universal Media Loader/video_downloader/downloads_test";
    QString customName = withPause ? "TwitchTest_WithPause" : "TwitchTest_NoPause";
    QString timeStart = "50";
    QString timeEnd = "";

    QStringList args = CommandBuilder::buildCommand(url, "360p", false, outputDir, customName, timeStart, timeEnd);

    std::cout << "COMMAND EXECUTED:" << std::endl;
    for (const QString &a : args) {
        std::cout << a.toStdString() << " ";
    }
    std::cout << "\n\n";

    QString logPath = QString("%1/diag_%2.log").arg(outputDir, customName);
    QFile logFile(logPath);
    if (!logFile.open(QFile::WriteOnly | QFile::Text)) {
        std::cerr << "Failed to open log file for writing: " << logPath.toStdString() << std::endl;
        return 1;
    }
    QTextStream logStream(&logFile);

    ProcessRunner runner;

    QObject::connect(&runner, &ProcessRunner::logOutputEmitted, [&logStream](const QString &line) {
        std::cout << "[RAW] " << line.toStdString() << "\n";
        logStream << line << "\n";
        logStream.flush();
    });

    if (withPause) {
        QTimer::singleShot(10000, [&runner]() {
            std::cout << "\n>>> PAUSING PROCESS AT T+10s <<<\n";
            runner.pause();
            QTimer::singleShot(5000, [&runner]() {
                std::cout << "\n>>> RESUMING PROCESS AT T+15s <<<\n";
                runner.resume();
            });
        });
    }

    QObject::connect(&runner, &ProcessRunner::finished, [&app, &logFile, &logStream](bool success, int exitCode) {
        std::cout << "\nPROCESS FINISHED! Success: " << (success ? "YES" : "NO") << " | ExitCode: " << exitCode << std::endl;
        logStream << "PROCESS FINISHED! Success: " << (success ? "YES" : "NO") << " | ExitCode: " << exitCode << "\n";
        logFile.close();
        app.quit();
    });

    runner.startDownload(args, outputDir);

    return app.exec();
}
