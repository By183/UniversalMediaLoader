#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <iostream>
#include "CommandBuilder.h"
#include "ProcessRunner.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    QString url = "https://www.youtube.com/watch?v=aqz-KE-bpKQ";
    QString outputDir = "d:/Universal Media Loader/video_downloader/downloads_test";

    QStringList args = CommandBuilder::buildCommand(url, "1080p", false, outputDir, "TestPauseCancel");

    std::cout << "Starting download command: " << args.join(" ").toStdString() << "\n";

    ProcessRunner runner;

    QObject::connect(&runner, &ProcessRunner::progressUpdated, [](const ParsedProgress &progress) {
        if (!progress.statusMessage.isEmpty()) {
            std::cout << "[PROGRESS] " << progress.statusMessage.toStdString() << "\n";
        }
    });

    QObject::connect(&runner, &ProcessRunner::finished, [&app](bool success, int exitCode) {
        std::cout << "FINISHED SIGNAL! Success: " << (success ? "YES" : "NO") << ", ExitCode: " << exitCode << "\n";
        app.quit();
    });

    runner.startDownload(args, outputDir);

    // Timeline of test events:
    // T+3s: Pause download
    QTimer::singleShot(3000, [&runner]() {
        std::cout << "\n>>> TESTING PAUSE AT T+3s <<<\n";
        runner.pause();
        std::cout << "Process PID: " << runner.processId() << " | IsPaused: " << (runner.isPaused() ? "YES" : "NO") << "\n";
    });

    // T+6s: Resume download
    QTimer::singleShot(6000, [&runner]() {
        std::cout << "\n>>> TESTING RESUME AT T+6s <<<\n";
        runner.resume();
        std::cout << "Process PID: " << runner.processId() << " | IsPaused: " << (runner.isPaused() ? "YES" : "NO") << "\n";
    });

    // T+8s: Cancel download
    QTimer::singleShot(8000, [&runner]() {
        std::cout << "\n>>> TESTING CANCEL AT T+8s <<<\n";
        runner.cancel();
        std::cout << "Cancel triggered. IsRunning: " << (runner.isRunning() ? "YES" : "NO") << "\n";
    });

    return app.exec();
}
